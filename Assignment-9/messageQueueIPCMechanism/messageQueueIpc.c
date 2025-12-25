#include <stdio.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <unistd.h>

#define ARRAY_SIZE 5

struct messageBuffer {
    long messageType;
    int numbers[ARRAY_SIZE];
};

void sortArray(int arr[]) {
    int temp;
    for (int i = 0; i < ARRAY_SIZE; i++)
        for (int j = i + 1; j < ARRAY_SIZE; j++)
            if (arr[i] > arr[j]) {
                temp = arr[i];
                arr[i] = arr[j];
                arr[j] = temp;
            }
}

int main() {
    int messageQueueId = msgget(IPC_PRIVATE, 0666 | IPC_CREAT);
    struct messageBuffer message;

    pid_t processId = fork();

    if (processId == 0) {
        msgrcv(messageQueueId, &message, sizeof(message.numbers), 1, 0);
        sortArray(message.numbers);
        message.messageType = 2;
        msgsnd(messageQueueId, &message, sizeof(message.numbers), 0);
    } else {
        message.messageType = 1;

        printf("Enter %d integers:\n", ARRAY_SIZE);
        for (int i = 0; i < ARRAY_SIZE; i++)
            scanf("%d", &message.numbers[i]);

        printf("Before Sorting:\n");
        for (int i = 0; i < ARRAY_SIZE; i++)
            printf("%d ", message.numbers[i]);

        msgsnd(messageQueueId, &message, sizeof(message.numbers), 0);
        msgrcv(messageQueueId, &message, sizeof(message.numbers), 2, 0);

        printf("\nAfter Sorting:\n");
        for (int i = 0; i < ARRAY_SIZE; i++)
            printf("%d ", message.numbers[i]);

        msgctl(messageQueueId, IPC_RMID, NULL);
    }

    return 0;
}
