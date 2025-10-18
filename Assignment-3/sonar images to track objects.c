#include <stdio.h>
#include <stdlib.h>
#include <time.h>

// Generate a random N×N matrix with values 0–255
void generateRandomMatrix(int **matrix, int size) {
    srand(time(0));
    for (int row = 0; row < size; row++) {
        for (int col = 0; col < size; col++) {
            *(*(matrix + row) + col) = rand() % 256;
        }
    }
}

// Print the matrix using pointer arithmetic
void printMatrix(int **matrix, int size) {
    for (int row = 0; row < size; row++) {
        for (int col = 0; col < size; col++) {
            printf("%4d", *(*(matrix + row) + col));
        }
        printf("\n");
    }
}

// Rotate the matrix 90° clockwise in-place
void rotateMatrix90Clockwise(int ***matrix, int size) {
    for (int row = 0; row < size; row++) {
        for (int col = row + 1; col < size; col++) {
            int *elementPtr1 = *(*matrix + row) + col;
            int *elementPtr2 = *(*matrix + col) + row;
            int temp = *elementPtr1;
            *elementPtr1 = *elementPtr2;
            *elementPtr2 = temp;
        }
    }

    for (int row = 0; row < size; row++) {
        int *leftPtr = *(*matrix + row);
        int *rightPtr = leftPtr + size - 1;

        while (leftPtr < rightPtr) {
            int temp = *leftPtr;
            *leftPtr = *rightPtr;
            *rightPtr = temp;
            leftPtr++;
            rightPtr--;
        }
    }
}

// Apply smoothing filter 
void applySmoothingFilter(int **matrix, int size) {
    int *prevRow = (int *)malloc(size * sizeof(int));
    int *currRow = (int *)malloc(size * sizeof(int));
    int *nextRow = (int *)malloc(size * sizeof(int));

    for (int col = 0; col < size; col++)
        *(currRow + col) = *(*(matrix + 0) + col);

    for (int row = 0; row < size; row++) {
        if (row < size - 1)
            for (int col = 0; col < size; col++)
                *(nextRow + col) = *(*(matrix + row + 1) + col);

        for (int col = 0; col < size; col++) {
            int sum = 0, count = 0;

            for (int rowOffset = -1; rowOffset <= 1; rowOffset++) {
                int neighborRow = row + rowOffset;
                if (neighborRow < 0 || neighborRow >= size)
                    continue;

                int *rowPointer;
                if (rowOffset == -1) rowPointer = prevRow;
                else if (rowOffset == 0) rowPointer = currRow;
                else rowPointer = nextRow;

                for (int colOffset = -1; colOffset <= 1; colOffset++) {
                    int neighborCol = col + colOffset;
                    if (neighborCol < 0 || neighborCol >= size)
                        continue;

                    sum += *(rowPointer + neighborCol);
                    count++;
                }
            }

            *(*(matrix + row) + col) = sum / count;
        }

        if (row < size - 1) {
            for (int col = 0; col < size; col++)
                *(prevRow + col) = *(currRow + col);
            for (int col = 0; col < size; col++)
                *(currRow + col) = *(nextRow + col);
        }
    }

    free(prevRow);
    free(currRow);
    free(nextRow);
}

int main() {
    int size;
    char ch;

    printf("Enter matrix size : ");

 
    if (scanf("%d%c", &size, &ch) != 2 || ch != '\n' || size < 2 || size > 10) {
        printf("Invalid input! Please enter a number between 2 and 10.\n");
        return 1;
    }

    int **matrix = (int **)malloc(size * sizeof(int *));
    for (int row = 0; row < size; row++)
        *(matrix + row) = (int *)malloc(size * sizeof(int));

    generateRandomMatrix(matrix, size);
    printf("\nOriginal Randomly Generated Matrix:\n");
    printMatrix(matrix, size);

    rotateMatrix90Clockwise(&matrix, size);
    printf("\nMatrix after 90 degree Clockwise Rotation:\n");
    printMatrix(matrix, size);

    applySmoothingFilter(matrix, size);
    printf("\nMatrix after Applying Smoothing Filter:\n");
    printMatrix(matrix, size);

    for (int row = 0; row < size; row++)
        free(*(matrix + row));
    free(matrix);

    return 0;
}
