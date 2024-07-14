import argparse
import numpy as np
import multiprocessing as mp
import time

class FFT_Analyzer:
    def __init__(self, filename, blocksize, shift, threshold):
        self.filename = filename
        self.blocksize = max(min(512, int(blocksize)), 64)
        self.shift = max(min(self.blocksize, int(shift)), 1)
        self.threshold = int(threshold)
    
    def process_chunk(self, chunk):
        bins = np.zeros(self.blocksize // 2)
        samples = len(chunk)
        offset = 0
        while offset < samples:
            if (offset + self.blocksize) > samples:
                break
            
            block = chunk[offset:offset+self.blocksize]
            fft_data = np.fft.fft(block)
            amplitude = np.abs(fft_data[:self.blocksize//2])
            bins += amplitude
            offset += self.shift
        return bins
    
    def get_amplitude_mean(self):
        data = np.fromfile(self.filename, dtype=np.int16)
        samples = len(data) // 2
        normalized_data_left = (data[::2] / (2 ** 15)).astype(np.float32)
        
        num_cores = mp.cpu_count()
        
        chunk_size = samples // num_cores
        chunks = [normalized_data_left[i:i+chunk_size] for i in range(0, samples, chunk_size)]
        
        with mp.Pool(num_cores) as pool:
            results = pool.map(self.process_chunk, chunks)
        
        bins = np.sum(results, axis=0)
        
        bins = bins / (samples // self.shift)
        bins_db = 20 * np.log10(bins)
        return bins_db


def main(args):
    fft_analyzer = FFT_Analyzer(args.filename, args.blocksize, args.shift, args.threshold)
    start_time = time.time()
    result = fft_analyzer.get_amplitude_mean()
    end_time = time.time()
    execution_time = end_time - start_time
    for x in range(len(result)):
        if(result[x] > fft_analyzer.threshold):
            print(f"{x*(44100//(fft_analyzer.blocksize/2))}Hz {result[x]}")
        
    print(f"Execution time: {execution_time} seconds")

if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument('filename')
    parser.add_argument('blocksize')
    parser.add_argument('shift')
    parser.add_argument('threshold')
    
    args = parser.parse_args()
    main(args)
