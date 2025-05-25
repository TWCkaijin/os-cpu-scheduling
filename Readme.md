# CPU scheduler

## 1. compilation

- First time setup

```bash
git submodule add https://github.com/pybind/pybind11
git submodule update --init --recursive
cmake .
pip install matplotlib numpy pybind11
```

```bash
make # First, build the cpp code in the project
python main.py # run the project with python controler
```

## 2. explaination of the results

The result are displayed in the new window, with the following information:

- **Title**: The title of the each simulation's method name.
- **Gantt chart**: The Gantt chart shows the execution order of processes over time.
- **Average waiting time**: The average time that processes spend waiting in the ready queue before they are executed.
- **Average turnaround time**: The average time taken for a process to complete its execution, including waiting time.
- **Average context switch time**: The average time taken for context switching between processes.

## 3. Simulation methods

- **Preemptive SJF (Shortest Job First)**: The process with the shortest burst time is executed next.
- **Round-Robin**: The CPU is allocated to each process for a fixed time slice in a cyclic order.
- **MultiLevel Queue Scheduling**: The processes are divided into multiple queues based on their priority, and each queue has its own scheduling algorithm.
- **Custom Scheduling**: A self-defined scheduling algorithm. With it quite similar to MLQS, but with a different priority assignment mechanism ---> the execute order is determined not only by priority but also by the total burst time of the queue in each level of priority.
  - Queue 1: Highest priority, uses RoundRobinwith time quantom=5.
  - Queue 2: Medium priority, uses Round-Robin with time quantom=10.
  - Queue 3: Lowest priority, uses Preemptive SJF.

## 4. Program structure

The program is structured as follows:

- **main.py**: The main entry point of the program, which initializes the simulation and handles user input. Also, it illustrates the results Gantt chart in a new window.

- **main.cpp** : The main C++ code that implements the CPU scheduling algorithms. It contains the process queue create si,mulation and the caller logic for each scheduling algorithm.

- **custom.h**: The header file that defines the custom scheduling algorithm and its associated functions. Also contains the simulation of process queue push-in behavior.

- **MLQS.h**: Contains the implementation of the MultiLevel Queue Scheduling algorithm. Also contains the simulation of process queue push-in behavior.

- **RRNP.h**: Contains the implementation of the Round-Robin Non-Preemptive scheduling algorithm. Also contains the simulation of process queue push-in behavior.

- **SJF.h**: Contains the implementation of the Preemptive Shortest Job First scheduling algorithm. Also contains the simulation of process queue push-in behavior.

- **type.h**: Defines the main data type of the program.
