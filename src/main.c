#include <stdio.h>
#include <stdlib.h>
#include <seq.h>
#include <mpi_par.h>
#include <omp_par.h>
#include <mpi.h>
#include <omp.h>
#include <string.h>

#define min(a, b) ((a) < (b) ? (a) : (b))

int main(int argc, char** argv) {

    //parameter handling
    if (argc < 3) {
        return 1;
    }

    char* implementation = argv[1];
    int numRuns, checkSym, transpose_algorithm;

    if (strcmp(implementation, "OMP") == 0) {
        numRuns = atoi(argv[2]);
    }
    else if (strcmp(implementation, "SEQ") == 0) {
        if (argc < 4) {
            return 1;
        }
        numRuns = atoi(argv[2]);
        checkSym = atoi(argv[3]);
    }
    else if (strcmp(implementation, "MPI") == 0) {
        if (argc < 4) {
            return 1;
        }
        numRuns = atoi(argv[2]);
        checkSym = atoi(argv[3]);
        if ((checkSym == 0) && (argc < 5)) {
            return 1;
        }
        else if (checkSym == 0) {
            transpose_algorithm = atoi(argv[4]);
        }
    }
    else {
        return 1;
    }
    //Running the tests
    if (strcmp(implementation, "SEQ") == 0) {
        if (checkSym == 1) {
            seqTest_checkSym(numRuns);
        }
        else {
            seqTest_transpose(numRuns);
        }
    }
    else if (strcmp(implementation, "OMP") == 0) {
        OMPtest_transpose(numRuns, min(32, N), omp_get_max_threads());
    }
    else {
        int rank, size;
        MPI_Init(&argc, &argv);
        MPI_Comm_rank(MPI_COMM_WORLD, &rank);
        MPI_Comm_size(MPI_COMM_WORLD, &size);
        if (checkSym == 1) {
            MPItest_checkSym(rank, size, numRuns);
        }
        else {
            MPItest_transpose(rank, size, numRuns, transpose_algorithm);
        }
        MPI_Finalize();
    }
    return 0;
}