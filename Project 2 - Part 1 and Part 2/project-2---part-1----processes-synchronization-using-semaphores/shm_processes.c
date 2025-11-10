/* Rojal Sapkota @03086974 */

#include  <stdio.h>
#include  <stdlib.h>
#include  <sys/types.h>
#include  <sys/ipc.h>
#include  <sys/shm.h>
#include <unistd.h>
#include <sys/wait.h>
#include <semaphore.h>
#include <fcntl.h>
#include <time.h>
#include <string.h>


// Shared memory structure
#define SEM_NAME_MUTEX "/bank_mutex"


// Function prototypes
void ParentProcess(int *SharedMem, const char *parent_type);
void ChildProcess(int *SharedMem, int child_id);
void cleanup(int *ShmPTR, int ShmID, sem_t *bank_mutex);


int main(int argc, char *argv[]) {    
  // Check command-line arguments
     if (argc != 3) {
          printf("Use: %s <num_parents> <num_children>\n", argv[0]);
          exit(1);
     }

     int num_parents = atoi(argv[1]);
     int num_children = atoi(argv[2]);

     if (num_parents < 1 || num_parents > 2 || num_children < 1) {
          printf("Inavaild arguments\n");
          exit(1);
     }

     // Create shared memory for bank account
     int ShmID = shmget(IPC_PRIVATE, sizeof(int), IPC_CREAT | 0666);
     if (ShmID < 0) {
          printf("*** shmget error (server) ***\n");
          exit(1);
     }

     int *ShmPTR = (int *)shmat(ShmID, NULL, 0);
     if ((int)(intptr_t)ShmPTR == -1) {
          printf("*** shmat error (server) ***\n");
          exit(1);
     }

     // Initialize bank account
     *ShmPTR = 0;

     // Create semaphore
     sem_t *mutex = sem_open(SEM_NAME_MUTEX, O_CREAT, 0666, 1);
     if (mutex == SEM_FAILED) {
          printf("*** semaphore initialization error ***\n");
          cleanup(ShmPTR, ShmID, mutex);
          exit(1);
     }

     // Seed random number generator
     srand(time(NULL));

     // Create parent processes
     pid_t dad_pid = fork();
     if (dad_pid < 0) {
          printf("*** fork error ***\n");
          cleanup(ShmPTR, ShmID, mutex);
          exit(1);
     }
     else if (dad_pid == 0) {
          ParentProcess(ShmPTR, "Dad");
          exit(0);
     }

     if (num_parents == 2) {
          pid_t mom_pid = fork();
          if (mom_pid < 0) {
               printf("*** fork error ***\n");
               cleanup(ShmPTR, ShmID, mutex);
               exit(1);
          }
          else if (mom_pid == 0) {
               ParentProcess(ShmPTR, "Mom");
               exit(0);
          }
     }

     // Create child processes
     int i;
     for (i = 0; i<num_children; i++) {
          pid_t child_pid = fork();
          if (child_pid < 0) {
               printf("*** fork error ***\n");
               cleanup(ShmPTR, ShmID, mutex);
               exit(1);
          }
          else if (child_pid == 0) {
               ChildProcess(ShmPTR, i + 1);
               exit(0);
          } 
     }

     // Wait for all processes
     while (wait(NULL) > 0);

     cleanup(ShmPTR, ShmID, mutex);
     return 0;
}
     
void ParentProcess(int *SharedMem, const char *parent_type) {
     sem_t *mutex = sem_open(SEM_NAME_MUTEX, 0);

     while (1) {
          int sleep_time;
          if (strcmp(parent_type, "Dad") == 0) {
               sleep_time = rand() % 6;
          } else {
               sleep_time = rand() % 11;
          }
          sleep(sleep_time);

          if (strcmp(parent_type, "Dad") == 0) {
               printf("Dear Old Dad: Attempting to Check Balance\n");
          } else {
               printf("Loveable Mom: Attempting to Check Balance\n");
          }

          sem_wait(mutex);
          int localBalance = *SharedMem;

          if (strcmp(parent_type, "Mom") == 0) {
               if (localBalance <= 100) {
                    int amount = rand() % 126;
                    if (rand() % 5 == 0) {
                         localBalance += amount;
                         printf("Loveable Mom: Deposits $%d / Balance = $%d\n", amount,localBalance);
                         *SharedMem = localBalance;
                    }     
               }
          } else {
               int random_number = rand() % 3;
               if (random_number == 0) {
                    if (localBalance < 100) {
                         int amount = rand() % 101;
                         if (rand() % 10 == 0) {
                              localBalance +=amount;
                              printf("Dear Old Dad: Deposits $%d / Balance = $%d\n", amount, localBalance);
                              *SharedMem = localBalance;
                         } else {
                              printf("Dear Old Dad: Doesn't have any money to give\n");
                         }
                    } else {
                         printf("Dear Old Dad: Thinks Student has enough Cash ($%d)\n", localBalance);
                    }
               } else {
                    printf("Dear Old Dad: Last Checking Balance = $%d\n", localBalance);
               }
          }
          sem_post(mutex);
     }
}


void ChildProcess(int *SharedMem, int child_id) {
     sem_t *mutex = sem_open(SEM_NAME_MUTEX, 0);

     while (1) {
          sleep(rand() % 6);

          printf("Poor Student #%d: Attempting to Check Balance\n", child_id);
          
          sem_wait(mutex);
          int localBalance = *SharedMem;

          if (rand() % 2 == 0) {
               int need = rand() % 51;
               printf("Poor Student #%d needs $%d\n", child_id, need);
               
               if (need <= localBalance) {
                    if (rand() % 10 == 0) {
                         localBalance -= need;
                         printf("Poor Student #%d: Withdraws $%d / Balance = $%d\n", child_id, need, localBalance);
                         *SharedMem = localBalance;
                    } else {
                         printf("Poor Student #%d: Not Enough Cash ($%d)\n", child_id, localBalance);   
                    } 
               } else {
                    printf("Poor Student #%d: Not Enough Cash ($%d)\n", child_id, localBalance);
               }        
          } else {
               printf("Poor Student #%d: Last Checking Balance = $%d\n", child_id, localBalance);
          }
          sem_post(mutex);
     }
}

void cleanup(int *ShmPTR, int ShmID, sem_t *mutex) {
     sem_close(mutex);
     sem_unlink(SEM_NAME_MUTEX);
     shmdt((void *)ShmPTR);
     shmctl(ShmID, IPC_RMID, NULL);

}