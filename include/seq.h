#ifndef SEQ_H
#define SEQ_H

double* matGenerate();
double* matGenerateSym();
void matRandomize(double* M);
int checkSymSeq(double* A);
double* matTranspose(double* M);
int isTransposed(double* A, double* B);
void seqTest(int argc, char** argv);

#endif