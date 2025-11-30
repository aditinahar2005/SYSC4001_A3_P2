# SYSC4001 Assignment 3 Part 2  
This repository includes my work for Part A, Part B, and Part C of the assignment.  
Everything was tested on Linux (WSL Ubuntu 24.04).

---

## 1. Folder Structure

```
SYSC4001_A3P2/
│
├── PartA/
│   ├── ta.c
│   ├── shared.h
│   ├── rubric.txt
│   └── exams/
│
├── PartB/
│   ├── ta_sem.c
│   ├── shared.h
│   ├── rubric.txt
│   └── exams/
│
├── PartC/
│   └── reportPartC.pdf
│
└── README.md
```

Each part has its own rubric and exams folder so the programs do not interfere with each other.

---

## 2. How to Compile and Run

All commands must be run in Linux.

---

### 2.1 Part A

Go into the folder:
```
cd PartA
```

Compile:
```
gcc ta.c -o ta
```

Run with any number of TAs:
```
./ta 3
```

The program will load `rubric.txt` and the exam files in the `exams` folder.  
It stops when student 9999 is found.

---

### 2.2 Part B (Semaphore Version)

Go into the folder:
```
cd PartB
```

Compile:
```
gcc ta_sem.c -o ta_sem -pthread -lrt
```

Run:
```
./ta_sem 3
```

Part B uses semaphores to control access to shared memory.

---

## 3. Design Discussion

This assignment relates to the three requirements of the critical section problem.  
Below is how Part B meets each requirement.

### 3.1 Mutual Exclusion  
Semaphores protect the rubric, exam loading, and marking array.  
Only one TA can enter these sections at a time, so no data corruption happens.

### 3.2 Progress  
If no TA is inside a critical section, another TA can enter right away.  
No TA is blocked without reason.

### 3.3 Bounded Waiting  
Every TA eventually gets its turn.  
The semaphore system prevents one TA from waiting forever.

---

## 4. Test Cases

Programs were tested with different TA counts:

```
./ta 2
./ta 3
./ta 4
```

and the same tests for the semaphore version:

```
./ta_sem 2
./ta_sem 3
```

All runs processed exams from 0001 to 0020 and ended on exam 9999.

---

## 5. Part C

The file `reportPartC.pdf` in the PartC folder contains my answers about deadlock, livelock, and execution order.

---

## 6. Notes

- Exam files must be saved as `0001.txt`, `0002.txt`, and so on, with a final `9999.txt` file to stop the program.
- All testing was done inside WSL on Ubuntu 24.04.
- Keep PartA and PartB separate so the files do not conflict.

