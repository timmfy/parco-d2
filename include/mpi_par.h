#ifndef MPI_PAR_H
#define MPI_PAR_H

int cheskSymMPI(int rank, int size, double* time, double* A);
double* matTransposeMPI(int rank, double* time, double* A);
double* matTransposeMPI_block_point2point(int rank, int size, double* time, double* A, int block_size);
double* matTransposeMPI_block_all2all(int rank, double* time, double* A, int block_size);
void MPItest_transpose(int size, int rank, int numRuns, int algorithm);
void MPItest_checkSym(int size, int rank, int numRuns);

#endif