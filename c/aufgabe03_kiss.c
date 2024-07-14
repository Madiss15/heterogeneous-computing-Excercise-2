#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <sys/time.h>
#include "kissfft/kiss_fft.h"
#include "kissfft/kiss_fftnd.h"
#include <pthread.h>
#include <unistd.h>



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
  double* bins; // Shared bins array for threads
  double* normalized_data_left; // Normalized data
  int samples; // Total number of samples
} FFT_Analyzer;

typedef struct {
  FFT_Analyzer* analyzer;
  int start;
  int end;
  pthread_mutex_t* lock;
} ThreadData;

void* process_chunk(void* arg) {
  ThreadData* data = (ThreadData*)arg;
  FFT_Analyzer* analyzer = data->analyzer;
  int start = data->start;
  int end = data->end;
  int blocksize = analyzer->blocksize;
  int shift = analyzer->shift;
  int bins_size = blocksize / 2;
  double* bins = calloc(bins_size, sizeof(double));
  double* normalized_data_left = analyzer->normalized_data_left;

  kiss_fft_cfg fft_cfg = kiss_fft_alloc(blocksize, 0, NULL, NULL);
  kiss_fft_cpx* fft_in = malloc(sizeof(kiss_fft_cpx) * blocksize);
  kiss_fft_cpx* fft_out = malloc(sizeof(kiss_fft_cpx) * blocksize);

  for (int offset = start; offset + blocksize <= end; offset += shift) {
    for (int i = 0; i < blocksize; i++) {
      fft_in[i].r = normalized_data_left[offset + i];
      fft_in[i].i = 0;
    }

    kiss_fft(fft_cfg, fft_in, fft_out);

    for (int i = 0; i < bins_size; i++) {
      double real = fft_out[i].r;
      double imag = fft_out[i].i;
      bins[i] += sqrt(real*real + imag*imag);
    }
  }

  free(fft_in);
  free(fft_out);
  free(fft_cfg);

  pthread_mutex_lock(data->lock);
  for (int i = 0; i < bins_size; i++) {
    analyzer->bins[i] += bins[i];
  }
  pthread_mutex_unlock(data->lock);

  free(bins);
  return NULL;
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
  short* data = malloc(file_size);
  fread(data, 2, samples * 2, file);
  fclose(file);

  analyzer->samples = samples;
  analyzer->normalized_data_left = malloc(samples * sizeof(double));
  for (int i = 0; i < samples; i++) {
    analyzer->normalized_data_left[i] = data[i * 2] / 32768.0;
  }
  free(data);

  int bins_size = analyzer->blocksize / 2;
  double* bins = calloc(bins_size, sizeof(double));
  analyzer->bins = bins;
  
  int num_cores = get_num_cores();
  printf("Using %d cores\n", num_cores);
  pthread_t threads[num_cores];
  ThreadData thread_data[num_cores];
  pthread_mutex_t lock;
  pthread_mutex_init(&lock, NULL);

  int chunk_size = (samples + num_cores - 1) / num_cores;
  for (int i = 0; i < num_cores; i++) {
    thread_data[i].analyzer = analyzer;
    thread_data[i].start = i * chunk_size;
    thread_data[i].end = MIN((i + 1) * chunk_size, samples);
    thread_data[i].lock = &lock;
    pthread_create(&threads[i], NULL, process_chunk, &thread_data[i]);
  }

  for (int i = 0; i < num_cores; i++) {
    pthread_join(threads[i], NULL);
  }

  for (int i = 0; i < bins_size; i++) {
    bins[i] /= (samples / analyzer->shift);
    bins[i] = 20 * log10(bins[i]);
  }

  free(analyzer->normalized_data_left);
  pthread_mutex_destroy(&lock);

  return bins;
}

int main(int argc, char* argv[]) {
  if (argc != 5) {
    fprintf(stderr, "Usage: %s <filename> <blocksize> <shift> <threshold>\n", argv[0]);
    return 1;
  }

  FFT_Analyzer analyzer = {
    .filename = strdup(argv[1]),
    .blocksize = MAX(MIN(512, atoi(argv[2])), 64),
    .shift = MAX(MIN(atoi(argv[2]), atoi(argv[3])), 1),
    .threshold = atoi(argv[4])
  };

  struct timeval start, end;
  gettimeofday(&start, NULL);
  double* result = get_amplitude_mean(&analyzer);
  gettimeofday(&end, NULL);

  if (result) {
    for (int i = 0; i < analyzer.blocksize/2; i++) {
      if(result[i] > analyzer.threshold) {
	printf("%dHz %f\n", i*(44100/(analyzer.blocksize/2)), result[i]);
      }
    }
    printf("\n");
    free(result);
  }


  long seconds = end.tv_sec - start.tv_sec;
  long microseconds = end.tv_usec - start.tv_usec;
  double elapsed_time = seconds + microseconds / 1e6;
  printf("Execution time: %f seconds\n", elapsed_time);

  free(analyzer.filename);
  return 0;
}
