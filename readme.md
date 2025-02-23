# Robot Cleaning Simulator

## Overview
This project is a **multi-threaded robot cleaning simulator**, developed as part of an advanced programming course. It features **two distinct cleaning algorithms** and a simulation environment that evaluates their performance based on predefined house layouts.

## Key Features
- **Multi-threading:** Runs multiple simulations in parallel to improve efficiency.
- **Modular architecture:** Uses shared libraries for different cleaning algorithms.
- **Error handling:** Logs errors in a dedicated directory.
- **Performance evaluation:** Generates a CSV report summarizing each algorithm’s effectiveness.
- **Simulation visualization:** Simulates and tracks the robot’s movements using input and output files.

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

## Tech Stack
- **C++** (Core simulator and algorithms)
- **Python** (Simulation visualization and execution script)
- **CMake** (Build system)
- **Pygame** (For visualization, optional)

## Contact
For more details, feel free to reach out!

