# Multithreaded Word Puzzle Solver

This C-based multithreaded application efficiently solves large-scale word search puzzles using dictionary-based matching. It leverages custom data structures and parallel computation to search for words across a 2D grid, simulating real-world text processing and search problems.

## Features

- Solves word search puzzles using:
  - Custom **hash set** for fast dictionary lookups
  - Optional **binary search tree** for alternative search modes
- Multithreaded implementation to:
  - Distribute puzzle regions across threads
  - Perform concurrent word matching for speed and scalability
- Supports large input files and puzzle boards
- Command-line interface for configuration

## How It Works

1. Loads a dictionary of valid words from a text file.
2. Loads a word search puzzle (character grid).
3. Spawns multiple threads to scan the puzzle in parallel.
4. Each thread checks for valid words in 8 directions.
5. Outputs matched words and locations.

## Example Usage

```bash
make
./solver dict.txt puzzle.txt
