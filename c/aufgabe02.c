#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

#define SAMPLE_RATE 44100
#define BITS_PER_SAMPLE 16
#define BYTES_PER_SAMPLE (BITS_PER_SAMPLE / 8)
#define NUM_CHANNELS 1

void generate_wav_header(FILE *file, int num_samples) {
  int data_size = num_samples * NUM_CHANNELS * BYTES_PER_SAMPLE;
  int file_size = data_size + 36;

  // RIFF chunk
  fwrite("RIFF", 1, 4, file);
  fwrite(&file_size, 4, 1, file);
  fwrite("WAVE", 1, 4, file);

  // fmt subchunk
  fwrite("fmt ", 1, 4, file);
  int fmt_chunk_size = 16;
  fwrite(&fmt_chunk_size, 4, 1, file);
  short audio_format = 1; // PCM
  fwrite(&audio_format, 2, 1, file);
  short num_channels = NUM_CHANNELS;
  fwrite(&num_channels, 2, 1, file);
  int sample_rate = SAMPLE_RATE;
  fwrite(&sample_rate, 4, 1, file);
  int byte_rate = SAMPLE_RATE * NUM_CHANNELS * BYTES_PER_SAMPLE;
  fwrite(&byte_rate, 4, 1, file);
  short block_align = NUM_CHANNELS * BYTES_PER_SAMPLE;
  fwrite(&block_align, 2, 1, file);
  short bits_per_sample = BITS_PER_SAMPLE;
  fwrite(&bits_per_sample, 2, 1, file);

  // data subchunk
  fwrite("data", 1, 4, file);
  fwrite(&data_size, 4, 1, file);
}

void write_wave(const char *filename, double (*signal_func)(double, double), double duration, double frequency) {
  int num_samples = (int)(duration * SAMPLE_RATE);
  FILE *file = fopen(filename, "wb");
  if (!file) {
    perror("Error opening file");
    exit(EXIT_FAILURE);
  }

  generate_wav_header(file, num_samples);

  for (int i = 0; i < num_samples; i++) {
    double t = (double)i / SAMPLE_RATE;
    double sample = signal_func(t, frequency);
    short sample_int = (short)(sample * 32767); // Convert to 16-bit PCM
    fwrite(&sample_int, BYTES_PER_SAMPLE, 1, file);
  }

  fclose(file);
}

double sine_wave(double t, double frequency) {
  return sin(2 * M_PI * frequency * t);
}

double sawtooth_wave(double t, double frequency) {
  return 2 * (t * frequency - floor(0.5 + t * frequency));
}

double square_wave(double t, double frequency) {
  return sin(2 * M_PI * frequency * t) >= 0 ? 1 : -1;
}

double triangle_wave(double t, double frequency) {
  return 2 * fabs(2 * (t * frequency - floor(0.5 + t * frequency))) - 1;
}

double chirp(double t, double frequency) {
  double f0 = 20, f1 = 20000;
  return sin(2 * M_PI * (f0 * t + (f1 - f0) / (2 * t) * t * t));
}

double am_modulation(double t, double frequency) {
  double carrier = sin(2 * M_PI * frequency * t);
  double modulator = 0.5 * (1 + sin(2 * M_PI * 100 * t));
  return carrier * modulator;
}

double fm_modulation(double t, double frequency) {
  double modulation_index = 5;
  double modulation_freq = 100;
  return sin(2 * M_PI * frequency * t + modulation_index * sin(2 * M_PI * modulation_freq * t));
}

double white_noise(double t, double frequency) {
  return (double)rand() / RAND_MAX * 2 - 1; // Random value between -1 and 1
}

double harmonics(double t, double frequency) {
  return sin(2 * M_PI * frequency * t) +
         0.5 * sin(2 * M_PI * 2 * frequency * t) +
         0.3 * sin(2 * M_PI * 3 * frequency * t) +
         0.2 * sin(2 * M_PI * 4 * frequency * t);
}

double exponential_decay(double t, double frequency) {
  double decay_rate = 5;
  return sin(2 * M_PI * frequency * t) * exp(-decay_rate * t);
}

double pulse_train(double t, double frequency) {
  double duty_cycle = 0.1;
  return ((t * frequency) - floor(t * frequency)) < duty_cycle ? 1 : 0;
}

double sinc_function(double t, double frequency) {
  double x = M_PI * frequency * (t - 0.5);
  return sin(x) / x; // sinc function
}

int main() {
  double duration = 6000.0;

  char output_dir[256];
  snprintf(output_dir, sizeof(output_dir), "../../generated/%f", duration);
  //    mkdir(output_dir, 0755);

  write_wave("./test/sine_wave_220.wav", sine_wave, duration, 220);
  /* write_wave("sawtooth_wave.wav", sawtooth_wave, duration, 440); */
  /* write_wave("square_wave.wav", square_wave, duration, 440); */
  /* write_wave("triangle_wave.wav", triangle_wave, duration, 440); */
  /* write_wave("chirp.wav", chirp, duration, 440); */
  /* write_wave("am_modulation.wav", am_modulation, duration, 440); */
  /* write_wave("fm_modulation.wav", fm_modulation, duration, 440); */
  /* write_wave("white_noise.wav", white_noise, duration, 440); */
  /* write_wave("harmonics.wav", harmonics, duration, 440); */
  /* write_wave("exponential_decay.wav", exponential_decay, duration, 440); */
  /* write_wave("pulse_train.wav", pulse_train, duration, 440); */
  /* write_wave("sinc_function.wav", sinc_function, duration, 440); */

  printf("Wave files generated\n");

  return 0;
}
