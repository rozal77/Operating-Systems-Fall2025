/* timer.c */
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <time.h>

volatile sig_atomic_t alarm_triggered = 0; // Flag for an alarm
volatile sig_atomic_t alarm_count = 0;     // Counts number of alarms
time_t program_start;

void timer_handler(int signum) {
    printf("Hello World!\n");
    alarm_triggered = 1;   // Signal main that alarm fired
}

// SIGINT handler (Ctrl+C)
void exit_handler(int signum) {
    time_t program_end;
    time(&program_end);
    printf("\nProgram ran for %ld seconds, total alarms: %d\n",
           program_end - program_start, alarm_count);
    exit(0);
}

int main() {
  // Start time
    time(&program_start);

    // Register signal handlers
    signal(SIGALRM, timer_handler);
    signal(SIGINT, exit_handler);

    while (1) {
        alarm(1);            // Alarm every 1 second
        alarm_triggered = 0; // Reset the flag

        // Wait until the alarm fires
        while (!alarm_triggered);

        alarm_count++;       // Total Alarms incremented
        printf("Turing was right!\n"); // Turing Message Printed
    }

    return 0;
}