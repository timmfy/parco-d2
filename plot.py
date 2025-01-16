import pandas as pd
import matplotlib.pyplot as plt
import numpy as np

# Set IEEE formatting for plots
plt.rcParams.update({
    'font.size': 10,
    'lines.linewidth': 1.0,
    'axes.labelsize': 10,
    'axes.titlesize': 10,
    'legend.fontsize': 8,
    'xtick.labelsize': 8,
    'ytick.labelsize': 8,
    'figure.dpi': 300,
    'figure.figsize': (6, 4)
})

def parse_csv(file_path):
    """
    Parses the CSV file and returns a DataFrame.
    """
    df = pd.read_csv(file_path)
    return df

def plot_strong_scaling(df, implementation, matrix_size):
    """
    Plots strong scaling (S_s), weak scaling (S_w), and efficiency (Eff)
    for a specified implementation and matrix size.
    """
    # Filter data for the specified implementation and matrix size
    data = df[(df['Implementation'] == implementation) &
              (df['Size'] == matrix_size) &
              (df['Function'] == 'transpose')]

    baseline_data = df[(df['Implementation'] == 'SEQ')]
    # Baseline execution time with 1 process
    baseline_time = baseline_data['AvgTime'][baseline_data['Size'] == matrix_size].values[0]
    # Calculate strong scaling speedup (S_s) and efficiency (Eff)
    data['Strong Scaling Speedup'] = baseline_time / data['AvgTime']
    data['Efficiency'] = data['Strong Scaling Speedup'] / data['Processes']

    # Plotting
    fig, ax1 = plt.subplots()

    ax1.set_xlabel('Processes')
    ax1.set_ylabel('Speedup', color='tab:blue')
    ax1.plot(data['Processes'], data['Strong Scaling Speedup'], color='tab:blue', label='Strong Scaling Speedup')
    ax1.tick_params(axis='y', labelcolor='tab:blue')
    ax1.set_title(f'Strong Scaling and Efficiency ({implementation}, Matrix Size={matrix_size})')

    # Set x-axis to logarithmic scale and define ticks
    ax1.set_xscale('log')
    ax1.set_xticks([2**i for i in range(1, int(np.log2(data['Processes'].max())))])
    ax1.tick_params(axis='x', which='minor', bottom=False, labelbottom=False)
    ax1.get_xaxis().set_major_formatter(plt.ScalarFormatter(useMathText=True))
    ax1.get_xaxis().set_minor_formatter(plt.NullFormatter())
    ax1.set_xlim([int(data['Processes'].min()), int(data['Processes'].max())])
    ax1.get_xaxis().set_major_formatter(plt.ScalarFormatter())

    ax2 = ax1.twinx()
    ax2.set_ylabel('Efficiency', color='tab:red')
    ax2.plot(data['Processes'], data['Efficiency'], color='tab:red', label='Efficiency')
    ax2.tick_params(axis='y', labelcolor='tab:red')

    # Combine legends
    lines, labels = ax1.get_legend_handles_labels()
    lines2, labels2 = ax2.get_legend_handles_labels()
    ax1.legend(lines + lines2, labels + labels2, loc='best')

    plt.savefig(f'figures/strong_scaling_{implementation}_{matrix_size}.png')

