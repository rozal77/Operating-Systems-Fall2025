#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <time.h>

#define MAX_ITERATIONS 30
#define MAX_SLEEP_TIME 10

void create_child_process(int child_num) {
    int i; 
    int sleep_time;

    // Seed random number generator uniquely for each process
    srandom(time(NULL) ^ (getpid() << 16));

    // // Random number between 1 and 30
    int iterations = random() % MAX_ITERATIONS + 1;

    for (i = 0; i < iterations; i++) {
        printf("Child %d (Pid: %d) is going to sleep!\n", child_num, getpid());
        fflush(stdout); // Ensure output is printed immediately

        sleep_time = random() % MAX_SLEEP_TIME + 1; // Random number between 1 and 10
        sleep(sleep_time);

        printf("Child %d (Pid: %d) is awake! My Parent is: %d\n",
               child_num, getpid(), getppid());
        fflush(stdout);
    }

    printf("Child %d (Pid: %d) is terminating.\n", child_num, getpid());
    fflush(stdout);
    exit(0);
}

int main() {
    pid_t child1, child2;
    int status;

    printf("Parent process (Pid: %d) starting.\n", getpid());
    fflush(stdout);

    // Fork first child
    child1 = fork();

    if (child1 < 0) {
        perror("Fork failed for child 1");
        exit(1);
    } else if (child1 == 0) {
        create_child_process(1);
    } else {
        printf("Parent created Child 1 (Pid: %d)\n", child1);
        fflush(stdout);

        // Fork second child
        child2 = fork();

        if (child2 < 0) {
            perror("Fork failed for child 2");
            exit(1);
        } else if (child2 == 0) {
            create_child_process(2);
        } else {
            printf("Parent created Child 2 (Pid: %d)\n", child2);
            fflush(stdout);

            // Parent waits for both children
            pid_t terminated_pid;

            terminated_pid = wait(&status);
            printf("Parent noticed Child (Pid: %d) has completed.\n", terminated_pid);
            fflush(stdout);

            terminated_pid = wait(&status);
            printf("Parent noticed Child (Pid: %d) has completed.\n", terminated_pid);
            fflush(stdout);

            printf("Parent process (Pid: %d) has finished waiting for both children.\n", getpid());
            fflush(stdout);
        }
    }

    return 0;
}