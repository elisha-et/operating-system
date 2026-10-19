#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <signal.h>

#define MAX_COMMAND_LINE_LEN 1024
#define MAX_COMMAND_LINE_ARGS 128

char prompt[] = "> ";
char delimiters[] = " \t\r\n";
extern char **environ;

// Global variable to keep track of the active foreground process
pid_t foreground_pid = 0;

// Task 4: Signal handler for SIGINT (Ctrl-C)
void sigint_handler(int sig) {
    // If there is a foreground process, forward the kill signal to it
    if (foreground_pid > 0) {
        kill(foreground_pid, SIGINT);
    } else {
        // Otherwise, simply reprint a newline and do nothing, keeping the shell alive
        printf("\n");
    }
}

// Task 5: Signal handler for the 10-second timeout
void alarm_handler(int sig) {
    if (foreground_pid > 0) {
        kill(foreground_pid, SIGKILL);
        printf("\nForeground process killed off after 10 seconds.\n");
    }
}

int main() {
    char command_line[MAX_COMMAND_LINE_LEN];
    char *arguments[MAX_COMMAND_LINE_ARGS];

    // Register signal handlers for Ctrl-C and our timeout alarm
    signal(SIGINT, sigint_handler);
    signal(SIGALRM, alarm_handler);
    	
    while (true) {
        do { 
            // Task 0: Modify the prompt to print the current working directory
            char cwd[1024];
            if (getcwd(cwd, sizeof(cwd)) != NULL) {
                printf("%s%s", cwd, prompt);
            } else {
                printf("%s", prompt); // Fallback if getcwd fails
            }
            fflush(stdout);

            // Read input from stdin
            if ((fgets(command_line, MAX_COMMAND_LINE_LEN, stdin) == NULL) && ferror(stdin)) {
                fprintf(stderr, "fgets error");
                exit(0);
            }
 
        } while(command_line[0] == 0x0A);  // Repeat while just ENTER is pressed
        
        command_line[strlen(command_line) - 1] = '\0';
      
        // Exit the shell if the user inputs EOF (Ctrl+D)
        if (feof(stdin)) {
            printf("\n");
            fflush(stdout);
            fflush(stderr);
            return 0;
        }

        // Check if string is completely empty
        if (strlen(command_line) == 0) continue;
			
        // Task 1: Tokenize the command line input (split it on whitespace)
        int arg_count = 0;
        char *token = strtok(command_line, delimiters);
        
        while (token != NULL && arg_count < MAX_COMMAND_LINE_ARGS - 1) {
            // Handle environment variables starting with '$'
            if (token[0] == '$') {
                char *env_val = getenv(token + 1); // skip the '$'
                if (env_val != NULL) {
                    arguments[arg_count++] = env_val;
                } else {
                    arguments[arg_count++] = ""; // Variable not found
                }
            } else {
                arguments[arg_count++] = token;
            }
            token = strtok(NULL, delimiters);
        }
        arguments[arg_count] = NULL; // Null terminate the arguments array

        // If no arguments were parsed, loop back
        if (arg_count == 0 || arguments[0] == NULL) continue;

        // Task 3: Check for background processes (&)
        bool is_background = false;
        if (strcmp(arguments[arg_count - 1], "&") == 0) {
            is_background = true;
            arguments[arg_count - 1] = NULL; // Remove the '&' from the argument list
            arg_count--;
        }

        // Task 6 Extra Credit: Check for simple Output Redirection (>)
        int fd_out = -1;
        for (int j = 0; arguments[j] != NULL; j++) {
            if (strcmp(arguments[j], ">") == 0) {
                if (arguments[j+1] != NULL) {
                    // Open target file for writing (create if it doesn't exist, truncate if it does)
                    fd_out = open(arguments[j+1], O_CREAT | O_WRONLY | O_TRUNC, 0644);
                    arguments[j] = NULL; // Truncate the arguments array at the ">" symbol
                }
                break;
            }
        }

        // Task 2: Implement Built-In Commands
        if (strcmp(arguments[0], "exit") == 0) {
            exit(0);
        } else if (strcmp(arguments[0], "cd") == 0) {
            if (arguments[1] != NULL && chdir(arguments[1]) != 0) {
                perror("cd failed");
            }
            continue;
        } else if (strcmp(arguments[0], "pwd") == 0) {
            char cwd[1024];
            if (getcwd(cwd, sizeof(cwd)) != NULL) {
                printf("%s\n", cwd);
            }
            continue;
        } else if (strcmp(arguments[0], "echo") == 0) {
            for (int i = 1; arguments[i] != NULL; i++) {
                printf("%s ", arguments[i]);
            }
            printf("\n");
            continue;
        } else if (strcmp(arguments[0], "env") == 0) {
            for (char **env = environ; *env != 0; env++) {
                printf("%s\n", *env);
            }
            continue;
        } else if (strcmp(arguments[0], "setenv") == 0) {
            if (arguments[1] != NULL && arguments[2] != NULL) {
                setenv(arguments[1], arguments[2], 1); // 1 = overwrite
            } else {
                printf("Usage: setenv <var> <value>\n");
            }
            continue;
        }
    
        // Task 2: Create a child process which will execute the command line input
        pid_t pid = fork();
        
        if (pid < 0) {
            perror("fork failed");
        } else if (pid == 0) {
            // === CHILD PROCESS ===
            
            // Re-enable default handling of Ctrl-C in the child
            signal(SIGINT, SIG_DFL);

            // Handle the Output Redirection (Task 6 Extra Credit)
            if (fd_out != -1) {
                dup2(fd_out, STDOUT_FILENO); // Point standard output to our file
                close(fd_out);
            }

            // Execute the system program
            if (execvp(arguments[0], arguments) == -1) {
                printf("execvp() failed: No such file or directory\nAn error occurred.\n");
                exit(1);
            }
        } else {
            // === PARENT PROCESS ===
            
            if (!is_background) {
                // Task 4 & 5: Save child PID and start 10 second countdown
                foreground_pid = pid;
                alarm(10); 
                
                int status;
                // Wait for the specific foreground process
                waitpid(pid, &status, 0);
                
                // Once waitpid finishes (either successfully or killed), cancel the alarm and clear PID
                alarm(0); 
                foreground_pid = 0;
            } else {
                // Do not wait if it is a background process
                printf("[Background Process Started] PID: %d\n", pid);
            }
        }
    }
    
    return -1;
}