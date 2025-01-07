#ifndef MPI_PAR_H
#define MPI_PAR_H

//int checkSymMPI(double* A);
double* matTransposeMPI(int rank, double* A);
void MPItest(int argc, char** argv);

#endif