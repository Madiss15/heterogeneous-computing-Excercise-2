#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <sys/time.h>
#include "kissfft/kiss_fft.h"
#include "kissfft/kiss_fftnd.h"

#define MAX(a,b) ((a) > (b) ? (a) : (b))
#define MIN(a,b) ((a) < (b) ? (a) : (b))

typedef struct {
  char* filename;
  int blocksize;
  int shift;
  int threshold;
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

  double* normalized_data_left = malloc(samples * sizeof(double));
  for (int i = 0; i < samples; i++) {
    normalized_data_left[i] = data[i*2] / 32768.0;
  }
  free(data);

  int bins_size = analyzer->blocksize / 2;
  double* bins = calloc(bins_size, sizeof(double));

  kiss_fft_cfg fft_cfg = kiss_fft_alloc(analyzer->blocksize, 0, NULL, NULL);
  kiss_fft_cpx* fft_in = malloc(sizeof(kiss_fft_cpx) * analyzer->blocksize);
  kiss_fft_cpx* fft_out = malloc(sizeof(kiss_fft_cpx) * analyzer->blocksize);

  int offset = 0;
  int count = 0;
  while (offset + analyzer->blocksize <= samples) {
    for (int i = 0; i < analyzer->blocksize; i++) {
      fft_in[i].r = normalized_data_left[offset + i];
      fft_in[i].i = 0;
    }

    kiss_fft(fft_cfg, fft_in, fft_out);

    for (int i = 0; i < bins_size; i++) {
      double real = fft_out[i].r;
      double imag = fft_out[i].i;
      bins[i] += sqrt(real*real + imag*imag);
    }

    offset += analyzer->shift;
    count++;
  }

  free(fft_in);
  free(fft_out);
  free(fft_cfg);
  free(normalized_data_left);

  for (int i = 0; i < bins_size; i++) {
    bins[i] /= count;
    bins[i] = 20 * log10(bins[i]);
  }

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
