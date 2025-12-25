#include <stdio.h>
#include <sys/shm.h>
#include <sys/wait.h>   
#include <unistd.h>

#define ARRAY_SIZE 5

void sortArray(int *sharedArray) {
    int temp;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        for (int j = i + 1; j < ARRAY_SIZE; j++) {
            if (sharedArray[i] > sharedArray[j]) {
                temp = sharedArray[i];
                sharedArray[i] = sharedArray[j];
                sharedArray[j] = temp;
            }
        }
    }
}

int main() {
    int sharedMemoryId = shmget(IPC_PRIVATE, ARRAY_SIZE * sizeof(int), 0666 | IPC_CREAT);
    int *sharedArray = (int *)shmat(sharedMemoryId, NULL, 0);

    pid_t processId = fork();

    if (processId == 0) {
        sortArray(sharedArray);
        shmdt(sharedArray);
    } else {
        printf("Enter %d numbers:\n", ARRAY_SIZE);
        for (int i = 0; i < ARRAY_SIZE; i++) {
            scanf("%d", &sharedArray[i]);
        }

        printf("Before Sorting:\n");
        for (int i = 0; i < ARRAY_SIZE; i++) {
            printf("%d ", sharedArray[i]);
        }

        wait(NULL);   

        printf("\nAfter Sorting:\n");
        for (int i = 0; i < ARRAY_SIZE; i++) {
            printf("%d ", sharedArray[i]);
        }

        shmdt(sharedArray);
        shmctl(sharedMemoryId, IPC_RMID, NULL);
    }

    return 0;
}
