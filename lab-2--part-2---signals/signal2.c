/* signal2.c */
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

    while (1) {
      // Every 5 seconds 
        alarm(5);
        alarm_triggered = 0;

        // Waiting until the handler sets the flag
        while (!alarm_triggered);

        // Turing Message Printed
        printf("Turing was right!\n");
    }

    return 0;
}