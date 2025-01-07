#include <seq.h>
#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
// Random matrix generation function
double* matGenerate(){
    //printf("Generating random matrix\n");
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
    //printf("Generating random symmetric matrix\n");
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
void seqTest(int argc, char** argv){
    int size, rank;
    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    if (rank == 0){
        double* A = matGenerate();
        printf("Sequential test\n");
        for (int i = 0; i < 10; i++){
            double start, end;
            start = MPI_Wtime();
            if (checkSymSeq(A)){
                end = MPI_Wtime();
                printf("Check symmetric time: %f\n", end - start);
                printf("Matrix is symmetric\n");
                matRandomize(A);
                continue;
            } else {
                printf("Matrix is not symmetric\n");
            }
            double* T = matTranspose(A);
            end = MPI_Wtime();
            printf("Sequential transpose time: %f\n", end - start);
            if(isTransposed(A, T)){
                printf("Matrix transposed correctly\n");
            } else {
                printf("Matrix transposed incorrectly\n");
            }
            matRandomize(A);
            free(T);
        }
        free(A);
    }

    MPI_Finalize();
}