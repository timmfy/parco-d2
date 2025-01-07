#include <stdio.h>
#include <stdlib.h>
#include <seq.h>
#include <mpi_par.h>
int main(int argc, char** argv) {
    //seqTest(argc, argv);
    MPItest(argc, argv);
    return 0;
}