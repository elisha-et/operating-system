#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <unistd.h>
#include <sys/wait.h>
#include <semaphore.h>
#include <fcntl.h>
#include <time.h>
#include <signal.h>

// Process roles
void DearOldDad(int *BankAccount, sem_t *mutex);
void LovableMom(int *BankAccount, sem_t *mutex);
void PoorStudent(int *BankAccount, sem_t *mutex);

// Global pointers for the signal handler cleanup
int ShmID;
int *BankAccount;
sem_t *mutex;
pid_t *pids;
int total_processes;

// Signal handler for Ctrl+C to terminate cleanly and avoid memory leaks
void cleanup_and_exit(int sig) {
    printf("\nReceived interrupt signal. Terminating program gracefully...\n");
    if (pids != NULL) {
        for (int i = 0; i < total_processes; i++) {
            if (pids[i] > 0) {
                kill(pids[i], SIGKILL); // Kill all active child processes
            }
        }
        free(pids);
    }
    // Clean up shared memory and semaphores
    shmdt((void *) BankAccount);
    shmctl(ShmID, IPC_RMID, NULL);
    sem_close(mutex);
    sem_unlink("/bank_mutex");
    printf("Resources successfully cleaned up. Exiting.\n");
    exit(0);
}

int main(int argc, char *argv[]) {
    int num_parents = 1;
    int num_children = 1;

    // Parse command-line arguments for Extra Credit
    if (argc >= 3) {
        num_parents = atoi(argv[1]);
        num_children = atoi(argv[2]);
    } else {
        printf("Usage: %s <parents> <children>\n", argv[0]);
        printf("Defaulting to 1 parent, 1 child.\n");
    }

    printf("Note: Program loops indefinitely. Use Ctrl+C to terminate cleanly.\n\n");

    // Register graceful exit signal
    signal(SIGINT, cleanup_and_exit);

    total_processes = num_parents + num_children;
    pids = malloc(sizeof(pid_t) * total_processes);

    // Initialize semaphore
    sem_unlink("/bank_mutex");
    mutex = sem_open("/bank_mutex", O_CREAT, 0666, 1);
    if (mutex == SEM_FAILED) {
        perror("sem_open failed");
        exit(1);
    }

    // Initialize shared memory
    ShmID = shmget(IPC_PRIVATE, sizeof(int), IPC_CREAT | 0666);
    if (ShmID < 0) {
        printf("*** shmget error ***\n");
        exit(1);
    }

    BankAccount = (int *) shmat(ShmID, NULL, 0);
    if (BankAccount == (int *)-1) {
        printf("*** shmat error ***\n");
        exit(1);
    }

    *BankAccount = 0; // Starting balance
    int pid_idx = 0;

    // Fork N Poor Student processes
    for (int i = 0; i < num_children; i++) {
        pid_t pid = fork();
        if (pid == 0) {
            PoorStudent(BankAccount, mutex);
            exit(0);
        }
        pids[pid_idx++] = pid;
    }

    // Fork Dear Old Dad
    if (num_parents >= 1) {
        pid_t pid = fork();
        if (pid == 0) {
            DearOldDad(BankAccount, mutex);
            exit(0);
        }
        pids[pid_idx++] = pid;
    }

    // Fork Lovable Mom if 2 parents are requested
    if (num_parents == 2) {
        pid_t pid = fork();
        if (pid == 0) {
            LovableMom(BankAccount, mutex);
            exit(0);
        }
        pids[pid_idx++] = pid;
    }

    // Main process simply waits (acting as the supervisor). 
    // It will block here until you press Ctrl+C to trigger cleanup.
    while(wait(NULL) > 0);

    return 0;
}

void DearOldDad(int *BankAccount, sem_t *mutex) {
    srand(getpid());
    int localBalance;

    while (1) {
        sleep(rand() % 6); // Sleep 0-5 seconds
        printf("Dear Old Dad: Attempting to Check Balance\n");
        
        sem_wait(mutex);
        localBalance = *BankAccount;
        int r = rand();
        
        if (r % 2 == 0) {
            if (localBalance < 100) {
                int amount = rand() % 101;
                if (amount % 2 == 0) {
                    localBalance += amount;
                    printf("Dear old Dad: Deposits $%d / Balance = $%d\n", amount, localBalance);
                    *BankAccount = localBalance;
                } else {
                    printf("Dear old Dad: Doesn't have any money to give\n");
                }
            } else {
                printf("Dear old Dad: Thinks Student has enough Cash ($%d)\n", localBalance);
            }
        } else {
            printf("Dear Old Dad: Last Checking Balance = $%d\n", localBalance);
        }
        sem_post(mutex);
    }
}

void LovableMom(int *BankAccount, sem_t *mutex) {
    srand(getpid());
    int localBalance;

    while(1) {
        sleep(rand() % 11); // Sleep 0-10 seconds
        printf("Loveable Mom: Attempting to Check Balance\n");
        
        sem_wait(mutex);
        localBalance = *BankAccount;
        
        if (localBalance <= 100) {
            int amount = rand() % 126; // Generates 0-125
            localBalance += amount;
            printf("Lovable Mom: Deposits $%d / Balance = $%d\n", amount, localBalance);
            *BankAccount = localBalance;
        }
        sem_post(mutex);
    }
}

void PoorStudent(int *BankAccount, sem_t *mutex) {
    srand(getpid());
    int localBalance;

    while (1) {
        sleep(rand() % 6); // Sleep 0-5 seconds
        printf("Poor Student: Attempting to Check Balance\n");
        
        sem_wait(mutex);
        localBalance = *BankAccount;
        int r = rand();
        
        if (r % 2 == 0) {
            int need = rand() % 51;
            printf("Poor Student needs $%d\n", need);
            
            if (need <= localBalance) {
                localBalance -= need;
                printf("Poor Student: Withdraws $%d / Balance = $%d\n", need, localBalance);
                *BankAccount = localBalance;
            } else {
                printf("Poor Student: Not Enough Cash ($%d)\n", localBalance);
            }
        } else {
            printf("Poor Student: Last Checking Balance = $%d\n", localBalance);
        }
        sem_post(mutex);
    }
}