import argparse
import numpy as np
import time
from reikna import cluda
from reikna.fft import FFT

class FFT_Analyzer:
    def __init__(self, filename, blocksize, shift, threshold):
        self.filename = filename
        self.blocksize = max(min(512, int(blocksize)), 64)
        self.shift = max(min(self.blocksize, int(shift)), 1)
        self.threshold = int(threshold)
        
        # Initialize OpenCL
        self.api, self.thr = self.initialize_gpu_api('auto')
        
        
        # Prepare FFT
        self.fft = FFT(np.zeros(self.blocksize, dtype=np.complex64))
        self.fft_c = self.fft.compile(self.thr)

    def initialize_gpu_api(self, preference):
            if preference == 'auto':
                try:
                    api = cluda.cuda_api()
                    thr = api.Thread.create()
                    print("Using CUDA")
                    return api, thr
                except:
                    try:
                        api = cluda.ocl_api()
                        thr = api.Thread.create()
                        print("Using OpenCL")
                        return api, thr
                    except:
                        raise RuntimeError("No GPU API available")
            elif preference == 'cuda':
                api = cluda.cuda_api()
                thr = api.Thread.create()
                print("Using CUDA")
                return api, thr
            elif preference == 'opencl':
                api = cluda.ocl_api()
                thr = api.Thread.create()
                print("Using OpenCL")
                return api, thr
            else:
                raise ValueError("Invalid API preference")
        
    def get_amplitude_mean(self):
        data = np.fromfile(self.filename, dtype=np.int16)
        samples = len(data) // 2
        normalized_data_left = (data[::2] / (2 ** 15)).astype(np.float32)
        bins = np.zeros(self.blocksize // 2)
        offset = 0
        
        while offset < samples:
            if (offset + self.blocksize) > samples:
                break
            
            block = normalized_data_left[offset:offset+self.blocksize]
            
            # Move data to GPU
            gpu_block = self.thr.to_device(block.astype(np.complex64))
            
            # Perform FFT on GPU
            gpu_fft_result = self.thr.array(gpu_block.shape, dtype=np.complex64)
            self.fft_c(gpu_fft_result, gpu_block)
            
            # Move result back to CPU
            fft_data = gpu_fft_result.get()
            
            amplitude = np.abs(fft_data[:self.blocksize//2])
            bins += amplitude
            offset += self.shift
        
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
