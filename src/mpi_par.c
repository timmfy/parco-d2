#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mpi.h>
#include <seq.h>

//int checkSymMPI(double* A){}

/*
    marTransposeMPI works in already initialized MPI environment
*/

//Dogshit, because here only one process is used
double* matTransposeMPIv1(int rank, double* A) {
    double* T = (double*) malloc(N * N * sizeof(double));
    MPI_Datatype row, transpose;
    MPI_Status status;
    
    MPI_Type_vector(N, 1, N, MPI_DOUBLE, &row);
    
    MPI_Type_create_hvector(N, 1, sizeof(double), row, &transpose);
    
    MPI_Type_commit(&transpose);
    
    MPI_Sendrecv(A, 1, transpose, rank, 0, T, N * N, MPI_DOUBLE, rank, 0, MPI_COMM_WORLD, &status);
    
    MPI_Type_free(&transpose);
    MPI_Type_free(&row);
    
    return T;
}
//Block-based distribution algorithm
//Assume that N is divisible by P
//NxN matrix; P processes; each process computes P matrices of size N/P x N/P locally
//Two steps: inner transpose and outer transpose
//Inner transpose:
// Transpose P local matrices
//Outer transpose:
// Reorder the local matrices in the global matrix
double* matTransposeMPIv2(int rank, int size, double* time, double* A, int block_size) {
    double* T = (double*) malloc(N * N * sizeof(double));

    double* local_block = (double*) malloc(block_size * N * sizeof(double));
    double* inner_transpose = (double*) malloc(block_size * N * sizeof(double));
    double start, end;

    int displacements[size * size];
    for (int i = 0; i < size; i++) {
        for (int j = 0; j < size; j++) {
            displacements[i * size + j] = (j * block_size * N + i * block_size);
        }
    }

    // Scatter the matrix
    MPI_Scatter(A, block_size * N, MPI_DOUBLE, local_block, block_size * N, MPI_DOUBLE, 0, MPI_COMM_WORLD);
    if (rank == 0) {
        start = MPI_Wtime();
    }


    //Transpose the local block
    for (int block_index = 0; block_index < N / block_size; block_index++) {
        for (int i = 0; i < block_size; i++) {
            for (int j = 0; j < block_size; j++) {
                inner_transpose[j * N + i + block_index * block_size] = local_block[i * N + j + block_index * block_size];
            }
        }
    }

    MPI_Datatype block_type;
    MPI_Type_vector(block_size, block_size, N, MPI_DOUBLE, &block_type);
    MPI_Type_commit(&block_type);

    if (rank == 0) {
        end = MPI_Wtime();
        *time = end - start;
        for (int b = 0; b < size; b++) {
            for (int p = 1; p < size; p++) {
                MPI_Recv(T + displacements[p * size + b], 1, block_type, p, p * size + b, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            }
        }
    }
    else {
        for (int b = 0; b < size; b++) {
            MPI_Send(inner_transpose + b * block_size, 1, block_type, 0, rank * size + b, MPI_COMM_WORLD);
        }
    }
    if (rank == 0) {
        for (int b = 0; b < size; b++) {
            MPI_Sendrecv(inner_transpose + b * block_size, 1, block_type, 0, b, T + displacements[b], 1, block_type, 0, b, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        } 
    }
    // Clean up
    MPI_Type_free(&block_type);
    free(local_block);
    free(inner_transpose);

    return T;
}

double* matTransposeMPIv3(int rank, int size, double* time, double* A, int block_size) {
    double* T = (double*) malloc(N * N * sizeof(double));

    double* local_block = (double*) malloc(block_size * N * sizeof(double));
    double* inner_transpose = (double*) malloc(block_size * N * sizeof(double));
    double* outer_transpose = (double*) malloc(block_size * N * sizeof(double));
    double start, end;
    MPI_Request requests[3];
    // Scatter the matrix
    MPI_Iscatter(A, block_size * N, MPI_DOUBLE, local_block, block_size * N, MPI_DOUBLE, 0, MPI_COMM_WORLD, &requests[0]);
    if (rank == 0) {
        start = MPI_Wtime();
    }
    // Wait for the scatter to finish
    MPI_Wait(&requests[0], MPI_STATUS_IGNORE);

    //Transpose the local block
    for (int block_index = 0; block_index < N / block_size; block_index++) {
        for (int i = 0; i < block_size; i++) {
            for (int j = 0; j < block_size; j++) {
                inner_transpose[j * N + i + block_index * block_size] = local_block[i * N + j + block_index * block_size];
            }
        }
    }

    // Define MPI datatype for blocks
    MPI_Datatype block_type, resized_block_type;
    MPI_Type_vector(block_size, block_size, N, MPI_DOUBLE, &block_type);
    MPI_Type_create_resized(block_type, 0, block_size * sizeof(double), &resized_block_type);
    MPI_Type_commit(&resized_block_type);

    MPI_Ialltoall(inner_transpose, 1, resized_block_type, outer_transpose, 1, resized_block_type, MPI_COMM_WORLD, &requests[1]);

    if (rank == 0) {
        end = MPI_Wtime();
        *time = end - start;
    }
    MPI_Wait(&requests[1], MPI_STATUS_IGNORE);
    //Gather the data
    MPI_Igather(outer_transpose, block_size * N, MPI_DOUBLE, T, block_size * N, MPI_DOUBLE, 0, MPI_COMM_WORLD, &requests[2]);
    MPI_Wait(&requests[2], MPI_STATUS_IGNORE);
    // Clean up
    MPI_Type_free(&block_type);
    MPI_Type_free(&resized_block_type);
    free(local_block);
    free(inner_transpose);

    return T;
}

void MPItest(int argc, char** argv) {
    int rank, size;
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    double* A = matGenerateSym();
    double start, end = 0;
    double* T = NULL;
    if(rank == 0){
        // //print the matrix
        // for(int i = 0; i < N; i++){
        //     for(int j = 0; j < N; j++){
        //         printf("%f ", A[i * N + j]);
        //     }
        //     printf("\n");
        // }
        matRandomize(A);
        if(checkSymSeq(A)){
            printf("Matrix is symmetric\n");
        } else {
            printf("Matrix is not symmetric\n");
        }
        start = MPI_Wtime();
        //print the matrix
        // printf("Original matrix\n");
        // for(int i = 0; i < N; i++){
        //     for(int j = 0; j < N; j++){
        //         printf("%f ", A[i * N + j]);
        //     }
        //     printf("\n");
        // }
    }
    double time;
    T = matTransposeMPIv2(rank, size, &time, A, N / size);

    if(rank == 0){
        end = MPI_Wtime();
        //print the transposed matrix
        // printf("Transposed matrix\n");
        // for(int i = 0; i < N; i++){
        //     for(int j = 0; j < N; j++){
        //         printf("%f ", T[i * N + j]);
        //     }
        //     printf("\n");
        // }
        if(isTransposed(A, T)){
            printf("Matrix is transposed correctly\n");
        } else {
            printf("Matrix is transposed incorrectly\n");
        }
        printf("Time taken: %f\n", time);
    }
    free(A);
    free(T);
    MPI_Finalize();
}

