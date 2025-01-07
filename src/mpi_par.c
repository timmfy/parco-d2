#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include <seq.h>

//int checkSymMPI(double* A){}

/*
    marTransposeMPI works in already initialized MPI environment
*/
double* matTransposeMPI(int rank, double* A){
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
void MPItest(int argc, char** argv){
    int rank, size;
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    double* A = matGenerateSym();
    double start, end = 0;
    
    if(rank == 0){
        matRandomize(A);
        if(checkSymSeq(A)){
            printf("Matrix is symmetric\n");
        } else {
            printf("Matrix is not symmetric\n");
        }
        start = MPI_Wtime();
    }

    double* T = matTransposeMPI(rank, A);

    if(rank == 0){
        end = MPI_Wtime();
        if(isTransposed(A, T)){
            printf("Matrix is transposed correctly\n");
        } else {
            printf("Matrix is transposed incorrectly\n");
        }
        printf("Time taken: %f\n", end - start);
    }
    free(A);
    free(T);
    MPI_Finalize();
}

