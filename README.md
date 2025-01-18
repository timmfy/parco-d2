# Parallelizing matrix operations using MPI

The project implements parallel matrix transpose algorithms using MPI and compares them to the sequential and OpenMP implementations

## Toolchain used

- `MPICH 3.2.1` (module `mpich-3.2.1--gcc-9.1.0` on the HPC Cluster)
- `GCC 9.1.0` (module `gcc91` on the HPC Cluster)
- `GNU Make 3.82`
- PBS job scheduler (for cluster execution)
- Python 3.10.12 with `matplotlib` v3.8.2, `numpy` v1.26.3 and `pandas` v2.2.3 (for plotting)

## Local Compiling/Executing Instructions
Clone or download the repository and navigate to the project directory
```sh
git clone https://github.com/timmfy/parco-d2
```
### Running using scripts
The section below describes how to run the code using the .sh scripts either locally or on the cluster using the interactive session.
Before running, make sure to enable the scripts using the following command
```sh
chmod +x *.sh
```
#### Running all the benchmarks
The following command will run the benchmarks:
 - Sequential transpose
 - Sequential matrix symmetry check function
 - OpenMP transpose
 - MPI transpose using `sendrecv()`
 - MPI transpose with block-based distribution and point-to-point communication
 - MPI transpose with block-based distribution and collective (all to all) communication
 - MPI matrix symmetry check function
The benchmarks are run for the square `NxN` matrices where N is 32, 64, 128, 256, 512, 1024, 2048, 4096 and for the number of processes 2, 4, 8, 16, 32, 64 (for the case of MPI and OpenMP benchmarks)
The parameter `<numRuns>` specifies the number of times each benchmark is run (recommended values are <30 in order to avoid long execution times)
```sh
./run_all_benchmarks.sh <numRuns>
```
The results will be stored in the `results.csv` file.
The results for each benchmark are also stored in separate `.csv` files
#### Running a single benchmark
In order to run a single benchmark(one of the above), use the following command:
```sh
./run_benchmark.sh --algorithm <int> --checkSym <int> -i <string> --processes-list <int,int,...> --size-list <int,int,...> --runs <int>
```
The parameters are as follows:
- `--algorithm` specifies the MPI transpose implementation to run. May be omitted when running other benchmarks. The possible values are:
  - `0` - Simple algorithm using `sendrecv()` (default)
  - `1` - Block-based distribution with point-to-point communication
  - `2` - Block-based distribution with collective communication
- `--checkSym` specifies whether to run the matrix symmetry check function. The possible values are:
    - `0` - Run the transpose function (default)
    - `1` - Run the symmetry check
- `i` specifies the implementation to run. The possible values are:
  - `SEQ` - Sequential
  - `OMP` - OpenMP
  - `MPI` - MPI (default)
