#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <string.h>

#define BUFFER_SIZE 25
#define READ_END 0
#define WRITE_END 1

int main(int argc, char **argv) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <search_term>\n", argv[0]);
        exit(1);
    }

    int first_pipe[2];  // For cat to grep
    int second_pipe[2]; // For grep to sort
    pid_t pid1, pid2;

    // Create the first pipe
    if (pipe(first_pipe) == -1) {
        fprintf(stderr, "First Pipe Failed");
        exit(1);
    }

    // Create the second pipe
    if (pipe(second_pipe) == -1) {
        fprintf(stderr, "Second Pipe Failed");
        exit(1);
    }

    // Fork the first child (P2 - will run grep)
    pid1 = fork();

    if (pid1 < 0) {
        fprintf(stderr, "Fork Failed");
        exit(1);
    }

    if (pid1 == 0) { // First child process (P2)
        // Fork the second child (P3 - will run sort)
        pid2 = fork();

        if (pid2 < 0) {
            fprintf(stderr, "Fork Failed");
            exit(1);
        }

        if (pid2 == 0) { // Second child process (P3 - sort)
            // Close unused pipes
            close(first_pipe[READ_END]);
            close(first_pipe[WRITE_END]);
            close(second_pipe[WRITE_END]);

            // Replace standard input with input part of second pipe
            dup2(second_pipe[READ_END], STDIN_FILENO);
            close(second_pipe[READ_END]);

            // Execute sort
            execlp("sort", "sort", NULL);
            
            // We only get here if exec failed
            fprintf(stderr, "Error executing sort\n");
            exit(1);
        } else { // First child process (P2 - grep)
            // Close unused pipes
            close(first_pipe[WRITE_END]);
            close(second_pipe[READ_END]);

            // Replace standard input with input part of first pipe
            dup2(first_pipe[READ_END], STDIN_FILENO);
            close(first_pipe[READ_END]);

            // Replace standard output with output part of second pipe
            dup2(second_pipe[WRITE_END], STDOUT_FILENO);
            close(second_pipe[WRITE_END]);

            // Execute grep with the argument
            execlp("grep", "grep", argv[1], NULL);
            
            // We only get here if exec failed
            fprintf(stderr, "Error executing grep\n");
            exit(1);
        }
    } else { // Parent process (P1)
        // Close unused pipes
        close(first_pipe[READ_END]);
        close(second_pipe[READ_END]);
        close(second_pipe[WRITE_END]);

        // Replace standard output with output part of first pipe
        dup2(first_pipe[WRITE_END], STDOUT_FILENO);
        close(first_pipe[WRITE_END]);

        // Execute cat
        execlp("cat", "cat", "scores", NULL);
        
        // We only get here if exec failed
        fprintf(stderr, "Error executing cat\n");
        exit(1);
    }

    return 0;
}