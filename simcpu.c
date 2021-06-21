#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include "simcpu.h"

/**
 * Student Name: Rehan Nagoor Mohideen
 * Student ID: 110592
 */

/**
 * @return the index of the parent of the current burst
 */
int parent(int i) {
    return (i - 1) / 2;
}

/**
 * @return the index of the left child of the current burst
 */
int leftChild(int i) {
    return ((2 * i) + 1);
}

/**
 * @return the index of the right child of the current burst
 */
int rightChild(int i) {
    return ((2 * i) + 2);
}

/**
 * swap function for minheap structure
 */
void swap(CPUBurst **burstList, int i, int j) {
    CPUBurst *temp = burstList[i];
    burstList[i] = burstList[j];
    burstList[j] = temp;
}
 
/**
 * shiftUp function for minheap structure
 */
void shiftUp(CPUBurst **burstList, int i) {
    while (i > 0 && burstList[parent(i)]->readyTime > burstList[i]->readyTime) {
        swap(burstList, parent(i), i);
        i = parent(i);
    }
}

/**
 * shiftDown function for minheap structure
 */
void shiftDown(CPUBurst **burstList, int size, int i) {
    int maxIndex = i;
    int l = leftChild(i);

    if (l <= size && burstList[l]->readyTime < burstList[maxIndex]->readyTime) {
        maxIndex = l;
    }

    int r = rightChild(i);
    if (r <= size && burstList[r]->readyTime < burstList[maxIndex]->readyTime) {
        maxIndex = r;
    }

    if (i != maxIndex) {
        swap(burstList, i, maxIndex);
        shiftDown(burstList, size, maxIndex);
    }
}

/**
 * insert function for minheap structure.
 * adds the readyTime supplied to the new position and shifts to appropriate position
 */
void insert(CPUBurst **burstList, int *size, int p) {
    *size = *size + 1;
    burstList[*size]->readyTime = p;
    shiftUp(burstList, *size);
}

/**
 * extractMin function for minheap structure
 * @returns the readyTime of the burst and removes the burst from the heap
 */
int extractMin(CPUBurst **burstList, int *size) {
    int result = burstList[0]->readyTime;
    burstList[0] = burstList[*size];
    *size = *size - 1;
    shiftDown(burstList, *size, 0);
    return result;
}

/**
 * change priority function for minheap structure
 */
void changePriority(CPUBurst **burstList, int size, int i, int p) {
    int oldp = burstList[i]->readyTime;
    burstList[i]->readyTime = p;
     
    if (p > oldp) {
        shiftUp(burstList, i);
    } else {
        shiftDown(burstList, size, i);
    }
}
 
/**
 * getMin function for minheap structure
 */
void *getMin (CPUBurst **burstList) {
    return burstList[0];
}

/**
 * Remove function for minheap structure
 */
void Remove(CPUBurst **burstList, int *size, int i) {
    burstList[i] = getMin(burstList);

    shiftUp(burstList, i);

    extractMin(burstList, size);
}

/**
 * frees all the mallocs within a single process
 */
void freeProcess(Process *tempProcess) {
    int i, j;
    for (i = 0; i < tempProcess->noThreads; i++) {
        Thread *tempThread = tempProcess->threadList[i];
        for (j = 0; j < tempThread->noCPUBursts; j++) {
            free(tempThread->cpuBurstList[j]);
        }
        free(tempThread->cpuBurstList);
        free(tempThread);
    }
    free(tempProcess->threadList);
    free(tempProcess);
}

/**
 * @return 1 if all processes in the list are complete
 */
int allProcessesComplete(Process **processList, int noProcesses) {
    for (int i = 0; i < noProcesses; i++) {
        Process *tempProcess = processList[i];
        for (int j = 0; j < tempProcess->noThreads; j++) {
            Thread *tempThread = tempProcess->threadList[j];
            if (tempThread->completed==0) {
                return 0;
            }
        }
    }
    return 1;
}

/**
 * gets the everage turnaround time from the processList and noProcesses
 * @return the value of the average turnaround time for the processes
 */
