#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/wait.h>

#define SHM_SIZE 2 * sizeof(int) // Size for two integers: BankAccount and Turn

void DadProcess(int *BankAccount, int *Turn) {
    int account;
    srand(time(NULL)); // Initialize random seed for parent
    for (int i = 0; i < 25; i++) {
        // Sleep random time (0-5 seconds)
        sleep(rand() % 6);
        // Copy BankAccount to local variable
        account = *BankAccount;
        // Wait while it's not parent's turn
        while (*Turn != 0);
        if (account <= 100) {
            // Try to deposit money
            int balance = rand() % 101; // Random amount 0-100
            if (balance % 2 == 0) { // If amount is even
                account += balance;
                printf("Dear old Dad: Deposits $%d / Balance = $%d\n", balance, account);
            } else {
                printf("Dear old Dad: Doesn't have any money to give\n");
            }
        } else {
            printf("Dear old Dad: Thinks Student has enough Cash ($%d)\n", account);
        }
        // Update shared memory and give turn to child
        *BankAccount = account;
        *Turn = 1;
    }
}

void StudentProcess(int *BankAccount, int *Turn) {
    int account;
    srand(time(NULL) + 1); // Different seed for child process
    for (int i = 0; i < 25; i++) {
        // Sleep random time (0-5 seconds)
        sleep(rand() % 6);
        // Copy BankAccount to local variable
        account = *BankAccount;
        // Wait while it's not child's turn
        while (*Turn != 1);
        // Generate random amount needed (0-50)
        int balance = rand() % 51;
        printf("Poor Student needs $%d\n", balance);
        if (balance <= account) {
            account -= balance;
            printf("Poor Student: Withdraws $%d / Balance = $%d\n", balance, account);
        } else {
            printf("Poor Student: Not Enough Cash ($%d)\n", account);
        }
        // Update shared memory and give turn to parent
        *BankAccount = account;
        *Turn = 0;
    }
}

int main() {
    int shmid;
    int *ShmPTR;
    pid_t pid;

    // Create shared memory segment
    shmid = shmget(IPC_PRIVATE, SHM_SIZE, IPC_CREAT | 0666);
    if (shmid < 0) {
        perror("shmget error");
        exit(1);
    }

    // Attach shared memory segment
    ShmPTR = (int *) shmat(shmid, NULL, 0);
    if (ShmPTR == (int *)-1) {
        perror("shmat error");
        exit(1);
    }

    // Initialize shared variables
    int *BankAccount = &ShmPTR[0];
    int *Turn = &ShmPTR[1];
    *BankAccount = 0; // Initial balance
    *Turn = 0; // Parent starts first

    // Create child process
    pid = fork();
    if (pid < 0) {
        perror("Fork failed");
        exit(1);
    }
    else if (pid == 0) {
        // Child process (Student)
        StudentProcess(BankAccount, Turn);
        exit(0);
    }
    else {
        // Parent process (Dad)
        DadProcess(BankAccount, Turn);
        // Wait for child to finish
        wait(NULL);
        // Detach and remove shared memory
        if (shmdt(ShmPTR) == -1) {
            perror("shmdt error");
        }
        if (shmctl(shmid, IPC_RMID, NULL) == -1) {
            perror("shmctl error");
        }
    }
    return 0;
}