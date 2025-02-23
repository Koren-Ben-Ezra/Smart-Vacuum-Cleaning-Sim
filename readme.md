# Smart Vacuum Cleaning Simulator

## Overview
**Multi-threaded simulation framework** for an autonomous vacuum cleaner, designed as part of an advanced programming course. The system efficiently cleans dynamically defined environments using **modular, optimized algorithms**. The architecture supports multiple algorithms, parallel execution, and a competitive scoring mechanism.

## Teaser - Spiral Algorithm Visualization

![Algorithm B Visualization](https://github.com/user-attachments/assets/72f4e994-b8d8-4f7c-be33-c9343df4d8f0)

## Features
- **Multi-Threaded Simulation**: Runs multiple cleaning algorithms concurrently for performance evaluation.
- **Modular Algorithm Design**: Supports dynamically loaded algorithms via shared libraries (.so files).
- **Scalable House Representation**: Handles various house layouts with different sizes, dirt levels, and obstacles.
- **Pathfinding Optimization**: Implements **DFS-based exploration** and **BFS-based shortest path calculations**.
- **Performance Scoring**: Evaluates algorithms using an optimized scoring function (based on efficiency, coverage, and docking success).
- **Error Handling & Logging**: Robust error reporting with separate logs for invalid houses and algorithms.
- **Automated Testing & Benchmarking**: Runs all possible algorithm-house combinations and generates detailed performance summaries.

## System Architecture
- **Simulator**: Loads house files and algorithm shared libraries, manages simulation runs, and records results.
- **Algorithms**:
  - **DFS Algorithm**: Uses **Depth-First Search** for exploration, mapping the environment dynamically.
  - **Spiral Algorithm**: Prioritizes structured movement for optimized coverage while minimizing redundant steps.
- **Input**:
  - **House File (.house)**: Defines the environment layout, including walls (W), open spaces ( ), dirt levels (0-9), and a single docking station (D).
  - **Algorithm Files (.so)**: Shared object libraries containing cleaning strategies that get dynamically loaded.
- **Output**
  - **Simulation Output**: Logs vacuum steps, remaining dirt, status (FINISHED, WORKING, DEAD), and performance score.
  - **Summary Report (.csv)**: Aggregates results across multiple runs for benchmarking.

## Usage
### Running the Simulator
```bash
./myrobot -house_path=houses/ -algo_path=algorithms/ -num_threads=10 -summary_only
```
### Optional Flags

- `-house_path=<directory>`: Specifies the directory containing `.house` files. Defaults to the current directory if not provided.
- `-algo_path=<directory>`: Specifies the directory containing algorithm `.so` files. Defaults to the current directory if not provided.
- `-num_threads=<number>`: Limits the number of concurrent threads. Defaults to 10.
- `-summary_only`: Generates only the `summary.csv` file, skipping individual simulation logs.
- `-log`: Enables logging for each algorithm-house pair, saving detailed execution logs.


## Input & Output
### Input
- **House File (.house)**: Defines the environment layout, including walls (W), open spaces ( ), dirt levels (0-9), and a single docking station (D).
- **Algorithm Files (.so)**: Shared object libraries containing cleaning strategies that get dynamically loaded.

### Output
- **Simulation Log**: Steps taken, dirt remaining, vacuum status (FINISHED/WORKING/DEAD), docking status (TRUE/FALSE).
- **Summary Report (.csv)**: Performance metrics for multiple algorithm-house combinations.

### Additional Results - DFS Algorithm Visualization
![DFS Algorithm Visualization](https://github.com/user-attachments/assets/d7f21669-663c-4bd5-81ee-172a76af8fbe)



