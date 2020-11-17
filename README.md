# CPU-Scheduling-Algorithm-Simulation
## 1.Introduction
### 1.1 Target
This project is to the simulation of an operating system. There are four algorithms: First Come First Serve(FCFS), Shortest Job Frist(SJF), Shortest Remaining Time (SRT), and Round Robin (RR). All four of these algorithms will be simulated for the same set of processes, which will allow for comparative analysis. This means when the program runs, all four algorithms are simulated. 
### 1.2 Process
A process is defined as a program in execution. For this assignment, processes are in one of the following three states:<br>
• READY: in the ready queue, ready to use the CPU<br>
• RUNNING: actively using the CPU<br>
• BLOCKED: blocked on I/O
### 1.3 Running Rule
In general, when a process reaches the front of the queue and the CPU is free to accept the next process, the given process enters the RUNNING state and starts executing its CPU burst. <br>
After the CPU burst is completed, if the process does not terminate, the process enters the BLOCKED state, waiting for an I/O operation to complete (e.g., waiting for data to be read in from a file). When the I/O operation completes, depending on the scheduling algorithm, the process either (1) returns to the READY state and is added to the ready queue or (2) preempts the currently running process and switches into the RUNNING state.
## 2. Algorithm And Design
### 2.1 Detailed Algorithms
#### 2.1.1 First Come First Serve(FCFS)
The FCFS algorithm is a non-preemptive algorithm in which processes line up in the ready queue, waiting to use the CPU. This is the baseline algorithm.
#### 2.1.2 Shortest Job First(SJF)
In SJF, processes are stored in the ready queue in order of priority based on their CPU burst times. More specifically, the process with the shortest CPU burst time will be selected as the next process executed by the CPU.
#### 2.1.3 Shortest Remaining Time(SRF)
The SRT algorithm is a preemptive version of the SJF algorithm. In SRT, when a process arrives, before it enters the ready queue, if it has a CPU burst time that is less than the remaining time of the currently running process, a preemption occurs. When such a preemption occurs, the currently running process is added back to the ready queue.
#### 2.1.4 Round Robin(RR)
The RR algorithm is essentially the FCFS algorithm with predefined time slice. Each process is given time slice amount of time to complete its CPU burst. If this time slice expires, the process is preempted and added to the end of the ready queue. If a process completes its CPU burst before a time slice expiration, the next process on the ready queue is immediately context-switched into the CPU.
#### 2.2 Handling "Ties"
For events that occur at the same time, use the following order to break the "tie": (a) CPU burst completion; (b) I/O burst completion (i.e., back to the ready queue); and then (c) new process arrival. <br>
All "ties" that occur within one of these three categories are to be broken using process ID order. As an example, if processes Q and T happen to both finish with their I/O at the same time, process Q wins this "tie" (because Q is alphabetically before T) and is added to the ready queue before process T.
#### 2.3 Statistical Information
#### 2.3.1 CPU Burst Time
CPU burst times are randomly generated for each process. CPU burst time is defined as the amount of time a process is actually using the CPU. Therefore, this measure does not include context switch times.
#### 2.3.2 Turnaround Time
Turnaround times are to be measured for each process that you simulate. Turnaround time is defined as the end-to-end time a process spends in executing a single CPU burst.
More specifically, this is measured from process arrival time through to when the CPU burst is completed and the process is switched out of the CPU. Therefore, this measure includes the second half of the initial context switch in and the first half of the final context switch out, as well as any other context switches that occur while the CPU burst is being completed (i.e., due to preemptions).
#### 2.3.3 Wait Time
Wait times are to be measured for each process. Wait time is defined as the amount of time a process spends waiting to use the CPU, which equates to the amount of time
the given process is actually in the ready queue. Therefore, this measure does not include context switch times that the given process experiences (i.e., only measure the time the given process is actually in the ready queue). More specifically, a process leaves the ready queue when it is switched into the CPU, which takes half of context switch time. Likewise, a preempted process leaves the CPU and enters the ready queue after the first half of switch time.
#### 2.4 "Interesting" Events
Aussme total running time is from 0 through 999, the project will catch the "interesting" events, they are: <br>
• Start of simulation  <br>
• Process arrival <br>
• Process starts using the CPU <br>
• Process finishes using the CPU (i.e., completes a CPU burst) <br>
• Process has its value recalculated (i.e., after a CPU burst completion) <br>
• Process preemption <br>
• Process starts performing I/O <br>
• Process finishes performing I/O <br>
• Process terminates by finishing its last CPU burst <br>
• End of simulation <br>
The "process arrival" event occurs every time a process arrives, i.e., based on the initial arrival time and when a process completes I/O. In other words, processes "arrive" within the subsystem that consists of the CPU and the ready queue. <br>
The "process preemption" event occurs every time a process is preempted by a time slice expiration (in RR) or by an arriving process (in SRT). When a preemption occurs, a context switch occurs (unless for RR there are no available processes in the ready queue). <br>
## 3.File
The only file is main.cpp, which contains seven arguments: <br>
First: Seed to generate pesudo-random numbers  <br>
Second: Parameter to generate exponential distribution <br>
Third: Bound of the exponential distribution, any generated values above this should be skipped <br>
Fourth: Number of processes will be simulated <br>
Fifth: Assumed context switch time, it should be a positive even integer <br>
Sixth: Coefficient for the CPU to predict the running time for SJF and SRF <br>
Seventh: Time slice for RR <br>
Eighth: For RR algorithm, define whether processes are added to the end or the beginning of the ready queue when they arrive or complete I/O. This optional command-line
argument, is set to either "BEGINNING" or "END", with "END" being the default behavior. <br>


