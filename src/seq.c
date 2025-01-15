#include <seq.h>
#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include <time.h>
// Random matrix generation function
double* matGenerate(){
    double* A = (double*) malloc(N * N * sizeof(double));
    if(A == NULL){
        printf("Memory allocation failed\n");
        exit(1);
    }
    for(int i = 0; i < N * N; i++){
        A[i] = (double) rand() / RAND_MAX;
    }
    return A;
}

// Random symmetric matrix generation function
double* matGenerateSym(){
    double* A = (double*) malloc(N * N * sizeof(double));
    if(A == NULL){
        printf("Memory allocation failed\n");
        exit(1);
    }
    for(int i = 0; i < N; i++){
        for(int j = 0; j <= i; j++){
            A[i * N + j] = (double) rand() / RAND_MAX;
            A[j * N + i] = A[i * N + j];
        }
    }
    return A;
}

// Matrix randomization function
void matRandomize(double* M){
    for (int i = 0; i < N * N; i++){
        M[i] = (double) rand() / RAND_MAX;
    }
}

// Simple checkSym function with the wall time measurement
int checkSymSeq(double* A){
    for(int i = 0; i < N; i++){
        for(int j = i + 1; j < N; j++){
            if(A[i * N + j] != A[j * N + i]){
                return 0;
            }
        }
    }
    return 1;
}

// Simple sequential transpose function
double* matTranspose(double* M){
    double* T = (double*) malloc(N * N * sizeof(double));
    if (T == NULL){
        printf("Memory allocation failed\n");
        exit(1);
    }
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            T[i * N + j] = M[j * N + i];
        }
    }
    return T;
}

// Function to check if the matrix is transposed
int isTransposed(double* A, double* B){
    for(int i = 0; i < N; i++){
        for(int j = 0; j < N; j++){
            if(A[i * N + j] != B[j * N + i]){
                return 0;
            }
        }
    }
    return 1;
}

// Sequential test function
void seqTest_transpose(int numRuns){
    double* A = matGenerate();
    double avgTime = 0;
    struct timespec start, end;
    for (int i = 0; i < numRuns; i++){
        clock_gettime(CLOCK_REALTIME, &start);
        double* T = matTranspose(A);
        clock_gettime(CLOCK_REALTIME, &end);
        double elapsed = (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1e9;
        avgTime += elapsed;
        matRandomize(A);
        free(T);
    }
    avgTime /= numRuns;
    printf("Seq, transpose, %d, %d, %.9f\n", N, 1, avgTime);
    free(A);
}

// Sequential test function for the symmetric matrix
void seqTest_checkSym(int numRuns){
    double* A;
    double avgTime = 0;
    struct timespec start, end;
    for (int i = 0; i < 10; i++){
        A = matGenerateSym();
        clock_gettime(CLOCK_REALTIME, &start);
        if (!checkSymSeq(A)){
            fprintf(stderr, "Error: Matrix is not symmetric\n");
            exit(1);
        }
        clock_gettime(CLOCK_REALTIME, &end);
        double elapsed = (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1e9;
        avgTime += elapsed;
    }
    avgTime /= numRuns;
    printf("Seq, checkSym, %d, %d, %.9f\n", N, 1, avgTime);
    free(A);
}