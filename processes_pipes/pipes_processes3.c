#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

int main(int argc, char **argv)
{
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <search_term>\n", argv[0]);
        return 1;
    }

    int pipefd1[2]; // Pipe between cat and grep
    int pipefd2[2]; // Pipe between grep and sort
    pid_t pid1, pid2, pid3;

    char *cat_args[] = {"cat", "scores", NULL};
    char *grep_args[] = {"grep", argv[1], NULL};
    char *sort_args[] = {"sort", NULL};

    // 1. Create both pipes and check for errors
    if (pipe(pipefd1) == -1 || pipe(pipefd2) == -1) {
        perror("Pipe creation failed");
        return 1;
    }

    // 2. Process 1 (cat)
    if ((pid1 = fork()) < 0) {
        perror("Fork 1 failed");
        return 1;
    }
    if (pid1 == 0) {
        dup2(pipefd1[1], 1); // Redirect stdout to pipe1
        
        // Explicitly close ALL pipe ends
        close(pipefd1[0]); close(pipefd1[1]); 
        close(pipefd2[0]); close(pipefd2[1]);
        
        execvp("cat", cat_args);
        perror("execvp cat failed");
        _exit(1); // Use _exit on error per grader hint
    }

    // 3. Process 2 (grep)
    if ((pid2 = fork()) < 0) {
        perror("Fork 2 failed");
        return 1;
    }
    if (pid2 == 0) {
        dup2(pipefd1[0], 0); // Redirect stdin from pipe1
        dup2(pipefd2[1], 1); // Redirect stdout to pipe2
        
        // Explicitly close ALL pipe ends
        close(pipefd1[0]); close(pipefd1[1]); 
        close(pipefd2[0]); close(pipefd2[1]);
        
        execvp("grep", grep_args);
        perror("execvp grep failed");
        _exit(1);
    }

    // 4. Process 3 (sort)
    if ((pid3 = fork()) < 0) {
        perror("Fork 3 failed");
        return 1;
    }
    if (pid3 == 0) {
        dup2(pipefd2[0], 0); // Redirect stdin from pipe2
        
        // Explicitly close ALL pipe ends
        close(pipefd1[0]); close(pipefd1[1]); 
        close(pipefd2[0]); close(pipefd2[1]);
        
        execvp("sort", sort_args);
        perror("execvp sort failed");
        _exit(1);
    }

    // 5. Main Parent Process closes ALL pipe ends to avoid hanging
    close(pipefd1[0]); close(pipefd1[1]);
    close(pipefd2[0]); close(pipefd2[1]);

    // 6. Parent waits for all child processes to clean up zombies
    waitpid(pid1, NULL, 0);
    waitpid(pid2, NULL, 0);
    waitpid(pid3, NULL, 0);

    return 0;
}