- `--processes-list` specifies the list of the number of processes to run the multiprocessor benchmarks for. Each argument `n` will set the number of processes to `2^n`. So `--processes-list 2,4,6` will run the benchmark for 4, 16 and 64 processes. (default is `2` to run for 4 processes)
- `--size-list` specifies the list of matrix sizes to run the benchmarks for. Each argument `n` will set the matrix size to `2^n`. So `--size-list 5,6,7` will run the benchmark for the matrix sizes 32, 64 and 128. (default is `10` to run for 1024x1024 matrices)
- `--runs` specifies the number of times to run the benchmark (default is `10`)
Examples:
This will run the MPI transpose benchmark using the block-based distribution with point-to-point communication for the matrix sizes 256, 512, 1024, and 2048 using 32 processes. The benchmark will be run 20 times
```sh
./run_benchmark.sh --algorithm 1 -i MPI --processes-list 5 --size-list 8,9,10,11 --runs 20
```
This will run the OMP transpose benchmark with the matrix size 4096 using 16 and 32 processes. The benchmark will be run 30 times
```sh
./run_benchmark.sh -i OMP --processes-list 4,5 --size-list 12 --runs 30
```
This will run the MPI checkSym benchmark for the matrix sizes 32, 64, 128, 256, 512, 1024, 2048, 4096 using 2, 4, 8, 16, 32, 64 processes. No number of runs is indicated so the benchmark will be run 10 times (as by default)
```sh
./run_benchmark.sh --checkSym 1 -i MPI --processes-list 1,2,3,4,5,6 --size-list 5,6,7,8,9,10,11,12
```
### Manual Compilation and Running
#### Compilation
It is recommended to compile the code using the provided Makefile. The following command will compile the code:
```sh
make N=<size>
```
where `<size>` is the size of the matrix to be used in the benchmarks (should be a power of 2).
Without the makefile, the following sequence of commands can be used to compile the code:
```sh
mpicc -DN=<matrix_size> -fopenmp -Iinclude -c src/main.c -o main.o
mpicc -DN=<matrix_size> -fopenmp -Iinclude -c src/mpi_par.c -o mpi_par.o
mpicc -DN=<matrix_size> -fopenmp -Iinclude -c src/omp_par.c -o omp_par.o
mpicc -DN=<matrix_size> -fopenmp -Iinclude -c src/seq.c -o seq.o
mpicc -fopenmp main.o mpi_par.o omp_par.o seq.o -o main
```
#### Running
If compiled manually:
- To run the sequential benchmark, use the following command:
```sh
./main SEQ <numRuns> <checkSym>
```
- To run the OpenMP benchmark, use the following command:
```sh
./main OMP <numRuns>
```
- To run the MPI transpose benchmark using the `sendrecv()` function, use the following command:
```sh
mpirun -np <numProcesses> ./main MPI <numRuns> <checkSym> <algorithm>
```
where `<numProcesses>` is the number of processes to run the benchmark for (should be a power of 2), `<numRuns>` is the number of times to run the benchmark, `<checkSym>` specifies whether to run the matrix symmetry check function and `<algorithm>` specifies the MPI transpose implementation to run (specified above) (default is `0`).
If compiled the code with the Makefile, use `./bin/main` instead of `./main`
## Cluster Compilation and Running
1. Using the interactive session:
    - Connect to the cluster using SSH
    - Request an interactive session using the following command:
    ```sh
    qsub -I -q short_cpuQ -l select=1:ncpus=64:mpiprocs=64:mem=1gb
    ```
    - Load the required modules using the following commands:
    ```sh
    module load gcc91
    module load mpich-3.2.1--gcc-9.1.0
    ```
    - Clone or download the repository and navigate to the project directory
    - Compile the code using the provided Makefile or manually
    - Run the benchmarks using the provided scripts or manually
2. Using the batch job:
    - Connect to the cluster using SSH
    - Clone or download the repository and navigate to the project directory
    - Modify the file `parco-d2.pbs` to specify the project directory path and if needed, the commands for compilation and running (described above) and the walltime. By default, it runs
    ```sh
    ./run_all_benchmarks.sh 10
    ```
    - Submit the job using the following command:
    ```sh
    qsub parco-d2.pbs
    ```
    - The results will be stored in the `results.csv` file (if running muliple benchmarks using a script)
    - Standard output and errors will be stored in the `parco-d2.out` and `parco-d2.err` files respectively
## (Optional) Plotting the results
The results can be plotted using the provided Python script `plot.py`. It requires the `matplotlib`, `numpy` and `pandas` libraries to be installed. The script reads the `results.csv` file and plots the execution times and strong/weak scaling for the different benchmarks. The script can be run using the following command:
```sh
python3 plot.py
```
Customization of the plots can be done by modifying the `main()` function in the script and changing the parameters of the following functions:
- `plot_strong_scaling(dataframe, implementation, matrix_size, function)`: Plots the strong scaling for the specified implementation(`MPI simple`, `MPI block all2all` or `MPI block point2point` for the transpose benchmark, while the checkSym benchmark has only `MPI` implementation), matrix size and function(`transpose` or `checkSym`)
- `plot_execution_time_variable_size(dataframe, processes_list, implementation, min_size, max_size, function)`: Plots the execution time for the specified implementation, various processes, various matrix sizes (from `min_size` to `max_size`) and function(`transpose` or `checkSym`)
- `plot_weak_scaling(dataframe, implementation, base_matrix_size, function)`: Plots the weak scaling for the specified implementation, base matrix size (used to set the 1-process baseline) and a function
- `plot_execution_time_fixed_size(dataframe, matrix_size, implementations, processes_list)`: Used only for the `transpose` benchmark in order to compare the execution times for different implementations and processes for a fixed matrix size.
The plots require the `results.csv` file to be present in the project directory. Otherwise, the sample data `sample_data.csv` can be used.
The plots will be saved in the `figures` directory. 
