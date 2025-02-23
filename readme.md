# Robot Cleaning Simulator

## Overview
This project is a **multi-threaded robot cleaning simulator**, developed as part of an advanced programming course. It features **two distinct cleaning algorithms** and a simulation environment that evaluates their performance based on predefined house layouts.

## Teaser - Algorithm B Visualization:

![Algorithm B Visualization](https://github.com/user-attachments/assets/72f4e994-b8d8-4f7c-be33-c9343df4d8f0)




## Key Features
- **Multi-threading:** Runs multiple simulations in parallel to improve efficiency.
- **Modular architecture:** Uses shared libraries for different cleaning algorithms.
- **Error handling:** Logs errors in a dedicated directory.
- **Performance evaluation:** Generates a CSV report summarizing each algorithm’s effectiveness.
- **Simulation visualization:** Simulates and tracks the robot’s movements using input and output files.
- **Custom scoring system:** Evaluates algorithm efficiency based on cleaning success and optimization.

## Cleaning Algorithms
### **Algorithm A (DFS Movement)**
- Explores the house using **Depth-First Search (DFS)**.
- Maps the environment while moving.
- Returns to the docking station using **Breadth-First Search (BFS)**.
- Completes cleaning when all reachable spots are clean within a certain battery range.

### **Algorithm B (Spiral Movement)**
- Moves in a spiral pattern, preferring rightward motion.
- When blocked, finds the closest unvisited or dirty spot using BFS.
- Calculates the shortest return path to the docking station.
- Completes cleaning similarly to Algorithm A.

## Expected Input & Output
### **Input:**
- `.house` files representing different environments for cleaning simulation.
- `.so` files containing dynamically linked algorithm implementations.

### **Output:**
- A text file per simulation run (`<HouseName>-<AlgorithmName>.txt`) with the following data:
  - `NumSteps = <NUMBER>` – Number of steps taken.
  - `DirtLeft = <NUMBER>` – Remaining dirt in the house.
  - `Status = <FINISHED/WORKING/DEAD>` – Final status of the robot.
  - `InDock = <TRUE/FALSE>` – Whether the robot ended at the docking station.
  - `Score = <NUMBER>` – Performance score (lower is better).
  - `Steps: <list of NESWsF>` – Movement sequence.
- A `summary.csv` file summarizing algorithm performance across multiple runs.

## How It Works
1. **Load shared libraries** containing the cleaning algorithms.
2. **Load house files** that represent different cleaning environments.
3. **Run simulations** on multiple algorithm-house pairs in parallel.
4. **Track performance** including steps taken, battery consumption, and remaining dirt.
5. **Generate reports** summarizing results in `summary.csv`.

## How to Run
### **Build the Project**
```bash
chmod +x ./build_all.sh
./build_all.sh
```
Or manually build each component:
```bash
cd ./Algorithm_A; mkdir build; cd build; cmake ..; make; cd ../../;
cd ./Algorithm_B; mkdir build; cd build; cmake ..; make; cd ../../;
cd ./Simulator; mkdir build; cd build; cmake ..; make; cd ../../;
```

### **Execute the Simulator**
```bash
./Simulator/build/myrobot
```
#### **Optional Flags:**
- `-house_path <dir>`: Directory containing `.house` files (default: current directory).
- `-algo_path <dir>`: Directory containing algorithm `.so` files (default: current directory).
- `-num_thread <N>`: Maximum number of threads to use (default: 10).
- `-summary_only`: Generate only a summary CSV (default: false).
- `-log`: Enable logging for each algorithm-house pair (default: false).

### **Run a Simulation**
```bash
python3 ./Simulation.py myHouse.house myOutput.txt
```

Algorithm A:

![Algorithm A Visualization](https://github.com/user-attachments/assets/d7f21669-663c-4bd5-81ee-172a76af8fbe)