double getAvgTurnaround(Process **processList, int noProcesses) {
    int totalTurnTime = 0;
    int earliestArrivalTime = 9999;
    int latestEndTime = -1;
    for (int i = 0; i < noProcesses; i++) {
        Process *tempProcess = processList[i];
        for (int j = 0; j < tempProcess->noThreads; j++) {
            Thread *tempThread = tempProcess->threadList[j];

            if (tempThread->arrivalTime < earliestArrivalTime) {//get earliest arrival time
                earliestArrivalTime = tempThread->arrivalTime;
            }

            if (tempThread->executionEndTime > latestEndTime) {//get latest end time
                latestEndTime = tempThread->executionEndTime;
            }
        }
        //calculate total turnaround time for the processes
        totalTurnTime += (latestEndTime - earliestArrivalTime);
    }
    return (double)totalTurnTime/(double)noProcesses;
}

/**
 * prints the detailed stats for each thread
 */
void printDetails(Process **processList, int noProcesses) {
    int serviceTime;
    int threadIOtime;
    for (int i = 0; i < noProcesses; i++) {//for each process
        for (int j = 0; j < processList[i]->noThreads; j++) {//for each thread in that process
            serviceTime = 0;
            threadIOtime = 0;
            Thread *tempThread = processList[i]->threadList[j];
            for (int k = 0; k < tempThread->noCPUBursts; k++) {
                serviceTime += tempThread->cpuBurstList[k]->cpuTime;
                threadIOtime += tempThread->cpuBurstList[k]->ioTime;
            }
            printf("Thread %d of Process %d:\n", tempThread->threadNo, processList[i]->processNo);
            printf("  arrival time: %d\n", tempThread->arrivalTime);
            printf("  service time: %d units, I/O time: %d units,", serviceTime, threadIOtime);
            printf(" turnaround time: %d units, finish time: %d units\n", tempThread->turnaroundTime, tempThread->executionEndTime);
        }
    }
    
}

