/* signal1.c */
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>

volatile sig_atomic_t alarm_triggered = 0;

void alarm_handler(int sig) {
    printf("Hello World!\n");
    alarm_triggered = 1;
}

int main() {
    signal(SIGALRM, alarm_handler);

    int done = 0;
    while (!done) {
        alarm(5);
        alarm_triggered = 0;

        // Wait for the alarm
        while (!alarm_triggered);

        // Print additional message
        printf("Turing was right!\n");
        done = 1; // exit after one iteration
    }

    return 0;
}