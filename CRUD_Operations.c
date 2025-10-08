#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define FILE_NAME "records.txt"   // File to store user records

// Structure to hold a user record
struct Record {
    int userId;
    char fullName[60];
    int userAge;
};

// Function to create a new record in the file
// If file doesn't exist, it will be created automatically
void addRecord() {
    struct Record userRecord;
    FILE *filePointer = fopen(FILE_NAME, "a");   // Open file in append mode

    if (filePointer == NULL) {
        printf("Error: Unable to open the file.\n");
        return;
    }

    printf("Enter User ID: ");
    scanf("%d", &userRecord.userId);
    getchar(); // consume newline

    printf("Enter Full Name: ");
    fgets(userRecord.fullName, sizeof(userRecord.fullName), stdin);
    userRecord.fullName[strcspn(userRecord.fullName, "\n")] = '\0';

    printf("Enter Age: ");
    scanf("%d", &userRecord.userAge);

    fprintf(filePointer, "%d,%s,%d\n", userRecord.userId, userRecord.fullName, userRecord.userAge);
    fclose(filePointer);

    printf("Record added successfully.\n");
}

// Function to display all records
void showAllRecords() {
    struct Record userRecord;
    FILE *filePointer = fopen(FILE_NAME, "r");

    if (filePointer == NULL) {
        printf("No data found in the file.\n");
        return;
    }

    printf("\n--- All User Records ---\n");
    printf("User ID\tName\t\tAge\n");

    while (fscanf(filePointer, "%d,%[^,],%d\n", &userRecord.userId, userRecord.fullName, &userRecord.userAge) != EOF) {
        printf("%d\t%-15s\t%d\n", userRecord.userId, userRecord.fullName, userRecord.userAge);
    }

    fclose(filePointer);
}

// Function to update a record by User ID
void updateRecordById() {
    int targetId, recordFound = 0;
    struct Record userRecord;
    FILE *filePointer = fopen(FILE_NAME, "r");
    FILE *tempFile = fopen("temp.txt", "w");

    if (!filePointer || !tempFile) {
        printf("Error: Unable to open file.\n");
        return;
    }

    printf("Enter User ID to update: ");
    scanf("%d", &targetId);
    getchar();

    while (fscanf(filePointer, "%d,%[^,],%d\n", &userRecord.userId, userRecord.fullName, &userRecord.userAge) != EOF) {
        if (userRecord.userId == targetId) {
            recordFound = 1;
            printf("Enter New Full Name: ");
            fgets(userRecord.fullName, sizeof(userRecord.fullName), stdin);
            userRecord.fullName[strcspn(userRecord.fullName, "\n")] = '\0';
            printf("Enter New Age: ");
            scanf("%d", &userRecord.userAge);
            getchar();
        }
        fprintf(tempFile, "%d,%s,%d\n", userRecord.userId, userRecord.fullName, userRecord.userAge);
    }

    fclose(filePointer);
    fclose(tempFile);

    remove(FILE_NAME);
    rename("temp.txt", FILE_NAME);

    if (recordFound)
        printf("Record updated successfully.\n");
    else
        printf("Record not found.\n");
}

// Function to remove a record by User ID
void removeRecordById() {
    int targetId, recordFound = 0;
    struct Record userRecord;
    FILE *filePointer = fopen(FILE_NAME, "r");
    FILE *tempFile = fopen("temp.txt", "w");

    if (!filePointer || !tempFile) {
        printf("Error: Unable to open file.\n");
        return;
    }

    printf("Enter User ID to delete: ");
    scanf("%d", &targetId);

    while (fscanf(filePointer, "%d,%[^,],%d\n", &userRecord.userId, userRecord.fullName, &userRecord.userAge) != EOF) {
        if (userRecord.userId == targetId) {
            recordFound = 1;
            continue;
        }
        fprintf(tempFile, "%d,%s,%d\n", userRecord.userId, userRecord.fullName, userRecord.userAge);
    }

    fclose(filePointer);
    fclose(tempFile);

    remove(FILE_NAME);
    rename("temp.txt", FILE_NAME);

    if (recordFound)
        printf("Record deleted successfully.\n");
    else
        printf("Record not found.\n");
}

// Main function — menu-driven program
int main() {
    int userChoice;

    while (1) {
        printf("\n--- MENU ---\n");
        printf("1. Add User\n");
        printf("2. Display All Users\n");
        printf("3. Update User\n");
        printf("4. Delete User\n");
        printf("5. Exit\n");
        printf("Enter your choice: ");
        scanf("%d", &userChoice);
        getchar();

        switch (userChoice) {
            case 1: addRecord(); break;
            case 2: showAllRecords(); break;
            case 3: updateRecordById(); break;
            case 4: removeRecordById(); break;
            case 5: exit(0);
            default: printf("Invalid choice. Try again.\n");
        }
    }

    return 0;
}
