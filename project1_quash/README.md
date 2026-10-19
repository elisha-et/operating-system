# Project 1: Create A Simple Shell (Quash)

**Author:** Elisha Etukudoh  

---

## 1. Introduction and Overview
Quash (Quite a Shell) is a custom, lightweight command-line interface written in C. The objective of this project was to understand the underlying mechanics of UNIX-like command shells (such as `bash` or `tcsh`) by implementing core features from scratch. This implementation successfully executes standard shell behaviors including parsing user input, executing built-in commands, managing foreground and background processes, handling inter-process signaling, and routing input/output via file redirection and piping.

## 2. Design Choices and Implementation Details

### Task 0 & 1: The Shell Prompt, Tokenization, and Environment Variables
**Prompt:** The shell dynamically retrieves the Current Working Directory (CWD) using the `getcwd()` system call to provide a contextual prompt (e.g., `/home/user> `). 
**Tokenization:** User input is fetched via `fgets()` and sanitized to remove trailing newlines. The input is then parsed into an array of argument strings using `strtok()`, split by standard whitespace delimiters (`" \t\r\n"`).
**Variable Expansion:** During tokenization, the shell actively checks if a token begins with the `$` character. If so, it treats the token as an environment variable and resolves it using `getenv()`. To mirror authentic bash behavior, if a variable does not exist, the shell drops it gracefully rather than crashing or printing blank strings.

### Task 2: Built-in Commands and Process Forking
Before falling back to system executables, the shell intercepts specific keywords to run as built-in commands directly within the parent process:
* **`cd`**: Changes the directory using `chdir()`. Error handling is provided if the path is invalid.
* **`pwd`**: Prints the working directory using `getcwd()`.
* **`echo`**: Iterates through the tokenized arguments and prints them to `stdout`.
* **`env`**: Iterates through the global `extern char **environ` pointer to print all active environment variables.
* **`setenv`**: Sets or overwrites an environment variable using `setenv()`.
* **`exit`**: Gracefully terminates the shell loop and cleans up.

**Process Execution:** For non-built-in commands, the shell uses the `fork()` system call to spawn a child process. The child process then uses `execvp()` to replace its image with the requested executable (e.g., `ls` or `cat`). The parent process uses `waitpid()` to suspend its execution until the child foreground process completes.

### Task 3: Background Processes
If the user appends an ampersand (`&`) to the end of their command, the shell identifies this as a background job. It nullifies the `&` token and forks the process normally. However, in the parent process, it explicitly skips the `waitpid()` call, allowing the shell prompt to return immediately while the child executes asynchronously. 

**Zombie Process Prevention:** A critical design choice was implementing a `SIGCHLD` signal handler. Whenever a background process finishes, it sends a `SIGCHLD` signal to the parent. The handler uses a `while (waitpid(-1, NULL, WNOHANG) > 0);` loop to silently reap the terminated child processes in the background, preventing resource leaks and zombie processes.

### Task 4 & 5: Signal Handling and Process Timeouts
Standard UNIX signals were hijacked and customized to protect the shell and enforce execution limits.
* **`SIGINT` (Ctrl-C):** By default, Ctrl-C kills the active process. If the user presses Ctrl-C while the shell is idle, it simply prints a newline and reprompts, preventing the shell from accidentally closing. If a foreground process is running, the shell captures the `SIGINT` and explicitly forwards it to the active child process via `kill(foreground_pid, SIGINT)`, terminating the child but keeping the shell alive.
* **10-Second Execution Limit:** To prevent runaway processes from hanging the shell, an `alarm(10)` is set immediately after forking a foreground process. If the child process does not finish within 10 seconds, a `SIGALRM` signal triggers a custom handler that fires a `SIGKILL` at the child process, forcibly terminating it and freeing the shell.

### Task 6: Extra Credit (I/O Redirection and Piping)
The shell was upgraded to support advanced I/O manipulation:
* **Output Redirection (`>`):** Parses the argument array for the `>` symbol. If found, it opens the target file using `open()` with `O_CREAT | O_WRONLY | O_TRUNC` flags. It then uses `dup2()` to redirect the child process's `STDOUT_FILENO` into the file descriptor before executing the command.
* **Input Redirection (`<`):** Operates similarly to output redirection, but opens the target file with `O_RDONLY` and redirects it into `STDIN_FILENO`, allowing commands to read from files instead of the keyboard.
* **Piping (`|`):** If a pipe is detected, the shell splits the arguments into two separate commands. It creates a unidirectional data channel using `pipe()`. Two child processes are forked. The "left" command uses `dup2()` to map its standard output to the write-end of the pipe. The "right" command uses `dup2()` to map its standard input to the read-end of the pipe. Both pipe ends are then closed in the parent, and `waitpid()` is called for both children.

## 3. Known Bugs and Edge Cases
* **Chained Redirections:** The current implementation of redirection handles a single `<` or `>` per command effectively. Extremely complex chained redirections (e.g., `< input.txt > output.txt` in the same line) or multi-pipe chains (e.g., `cmd1 | cmd2 | cmd3`) fall outside the current scope of the token parsing logic.
* **String Quotation:** The tokenizer splits strictly on whitespace. Strings enclosed in quotation marks (e.g., `echo "hello world"`) are currently tokenized into separate arguments rather than a single string literal.

## 4. How to Compile and Run
A standard `Makefile` is provided. To compile and run the shell:
1. Open the terminal in the project directory.
2. Run `make` to compile the `shell.c` source code.
3. Execute the program by running `./quash`.
4. Type commands normally. To exit, type `exit` or press `Ctrl+D`.