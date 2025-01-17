#!/bin/bash

if [ "$#" -ne 1 ]; then
    echo "Usage: $0 <numRuns>"
    exit 1
fi
numRuns=$1
echo "Running Sequential Transpose"
#Run Sequential Transpose
./run_benchmark.sh --processes-list 0 --size-list 5,6,7,8,9,10,11,12 -i SEQ --runs $numRuns >> /dev/null

echo "Running Sequential checkSym"
#Run Sequential checkSym
./run_benchmark.sh --processes-list 0 --size-list 5,6,7,8,9,10,11,12 -i SEQ --runs $numRuns --checkSym 1 >> /dev/null

echo "Running OpenMP Transpose"
#Run OpenMP Transpose
./run_benchmark.sh --processes-list 1,2,3,4,5,6 --size-list 5,6,7,8,9,10,11,12 -i OMP --runs $numRuns >> /dev/null

echo "Running MPI Transpose with simple algorithm"
#Run MPI Transpose with simple algorithms
./run_benchmark.sh --processes-list 1,2,3,4,5,6 --size-list 5,6,7,8,9,10,11,12 -i MPI --algorithm 0 --runs $numRuns >> /dev/null

echo "Running MPI Transpose with blocking algorithm with point to point communication"
#Run MPI Transpose with blocking algorithm with point to point communication
./run_benchmark.sh --processes-list 1,2,3,4,5,6 --size-list 5,6,7,8,9,10,11,12 -i MPI --algorithm 1 --runs $numRuns >> /dev/null

echo "Running MPI Transpose with blocking algorithm with all to all communication"
#Run MPI Transpose with blocking algorithm with all to all communication
./run_benchmark.sh --processes-list 1,2,3,4,5,6 --size-list 5,6,7,8,9,10,11,12 -i MPI --algorithm 2 --runs $numRuns >> /dev/null

echo "Running MPI checkSym"
#Run MPI checkSym
./run_benchmark.sh --processes-list 1,2,3,4,5,6 --size-list 5,6,7,8,9,10,11,12 -i MPI --checkSym 1 --runs $numRuns >> /dev/null
