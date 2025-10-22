#include <stdio.h>
#include <stdlib.h>
#include <time.h>

// Generate a random N×N matrix with values 0–255
void generateRandomMatrix(int **matrixData, int matrixSize) {
  
    for (int rowIndex = 0; rowIndex < matrixSize; rowIndex++) {
        for (int colIndex = 0; colIndex < matrixSize; colIndex++) {
            *(*(matrixData + rowIndex) + colIndex) = rand() % 256;
        }
    }
}

// Print the matrix using pointer arithmetic
void printMatrix(int **matrixData, int matrixSize) {
    for (int rowIndex = 0; rowIndex < matrixSize; rowIndex++) {
        for (int colIndex = 0; colIndex < matrixSize; colIndex++) {
            printf("%4d", *(*(matrixData + rowIndex) + colIndex));
        }
        printf("\n");
    }
}

// Rotate the matrix 90° clockwise in-place
void rotateMatrix90Clockwise(int **matrixData, int matrixSize) {
    // Step 1: Transpose the matrix
    for (int rowIndex = 0; rowIndex < matrixSize; rowIndex++) {
        for (int colIndex = rowIndex + 1; colIndex < matrixSize; colIndex++) {
            int *elementPtr1 = *(matrixData + rowIndex) + colIndex;
            int *elementPtr2 = *(matrixData + colIndex) + rowIndex;
            int temp = *elementPtr1;
            *elementPtr1 = *elementPtr2;
            *elementPtr2 = temp;
        }
    }

    // Step 2: Reverse each row
    for (int rowIndex = 0; rowIndex < matrixSize; rowIndex++) {
        int *leftPtr = *(matrixData + rowIndex);
        int *rightPtr = leftPtr + matrixSize - 1;

        while (leftPtr < rightPtr) {
            int temp = *leftPtr;
            *leftPtr = *rightPtr;
            *rightPtr = temp;
            leftPtr++;
            rightPtr--;
        }
    }
}

// Apply 3×3 smoothing filter 
void applySmoothingFilter(int **matrixData, int matrixSize) {
    int *prevRow = (int *)malloc(matrixSize * sizeof(int));
    int *currRow = (int *)malloc(matrixSize * sizeof(int));
    int *nextRow = (int *)malloc(matrixSize * sizeof(int));

     if (prevRow == NULL || currRow == NULL || nextRow == NULL) {
        printf("Memory allocation failed while creating row buffers!\n");
        free(prevRow);
        free(currRow);
        free(nextRow);
        return ;
    }
    
    for (int colIndex = 0; colIndex < matrixSize; colIndex++) {
        *(prevRow + colIndex) = 0;  // Prevent uninitialized reads
        *(currRow + colIndex) = *(*(matrixData + 0) + colIndex);
        *(nextRow + colIndex) = *(*(matrixData + 1) + colIndex);  
    }

    for (int rowIndex = 0; rowIndex < matrixSize; rowIndex++) {
        if (rowIndex < matrixSize - 1) {
            for (int colIndex = 0; colIndex < matrixSize; colIndex++)
                *(nextRow + colIndex) = *(*(matrixData + rowIndex + 1) + colIndex);
        }

        for (int colIndex = 0; colIndex < matrixSize; colIndex++) {
            int sum = 0, count = 0;

            for (int rowOffset = -1; rowOffset <= 1; rowOffset++) {
                int neighborRow = rowIndex + rowOffset;
                if (neighborRow < 0 || neighborRow >= matrixSize)
                    continue;

                int *rowPointer;
                if (rowOffset == -1) {
                    // Skip prevRow for first row - prevent incorrect averaging
                    if (rowIndex == 0) continue;
                    rowPointer = prevRow;
                }
                else if (rowOffset == 0) rowPointer = currRow;
                else {
                    // Skip nextRow for last row - prevent incorrect averaging
                    if (rowIndex == matrixSize - 1) continue;
                    rowPointer = nextRow;
                }

                for (int colOffset = -1; colOffset <= 1; colOffset++) {
                    int neighborCol = colIndex + colOffset;
                    if (neighborCol < 0 || neighborCol >= matrixSize)
                        continue;

                    sum += *(rowPointer + neighborCol);
                    count++;
                }
            }

            if (count > 0) {
                *(*(matrixData + rowIndex) + colIndex) = sum / count;
            }
        }

        if (rowIndex < matrixSize - 1) {
            // Shift rows: prevRow = currRow, currRow = nextRow
            for (int colIndex = 0; colIndex < matrixSize; colIndex++)
                *(prevRow + colIndex) = *(currRow + colIndex);
            for (int colIndex = 0; colIndex < matrixSize; colIndex++)
                *(currRow + colIndex) = *(nextRow + colIndex);
        }
    }

    free(prevRow);
    free(currRow);
    free(nextRow);
}

int main() {
    int matrixSize;
    char inputChar;

    printf("Enter matrix size (2-10): ");

    if (scanf("%d%c", &matrixSize, &inputChar) != 2 || inputChar != '\n' || matrixSize < 2 || matrixSize > 10) {
        printf("Invalid input! Please enter a number between 2 and 10.\n");
        return 1;
    }

      srand(time(0));
    
    // Allocate memory for matrix
    int **matrixData = (int **)malloc(matrixSize * sizeof(int *));
     if (matrixData == NULL) {
        printf("Memory allocation failed for matrix rows!\n");
        return 1;
    }
    
    for (int rowIndex = 0; rowIndex < matrixSize; rowIndex++)
        *(matrixData + rowIndex) = (int *)malloc(matrixSize * sizeof(int));

    // Generate and display original matrix
    generateRandomMatrix(matrixData, matrixSize);
    printf("\nOriginal Randomly Generated Matrix:\n");
    printMatrix(matrixData, matrixSize);

    // Rotate matrix 90° clockwise
    rotateMatrix90Clockwise(matrixData, matrixSize);
    printf("\nMatrix after 90° Clockwise Rotation:\n");
    printMatrix(matrixData, matrixSize);

    // Apply smoothing filter
    applySmoothingFilter(matrixData, matrixSize);
    printf("\nMatrix after Applying 3×3 Smoothing Filter:\n");
    printMatrix(matrixData, matrixSize);

    // Free memory
    for (int rowIndex = 0; rowIndex < matrixSize; rowIndex++)
        free(*(matrixData + rowIndex));
    free(matrixData);

    return 0;
}
