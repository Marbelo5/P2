#include <math.h>
#include "pav_analysis.h"

float compute_power(const float *x, unsigned int N) {
    if (N == 0) return -100.0f;
    float sum_sq = 0.0f;
    for (unsigned int n = 0; n < N; n++) {
        sum_sq += x[n] * x[n];
    }
    float mean_power = sum_sq / (float)N;
    if (mean_power <= 1e-12f) { // Evitar log10 de cero
        return -120.0f;
    }
    return 10.0f * log10f(mean_power);
}

float compute_am(const float *x, unsigned int N) {
    if (N == 0) return 0.0f;
    float sum_abs = 0.0f;
    for (unsigned int n = 0; n < N; n++) {
        sum_abs += fabsf(x[n]);
    }
    return sum_abs / (float)N;
}

float compute_zcr(const float *x, unsigned int N, float fm) {
    if (N <= 1) return 0.0f;
    unsigned int count = 0;
    for (unsigned int n = 1; n < N; n++) {
        // Un cruce por cero ocurre cuando cambia el signo entre muestras adyacentes
        if ((x[n] >= 0.0f && x[n - 1] < 0.0f) || (x[n] < 0.0f && x[n - 1] >= 0.0f)) {
            count++;
        }
    }
    return (fm / 2.0f) * ((float)count / (float)(N - 1));
}