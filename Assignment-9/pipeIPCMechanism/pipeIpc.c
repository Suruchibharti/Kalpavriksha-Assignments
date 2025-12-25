#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>

#define ARRAY_SIZE 5

void sortArray(int data[]) {
    int temp;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        for (int j = i + 1; j < ARRAY_SIZE; j++) {
            if (data[i] > data[j]) {
                temp = data[i];
                data[i] = data[j];
                data[j] = temp;
            }
        }
    }
}

int main() {
    int pipeFd[2];
    int numbers[ARRAY_SIZE];

    pipe(pipeFd);
    pid_t processId = fork();

    if (processId == 0) {
        read(pipeFd[0], numbers, sizeof(numbers));
        sortArray(numbers);
        write(pipeFd[1], numbers, sizeof(numbers));
    } else {
        printf("Enter %d numbers:\n", ARRAY_SIZE);
        for (int i = 0; i < ARRAY_SIZE; i++)
            scanf("%d", &numbers[i]);

        printf("Before Sorting:\n");
        for (int i = 0; i < ARRAY_SIZE; i++)
            printf("%d ", numbers[i]);

        write(pipeFd[1], numbers, sizeof(numbers));
        wait(NULL);

        read(pipeFd[0], numbers, sizeof(numbers));

        printf("\nAfter Sorting:\n");
        for (int i = 0; i < ARRAY_SIZE; i++)
            printf("%d ", numbers[i]);
    }

    return 0;
}
