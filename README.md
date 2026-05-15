# S.T.C.E. Engine (Spatial-Time Complexity Expansion)

A high-performance, multi-threaded C++ engine designed to solve large-scale instances of the Traveling Salesman Problem (TSP). 

This benchmark demonstrates an advanced hybrid heuristic approach, capable of outperforming standard constructive algorithms (like Nearest Neighbor) in both execution time and path optimality on massive datasets.

## Core Architecture

The S.T.C.E. Engine abandons linear logic in favor of a spatially-aware pipeline:

1. **Boustrophedon 2D Sort:** Prevents the "comb effect" on grid-like maps by organizing nodes into dynamic vertical columns based on the map's aspect ratio.
2. **Triangle Clustering & Median Split:** Groups nodes into triangular structures and divides the map into completely independent memory hemispheres.
3. **Multi-Threading:** Executes the initial greedy expansion in parallel across hemispheres, ensuring zero race conditions.
4. **KD-Tree Accelerated 2-Opt:** The final surgical optimization phase. By implementing a custom KD-Tree spatial radar, the 2-Opt complexity is drastically reduced from $O(n^2)$ to $O(n \log n)$, allowing massive local search windows in milliseconds.

## Benchmark Results

Tested on an Intel CPU against a standard Nearest Neighbor implementation using the `pla85900.tsp` dataset (85,900 nodes representing a Printed Circuit Board).

| Algorithm | Distance | Execution Time |
| :--- | :--- | :--- |
| **Nearest Neighbor** | 163,385,000 | ~46,500 ms |
| **S.T.C.E. Engine V1.4** | **158,349,000** | **~13,500 ms** |

## Usage
Compile the source code using standard `g++` with the `-O3` optimization flag. The engine accepts custom random maps for simulation or standard `.tsp` coordinate files.

```bash
g++ src/main.cpp src/geometria.cpp src/motore_stce.cpp src/parser.cpp -O3 -std=c++14 -o stce_engine.exe
./stce_engine.exe
