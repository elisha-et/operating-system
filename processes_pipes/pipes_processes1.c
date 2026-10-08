#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>

int main(void)
{
    int fd1[2], fd2[2];
    pid_t p;
    char input_str[100];
    char str_buffer[300];
    char second_input[100];
    
    const char fixed_str[] = "howard.edu";
    const char final_str[] = "gobison.org";

    // Create both pipes
    if (pipe(fd1) == -1) {
        fprintf(stderr, "Pipe 1 Failed");
        return 1;
    }
    if (pipe(fd2) == -1) {
        fprintf(stderr, "Pipe 2 Failed");
        return 1;
    }

    // P1 Prompts for first input
    printf("Input: ");
    if (scanf("%99s", input_str) != 1) {
        fprintf(stderr, "Input Failed");
        return 1;
    }

    p = fork();

    if (p < 0) {
        fprintf(stderr, "fork Failed");
        return 1;
        
    } else if (p > 0) {
        // --- PARENT PROCESS (P1) ---
        close(fd1[0]); // Close reading end of first pipe
        close(fd2[1]); // Close writing end of second pipe

        // Pass the first string to P2
        write(fd1[1], input_str, strlen(input_str) + 1);
        close(fd1[1]);

        // Wait for child to process and send the string back
        wait(NULL);

        // Read string back from P2
        read(fd2[0], str_buffer, sizeof(str_buffer));
        close(fd2[0]);

        // P1 concatenates "gobison.org" to the original string
        strcat(str_buffer, final_str);
        
        // P1 prints the final string to stdout
        printf("Output: %s\n", str_buffer);

    } else {
        // --- CHILD PROCESS (P2) ---
        close(fd1[1]); // Close writing end of first pipe
        close(fd2[0]); // Close reading end of second pipe

        // Read string from P1
        read(fd1[0], str_buffer, sizeof(str_buffer));
        close(fd1[0]);

        // P2 concatenates "howard.edu" and prints to stdout
        strcat(str_buffer, fixed_str);
        printf("Output: %s\n", str_buffer);

        // P2 prompts user for a second input
        printf("Input: ");
        if (scanf("%99s", second_input) == 1) {
            // P2 appends that to the string just received from P1
            strcat(str_buffer, second_input);
        }

        // P2 passes this updated string back to P1
        write(fd2[1], str_buffer, strlen(str_buffer) + 1);
        close(fd2[1]);
        
        exit(0);
    }

    return 0;
}