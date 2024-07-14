#define PI 3.14159265358979323846

__kernel void fft(__global double2 *a, int n, int step) {
    int gid = get_global_id(0);
    int butterfly_size = 2 * step;
    
    if (gid < n / 2) {
        int i = gid % step + (gid / step) * butterfly_size;
        int j = i + step;
        
        double angle = -2.0 * PI * gid / n;
        double2 twiddle = (double2)(cos(angle), sin(angle));
        
        double2 temp = a[j];
        a[j] = a[i] - (double2)(twiddle.x * temp.x - twiddle.y * temp.y,
                                twiddle.x * temp.y + twiddle.y * temp.x);
        a[i] += (double2)(twiddle.x * temp.x - twiddle.y * temp.y,
                          twiddle.x * temp.y + twiddle.y * temp.x);
    }
}

__kernel void compute_magnitude_squared(__global double2 *input, __global double *output, int n) {
    int gid = get_global_id(0);
    if (gid < n) {
        output[gid] = input[gid].x * input[gid].x + input[gid].y * input[gid].y;
    }
}

__kernel void compute_db(__global double *input, __global double *output, int n) {
    int gid = get_global_id(0);
    if (gid < n) {
        output[gid] = 10 * log10(input[gid] + 1e-9);
    }
}