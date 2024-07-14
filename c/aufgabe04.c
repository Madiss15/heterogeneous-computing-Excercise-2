#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <sys/time.h>
#define CL_TARGET_OPENCL_VERSION 300
#include "CL/cl.h"

#define MAX(a,b) ((a) > (b) ? (a) : (b))
#define MIN(a,b) ((a) < (b) ? (a) : (b))

#define CHECK_CL_ERROR(err, msg) if (err != CL_SUCCESS) { fprintf(stderr, "%s failed: %d\n", msg, err); exit(EXIT_FAILURE); }
#define PI 3.14159265358979323846

typedef struct {
  char* filename;
  int blocksize;
  int shift;
  int threshold;
} FFT_Analyzer;

FFT_Analyzer* create_fft_analyzer(const char* filename, int blocksize, int shift, int threshold) {
  FFT_Analyzer* analyzer = (FFT_Analyzer*)malloc(sizeof(FFT_Analyzer));
  analyzer->filename = strdup(filename);
  analyzer->blocksize = MAX(MIN(512, blocksize), 64);
  analyzer->shift = MAX(MIN(analyzer->blocksize, shift), 1);
  analyzer->threshold = threshold;
  return analyzer;
}

void destroy_fft_analyzer(FFT_Analyzer* analyzer) {
  free(analyzer->filename);
  free(analyzer);
}

char* read_kernel_source(const char* filename) {
  FILE* file = fopen(filename, "r");
  if (!file) {
    perror("Error opening kernel file");
    return NULL;
  }

  fseek(file, 0, SEEK_END);
  long size = ftell(file);
  fseek(file, 0, SEEK_SET);

  char* source = (char*)malloc(size + 1);
  fread(source, 1, size, file);
  source[size] = '\0';

  fclose(file);
  return source;
}

void apply_hann_window(double* data, int size) {
  for (int i = 0; i < size; i++) {
    data[i] *= 0.5 * (1 - cos(2 * PI * i / (size - 1)));
  }
}

