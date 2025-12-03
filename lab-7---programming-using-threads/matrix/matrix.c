#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>

// Define Matrix dimensions and thread count
#define M 20
#define N 20
#define MAX_THREAD 10

// Global matrices to store data and results
// Using global variables simplifies thread access without passing complex structures
int matA[M][N];
int matB[M][N];
int matSum[M][N];
int matSub[M][N];
int matMul[M][N];

// Array to hold thread IDs. 
// We use this array to pass unique pointers to each thread. 
// If we passed the address of the loop variable 'i' directly (&i), 
// the value might change before the thread starts, causing race conditions.
int thread_ids[MAX_THREAD];

// Function to fill a matrix with random numbers (0-9)
void fillMatrix(int matrix[M][N]) {
    for (int i = 0; i < M; i++) {
        for (int j = 0; j < N; j++) {
            matrix[i][j] = rand() % 10; 
        }
    }
}

// Function to print a matrix to the console
void printMatrix(int matrix[M][N]) {
    for (int i = 0; i < M; i++) {
        for (int j = 0; j < N; j++) {
            printf("%5d ", matrix[i][j]);
        }
        printf("\n");
    }
    printf("\n");
}

// Thread function for Matrix Addition
// Each thread calculates a specific slice of the result matrix.
void* computeAdd(void* arg) {
    // 1. Extract the thread index (0 to 9) from the argument
    int core = *((int*)arg);
    
    // 2. Calculate workload (Data Partitioning)
    // We divide the 20 rows (M) by the 10 threads (MAX_THREAD).
    // Each thread is responsible for 2 rows.
    // e.g., Core 0: rows 0-1, Core 1: rows 2-3, ... Core 9: rows 18-19.
    int start_row = core * (M / MAX_THREAD);
    int end_row = (core + 1) * (M / MAX_THREAD);

    // 3. Perform Addition for the assigned rows
    for (int i = start_row; i < end_row; i++) {
        for (int j = 0; j < N; j++) {
            matSum[i][j] = matA[i][j] + matB[i][j];
        }
    }
    pthread_exit(0);
}

// Thread function for Matrix Subtraction
// Follows the same data partitioning logic as Addition.
void* computeSub(void* arg) {
    int core = *((int*)arg);
    
    // Determine the specific rows this thread is responsible for
    int start_row = core * (M / MAX_THREAD);
    int end_row = (core + 1) * (M / MAX_THREAD);

    // Perform Subtraction for the assigned rows
    for (int i = start_row; i < end_row; i++) {
        for (int j = 0; j < N; j++) {
            matSub[i][j] = matA[i][j] - matB[i][j];
        }
    }
    pthread_exit(0);
}

// Thread function for Matrix Multiplication (Dot Product)
// Computes C[i][j] = Row A[i] * Col B[j]
void* computeDot(void* arg) {
    int core = *((int*)arg);
    
    // Determine the specific rows this thread is responsible for
    int start_row = core * (M / MAX_THREAD);
    int end_row = (core + 1) * (M / MAX_THREAD);

    // Perform Dot Product for the assigned rows
    for (int i = start_row; i < end_row; i++) {
        for (int j = 0; j < N; j++) {
            // Initialize result cell to 0
            matMul[i][j] = 0;
            // Iterate over the columns of A / rows of B (dimension N)
            for (int k = 0; k < N; k++) {
                matMul[i][j] += matA[i][k] * matB[k][j];
            }
        }
    }
    pthread_exit(0);
}

int main() {
    // Seed random number generator to ensure different results each run
    srand(time(NULL));

    // 1. Initialization Phase
    printf("Generating Matrices with random values...\n");
    fillMatrix(matA);
    fillMatrix(matB);

    printf("Matrix A (Input):\n");
    printMatrix(matA);
    printf("Matrix B (Input):\n");
    printMatrix(matB);

    // Thread handles array
    pthread_t threads[MAX_THREAD];
    int i, rc;

    // Initialize thread ID array. 
    // We do this once so we can pass persistent addresses to the threads.
    for(i = 0; i < MAX_THREAD; i++) {
        thread_ids[i] = i;
    }

    // 2. Perform Parallel Addition
    // Create threads, passing the specific ID from our array
    for (i = 0; i < MAX_THREAD; i++) {
        rc = pthread_create(&threads[i], NULL, computeAdd, &thread_ids[i]);
        if (rc) {
            fprintf(stderr, "ERROR: pthread_create() failed for Addition thread %d, code: %d\n", i, rc);
            exit(-1);
        }
    }
    // Wait for all threads to complete their slice of work
    for (i = 0; i < MAX_THREAD; i++) {
        rc = pthread_join(threads[i], NULL);
        if (rc) {
            fprintf(stderr, "ERROR: pthread_join() failed for Addition thread %d, code: %d\n", i, rc);
            exit(-1);
        }
    }
    printf("Addition Result (A + B):\n");
    printMatrix(matSum);

    // 3. Perform Parallel Subtraction
    for (i = 0; i < MAX_THREAD; i++) {
        rc = pthread_create(&threads[i], NULL, computeSub, &thread_ids[i]);
        if (rc) {
            fprintf(stderr, "ERROR: pthread_create() failed for Subtraction thread %d, code: %d\n", i, rc);
            exit(-1);
        }
    }
    // Join threads ensures main doesn't print before calculation is done
    for (i = 0; i < MAX_THREAD; i++) {
        rc = pthread_join(threads[i], NULL);
        if (rc) {
            fprintf(stderr, "ERROR: pthread_join() failed for Subtraction thread %d, code: %d\n", i, rc);
            exit(-1);
        }
    }
    printf("Subtraction Result (A - B):\n");
    printMatrix(matSub);

    // 4. Perform Parallel Multiplication (Dot Product)
    for (i = 0; i < MAX_THREAD; i++) {
        rc = pthread_create(&threads[i], NULL, computeDot, &thread_ids[i]);
        if (rc) {
            fprintf(stderr, "ERROR: pthread_create() failed for Multiplication thread %d, code: %d\n", i, rc);
            exit(-1);
        }
    }
    // Wait for completion
    for (i = 0; i < MAX_THREAD; i++) {
        rc = pthread_join(threads[i], NULL);
        if (rc) {
            fprintf(stderr, "ERROR: pthread_join() failed for Multiplication thread %d, code: %d\n", i, rc);
            exit(-1);
        }
    }
    printf("Multiplication Result (A * B):\n");
    printMatrix(matMul);

    return 0;
}