#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <sys/time.h>
#include <pthread.h>
#include "fftw3.h"
#include <unistd.h>


// https://www.fftw.org/fftw3_doc/How-Many-Threads-to-Use_003f.html
#define MAX(a,b) ((a) > (b) ? (a) : (b))
#define MIN(a,b) ((a) < (b) ? (a) : (b))

int get_num_cores() {
  return sysconf(_SC_NPROCESSORS_ONLN);
}

typedef struct {
  char* filename;
  int blocksize;
  int shift;
  int threshold;
  int start_index;
  int end_index;
  double* bins;
  int num_bins;
} FFT_Analyzer;

FFT_Analyzer* create_fft_analyzer(const char* filename, int blocksize, int shift, int threshold) {
  FFT_Analyzer* analyzer = malloc(sizeof(FFT_Analyzer));
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

void* process_chunk(void* arg) {
  FFT_Analyzer* analyzer = (FFT_Analyzer*) arg;
  FILE* file = fopen(analyzer->filename, "rb");
  if (!file) {
    perror("Error opening file");
    pthread_exit(NULL);
  }

  fseek(file, 0, SEEK_END);
  long file_size = ftell(file);
  fseek(file, 0, SEEK_SET);

  int samples = file_size / 4;
  short* data = malloc(file_size);
  fread(data, 2, samples * 2, file);
  fclose(file);

  double* normalized_data_left = malloc(samples * sizeof(double));
  for (int i = 0; i < samples; i++) {
    normalized_data_left[i] = data[i * 2] / 32768.0;
  }
  free(data);

  int bins_size = analyzer->blocksize / 2;
  double* bins = calloc(bins_size, sizeof(double));

  fftw_complex* fft_out = fftw_malloc(sizeof(fftw_complex) * (analyzer->blocksize / 2 + 1));
  double* fft_in = fftw_malloc(sizeof(double) * analyzer->blocksize);
  fftw_plan plan = fftw_plan_dft_r2c_1d(analyzer->blocksize, fft_in, fft_out, FFTW_ESTIMATE);

  for (int offset = analyzer->start_index; offset < analyzer->end_index - analyzer->blocksize + 1; offset += analyzer->shift) {
    memcpy(fft_in, normalized_data_left + offset, analyzer->blocksize * sizeof(double));
    fftw_execute(plan);

    for (int i = 0; i < bins_size; i++) {
      double real = fft_out[i][0];
      double imag = fft_out[i][1];
      bins[i] += sqrt(real * real + imag * imag);
    }
  }

  fftw_destroy_plan(plan);
  fftw_free(fft_in);
  fftw_free(fft_out);
  free(normalized_data_left);

  for (int i = 0; i < bins_size; i++) {
    analyzer->bins[i] += bins[i];
  }
  free(bins);

  pthread_exit(NULL);
}

double* get_amplitude_mean(FFT_Analyzer* analyzer) {
  int num_cores = get_num_cores();
  fftw_init_threads();
  fftw_plan_with_nthreads(num_cores); // Set the number of threads you want to use
  printf("Using %d cores\n", num_cores);
  int samples;
  FILE* file = fopen(analyzer->filename, "rb");
  if (!file) {
    perror("Error opening file");
    return NULL;
  }

  fseek(file, 0, SEEK_END);
  long file_size = ftell(file);
  fseek(file, 0, SEEK_SET);

  samples = file_size / 4;
  fclose(file);

  double* bins = calloc(analyzer->blocksize / 2, sizeof(double));

  pthread_t threads[num_cores];
  FFT_Analyzer thread_analyzers[num_cores];
  int chunk_size = samples / num_cores;

  for (int i = 0; i < num_cores; i++) {
    thread_analyzers[i] = *analyzer;
    thread_analyzers[i].start_index = i * chunk_size;
    thread_analyzers[i].end_index = (i == num_cores - 1) ? samples : (i + 1) * chunk_size;
    thread_analyzers[i].bins = calloc(analyzer->blocksize / 2, sizeof(double));
    
    pthread_create(&threads[i], NULL, process_chunk, (void*)&thread_analyzers[i]);
  }

  for (int i = 0; i < num_cores; i++) {
    pthread_join(threads[i], NULL);
    for (int j = 0; j < analyzer->blocksize / 2; j++) {
      bins[j] += thread_analyzers[i].bins[j];
    }
    free(thread_analyzers[i].bins);
  }

  for (int i = 0; i < analyzer->blocksize / 2; i++) {
    bins[i] /= (samples / analyzer->shift);
    bins[i] = 20 * log10(bins[i]);
  }

  fftw_cleanup_threads();

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
