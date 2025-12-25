#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 8080
pthread_mutex_t fileMutex;

void* handleClientRequest(void* socketDescriptor) {
    int clientSocket = *(int*)socketDescriptor;
    int choice, amount, balance;
    FILE *filePointer;

    pthread_mutex_lock(&fileMutex);
    filePointer = fopen("../accountDB.txt", "r");
    fscanf(filePointer, "%d", &balance);
    fclose(filePointer);
    pthread_mutex_unlock(&fileMutex);

    recv(clientSocket, &choice, sizeof(choice), 0);

    if (choice == 1 || choice == 2) {
        recv(clientSocket, &amount, sizeof(amount), 0);
    }

    pthread_mutex_lock(&fileMutex);

    if (choice == 1) {
        if (amount <= balance) balance -= amount;
        else amount = -1;
    } else if (choice == 2) {
        balance += amount;
    }

    filePointer = fopen("../accountDB.txt", "w");
    fprintf(filePointer, "%d", balance);
    fclose(filePointer);

    pthread_mutex_unlock(&fileMutex);

    send(clientSocket, &balance, sizeof(balance), 0);
    close(clientSocket);
    free(socketDescriptor);
    return NULL;
}

int main() {
    int serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in serverAddress;

    pthread_mutex_init(&fileMutex, NULL);

    serverAddress.sin_family = AF_INET;
    serverAddress.sin_addr.s_addr = INADDR_ANY;
    serverAddress.sin_port = htons(PORT);

    bind(serverSocket, (struct sockaddr*)&serverAddress, sizeof(serverAddress));
    listen(serverSocket, 5);

    printf("ATM Server Started...\n");

    while (1) {
        int clientSocket = accept(serverSocket, NULL, NULL);
        pthread_t clientThread;
        int *socketPtr = malloc(sizeof(int));
        *socketPtr = clientSocket;
        pthread_create(&clientThread, NULL, handleClientRequest, socketPtr);
    }

    return 0;
}
