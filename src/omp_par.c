#include "stdio.h"
#include "stdlib.h"
#include "omp.h"
#include "time.h"
#include "seq.h"

double* matTransposeOMP(double* M, int blockSize, int numThreads, double* time){
    struct timespec start, end;
    double* T = (double*)malloc(N*N*sizeof(double));
    clock_gettime(CLOCK_REALTIME, &start);
    #pragma omp parallel for schedule(dynamic) num_threads(numThreads)
    for (int i = 0; i < N; i += blockSize) {
        for (int j = 0; j < N; j += blockSize) {
            for (int ii = i; ii < i + blockSize; ii++) {
                for (int jj = j; jj < j + blockSize; jj++) {
                    T[ii*N + jj] = M[jj*N + ii];
                }
            }
        }
    }
    clock_gettime(CLOCK_REALTIME, &end);
    double elapsed = (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1e9;
    *time = elapsed;
    return T;
}

void OMPtest_transpose(int numRuns, int blockSize, int numThreads) {
    double* M = (double*)malloc(N*N*sizeof(double));
    double* T;
    double time;
    double avgTime = 0;
    for (int i = 0; i < numRuns; i++){
        matRandomize(M);
        T = matTransposeOMP(M, blockSize, numThreads, &time);
        avgTime += time;
    }
    avgTime /= numRuns;
    printf("OMP,transpose,%d,%d,%f\n", N, numThreads, avgTime);
    free(M);
    free(T);
}