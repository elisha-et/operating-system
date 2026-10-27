#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>

// Hardcode MAX to 20 exactly as requested by the autograder
#define MAX 20

int matA[MAX][MAX]; 
int matB[MAX][MAX]; 

int matSumResult[MAX][MAX];
int matDiffResult[MAX][MAX]; 
int matProductResult[MAX][MAX]; 

void fillMatrix(int matrix[MAX][MAX]) {
    for(int i = 0; i < MAX; i++) {
        for(int j = 0; j < MAX; j++) {
            matrix[i][j] = rand() % 10 + 1;
        }
    }
}

void printMatrix(int matrix[MAX][MAX]) {
    for(int i = 0; i < MAX; i++) {
        for(int j = 0; j < MAX; j++) {
            printf("%5d", matrix[i][j]);
        }
        printf("\n");
    }
    printf("\n");
}

// Fetches the appropriate chunk of rows from the thread ID, and sets
// the subset of matSumResult to the sum of matA and matB.
void* computeSum(void* args) { 
    int thread_id = *((int*)args);
    int chunk_size = MAX / 10;
    int start = thread_id * chunk_size;
    int end = start + chunk_size;
    
    for (int i = start; i < end; i++) {
        for (int j = 0; j < MAX; j++) {
            matSumResult[i][j] = matA[i][j] + matB[i][j];
        }
    }
    return NULL;
}

// Fetches the appropriate chunk of rows from the thread ID, and sets
// the subset of matDiffResult to the difference of matA and matB.
void* computeDiff(void* args) { 
    int thread_id = *((int*)args);
    int chunk_size = MAX / 10;
    int start = thread_id * chunk_size;
    int end = start + chunk_size;
    
    for (int i = start; i < end; i++) {
        for (int j = 0; j < MAX; j++) {
            matDiffResult[i][j] = matA[i][j] - matB[i][j];
        }
    }
    return NULL;
}

// Fetches the appropriate chunk of rows from the thread ID, and sets
// the subset of matProductResult to the inner product of matA and matB.
void* computeProduct(void* args) { 
    int thread_id = *((int*)args);
    int chunk_size = MAX / 10;
    int start = thread_id * chunk_size;
    int end = start + chunk_size;
    
    for (int i = start; i < end; i++) {
        for (int j = 0; j < MAX; j++) {
            matProductResult[i][j] = 0;
            for (int k = 0; k < MAX; k++) {
                matProductResult[i][j] += matA[i][k] * matB[k][j];
            }
        }
    }
    return NULL;
}

int main() {
    srand(time(0));
    
    // 1. Fill the matrices (matA and matB) with random values.
    fillMatrix(matA);
    fillMatrix(matB);
    
    // 2. Print the initial matrices.
    printf("Matrix A:\n");
    printMatrix(matA);
    printf("Matrix B:\n");
    printMatrix(matB);
    
    // 3. Create exactly 10 threads to divide and conquer the operations
    pthread_t threads[10];
    int thread_ids[10];
    
    // Compute Sum with 10 threads
    for (int i = 0; i < 10; i++) {
        thread_ids[i] = i;
        pthread_create(&threads[i], NULL, computeSum, &thread_ids[i]);
    }
    for (int i = 0; i < 10; i++) {
        pthread_join(threads[i], NULL);
    }
    
    // Compute Difference with 10 threads
    for (int i = 0; i < 10; i++) {
        pthread_create(&threads[i], NULL, computeDiff, &thread_ids[i]);
    }
    for (int i = 0; i < 10; i++) {
        pthread_join(threads[i], NULL);
    }
    
    // Compute Product with 10 threads
    for (int i = 0; i < 10; i++) {
        pthread_create(&threads[i], NULL, computeProduct, &thread_ids[i]);
    }
    for (int i = 0; i < 10; i++) {
        pthread_join(threads[i], NULL);
    }
    
    // 4. Print the results.
    printf("Results:\n");
    printf("Sum:\n");
    printMatrix(matSumResult);
    printf("Difference:\n");
    printMatrix(matDiffResult);
    printf("Product:\n");
    printMatrix(matProductResult);
    
    return 0;
}