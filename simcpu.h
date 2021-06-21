#ifndef SIMCPU_h
#define SIMCPU_H

typedef struct {
    int burstNo;
    int cpuTime;
    int remCPUTime;
    int ioTime;
    int readyTime;
    int cpuEnterTime;
    int completed;
    void *parentThread;//holds parent thread pointer
}CPUBurst;

typedef struct {
    int threadNo;
    int arrivalTime;
    int noCPUBursts;
    int executionStartTime;
    int executionEndTime;
    int turnaroundTime;
    int completed;
    void *parentProcess;//holds pointer to parent process
    CPUBurst **cpuBurstList;
}Thread;

typedef struct {
    int processNo;
    int noThreads;
    Thread **threadList;
}Process;

#endif