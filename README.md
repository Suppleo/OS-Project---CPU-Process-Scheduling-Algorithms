# Process Scheduling Simulator

This project is a C++ program that simulates various process scheduling algorithms commonly used in operating systems. It was developed as part of the Introduction to Operating Systems course.

## Features

- Simulates four different scheduling algorithms:
  1. First-Come, First-Served (FCFS)
  2. Round Robin (RR)
  3. Shortest Job First (SJF)
  4. Shortest Remaining Time Next (SRTN)
- Reads process information from an input file
- Generates a Gantt chart for both CPU and resource (R) scheduling
- Outputs the scheduling results to a file

## Requirements

- C++ compiler (C++11 or later)
- Input and output files should be in text format (\*.txt)

## Usage

Compile the program using your C++ compiler. Then run the executable from the command line with the following parameters:

```
./scheduler <input_file> <output_file> <algorithm> [quantum]
```

Where:

- `<input_file>`: Path to the input file containing process information
- `<output_file>`: Path to the output file where the Gantt chart will be saved
- `<algorithm>`: An integer from 1 to 4 representing the scheduling algorithm:
  1. FCFS
  2. RR (default quantum = 2)
  3. SJF
  4. SRTN
- `[quantum]`: (Optional) User-defined quantum value for the RR algorithm

## Input File Format

The input file should follow this format:

- The first line specifies the number of processes to be scheduled.
- Each subsequent line describes a process with the following information:
  ```
  <Arrival Time> [<CPU Burst Time> <Resource Usage Time>]*
  ```
  Where the pattern `[<CPU Burst Time> <Resource Usage Time>]` can repeat multiple times for each process.

Example:

```
3
0 5 3 4
1 4
2 3 3
```

## Output File Format

The output file contains two lines:

1. A sequence of integers representing the CPU scheduling Gantt chart
2. A sequence of integers representing the resource R scheduling Gantt chart

Numbers 1, 2, 3, and 4 denote processes P1, P2, P3, and P4, respectively. A hyphen (\_) indicates an idle time slot.

Example:

```
1 1 1 1 1 2 2 2 2 3 3 3 1 1 1 1
_ _ _ _ _ 1 1 1 _ _ _ _ 3 3 3 _
```

## Notes

- Each process can use CPU and R multiple times.
- The number of CPU and R usages for each process can vary.
- The system only has one resource R scheduled by the FCFS algorithm.
- In case of conflict in the entrance of the ready queue, new processes will be prioritized.

From Suppleo with <3
