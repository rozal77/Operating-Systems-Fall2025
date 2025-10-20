// Rojal Sapkota @03086974

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
int command_p = -1;

// Function to handle timeout for long-running processes
void terminate_pro(int time, int pid){
  sleep(time);
  printf("Time out of foreground process.\n");
  kill(pid, SIGKILL);
}

// Signal Handler to prevent shell from exiting on Ctrl+C
void signal_handler(int signum){
  if(command_p != -1){
    kill(command_p, SIGINT);
  }
}


int main() {
    char command_line[MAX_COMMAND_LINE_LEN];
    char *arguments[MAX_COMMAND_LINE_ARGS];
    char wd[400];
    char *cwd;

    signal(SIGINT, signal_handler);

    while (true) {
        // Display prompt with current working directory
        cwd = getcwd(wd, sizeof(wd));
        printf("%s> ", cwd);
        fflush(stdout);

        // Get user input
        if ((fgets(command_line, MAX_COMMAND_LINE_LEN, stdin) == NULL) && ferror(stdin)) {
            fprintf(stderr, "fgets error\n");
            exit(1);
        }

        // Handle empty input
        if (command_line[0] == '\n') continue;

        // Tokenize the command line input
        arguments[0] = strtok(command_line, delimiters);
        int i = 0;
        while (arguments[i] != NULL) {
            i++;
            arguments[i] = strtok(NULL, delimiters);
        }

        // Handle EOF (Ctrl+D)
        if (feof(stdin)) {
            printf("\n");
            fflush(stdout);
            return 0;
        }

        // Check if command should run in background
        char *last_arg = arguments[i - 1];
        bool background_process = false;
        if (strcmp(last_arg, "&") == 0) {
            background_process = true;
            arguments[i - 1] = NULL;
        }

        // Built-in commands
        if (strcmp(arguments[0], "cd") == 0) {
            if (chdir(arguments[1]) != 0) {
                perror("chdir");
            }
        } else if (strcmp(arguments[0], "pwd") == 0) {
            printf("%s\n", getcwd(wd, sizeof(wd)));
        } else if (strcmp(arguments[0], "echo") == 0) {
            i = 1;
            while (arguments[i] != NULL) {
                if (arguments[i][0] == '$') {
                    printf("%s ", getenv(arguments[i] + 1));
                } else {
                    printf("%s ", arguments[i]);
                }
                i++;
            }
            printf("\n");
        } else if (strcmp(arguments[0], "env") == 0) {
            if (arguments[1] != NULL) {
                printf("%s\n", getenv(arguments[1]));
            } else {
                char **env = environ;
                for (; *env; env++) {
                    printf("%s\n", *env);
                }
            }
        } else if (strcmp(arguments[0], "setenv") == 0) {
            if (arguments[1] && strchr(arguments[1], '=')) {
                char *name = strtok(arguments[1], "=");
                char *value = strtok(NULL, "=");
                if (setenv(name, value, 1) != 0) {
                    perror("setenv");
                }
            } else {
                fprintf(stderr, "setenv: Use format VAR=VALUE\n");
            }
        } else if (strcmp(arguments[0], "exit") == 0) {
            exit(0);
        } else {
            // Check for I/O redirection
            int fd_in = -1, fd_out = -1;
            int j;
            for (j = 0; arguments[j] != NULL; j++) {
                if (strcmp(arguments[j], "<") == 0) {
                    fd_in = open(arguments[j + 1], O_RDONLY);
                    if (fd_in < 0) perror("open");
                    arguments[j] = NULL;
                } else if (strcmp(arguments[j], ">") == 0) {
                    fd_out = open(arguments[j + 1], O_CREAT | O_WRONLY | O_TRUNC, 0644);
                    if (fd_out < 0) perror("open");
                    arguments[j] = NULL;
                }
            }

            // Fork for executing external commands
            pid_t pid = fork();
            if (pid == 0) {
                // Child process
                if (fd_in != -1) {
                    dup2(fd_in, STDIN_FILENO);
                    close(fd_in);
                }
                if (fd_out != -1) {
                    dup2(fd_out, STDOUT_FILENO);
                    close(fd_out);
                }
                signal(SIGINT, SIG_DFL);
                execvp(arguments[0], arguments);
                perror("execvp");
                exit(1);
            } else if (pid > 0) {
                // Parent process
                if (!background_process) {
                    command_p = pid;
                    int timer_pid = fork();
                    if (timer_pid == 0) {
                        terminate_pro(10, pid);
                        exit(0);
                    }
                    waitpid(pid, NULL, 0);
                    kill(timer_pid, SIGKILL);
                    waitpid(timer_pid, NULL, 0);
                } else {
                    printf("Process %d running in background\n", pid);
                }
            } else {
                perror("fork");
            }
        }
    }
    return 0;
}
