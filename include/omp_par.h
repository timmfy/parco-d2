#ifndef OMP_PAR_H
#define OMP_PAR_H

double* matTransposeOMP(double* M, int blockSize, int numThreads, double* time);
void OMPtest_transpose(int numRuns, int blockSize, int numThreads);

#endif