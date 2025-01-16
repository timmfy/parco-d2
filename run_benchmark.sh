#!/bin/bash

nRange=(0 0 0 0 0 0 0 0 0 0 1 0 0) # Default matrix size 1024
pRange=(0 0 1 0 0 0 0) # Default number of processes 4
numRuns=10 #default number of runs
implementation="MPI" #default implementation
checkSym=0 #benchmark the checkSym function (0 by default)
algorithm_version=0 #default algorithm version for MPI transpose

# Function to display help
show_help() {
    echo "Usage: $0 [OPTION]..."
    echo "Options:"
    echo "  --algorithm <int> Set the algorithm version for MPI transpose (default: 0)"
    echo " --checkSym <int>       Benchmark the checkSym function (default: 0, runs the transpose function)"
    echo "  -h --help              Display this information"
    echo " -i <string>             Set the implementation (options MPI, OMP, SEQ; default: MPI)"
    echo " --processes-list <int,int,...> Run the test for the list of threads 2^<int> (default: 2^2, max: 2^6)"
    echo " --size-list <int,int,...> Run the test for the list of sizes 2^<int> (default: 2^10, max: 2^12)"
    echo "  --runs <int>        Set the number of runs (default: 10)"
}

# Parse command line arguments
while [[ $# -gt 0 ]]; do
    case $1 in
        -h|--help)
            show_help
            exit 0
            ;;
        --size-list)
            if [[ -n $2 ]]; then
                nRange=(0 0 0 0 0 0 0 0 0 0 0 0 0)
                for i in $(echo $2 | sed "s/,/ /g"); do
                    if [[ $i -le 0 || $i -gt 12 ]]; then
                        echo "Error: Invalid size"
                        exit 1
                    fi
                    nRange[$i]=1
                done
                shift
            else
                echo "Error: --size-range flag requires an argument"
                exit 1
            fi
            ;;
        --processes-list)
            if [[ -n $2 ]]; then
                pRange=(0 0 0 0 0 0 0)
                for i in $(echo $2 | sed "s/,/ /g"); do
                    if [[ $i -lt 0 || $i -gt 6 ]]; then
                        echo "Error: Invalid number of processes"
                        exit 1
                    fi
                    pRange[$i]=1
                done
                shift
            else
                echo "Error: --processes-list flag requires an argument"
                exit 1
            fi
            ;;
        --runs)
            if [[ -n $2 ]]; then
                numRuns=$2
                shift
            else
                echo "Error: --runs flag requires an argument"
                exit 1
            fi
            ;;
        --checkSym)
            if [[ -n $2 ]]; then
                checkSym=$2
                shift
            else
                echo "Error: --checkSym flag requires an argument"
                exit 1
            fi
            ;;
        -i)
            if [[ -n $2 ]]; then
                implementation=$2
                shift
            else
                echo "Error: -i flag requires an argument"
                exit 1
            fi
            ;;
        --algorithm)
            if [[ -n $2 ]]; then
                algorithm_version=$2
                shift
            else
                echo "Error: --algorithm flag requires an argument"
                exit 1
            fi
            ;;
        *)
            echo "Unknown option: $1"
            exit 1
            ;;
    esac
    shift
done
filename=""
if [[ $checkSym -eq 1 ]]; then
    filename="results_${implementation}_checkSym.csv"
else
    filename="results_${implementation}_transpose.csv"
fi
#change the filename if the MPI implementation for transpose is selected
if [[ $implementation == "MPI" ]] && [[ $checkSym -eq 0 ]]; then
    if [[ $algorithm_version -eq 0 ]]; then
        filename="results_${implementation}_simple_transpose.csv"
    elif [[ $algorithm_version -eq 1 ]]; then
        filename="results_${implementation}_block_point2point_transpose.csv"
    elif [[ $algorithm_version -eq 2 ]]; then
        filename="results_${implementation}_block_all2all_transpose.csv"
    fi
fi
echo "Implementation,Function,Size,Processes,AvgTime" > $filename
numTests=0
for i in "${!nRange[@]}"; do
    if [[ ${nRange[$i]} -eq 1 ]]; then
        for j in "${!pRange[@]}"; do
            if [[ ${pRange[$j]} -eq 1 ]]; then
                processes=$((2**$j))
                numTests=$((numTests+1))
            fi
        done
    fi
done
echo "Running..."
echo "Implementation: $implementation"
echo "Number of runs: $numRuns"
currentTest=0
echo "-------------------------"
for i in "${!nRange[@]}"; do
    if [[ ${nRange[$i]} -eq 1 ]]; then
        N=$((2**$i))
        for j in "${!pRange[@]}"; do
            if [[ ${pRange[$j]} -eq 1 ]]; then
                processes=$((2**$j))
                currentTest=$((currentTest+1))
                make all N=$N > /dev/null
                printf "\rRunning tests with N=$N and $processes processes ($currentTest/$numTests) "
                if [[ $implementation == "SEQ" ]]; then
                    ./bin/main SEQ $numRuns $checkSym | tail -n 1 >> $filename
                elif [[ $implementation == "OMP" ]]; then
                    export OMP_NUM_THREADS=$processes
                    ./bin/main OMP $numRuns | tail -n 1 >> $filename
                elif [[ $implementation == "MPI" ]]; then
                    #Run nly if the number of processes is less or equal to the matrix size
                    if [[ $processes -le $N ]]; then
                        mpirun -np $processes ./bin/main MPI $numRuns $checkSym $algorithm_version | tail -n 1 >> $filename
                    fi
                fi
                make clean > /dev/null
            fi
        done
    fi
done
printf "\r     Tests completed!                             \n"
echo "-------------------------"
if [[ ! -f "results.csv" ]]; then
    echo "Implementation,Function,Size,Processes,AvgTime" > results.csv
fi
tail -n +2 $filename >> results.csv
cat $filename
