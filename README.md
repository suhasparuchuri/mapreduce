# MapReduce 
## Project Overview
This project implements a MapReduce framework, designed for parallel and scalable processing of large datasets. The framework splits input data, processes it concurrently, and aggregates results efficiently.

### **Tasks Implemented**
1. **Letter Counter**: Counts the frequency of each letter in the input data.
   - Input: Text file.
   - Output: Letter frequencies.

2. **Word Finder**: Finds lines containing a specific word in the input data.
   - Input: Text file and target word.
   - Output: Lines containing the target word.

---

## Key Features
- Parallel Processing via forked child processes for mapping tasks.
- Intermediate Data Storage using temporary partition files (namely `partion-*.itm`).
- Large Input Support by dividing data into configurable splits.
- Robust Error Handling for file I/O, memory allocation, and process failures.

---
## **Compilation**
To build the project, use the provided `Makefile`. Run the following command:
```bash
make
```
For Counter task
```
./run-mapreduce <mode> <input_file> <split_num>
```
For Finder task
```
./run-mapreduce <mode> <input_file> <split_num> [word_to_find]
```
Arguments
```
mode           # Task to execute: "counter" for letter counting or "finder" for word search
input_file     # Path to a regular input text file
split_num      # Number of chunks to split the input file into for parallel map processes
word_to_find   # (Only for "finder" mode) Word to search for in the input file
```
Examples
```
./run-mapreduce "counter" input-warpeace.txt 8
./run-mapreduce "finder" input-warpeace.txt 8 "Revolution"
```



