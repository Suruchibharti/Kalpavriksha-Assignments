#include <stdio.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 8080

int main() {
    int clientSocket = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in serverAddress;
    int choice, amount, balance;

    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(PORT);
    serverAddress.sin_addr.s_addr = inet_addr("127.0.0.1");

    connect(clientSocket, (struct sockaddr*)&serverAddress, sizeof(serverAddress));

    printf("1. Withdraw\n2. Deposit\n3. Display Balance\n4. Exit\n");
    scanf("%d", &choice);
    send(clientSocket, &choice, sizeof(choice), 0);

    if (choice == 1 || choice == 2) {
        printf("Enter amount: ");
        scanf("%d", &amount);
        send(clientSocket, &amount, sizeof(amount), 0);
    }

    recv(clientSocket, &balance, sizeof(balance), 0);
    printf("Current Balance: %d\n", balance);

    close(clientSocket);
    return 0;
}
