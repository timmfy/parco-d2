#ifndef MPI_PAR_H
#define MPI_PAR_H

//int checkSymMPI(double* A);
double* matTransposeMPIv1(int rank, double* A);
double* matTransposeMPIv2(int rank, double* A, int local_size);
void MPItest(int argc, char** argv);

#endif