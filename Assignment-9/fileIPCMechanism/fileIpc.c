#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

#define FILE_NAME "data.txt"
#define ARRAY_SIZE 5

void sortArray(int numbers[]) {
    int i, j, temp;
    for (i = 0; i < ARRAY_SIZE; i++) {
        for (j = i + 1; j < ARRAY_SIZE; j++) {
            if (numbers[i] > numbers[j]) {
                temp = numbers[i];
                numbers[i] = numbers[j];
                numbers[j] = temp;
            }
        }
    }
}

int main() {
    int numbers[ARRAY_SIZE];
    FILE *filePointer;

    pid_t processId = fork();

    if (processId == 0) {
        filePointer = fopen(FILE_NAME, "r");
        fread(numbers, sizeof(int), ARRAY_SIZE, filePointer);
        fclose(filePointer);

        sortArray(numbers);

        filePointer = fopen(FILE_NAME, "w");
        fwrite(numbers, sizeof(int), ARRAY_SIZE, filePointer);
        fclose(filePointer);
    } else {
        printf("Enter %d integers:\n", ARRAY_SIZE);
        for (int i = 0; i < ARRAY_SIZE; i++)
            scanf("%d", &numbers[i]);

        printf("Before Sorting:\n");
        for (int i = 0; i < ARRAY_SIZE; i++)
            printf("%d ", numbers[i]);

        filePointer = fopen(FILE_NAME, "w");
        fwrite(numbers, sizeof(int), ARRAY_SIZE, filePointer);
        fclose(filePointer);

        wait(NULL);

        filePointer = fopen(FILE_NAME, "r");
        fread(numbers, sizeof(int), ARRAY_SIZE, filePointer);
        fclose(filePointer);

        printf("\nAfter Sorting:\n");
        for (int i = 0; i < ARRAY_SIZE; i++)
            printf("%d ", numbers[i]);
    }

    return 0;
}