def plot_execution_time_fixed_size(df, matrix_size, implementations=['OMP', 'MPI simple', 'MPI block all2all', 'MPI block point2point'], processes_list=[2,4,8,16,32,64]):
    """
    Plots execution time for a fixed matrix size across all implementations
    with the number of processes on the X axis, including the sequential baseline.
    """
    fig, ax = plt.subplots()
    
    # Create local copies to avoid modifying input parameters
    impl_list = implementations.copy()
    proc_list = processes_list.copy()
    
    # Plot sequential baseline first
    seq_data = df[(df['Implementation'] == 'SEQ') &
                  (df['Size'] == matrix_size) &
                  (df['Function'] == 'transpose')]
    if not seq_data.empty:
        exec_time = seq_data['AvgTime'].values[0]
        ax.hlines(exec_time, xmin=min(proc_list), xmax=max(proc_list),
                 colors='gray', linestyles='dashed', label='SEQ Baseline')
    
    # Plot parallel implementations
    for impl in impl_list:
        data = df[(df['Implementation'] == impl) &
                  (df['Size'] == matrix_size) &
                  (df['Function'] == 'transpose') &
                  (df['Processes'].isin(proc_list))]
        if not data.empty:
            ax.plot(data['Processes'], data['AvgTime'], label=impl)
    
    ax.set_xscale('log')
    ax.set_xticks([2**i for i in range(1, int(np.log2(max(proc_list))) + 1)])
    ax.tick_params(axis='x', which='minor', bottom=False)
    ax.get_xaxis().set_major_formatter(plt.ScalarFormatter(useMathText=True))
    ax.get_xaxis().set_minor_formatter(plt.NullFormatter())
    ax.set_xlim([min(proc_list), max(proc_list)])
    ax.set_xlabel('Processes')
    ax.set_ylabel('Average Execution Time (s)')
    ax.set_title(f'Execution Time vs Number of Processes (Matrix Size={matrix_size})')
    ax.legend()
    
    plt.savefig(f'figures/execution_time_fixed_size_{matrix_size}.png')
    plt.close()

def plot_execution_time_variable_size(df, processes_list, implementation, min_size, max_size):
    """
    Plots execution time for variable matrix sizes on the X axis, having lines for
    different numbers of processes of one implementation plus the sequential baseline.
    """
    matrix_sizes = sorted(df['Size'].unique())
    fig, ax = plt.subplots()

    # Plot for different numbers of processes
    for num_procs in processes_list:
        data = df[(df['Implementation'] == implementation) &
                  (df['Processes'] == num_procs) &
                  (df['Function'] == 'transpose') &
                  (df['Size'].isin(matrix_sizes))]

        ax.plot(data['Size'], data['AvgTime'], label=f'{num_procs} Processes')
    # Plot sequential baseline
    seq_data = df[(df['Implementation'] == 'SEQ') &
                  (df['Function'] == 'transpose') &
                  (df['Size'].isin(matrix_sizes))]

    ax.plot(seq_data['Size'], seq_data['AvgTime'], label='SEQ Baseline', linestyle='dashed', color='gray')

    ax.set_xscale('log')
    if min_size < data['Size'].min():
        min_size = data['Size'].min()
    if max_size > data['Size'].max():
        max_size = data['Size'].max()
    ax.set_xticks([2**i for i in range(int(np.log2(min_size)), int(np.log2(max_size)) + 1)], minor=False)
    ax.tick_params(axis='x', which='minor', bottom=False, labelbottom=False)
    ax.get_xaxis().set_major_formatter(plt.ScalarFormatter(useMathText=True))
    ax.get_xaxis().set_minor_formatter(plt.NullFormatter())
    ax.set_xlim([min_size, max_size])
    ax.set_xlabel('Size')

    ax.set_ylabel('Average Execution Time (s)')
    ax.set_title(f'Execution Time vs Matrix Size ({implementation} Implementation)')
    ax.legend()
    plt.savefig(f'figures/execution_time_variable_size_{implementation}.png')

def plot_execution_time_comparison_mpi(df, matrix_size, mpi_implementations):
    """
    Plots execution time for the same matrix size, comparing two MPI implementations with 
    different algorithms. X axis has the number of processes.
    """
    fig, ax = plt.subplots()

    for mpi_impl in mpi_implementations:
        data = df[(df['Implementation'] == mpi_impl) &
                  (df['Size'] == matrix_size) &
                  (df['Function'] == 'transpose')]

        ax.plot(data['Processes'], data['AvgTime'], label=mpi_impl)

    ax.set_xscale('log')
    ax.set_xticks([2**i for i in range(1, int(np.log2(data['Processes'].max())) + 1)], minor=False)
    ax.tick_params(axis='x', which='minor', bottom=False)
    ax.set_xlim([int(data['Processes'].min()), int(data['Processes'].max())])
    ax.get_xaxis().set_major_formatter(plt.ScalarFormatter())
    ax.set_xlabel('Processes')
    ax.set_ylabel('Average Execution Time (s)')
    ax.set_title(f'Execution Time Comparison of MPI Implementations, Matrix Size={matrix_size}')
    ax.legend()
    plt.savefig(f'figures/execution_time_comparison_mpi_{matrix_size}.png')

