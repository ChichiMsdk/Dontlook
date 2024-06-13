#define _USE_MATH_DEFINES // for C
#define WIN32_LEAN_AND_MEAN
#define _CRT_SECURE_NO_WARNINGS

#include <complex.h> 
#include <stdio.h> 
#include <math.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <assert.h>

#ifdef _MSC_VER
#    define Float_Complex _Fcomplex
#    define cfromreal(re) _FCbuild(re, 0)
#    define cfromimag(im) _FCbuild(0, im)
#    define mulcc _FCmulcc
#    define addcc(a, b) _FCbuild(crealf(a) + crealf(b), cimagf(a) + cimagf(b))
#    define subcc(a, b) _FCbuild(crealf(a) - crealf(b), cimagf(a) - cimagf(b))
#else
#    define Float_Complex float complex
#    define cfromreal(re) (re)
#    define cfromimag(im) ((im)*I)
#    define mulcc(a, b) ((a)*(b))
#    define addcc(a, b) ((a)+(b))
#    define subcc(a, b) ((a)-(b))
#endif 

#define FFT_SIZE (1<<13)

float in_raw[FFT_SIZE];
float in_win[FFT_SIZE];
Float_Complex out_raw[FFT_SIZE];
float out_log[FFT_SIZE];
float out_smooth[FFT_SIZE];
float out_smear[FFT_SIZE];

static bool fft_settled(void)
{
    float eps = 1e-3;
    for (size_t i = 0; i < FFT_SIZE; ++i) {
        if (out_smooth[i] > eps) return false;
        if (out_smear[i] > eps) return false;
    }
    return true;
}

static void fft_clean(void)
{
    memset(in_raw, 0, sizeof(in_raw));
    memset(in_win, 0, sizeof(in_win));
    memset(out_raw, 0, sizeof(out_raw));
    memset(out_log, 0, sizeof(out_log));
    memset(out_smooth, 0, sizeof(out_smooth));
    memset(out_smear, 0, sizeof(out_smear));
}

// Ported from https://rosettacode.org/wiki/Fast_Fourier_transform#Python
static void fft(float in[], size_t stride, Float_Complex out[], size_t n)
{
    assert(n > 0);

    if (n == 1) {
        out[0] = cfromreal(in[0]);
        return;
    }

    fft(in, stride*2, out, n/2);
    fft(in + stride, stride*2,  out + n/2, n/2);

    for (size_t k = 0; k < n/2; ++k) {
        float t = (float)k/n;
        Float_Complex v = mulcc(cexpf(cfromimag(-2*M_PI*t)), out[k + n/2]);
        Float_Complex e = out[k];
        out[k]       = addcc(e, v);
        out[k + n/2] = subcc(e, v);
    }
}

static inline float amp(Float_Complex z)
{
    float a = crealf(z);
    float b = cimagf(z);
    return logf(a*a + b*b);
}

static size_t fft_analyze(float dt)
{
    // Apply the Hann Window on the Input - https://en.wikipedia.org/wiki/Hann_function
    for (size_t i = 0; i < FFT_SIZE; ++i) {
        float t = (float)i/(FFT_SIZE - 1);
        float hann = 0.5 - 0.5*cosf(2*M_PI*t);
        in_win[i] = in_raw[i]*hann;
    }

    // FFT
    fft(in_win, 1, out_raw, FFT_SIZE);

    // "Squash" into the Logarithmic Scale
    float step = 1.06;
    float lowf = 1.0f;
    size_t m = 0;
    float max_amp = 1.0f;
    for (float f = lowf; (size_t) f < FFT_SIZE/2; f = ceilf(f*step)) {
        float f1 = ceilf(f*step);
        float a = 0.0f;
        for (size_t q = (size_t) f; q < FFT_SIZE/2 && q < (size_t) f1; ++q) {
            float b = amp(out_raw[q]);
            if (b > a) a = b;
        }
        if (max_amp < a) max_amp = a;
        out_log[m++] = a;
    }

    // Normalize Frequencies to 0..1 range
    for (size_t i = 0; i < m; ++i) {
        out_log[i] /= max_amp;
    }

    // Smooth out and smear the values
    for (size_t i = 0; i < m; ++i) {
        float smoothness = 8;
        out_smooth[i] += (out_log[i] - out_smooth[i])*smoothness*dt;
        float smearness = 3;
        out_smear[i] += (out_smooth[i] - out_smear[i])*smearness*dt;
    }

    return m;
}

static void fft_push(float frame)
{
    memmove(in_raw, in_raw + 1, (FFT_SIZE - 1)*sizeof(in_raw[0]));
    in_raw[FFT_SIZE-1] = frame;
}
 
/*
 * int main(void) 
 * { 
 * 	float z = 1000;
 * 	size_t z2 = fft_analyze(z);
 * 	printf("z: %f\nz2: %llu\n", z, z2);
 * 	return 0;
 * } 
 */
