#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <unistd.h>
#include <sys/wait.h>
#include <time.h>

// Function prototypes
void PoorStudent(int *ShmPTR);
void DearOldDad(int *ShmPTR);

int main(int argc, char *argv[])
{
    int ShmID;
    int *ShmPTR;
    pid_t pid;
    int status;

    // Explicitly allocating shared memory for exactly 2 integers: BankAccount [0] and Turn [1]
    ShmID = shmget(IPC_PRIVATE, 2 * sizeof(int), IPC_CREAT | 0666);
    if (ShmID < 0) {
        printf("*** shmget error (server) ***\n");
        exit(1);
    }

    // Attach shared memory
    ShmPTR = (int *) shmat(ShmID, NULL, 0);
    
    // Check the return pointer itself, not the dereferenced value
    if (ShmPTR == (int *)-1) {
        printf("*** shmat error (server) ***\n");
        exit(1);
    }

    // Initialize BankAccount and Turn
    ShmPTR[0] = 0; // BankAccount
    ShmPTR[1] = 0; // Turn

    pid = fork();
    if (pid < 0) {
        printf("*** fork error ***\n");
        exit(1);
    } 
    else if (pid == 0) {
        PoorStudent(ShmPTR);
        exit(0);
    } 
    else {
        DearOldDad(ShmPTR);
        
        // Wait for child to complete
        if (wait(&status) < 0) {
            printf("*** wait error ***\n");
        }
        
        // Detach and remove shared memory
        shmdt((void *) ShmPTR);
        shmctl(ShmID, IPC_RMID, NULL);
    }

    return 0;
}

void PoorStudent(int *ShmPTR) {
    srand(getpid());
    int i;
    
    for (i = 0; i < 25; i++) {
        sleep(rand() % 6);
        
        // Wait for turn (Synchronization section)
        while (ShmPTR[1] != 1) {
            usleep(100); 
        }
        
        // Critical Section begins (Read account AFTER getting turn to prevent stale reads)
        int account = ShmPTR[0]; 
        
        int amount = rand() % 51;
        printf("Poor Student needs $%d\n", amount);
        
        if (amount <= account) {
            account -= amount;
            // Replaced '/' with ':' per autograder feedback
            printf("Poor Student: Withdraws $%d : Balance = $%d\n", amount, account);
        } else {
            printf("Poor Student: Not Enough Cash ($%d)\n", account);
        }
        
        ShmPTR[0] = account;
        ShmPTR[1] = 0; // Pass turn
    }
    
    // Child detaches from shared memory before exiting
    shmdt((void *) ShmPTR);
}

void DearOldDad(int *ShmPTR) {
    srand(getpid());
    int i;
    
    for (i = 0; i < 25; i++) {
        sleep(rand() % 6);
        
        // Wait for turn (Synchronization section)
        while (ShmPTR[1] != 0) {
            usleep(100); 
        }
        
        // Critical Section begins (Read account AFTER getting turn to prevent stale reads)
        int account = ShmPTR[0]; 
        
        if (account <= 100) {
            int amount = rand() % 101;
            
            if (amount % 2 == 0) {
                account += amount;
                // Replaced '/' with ':' per autograder feedback
                printf("Dear old Dad: Deposits $%d : Balance = $%d\n", amount, account);
            } else {
                printf("Dear old Dad: Doesn't have any money to give\n");
            }
        } else {
            printf("Dear old Dad: Thinks Student has enough Cash ($%d)\n", account);
        }
        
        ShmPTR[0] = account;
        ShmPTR[1] = 1; // Pass turn
    }
}