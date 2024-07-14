import numpy as np
import os
import sys

def generate_wav_header(num_samples, sample_rate, num_channels=1, bytes_per_sample=2):
    data_size = num_samples * num_channels * bytes_per_sample
    riff = b'RIFF'
    file_size = data_size + 36
    wave = b'WAVE'
    fmt = b'fmt '
    fmt_chunk_size = 16
    audio_format = 1  # PCM
    num_channels = num_channels
    byte_rate = sample_rate * num_channels * bytes_per_sample
    block_align = num_channels * bytes_per_sample
    bits_per_sample = bytes_per_sample * 8
    fmt_chunk = (fmt_chunk_size).to_bytes(4, 'little') + \
                (audio_format).to_bytes(2, 'little') + \
                (num_channels).to_bytes(2, 'little') + \
                (sample_rate).to_bytes(4, 'little') + \
                (byte_rate).to_bytes(4, 'little') + \
                (block_align).to_bytes(2, 'little') + \
                (bits_per_sample).to_bytes(2, 'little')
    data = b'data'
    data_size_bytes = (data_size).to_bytes(4, 'little')
    header = riff + \
             (file_size).to_bytes(4, 'little') + \
             wave + \
             fmt + \
             fmt_chunk + \
             data + \
             data_size_bytes

    return header


def write_wave(filename, signal_func, duration=3, frequency=440):
    sample_rate = 44100
    num_samples = int(duration * sample_rate)
    t = np.linspace(0, duration, num_samples, endpoint=False)
    
    signal = signal_func(t, sample_rate, frequency)
    signal = np.int16(signal / np.max(np.abs(signal)) * 32767)
    
    wav_header = generate_wav_header(num_samples, sample_rate)
    
    with open(filename, 'wb') as wav_file:
        wav_file.write(wav_header)
        wav_file.write(signal.tobytes())



def sine_wave_220(t, sample_rate, frequency=440):
    return np.sin(2 * np.pi * frequency * t)


def sawtooth_wave(t, sample_rate, frequency=440):
    return 2 * (t * frequency - np.floor(0.5 + t * frequency))

def square_wave(t, sample_rate, frequency=440):
    return np.sign(np.sin(2 * np.pi * frequency * t))

def triangle_wave(t, sample_rate, frequency=440):
    return 2 * np.abs(2 * (t * frequency - np.floor(0.5 + t * frequency))) - 1

def chirp(t, sample_rate, frequency=440):
    f0, f1 = 20, 20000 
    return np.sin(2 * np.pi * (f0 * t + (f1 - f0) / (2 * t[-1]) * t**2))

def am_modulation(t, sample_rate, frequency=440):
    carrier = np.sin(2 * np.pi * frequency * t)
    modulator = 0.5 * (1 + np.sin(2 * np.pi * 100 * t))
    return carrier * modulator

def fm_modulation(t, sample_rate, frequency=440):
    modulation_index = 5
    modulation_freq = 100
    return np.sin(2 * np.pi * frequency * t + modulation_index * np.sin(2 * np.pi * modulation_freq * t))

def white_noise(t, sample_rate, frequency=None):
    return np.random.normal(0, 1, len(t))

def harmonics(t, sample_rate, frequency=440):
    return (np.sin(2 * np.pi * frequency * t) +
            0.5 * np.sin(2 * np.pi * 2 * frequency * t) +
            0.3 * np.sin(2 * np.pi * 3 * frequency * t) +
            0.2 * np.sin(2 * np.pi * 4 * frequency * t))

def exponential_decay(t, sample_rate, frequency=440):
    decay_rate = 5
    return np.sin(2 * np.pi * frequency * t) * np.exp(-decay_rate * t)

def pulse_train(t, sample_rate, frequency=440):
    duty_cycle = 0.1
    return ((t * frequency) % 1) < duty_cycle

def sinc_function(t, sample_rate, frequency=440):
    x = np.pi * frequency * (t - 0.5 * t[-1])
    return np.sinc(x)

if __name__ == "__main__":

    #ask for duration
    duration = 5
    try:
        duration = float(input("Enter the duration of the waveforms in seconds: "))
        if duration <= 0:
            print("Duration must be a positive number. Please try again.")
            sys.exit()
    except ValueError:
        print("Invalid input. Please enter a numeric value.")

    
    output_dir = f'../generated/{duration}'
    if not os.path.exists(output_dir):
        os.makedirs(output_dir)
 
    write_wave(os.path.join(output_dir,'sine_wave_220.wav'), sine_wave_220,duration)
    write_wave(os.path.join(output_dir,'sawtooth_wave.wav'), sawtooth_wave,duration)
    write_wave(os.path.join(output_dir,'square_wave.wav'), square_wave,duration)
    write_wave(os.path.join(output_dir,'triangle_wave.wav'), triangle_wave,duration)
    write_wave(os.path.join(output_dir,'chirp.wav'), chirp,duration)
    write_wave(os.path.join(output_dir,'am_modulation.wav'), am_modulation,duration)
    write_wave(os.path.join(output_dir,'fm_modulation.wav'), fm_modulation,duration)
    write_wave(os.path.join(output_dir,'white_noise.wav'), white_noise,duration)
    write_wave(os.path.join(output_dir,'harmonics.wav'), harmonics,duration)
    write_wave(os.path.join(output_dir,'exponential_decay.wav'), exponential_decay,duration)
    write_wave(os.path.join(output_dir,'pulse_train.wav'), pulse_train,duration)
    write_wave(os.path.join(output_dir,'sinc_function.wav'), sinc_function,duration)
    
    print("wave files generated")


    
