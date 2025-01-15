#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mpi.h>
#include <seq.h>
int cheskSymMPI(int rank, int size, double* time, double* A) {
    double start, end;
    int local_symm, symm;
    if (rank == 0) {
        start = MPI_Wtime();
    }
    
    MPI_Datatype tmp, column_type;
    MPI_Type_vector(N, 1, N, MPI_DOUBLE, &tmp);
    MPI_Type_create_resized(tmp, 0, sizeof(double), &column_type);
    MPI_Type_commit(&column_type);

    double* local_rows = (double*)malloc(N/size * N * sizeof(double));
    double* local_columns = (double*)malloc(N/size * N * sizeof(double));

    // Scatter rows and columns
    MPI_Scatter(A, N/size * N, MPI_DOUBLE, local_rows, N/size * N, MPI_DOUBLE, 0, MPI_COMM_WORLD);
    MPI_Scatter(A, N/size, column_type, local_columns, N/size * N, MPI_DOUBLE, 0, MPI_COMM_WORLD);

    local_symm = 1;
    for (int i = 0; i < N/size; i++) {
        for (int j = 0; j < N; j++) {
            if (local_rows[i * N + j] != local_columns[i * N + j]) {
                local_symm = 0;
                break;
            }
        }
    }

    MPI_Reduce(&local_symm, &symm, 1, MPI_INT, MPI_LAND, 0, MPI_COMM_WORLD);

    if (rank == 0) {
        end = MPI_Wtime();
        *time = end - start;
    }

    MPI_Type_free(&column_type);
    MPI_Type_free(&tmp);
    free(local_rows);
    free(local_columns);
    
    return symm;
}
double* matTransposeMPI(int rank, double* time, double* A) {
    double* T = (double*) malloc(N * N * sizeof(double));
    MPI_Datatype row, transpose;
    MPI_Status status;
    
    MPI_Type_vector(N, 1, N, MPI_DOUBLE, &row);
    
    MPI_Type_create_hvector(N, 1, sizeof(double), row, &transpose);
    
    MPI_Type_commit(&transpose);
    double start, end;

    if (rank == 0) {
       start = MPI_Wtime();
    }

    MPI_Sendrecv(A, 1, transpose, rank, 0, T, N * N, MPI_DOUBLE, rank, 0, MPI_COMM_WORLD, &status);

    if (rank == 0) {
       end = MPI_Wtime();
       *time = end - start;
    }
        
    MPI_Type_free(&transpose);
    MPI_Type_free(&row);
    
    return T;
}

double* matTransposeMPI_block_point2point(int rank, int size, double* time, double* A, int block_size) {
    double* T = NULL;
    double* local_block = (double*) malloc(block_size * N * sizeof(double));
    double* inner_transpose = (double*) malloc(block_size * N * sizeof(double));
    double start, end;
    MPI_Datatype block_type;
    MPI_Type_vector(block_size, block_size, N, MPI_DOUBLE, &block_type);
    MPI_Type_commit(&block_type);
    int displacements[size * size];

    for (int i = 0; i < size; i++) {
        for (int j = 0; j < size; j++) {
            displacements[i * size + j] = (j * block_size * N + i * block_size);
        }
    }

    if (rank == 0) {
        T = (double*) malloc(N * N * sizeof(double));
    }

    if (rank == 0) {
        start = MPI_Wtime();
    }

    // Scatter the matrix
    MPI_Scatter(A, block_size * N, MPI_DOUBLE, local_block, block_size * N, MPI_DOUBLE, 0, MPI_COMM_WORLD);


    //Transpose the local block
    for (int block_index = 0; block_index < N / block_size; block_index++) {
        for (int i = 0; i < block_size; i++) {
            for (int j = 0; j < block_size; j++) {
                inner_transpose[j * N + i + block_index * block_size] = local_block[i * N + j + block_index * block_size];
            }
        }
    }

    if (rank == 0) {
        for (int b = 0; b < size; b++) {
            MPI_Sendrecv(inner_transpose + b * block_size, 1, block_type, 0, b, T + displacements[b], 1, block_type, 0, b, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
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
        end = MPI_Wtime();
        *time = end - start; 
    }
    // Clean up
    MPI_Type_free(&block_type);
    free(local_block);
    free(inner_transpose);

    return T;
}

double* matTransposeMPI_block_all2all(int rank, double* time, double* A, int block_size) {
    double* T = NULL;
    double* local_block = (double*) malloc(block_size * N * sizeof(double));
    double* inner_transpose = (double*) malloc(block_size * N * sizeof(double));
    double* outer_transpose = (double*) malloc(block_size * N * sizeof(double));
    double start, end;

    if (rank == 0) {
        T = (double*) malloc(N * N * sizeof(double));
        start = MPI_Wtime();
    }

    // Scatter the matrix
    MPI_Scatter(A, block_size * N, MPI_DOUBLE, local_block, block_size * N, MPI_DOUBLE, 0, MPI_COMM_WORLD);
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

    MPI_Alltoall(inner_transpose, 1, resized_block_type, outer_transpose, 1, resized_block_type, MPI_COMM_WORLD);

    if (rank == 0) {
        end = MPI_Wtime();
        *time = end - start;
    }
    MPI_Gather(outer_transpose, block_size * N, MPI_DOUBLE, T, block_size * N, MPI_DOUBLE, 0, MPI_COMM_WORLD);

    // Clean up
    MPI_Type_free(&block_type);
    MPI_Type_free(&resized_block_type);
    free(local_block);
    free(inner_transpose);
    return T;
}

void MPItest_transpose(int rank, int size, int numRuns, int algorithm) {
    double* A = (malloc) (N * N * sizeof(double));
    double total_time = 0;

    for (int i = 0; i < numRuns; i++) {
        double* T = NULL;
        if(rank == 0){
            matRandomize(A);
        }

        double time;
        
        if (algorithm == 0) {
            T = matTransposeMPI(rank, &time, A);
        }
        else if (algorithm == 1) {
            T = matTransposeMPI_block_point2point(rank, size, &time, A, N / size);
        }
        else {
            T = matTransposeMPI_block_all2all(rank, &time, A, N / size);
        }

        if(rank == 0){
            if (!(isTransposed(A, T))) {
                fprintf(stderr, "Error: Transpose is not correct\n");
                MPI_Abort(MPI_COMM_WORLD, 1);
            }
            total_time += time;
        }
        free(T);
    }
    if(rank == 0){
        if (algorithm == 0) {
            printf("MPI simple,transpose,%d,%d,%.9f\n", N, size, total_time / numRuns);
        }
        else if (algorithm == 1) {
            printf("MPI block point2point,transpose,%d,%d,%.9f\n", N, size, total_time / numRuns);
        }
        else {
            printf("MPI block all2all,transpose,%d,%d,%.9f\n", N, size, total_time / numRuns);
        }
    }
    free(A);
}

void MPItest_checkSym(int rank, int size, int numRuns) {
    double total_time = 0;
    double* A = NULL;

    for (int i = 0; i < numRuns; i++) {
        double time;
        if(rank == 0){
            A = matGenerateSym();
        }
        if (!cheskSymMPI(rank, size, &time, A)) {
            fprintf(stderr, "Error: Matrix is not symmetric\n");
            MPI_Abort(MPI_COMM_WORLD, 1);
        }
        if(rank == 0){
            total_time += time;
        }
    }
    if(rank == 0){
        printf("MPI, checkSym,%d,%d,%.9f\n", N, size, total_time / numRuns);
        free(A);
    }
}

