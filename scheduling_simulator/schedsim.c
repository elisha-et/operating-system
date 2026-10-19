// C program for implementation of Simulation 
#include <stdio.h> 
#include <limits.h>
#include <stdlib.h>
#include <stdbool.h>
#include "process.h"
#include "util.h"

// Function to find the waiting time for all processes (Round Robin)
void findWaitingTimeRR(ProcessType plist[], int n, int quantum) 
{ 
    int rem_bt[n];
    for (int i = 0; i < n; i++) {
        rem_bt[i] = plist[i].bt;
        plist[i].wt = 0;
    }

    int t = 0; // Current time
    bool done = false;

    // Keep traversing processes in round-robin fashion
    while (!done) {
        done = true;
        for (int i = 0; i < n; i++) {
            if (rem_bt[i] > 0) {
                done = false; // There is a pending process
                
                // If remaining burst time is greater than quantum, process runs for a quantum
                if (rem_bt[i] > quantum) {
                    t += quantum;
                    rem_bt[i] -= quantum;
                } else {
                    // Last cycle for this process
                    t += rem_bt[i];
                    plist[i].wt = t - plist[i].bt - plist[i].art;
                    if (plist[i].wt < 0) plist[i].wt = 0;
                    rem_bt[i] = 0; // This process is over
                }
            }
        }
    }
} 

// Function to find the waiting time for all processes
// NOTE: This implements preemptive SJF, strictly known as SRTF (Shortest Remaining Time First)
void findWaitingTimeSJF(ProcessType plist[], int n)
{ 
    int rem_bt[n];
    for (int i = 0; i < n; i++) {
        rem_bt[i] = plist[i].bt;
    }

    int complete = 0, t = 0;
    int shortest = 0;

    while (complete != n) {
        int minm = INT_MAX;
        bool check = false;

        // Find process with minimum remaining time that has arrived by time 't'
        for (int j = 0; j < n; j++) {
            if ((plist[j].art <= t) && (rem_bt[j] < minm) && rem_bt[j] > 0) {
                minm = rem_bt[j];
                shortest = j;
                check = true;
            }
        }

        // If no process has arrived yet, CPU is idle. Advance time.
        if (check == false) {
            t++;
            continue;
        }

        // Reduce remaining time by 1 for the shortest job
        rem_bt[shortest]--;

        // If a process gets completely executed
        if (rem_bt[shortest] == 0) {
            complete++;

            // Calculate completion time
            int completion_time = t + 1;
            
            // Calculate waiting time: Completion time - Burst Time - Arrival Time
            plist[shortest].wt = completion_time - plist[shortest].bt - plist[shortest].art;

            // Ensure wait time is never negative
            if (plist[shortest].wt < 0) {
                plist[shortest].wt = 0;
            }
        }
        // Increment time lap by one
        t++;
    }
} 

// Function to find the waiting time for all processes (FCFS)
void findWaitingTime(ProcessType plist[], int n)
{ 
    if (n == 0) return;
    
    // waiting time for first process is 0 because it starts execution immediately upon arrival
    plist[0].wt = 0; 
    int current_time = plist[0].art; // CPU starts when the first process arrives
  
    // calculating waiting time for subsequent processes
    for (int i = 1; i < n; i++) { 
        // CPU finishes the previous process
        current_time += plist[i-1].bt; 

        // If the next process hasn't arrived yet, CPU is idle until it does
        if (current_time < plist[i].art) {
            current_time = plist[i].art;
        }

        // Waiting time is current clock time minus when it actually arrived
        plist[i].wt = current_time - plist[i].art; 
    }
} 
  
// Function to calculate turn around time 
void findTurnAroundTime(ProcessType plist[], int n)
{ 
    // calculating turnaround time by adding bt[i] + wt[i] 
    for (int i = 0; i < n; i++) {
        plist[i].tat = plist[i].bt + plist[i].wt; 
    }
} 
  
