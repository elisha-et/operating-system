/*
 * Program: main3.c
 * Description: This program demonstrates process creation using fork().
 * It creates two child processes that each loop a random number of times,
 * sleeping for random durations and printing their state. The parent process 
 * waits for both children to terminate before exiting itself, verifying 
 * the exit status of each child.
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>

void child_behavior() {
    // Seed the random number generator using time offset by the unique process ID 
    // for more robust randomization
    srandom(time(NULL) ^ getpid()); 
    
    // Random number of iterations (between 1 and 30)
    int iterations = (random() % 30) + 1; 

    for (int i = 0; i < iterations; i++) {
        printf("Child Pid: %d is going to sleep!\n", getpid());
        
        // Sleep for a random amount of time (0 to 10 seconds)
        sleep(random() % 11); 
        
        printf("Child Pid: %d is awake!\nWhere is my Parent: %d?\n", getpid(), getppid());
    }
    
    // Terminate child process
    exit(0); 
}

int main() {
    pid_t pid1, pid2;
    int status;
    pid_t finished_pid;

    // Fork first child
    pid1 = fork();
    if (pid1 < 0) {
        perror("Fork failed");
        exit(1);
    } else if (pid1 == 0) {
        child_behavior();
    }

    // Fork second child
    pid2 = fork();
    if (pid2 < 0) {
        perror("Fork failed");
        exit(1);
    } else if (pid2 == 0) {
        child_behavior();
    }

    // Parent specific message before waiting
    printf("Parent Pid: %d is waiting for children to complete...\n", getpid());

    // Parent uses a loop to wait for both child processes
    for (int i = 0; i < 2; i++) {
        finished_pid = wait(&status);
        if (finished_pid == -1) {
            perror("Wait failed");
        } else if (WIFEXITED(status)) {
            printf("Child Pid: %d has completed normally with exit status %d\n", finished_pid, WEXITSTATUS(status));
        } else {
            printf("Child Pid: %d terminated abnormally\n", finished_pid);
        }
    }

    return 0;
}