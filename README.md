# CPU Scheduling Simulator

This C-based simulator models the behavior of core CPU scheduling algorithms used in modern operating systems. It evaluates algorithm performance using randomized workloads, tracks turnaround times, and compares scheduling efficiency across multiple strategies.

## Features

- Implements four classical scheduling algorithms:
  - **FCFS** (First Come First Serve)
  - **SJF** (Shortest Job First)
  - **SRT** (Shortest Remaining Time)
  - **Priority with Round Robin**
- Simulates realistic process workloads using:
  - Gaussian-distributed CPU burst times
  - Randomized arrival intervals
  - Configurable time quantum for Round Robin
- Tracks and reports:
  - Turnaround Time
  - Process Completion Order
  - Comparative scheduling performance
- Includes:
  - Custom linked-list queue implementation
  - Heap-based priority queues for efficient dispatching

## How It Works

The program accepts command-line arguments to define simulation parameters:

```bash
./simulate -n <num_processes> -k <arrival_interval> -d <mean_cpu_time> -v <stddev_cpu_time> -q <time_quantum>

-n: Number of processes to simulate

-k: Range of arrival times (0 to k)

-d: Mean CPU burst time

-v: Standard deviation of CPU burst time

-q: Time quantum for round robin

Example:
./simulate -n 100 -k 1000 -d 50 -v 12 -q 60

Example Output:
FCFS    ATT: 182.43
SJF     ATT: 141.22
SRT     ATT: 135.97
PRI_RR  ATT: 147.88