// Function to sort the Process acc. to priority
int my_comparer(const void *this, const void *that)
{ 
    ProcessType *p1 = (ProcessType *)this;
    ProcessType *p2 = (ProcessType *)that;

    // 1. Sort by Priority (Descending)
    if (p1->pri != p2->pri) {
        return p2->pri - p1->pri;
    }
    
    // 2. Tie-Breaker: FCFS (Ascending Arrival Time)
    return p1->art - p2->art;
} 

//Function to calculate average time 
void findavgTimeFCFS(ProcessType plist[], int n) 
{ 
    findWaitingTime(plist, n); 
    findTurnAroundTime(plist, n); 
    printf("\n*********\nFCFS\n");
}

//Function to calculate average time 
void findavgTimeSJF(ProcessType plist[], int n) 
{ 
    findWaitingTimeSJF(plist, n); 
    findTurnAroundTime(plist, n); 
    printf("\n*********\nSJF\n");
}

//Function to calculate average time 
void findavgTimeRR(ProcessType plist[], int n, int quantum) 
{ 
    findWaitingTimeRR(plist, n, quantum); 
    findTurnAroundTime(plist, n); 
    printf("\n*********\nRR Quantum = %d\n", quantum);
}

//Function to calculate average time 
void findavgTimePriority(ProcessType plist[], int n) 
{ 
    // 1- Sort the processes according to priority.
    qsort(plist, n, sizeof(ProcessType), my_comparer);
    
    // 2- Now simply apply FCFS algorithm.
    findWaitingTime(plist, n);
    findTurnAroundTime(plist, n);
  
    printf("\n*********\nPriority\n");
}

void printMetrics(ProcessType plist[], int n)
{
    int total_wt = 0, total_tat = 0; 
    float awt, att;
    
    printf("\tProcesses\tBurst time\tWaiting time\tTurn around time\n"); 
  
    for (int i = 0; i < n; i++) 
    { 
        total_wt = total_wt + plist[i].wt; 
        total_tat = total_tat + plist[i].tat; 
        printf("\t%d\t\t%d\t\t%d\t\t%d\n", plist[i].pid, plist[i].bt, plist[i].wt, plist[i].tat); 
    } 
  
    awt = ((float)total_wt / (float)n);
    att = ((float)total_tat / (float)n);
    
    printf("\nAverage waiting time = %.2f", awt); 
    printf("\nAverage turn around time = %.2f\n", att); 
} 

ProcessType * initProc(char *filename, int *n) 
{
    FILE *input_file = fopen(filename, "r");
    if (!input_file) {
        fprintf(stderr, "Error: Invalid filepath\n");
        fflush(stdout);
        exit(0);
    }

    ProcessType *plist = parse_file(input_file, n);
    fclose(input_file);
    return plist;
}

// Higher-level wrapper to run and clean up a scheduling algorithm simulation
void runSimulation(void (*algorithm)(ProcessType[], int), char *filename) 
{
    int n = 0;
    ProcessType *proc_list = initProc(filename, &n);
    
    algorithm(proc_list, n);
    printMetrics(proc_list, n);
    
    free(proc_list); // Free memory to prevent memory leaks
}

void runSimulationRR(void (*algorithm)(ProcessType[], int, int), char *filename, int quantum) 
{
    int n = 0;
    ProcessType *proc_list = initProc(filename, &n);
    
    algorithm(proc_list, n, quantum);
    printMetrics(proc_list, n);
    
    free(proc_list);
}
  
// Driver code 
int main(int argc, char *argv[]) 
{ 
    int quantum = 2;
  
    if (argc < 2) {
        fprintf(stderr, "Usage: ./schedsim <input-file-path>\n");
        fflush(stdout);
        return 1;
    }
    
    // Run simulations using wrapper functions to improve design and avoid repetition
    runSimulation(findavgTimeFCFS, argv[1]);
    runSimulation(findavgTimeSJF, argv[1]);
    runSimulation(findavgTimePriority, argv[1]);
    runSimulationRR(findavgTimeRR, argv[1], quantum);
    
    return 0; 
}