int main(int argc, char *argv[]) {
    // for (int l = 0; l < argc; l++) {
    //     printf("%s \n", argv[l]);
    // }
    
    // if (argc < 3) {
    //     printf("Usage: ./simcpu [-d] [-v] [-r quantum] < input_file %d\n", argc);
    //     exit(0);
    // }

    int dCheck = 0;
    int vCheck = 0;
    int rCheck = 0;
    int i;

    
    int opt;
    while ((opt = getopt(argc, argv, "dvr:")) != -1) {
        switch (opt) {
        case 'd':
            dCheck = 1;
            break;
        case 'v':
            vCheck = 1;
            break;
        case 'r':
            rCheck = atoi(optarg);
            if (rCheck <= 0) {
                printf("Error: quantum cannot be less than 1\n");
                exit(0);
            }
            
            break;
        case '?':
            printf("unknown option: -%c\n", optopt);
            break;
        }
    }
    //printf("%d\n", rCheck);

    for (i = optind; i < argc; i++) {
        printf("Unkown argument: %s\n", argv[i]);
    }


    char line[100];
    int noProcesses;
    int threadSwitch;
    int processSwitch;
    int processNo;
    int tempProcessNo;
    int noThreads;
    int threadNo;
    int tmpThreadNo;
    int arrivalTime;
    int noBursts;
    int burstNo;
    int totalBursts = 0;
    Process **processList;

    if (fgets(line, 100, stdin)!=NULL) {//gets first line
        if (strlen(line) < 6) {
            printf("Error: Input file is in  invalid format\n");
            exit(0);
        }
        
        sscanf(line, "%d %d %d", &noProcesses, &threadSwitch, &processSwitch);
        processList = malloc(sizeof(Process *)*noProcesses);

        for (processNo = 0; processNo < noProcesses && fgets(line, 100, stdin)!=NULL; processNo++) {//gets processes line
            if (strlen(line) < 4) {
                printf("Error: Input file is in  invalid format\n");
                exit(0);
            }

            sscanf(line, "%d %d", &tempProcessNo, &noThreads);
            Process *tempProcess = malloc(sizeof(Process));
            processList[processNo] = tempProcess;
            tempProcess->noThreads = noThreads;
            tempProcess->processNo = tempProcessNo;
            tempProcess->threadList = malloc(sizeof(Thread*)*noThreads);

            for (threadNo = 0; threadNo < noThreads && fgets(line, 100, stdin)!=NULL; threadNo++) {//gets threads line
                if (strlen(line) < 6) {
                    printf("Error: Input file is in  invalid format\n");
                    exit(0);
                }

                sscanf(line, "%d %d %d", &tmpThreadNo, &arrivalTime, &noBursts);
                Thread *tempThread = malloc(sizeof(Thread));
                tempThread->threadNo = tmpThreadNo;
                tempThread->arrivalTime = arrivalTime;
                tempThread->noCPUBursts = noBursts;
                tempThread->completed = 0;
                tempThread->parentProcess = tempProcess;
                tempThread->cpuBurstList = malloc(sizeof(CPUBurst*)*noBursts);
                tempProcess->threadList[threadNo] = tempThread;
                
                for (burstNo = 0; burstNo < noBursts && fgets(line, 100, stdin)!=NULL; burstNo++) {//gets bursts line
                    if (strlen(line) < 4) {
                        printf("4Error: Input file is in  invalid format\n");
                        exit(0);
                    }

                    char tempLine[100];
                    strcpy(tempLine, line);
                    char *token = strtok(tempLine, " ");
                    totalBursts++;
                    CPUBurst *tempCPUBurst = malloc(sizeof(CPUBurst));
                    tempCPUBurst->burstNo = burstNo+1;
                    tempCPUBurst->cpuTime = atoi(strtok(NULL, " "));
                    tempCPUBurst->remCPUTime = tempCPUBurst->cpuTime;
                    token = strtok(NULL, " ");
                    if (token != NULL) {
                        tempCPUBurst->ioTime = atoi(token);
                    } else {
                        tempCPUBurst->ioTime = 0;
                    }
                    tempCPUBurst->completed = 0;
                    tempCPUBurst->parentThread = tempThread;
                    tempThread->cpuBurstList[burstNo] = tempCPUBurst;
                }
            }
        }
    } else {
        printf("Error: Could not access file\n");
        exit(0);
    }

    CPUBurst **readyQueue = malloc(sizeof(CPUBurst *)* totalBursts);
    int queueSize = -1;
    int time;
    int running = 0;
    int totalCStime = 0;
    double CPUUtil = 0;
    double avgTurnaroundTime = 0;
    CPUBurst *currentBurst;
    CPUBurst *prevBurst = NULL;

    //for each second until all threads are complete.
    for (time = 0; allProcessesComplete(processList, noProcesses) == 0; time++) {
        // if (time == 500) {
        //     exit(0);
        // }

        //add the first burst of thread to the ready queue by checking if thread arrives/becomes ready
        for (int j = 0; j < noProcesses; j++) {//for each process
            for (int k = 0; k < processList[j]->noThreads; k++) {//for each thread
                if (processList[j]->threadList[k]->arrivalTime == time) {//if it arrives
                    readyQueue[queueSize+1] = processList[j]->threadList[k]->cpuBurstList[0];
                    insert(readyQueue, &queueSize, time);

                    if (vCheck == 1) {
                        printf("At time %4d: Thread %d of Process %d moves from new to ready\n", time, k+1, j+1);
                    }
                }
            }
        }

        //check if anything is running if yes
        if (running == 1) {
            //if round robin selected and it is at quantum and is not completed
            if ((currentBurst->cpuEnterTime + currentBurst->remCPUTime) != time && rCheck != 0 && (currentBurst->cpuEnterTime + rCheck) == time) {
                
                extractMin(readyQueue, &queueSize);//remove the burst from the ready queue
                //remove the quantum from the time to get the remaining time
                currentBurst->remCPUTime -= rCheck;
                //reinsert it at the back of the queue
                readyQueue[queueSize+1] = currentBurst;
                insert(readyQueue, &queueSize, time);
                running = 0;

                //add appropriate context switch
                if (queueSize != -1) {
                    CPUBurst *nextBurst = getMin(readyQueue);
                    int switchTime;
                    //if switching to different process change process switch time
                    if (((Process *)((Thread *)nextBurst->parentThread)->parentProcess)->processNo != ((Process *)((Thread *)currentBurst->parentThread)->parentProcess)->processNo) {
                        switchTime = processSwitch;

                        if (vCheck == 1) {
                            printf("At time %4d: Thread %d of Process %d moves from running to blocked\n", time, ((Thread *)currentBurst->parentThread)->threadNo, ((Process *)((Thread *)currentBurst->parentThread)->parentProcess)->processNo);
                        }
                    } else if (((Thread *)nextBurst->parentThread)->threadNo != ((Thread *)currentBurst->parentThread)->threadNo){//if it switches to a different thread in the same process
                        switchTime = threadSwitch;

                        if (vCheck == 1) {
                            printf("At time %4d: Thread %d of Process %d moves from running to blocked\n", time, ((Thread *)currentBurst->parentThread)->threadNo, ((Process *)((Thread *)currentBurst->parentThread)->parentProcess)->processNo);
                        }
                    } else {//it is in the same process and thread
                        switchTime = 0;
                        if (vCheck == 1) {
                            printf("At time %4d: Thread %d of Process %d moves from running to blocked\n", time, ((Thread *)currentBurst->parentThread)->threadNo, ((Process *)((Thread *)currentBurst->parentThread)->parentProcess)->processNo);
                        }
                    }
                    
                    totalCStime +=switchTime;
                    //for the duration of the switch only check for new arriving threads if context switch is not 0
                    int ogtime = time;
                    if (switchTime != 0) {
                        for (time = time+1; time <(ogtime+switchTime); time++) {
                            for (int j = 0; j < noProcesses; j++) {
                                for (int k = 0; k < processList[j]->noThreads; k++) {
                                    if (processList[j]->threadList[k]->arrivalTime == time) {
                                        readyQueue[queueSize+1] = processList[j]->threadList[k]->cpuBurstList[0];
                                        insert(readyQueue, &queueSize, time);

                                        if (vCheck == 1) {
                                            printf("At time %4d: Thread %d of Process %d moves from new to ready\n", time, k+1, j+1);
                                        }
                                    }
                                }
                            }
                        }
                    }
                }

                //check if it finishes
            } else if ((currentBurst->cpuEnterTime + currentBurst->remCPUTime) == time) {
                extractMin(readyQueue, &queueSize);//remove the burst from the ready queue
                currentBurst->completed = 1;
                //to start next burst from the queue
                running = 0;
                //check if thread is completed
                if (currentBurst->burstNo == ((Thread *)currentBurst->parentThread)->noCPUBursts) {
                    ((Thread *)currentBurst->parentThread)->completed = 1;
                    ((Thread *)currentBurst->parentThread)->executionEndTime = time;
                    ((Thread *)currentBurst->parentThread)->turnaroundTime = time - ((Thread *)currentBurst->parentThread)->arrivalTime;

                    if (vCheck == 1) {
                        printf("At time %4d: Thread %d of Process %d moves from running to terminated\n", time+currentBurst->ioTime, ((Thread *)currentBurst->parentThread)->threadNo, ((Process *)((Thread *)currentBurst->parentThread)->parentProcess)->processNo);
                    }
                } else {//else add next burst in the thread to the ready queue
                    readyQueue[queueSize+1] = ((Thread *)currentBurst->parentThread)->cpuBurstList[currentBurst->burstNo];
                    insert(readyQueue, &queueSize, time+currentBurst->ioTime);
                }

                //add appropriate context switch
                if (queueSize != -1) {
                    CPUBurst *nextBurst = getMin(readyQueue);
                    int switchTime;
                    //if switching to different process change process switch time
                    if (((Process *)((Thread *)nextBurst->parentThread)->parentProcess)->processNo != ((Process *)((Thread *)currentBurst->parentThread)->parentProcess)->processNo) {
                        switchTime = processSwitch;

                        if (vCheck == 1 && ((Thread *)currentBurst->parentThread)->completed == 0) {
                            printf("At time %4d: Thread %d of Process %d moves from running to blocked\n", time, ((Thread *)currentBurst->parentThread)->threadNo, ((Process *)((Thread *)currentBurst->parentThread)->parentProcess)->processNo);
                        }
                    } else if (((Thread *)nextBurst->parentThread)->threadNo != ((Thread *)currentBurst->parentThread)->threadNo){//if it switches to a different thread in the same process
                        switchTime = threadSwitch;

                        if (vCheck == 1 && ((Thread *)currentBurst->parentThread)->completed == 0) {
                            printf("At time %4d: Thread %d of Process %d moves from running to blocked\n", time, ((Thread *)currentBurst->parentThread)->threadNo, ((Process *)((Thread *)currentBurst->parentThread)->parentProcess)->processNo);
                        }
                    } else {//it is in the same process and thread
                        switchTime = 0;
                        if (vCheck == 1 && ((Thread *)currentBurst->parentThread)->completed == 0) {
                            printf("At time %4d: Thread %d of Process %d moves from running to blocked\n", time, ((Thread *)currentBurst->parentThread)->threadNo, ((Process *)((Thread *)currentBurst->parentThread)->parentProcess)->processNo);
                        }
                    }

                    totalCStime +=switchTime;
                    //for the duration of the switch only check for new arriving threads if context switch is not 0
                    int ogtime = time;
                    if (switchTime != 0) {
                        for (time = time+1; time <(ogtime+switchTime); time++) {
                            for (int j = 0; j < noProcesses; j++) {
                                for (int k = 0; k < processList[j]->noThreads; k++) {
                                    if (processList[j]->threadList[k]->arrivalTime == time) {
                                        readyQueue[queueSize+1] = processList[j]->threadList[k]->cpuBurstList[0];
                                        insert(readyQueue, &queueSize, time);

                                        if (vCheck == 1) {
                                            printf("At time %4d: Thread %d of Process %d moves from new to ready\n", time, k+1, j+1);
                                        }
                                    }
                                }
                            }
                        }
                    }
                }

            }
        }
 
        //start next burst from queue
        if (running == 0 && queueSize != -1) {
            currentBurst = getMin(readyQueue);
            
            if (currentBurst->readyTime <= time) {//if the readytime is now
                currentBurst->cpuEnterTime = time;
                running = 1;

                //if its the first a new thread starts, start the start time for the thread
                if (currentBurst->burstNo == 1) {
                    ((Thread *)currentBurst->parentThread)->executionStartTime = time;
                }

                // if (vCheck == 1 && time == 0) {
                //     printf("At time %4d: Thread %d of Process %d moves from ready to running\n", time, ((Thread *)currentBurst->parentThread)->threadNo, ((Process *)((Thread *)currentBurst->parentThread)->parentProcess)->processNo);
                // }
                if (vCheck == 1 ) {
                    if (currentBurst->burstNo != 1 || (currentBurst->burstNo ==1 && currentBurst->cpuTime != currentBurst->remCPUTime)) {
                        printf("At time %4d: Thread %d of Process %d moves from blocked to running\n", time, ((Thread *)currentBurst->parentThread)->threadNo, ((Process *)((Thread *)currentBurst->parentThread)->parentProcess)->processNo);
                    } else {
                        printf("At time %4d: Thread %d of Process %d moves from ready to running\n", time, ((Thread *)currentBurst->parentThread)->threadNo, ((Process *)((Thread *)currentBurst->parentThread)->parentProcess)->processNo);
                    }
                }
                // printf("\n");
                // for (int ls = 0; ls <= queueSize; ls++) {
                //     printf("run%d: time: %d, P%d T%d B%d\n", ls, time, ((Process *)((Thread *)readyQueue[ls]->parentThread)->parentProcess)->processNo, ((Thread *)readyQueue[ls]->parentThread)->threadNo, readyQueue[ls]->burstNo);
                // }
                // printf("\n");
            }
            prevBurst = getMin(readyQueue);
            if (running == 0) {
                totalCStime++;
            }
            
        }

    }
    free(readyQueue);
    time --;

    //process and print outputs
    avgTurnaroundTime = getAvgTurnaround(processList, noProcesses);
    CPUUtil = ((double)time-(double)totalCStime)*100/(double)time;
    if (rCheck == 0) {//print info for FCFS
        printf("\nFCFS Scheduling\n");
    } else {
        printf("\nRound Robin Scheduling (quantum = %d units)\n", rCheck);
    }
    printf("Total Time required is %d units\nAverage Turnaround Time is %.1lf units\n", time , avgTurnaroundTime);// fix thisssss
    printf("CPU Utilization is %.1lf%%\n", CPUUtil);
    if (dCheck == 1 || vCheck == 1) {
        printDetails(processList, noProcesses);
    }

    //free all elements
    for (i = 0; i < noProcesses; i++) {
        freeProcess(processList[i]);
    }
    free(processList);
    
    return 0;
}
