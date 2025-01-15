#include <stdio.h>
#include <stdlib.h>
#include <seq.h>
#include <mpi_par.h>
#include <omp_par.h>
#include <mpi.h>
#include <omp.h>
int main(int argc, char** argv) {
    int rank, size;
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    if (rank == 0) {
        seqTest_transpose(atoi(argv[1]));
    }
    MPItest_transpose(rank, size, atoi(argv[1]), atoi(argv[2]));
    if (rank == 0) {
        OMPtest_transpose(atoi(argv[1]), 64, omp_get_max_threads());
    }
    if (rank == 0) {
        seqTest_checkSym(atoi(argv[1]));
    }
    MPItest_checkSym(rank, size, atoi(argv[1]));
    MPI_Finalize();
    return 0;
}