double* get_amplitude_mean(FFT_Analyzer* analyzer) {
  FILE* file = fopen(analyzer->filename, "rb");
  if (!file) {
    perror("Error opening file");
    return NULL;
  }

  fseek(file, 0, SEEK_END);
  long file_size = ftell(file);
  fseek(file, 0, SEEK_SET);
  int samples = file_size / 4;
  short* data = (short*)malloc(file_size);
  fread(data, 2, samples * 2, file);
  fclose(file);

  double* normalized_data_left = (double*)malloc(samples * sizeof(double));
  for (int i = 0; i < samples; i++) {
    normalized_data_left[i] = data[i*2] / 32768.0;
  }
  free(data);

  int bins_size = analyzer->blocksize / 2 + 1;
  double* bins = (double*)calloc(bins_size, sizeof(double));

  // OpenCL setup
  cl_platform_id platform;
  cl_device_id device;
  cl_context context;
  cl_command_queue queue;
  cl_program program;
  cl_kernel fft_kernel, magnitude_kernel, db_kernel;
  cl_int err;

  // Initialize OpenCL
  err = clGetPlatformIDs(1, &platform, NULL);
  CHECK_CL_ERROR(err, "clGetPlatformIDs");

  err = clGetDeviceIDs(platform, CL_DEVICE_TYPE_GPU, 1, &device, NULL);
  CHECK_CL_ERROR(err, "clGetDeviceIDs");

  context = clCreateContext(NULL, 1, &device, NULL, NULL, &err);
  CHECK_CL_ERROR(err, "clCreateContext");

  cl_queue_properties properties[] = {CL_QUEUE_PROPERTIES, 0, 0};
  queue = clCreateCommandQueueWithProperties(context, device, properties, &err);
  CHECK_CL_ERROR(err, "clCreateCommandQueueWithProperties");

  // Load and compile the kernel
  const char* source = read_kernel_source("../fft_kernel.cl");
  if (!source) {
    fprintf(stderr, "Error reading kernel source\n");
    exit(EXIT_FAILURE);
  }

  program = clCreateProgramWithSource(context, 1, &source, NULL, &err);
  CHECK_CL_ERROR(err, "clCreateProgramWithSource");
  free((void*)source);

  err = clBuildProgram(program, 1, &device, NULL, NULL, NULL);
  if (err != CL_SUCCESS) {
    size_t log_size;
    clGetProgramBuildInfo(program, device, CL_PROGRAM_BUILD_LOG, 0, NULL, &log_size);
    char* log = (char*)malloc(log_size + 1);
    clGetProgramBuildInfo(program, device, CL_PROGRAM_BUILD_LOG, log_size, log, NULL);
    log[log_size] = '\0';
    fprintf(stderr, "clBuildProgram failed:\n%s\n", log);
    free(log);
    exit(EXIT_FAILURE);
  }

  // Create kernels
  fft_kernel = clCreateKernel(program, "fft", &err);
  CHECK_CL_ERROR(err, "clCreateKernel fft");

  magnitude_kernel = clCreateKernel(program, "compute_magnitude_squared", &err);
  CHECK_CL_ERROR(err, "clCreateKernel compute_magnitude_squared");

  db_kernel = clCreateKernel(program, "compute_db", &err);
  CHECK_CL_ERROR(err, "clCreateKernel compute_db");

  // Create buffers
  cl_mem d_input = clCreateBuffer(context, CL_MEM_READ_WRITE, analyzer->blocksize * sizeof(cl_double2), NULL, &err);
  CHECK_CL_ERROR(err, "clCreateBuffer d_input");

  cl_mem d_magnitude = clCreateBuffer(context, CL_MEM_WRITE_ONLY, bins_size * sizeof(cl_double), NULL, &err);
  CHECK_CL_ERROR(err, "clCreateBuffer d_magnitude");

  cl_mem d_output = clCreateBuffer(context, CL_MEM_WRITE_ONLY, bins_size * sizeof(cl_double), NULL, &err);
  CHECK_CL_ERROR(err, "clCreateBuffer d_output");

  int offset = 0;
  int count = 0;
  size_t global_size;

  while (offset + analyzer->blocksize <= samples) {
    // Apply Hann window
    double* windowed_data = (double*)malloc(analyzer->blocksize * sizeof(double));
    for (int i = 0; i < analyzer->blocksize; i++) {
      windowed_data[i] = normalized_data_left[offset + i];
    }
    apply_hann_window(windowed_data, analyzer->blocksize);

    // Copy input data to device
    cl_double2* input_data = (cl_double2*)malloc(analyzer->blocksize * sizeof(cl_double2));
    for (int i = 0; i < analyzer->blocksize; i++) {
      input_data[i].x = windowed_data[i];
      input_data[i].y = 0.0;
    }
    free(windowed_data);

    err = clEnqueueWriteBuffer(queue, d_input, CL_TRUE, 0, analyzer->blocksize * sizeof(cl_double2), input_data, 0, NULL, NULL);
    CHECK_CL_ERROR(err, "clEnqueueWriteBuffer d_input");
    free(input_data);

    // Perform FFT
    for (int step = 1; step < analyzer->blocksize; step *= 2) {
      err = clSetKernelArg(fft_kernel, 0, sizeof(cl_mem), &d_input);
      CHECK_CL_ERROR(err, "clSetKernelArg fft_kernel 0");
      err = clSetKernelArg(fft_kernel, 1, sizeof(int), &analyzer->blocksize);
      CHECK_CL_ERROR(err, "clSetKernelArg fft_kernel 1");
      err = clSetKernelArg(fft_kernel, 2, sizeof(int), &step);
      CHECK_CL_ERROR(err, "clSetKernelArg fft_kernel 2");

      global_size = analyzer->blocksize / 2;
      err = clEnqueueNDRangeKernel(queue, fft_kernel, 1, NULL, &global_size, NULL, 0, NULL, NULL);
      CHECK_CL_ERROR(err, "clEnqueueNDRangeKernel fft_kernel");
    }

    // Compute magnitude squared
    err = clSetKernelArg(magnitude_kernel, 0, sizeof(cl_mem), &d_input);
    CHECK_CL_ERROR(err, "clSetKernelArg magnitude_kernel 0");
    err = clSetKernelArg(magnitude_kernel, 1, sizeof(cl_mem), &d_magnitude);
    CHECK_CL_ERROR(err, "clSetKernelArg magnitude_kernel 1");
    err = clSetKernelArg(magnitude_kernel, 2, sizeof(int), &bins_size);
    CHECK_CL_ERROR(err, "clSetKernelArg magnitude_kernel 2");

    global_size = bins_size;
    err = clEnqueueNDRangeKernel(queue, magnitude_kernel, 1, NULL, &global_size, NULL, 0, NULL, NULL);
    CHECK_CL_ERROR(err, "clEnqueueNDRangeKernel magnitude_kernel");

    // Accumulate results
    double* magnitude = (double*)malloc(bins_size * sizeof(double));
    err = clEnqueueReadBuffer(queue, d_magnitude, CL_TRUE, 0, bins_size * sizeof(double), magnitude, 0, NULL, NULL);
    CHECK_CL_ERROR(err, "clEnqueueReadBuffer d_magnitude");

    for (int i = 0; i < bins_size; i++) {
      bins[i] += magnitude[i];
    }
    free(magnitude);

    offset += analyzer->shift;
    count++;
  }

  // Compute average and convert to dB
  for (int i = 0; i < bins_size; i++) {
    bins[i] /= count;
  }

  err = clEnqueueWriteBuffer(queue, d_output, CL_TRUE, 0, bins_size * sizeof(double), bins, 0, NULL, NULL);
  CHECK_CL_ERROR(err, "clEnqueueWriteBuffer d_output");

  err = clSetKernelArg(db_kernel, 0, sizeof(cl_mem), &d_output);
  CHECK_CL_ERROR(err, "clSetKernelArg db_kernel 0");
  err = clSetKernelArg(db_kernel, 1, sizeof(cl_mem), &d_output);
  CHECK_CL_ERROR(err, "clSetKernelArg db_kernel 1");
  err = clSetKernelArg(db_kernel, 2, sizeof(int), &bins_size);
  CHECK_CL_ERROR(err, "clSetKernelArg db_kernel 2");

  global_size = bins_size;
  err = clEnqueueNDRangeKernel(queue, db_kernel, 1, NULL, &global_size, NULL, 0, NULL, NULL);
  CHECK_CL_ERROR(err, "clEnqueueNDRangeKernel db_kernel");

  err = clEnqueueReadBuffer(queue, d_output, CL_TRUE, 0, bins_size * sizeof(double), bins, 0, NULL, NULL);
  CHECK_CL_ERROR(err, "clEnqueueReadBuffer d_output");

  // Clean up
  clReleaseMemObject(d_input);
  clReleaseMemObject(d_output);
  clReleaseMemObject(d_magnitude);
  clReleaseKernel(fft_kernel);
  clReleaseKernel(magnitude_kernel);
  clReleaseKernel(db_kernel);
  clReleaseProgram(program);
  clReleaseCommandQueue(queue);
  clReleaseContext(context);

  free(normalized_data_left);

  return bins;
}

int main(int argc, char* argv[]) {
  if (argc != 5) {
    fprintf(stderr, "Usage: %s <filename> <blocksize> <shift> <threshold>\n", argv[0]);
    return 1;
  }

  FFT_Analyzer* analyzer = create_fft_analyzer(argv[1], atoi(argv[2]), atoi(argv[3]), atoi(argv[4]));

  struct timeval start, end;
  gettimeofday(&start, NULL);

  double* result = get_amplitude_mean(analyzer);

  gettimeofday(&end, NULL);

  if (result) {
    for (int i = 0; i < analyzer->blocksize/2; i++) {
      if(result[i] > analyzer->threshold) {
	printf("%dHz %f\n", i*(44100/(analyzer->blocksize/2)), result[i]);
      }
    }
    printf("\n");
    free(result);
  }


  long seconds = end.tv_sec - start.tv_sec;
  long microseconds = end.tv_usec - start.tv_usec;
  double elapsed_time = seconds + microseconds / 1e6;
  printf("Execution time: %f seconds\n", elapsed_time);

  destroy_fft_analyzer(analyzer);
  return 0;
}