def plot_weak_scaling(df, implementation, base_matrix_size):
    """
    Plots weak scaling for a specified implementation and base matrix size.
    Weak scaling compares performance as both problem size and processors scale.
    
    Args:
        df: DataFrame with benchmark results
        implementation: Implementation name to analyze
        base_matrix_size: Starting matrix size for 1 process
    """
    # Filter data for the specified implementation and transpose function
    data = df[(df['Implementation'] == implementation) &
              (df['Function'] == 'transpose')]
    
    # Get baseline sequential time for base size
    baseline_data = df[(df['Implementation'] == 'SEQ') & 
                      (df['Size'] == base_matrix_size)]
    baseline_time = baseline_data['AvgTime'].values[0]
    
    # Calculate weak scaling efficiency
    weak_scaling = []
    processes = []
    sizes = []
    size = base_matrix_size * 2
    p = 2

    while p <= data['Processes'].max():
        case = data[(data['Processes'] == p) & (data['Size'] == size)]
        if not case.empty:
            weak_scaling.append(baseline_time / case['AvgTime'].values[0])
            sizes.append(size)
            processes.append(p)
        size *= 2
        p *= 2
    
    # Create the plot
    plt.figure(figsize=(10, 6))
    plt.xscale('log')
    
    # Sort processes and sizes for consistency
    sizes.sort()
    processes.sort()

    # Generate pairs of (size, processes) for x-tick labels
    pairs = list(zip(processes, sizes))
    plt.xticks(processes, [f'({s}, {p})' for s, p in pairs])
    
    # Plot weak scaling efficiency against the number of processes
    plt.plot(processes, weak_scaling, label=implementation)
    # Plot the baseline efficiency line at 1 for base_matrix_size and 1 process
    plt.hlines(1, processes[0], processes[-1], colors='gray', linestyles='dashed', label=f'SEQ Baseline ({base_matrix_size}x{base_matrix_size})')
    # Remove minor ticks
    ax = plt.gca()
    ax.xaxis.set_minor_locator(plt.NullLocator())
    ax.xaxis.set_minor_formatter(plt.NullFormatter())

    plt.xlim([processes[0], processes[-1]])

    plt.xlabel('Number of Processes (Size, Processes)')
    plt.ylabel('Weak Scaling Efficiency')
    plt.title(f'Weak Scaling - {implementation} (Base Matrix Size: {base_matrix_size})')
    plt.legend()
    
    # Save plot
    plt.savefig(f'figures/weak_scaling_{implementation}_{base_matrix_size}.png')
    plt.close()





def main():
    # Example usage:
    file_path = 'results.csv'
    df = parse_csv(file_path)
    # Plot strong scaling and efficiency for MPI implementation and matrix size 1024
    plot_strong_scaling(df, implementation='MPI simple', matrix_size=1024)

    # Plot execution time for fixed matrix size 1024 across all implementations
    plot_execution_time_fixed_size(df, matrix_size=1024)

    # Plot execution time for variable matrix sizes with different numbers of processes for MPI simple
    processes_list = [4, 8, 16, 32]
    implementations = ['MPI simple']
    plot_execution_time_variable_size(df, processes_list=processes_list, implementation='MPI simple', min_size=512, max_size=4096)

    # Plot weak scaling for MPI implementation and base matrix size 1024
    plot_weak_scaling(df, implementation='MPI simple', base_matrix_size=128)

if __name__ == "__main__":
    main()
