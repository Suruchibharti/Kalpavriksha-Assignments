#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define BLOCK_SIZE 512
#define DEFAULT_NUM_BLOCKS 1024
#define MAX_NAME_LENGTH 50
#define MAX_BLOCK_POINTERS 20

typedef struct FreeBlock {
    int blockIndex;
    struct FreeBlock *nextBlock;
    struct FreeBlock *previousBlock;
} FreeBlock;

typedef struct FileNode {
    char entryName[MAX_NAME_LENGTH];
    int isDirectory;
    struct FileNode *nextEntry;
    struct FileNode *previousEntry;
    struct FileNode *parentDirectory;
    struct FileNode *childEntry;
    int fileSize;
    int allocatedBlocksCount;
    int blockPointers[MAX_BLOCK_POINTERS];
} FileNode;

unsigned char **virtualDisk;
FreeBlock *freeBlockHead = NULL;
FreeBlock *freeBlockTail = NULL;
FileNode *rootDirectory = NULL;
FileNode *currentDirectory = NULL;
int totalBlockCount = 0;
int usedBlockCount = 0;

// Initializes virtual disk and root directory
void initializeFileSystem(int blockCount) {
    totalBlockCount = blockCount;
    usedBlockCount = 0;
    virtualDisk = malloc(blockCount * sizeof(unsigned char *));
    for (int blockIndex = 0; blockIndex < blockCount; blockIndex++) {
        virtualDisk[blockIndex] = calloc(BLOCK_SIZE, sizeof(unsigned char));
        FreeBlock *newBlock = malloc(sizeof(FreeBlock));
        newBlock->blockIndex = blockIndex;
        newBlock->nextBlock = NULL;
        newBlock->previousBlock = freeBlockTail;
        if (freeBlockTail) freeBlockTail->nextBlock = newBlock;
        else freeBlockHead = newBlock;
        freeBlockTail = newBlock;
    }

    rootDirectory = malloc(sizeof(FileNode));
    strcpy(rootDirectory->entryName, "/");
    rootDirectory->isDirectory = 1;
    rootDirectory->nextEntry = rootDirectory->previousEntry = NULL;
    rootDirectory->parentDirectory = NULL;
    rootDirectory->childEntry = NULL;
    currentDirectory = rootDirectory;
}

// Creates a new directory
void makeDirectory(const char *directoryName) {
    FileNode *iterator = currentDirectory->childEntry;
    if (iterator) {
        do {
            if (strcmp(iterator->entryName, directoryName) == 0) {
                printf("Name already exists in current directory.\n");
                return;
            }
            iterator = iterator->nextEntry;
        } while (iterator != currentDirectory->childEntry);
    }

    FileNode *newDirectory = malloc(sizeof(FileNode));
    strcpy(newDirectory->entryName, directoryName);
    newDirectory->isDirectory = 1;
    newDirectory->nextEntry = newDirectory->previousEntry = NULL;
    newDirectory->parentDirectory = currentDirectory;
    newDirectory->childEntry = NULL;

    if (!currentDirectory->childEntry) {
        currentDirectory->childEntry = newDirectory->nextEntry = newDirectory->previousEntry = newDirectory;
    } else {
        FileNode *tailEntry = currentDirectory->childEntry->previousEntry;
        tailEntry->nextEntry = newDirectory;
        newDirectory->previousEntry = tailEntry;
        newDirectory->nextEntry = currentDirectory->childEntry;
        currentDirectory->childEntry->previousEntry = newDirectory;
    }

    printf("Directory '%s' created successfully.\n", directoryName);
}

// Creates a new file
void createFile(const char *fileName) {
    FileNode *iterator = currentDirectory->childEntry;
    if (iterator) {
        do {
            if (strcmp(iterator->entryName, fileName) == 0) {
                printf("Name already exists in current directory.\n");
                return;
            }
            iterator = iterator->nextEntry;
        } while (iterator != currentDirectory->childEntry);
    }

    FileNode *newFile = malloc(sizeof(FileNode));
    strcpy(newFile->entryName, fileName);
    newFile->isDirectory = 0;
    newFile->nextEntry = newFile->previousEntry = NULL;
    newFile->parentDirectory = currentDirectory;
    newFile->childEntry = NULL;
    newFile->fileSize = 0;
    newFile->allocatedBlocksCount = 0;

    if (!currentDirectory->childEntry) {
        currentDirectory->childEntry = newFile->nextEntry = newFile->previousEntry = newFile;
    } else {
        FileNode *tailEntry = currentDirectory->childEntry->previousEntry;
        tailEntry->nextEntry = newFile;
        newFile->previousEntry = tailEntry;
        newFile->nextEntry = currentDirectory->childEntry;
        currentDirectory->childEntry->previousEntry = newFile;
    }

    printf("File '%s' created successfully.\n", fileName);
}

// Writes data to file
void writeFile(const char *fileName, const char *fileData) {
    FileNode *iterator = currentDirectory->childEntry;
    while (iterator && strcmp(iterator->entryName, fileName) != 0) {
        iterator = iterator->nextEntry;
        if (iterator == currentDirectory->childEntry) {
            iterator = NULL;
            break;
        }
    }
    if (!iterator) {
        printf("File not found.\n");
        return;
    }
    if (iterator->isDirectory) {
        printf("Cannot write to directory.\n");
        return;
    }

    int dataLength = strlen(fileData);
    int requiredBlocks = (dataLength + BLOCK_SIZE - 1) / BLOCK_SIZE;

    if (requiredBlocks > MAX_BLOCK_POINTERS) {
        printf("File too large to store.\n");
        return;
    }

    if (usedBlockCount + requiredBlocks > totalBlockCount) {
        printf("Not enough free space.\n");
        return;
    }

    for (int blockIndex = 0; blockIndex < requiredBlocks; blockIndex++) {
        if (!freeBlockHead) {
            printf("Disk full.\n");
            return;
        }
        FreeBlock *allocatedBlock = freeBlockHead;
        freeBlockHead = freeBlockHead->nextBlock;
        if (freeBlockHead) freeBlockHead->previousBlock = NULL;
        else freeBlockTail = NULL;

        int diskBlockIndex = allocatedBlock->blockIndex;
        iterator->blockPointers[iterator->allocatedBlocksCount++] = diskBlockIndex;

        int copyLength = (dataLength > BLOCK_SIZE) ? BLOCK_SIZE : dataLength;
        memcpy(virtualDisk[diskBlockIndex], fileData + blockIndex * BLOCK_SIZE, copyLength);
        dataLength -= copyLength;
        free(allocatedBlock);
        usedBlockCount++;
    }

    iterator->fileSize = strlen(fileData);
    printf("Data written successfully (size=%d bytes).\n", iterator->fileSize);
}

// Reads content of a file
void readFile(const char *fileName) {
    FileNode *iterator = currentDirectory->childEntry;
    while (iterator && strcmp(iterator->entryName, fileName) != 0) {
        iterator = iterator->nextEntry;
        if (iterator == currentDirectory->childEntry) {
            iterator = NULL;
            break;
        }
    }
    if (!iterator) {
        printf("File not found.\n");
        return;
    }
    if (iterator->isDirectory) {
        printf("Cannot read a directory.\n");
        return;
    }
    if (iterator->allocatedBlocksCount == 0) {
        printf("(empty)\n");
        return;
    }

    for (int blockIndex = 0; blockIndex < iterator->allocatedBlocksCount; blockIndex++) {
        printf("%s", virtualDisk[iterator->blockPointers[blockIndex]]);
    }
    printf("\n");
}

// Deletes a file
void deleteFile(const char *fileName) {
    FileNode *iterator = currentDirectory->childEntry;
    if (!iterator) {
        printf("File not found.\n");
        return;
    }

    do {
        if (!iterator->isDirectory && strcmp(iterator->entryName, fileName) == 0) {
            for (int blockIndex = 0; blockIndex < iterator->allocatedBlocksCount; blockIndex++) {
                int diskBlockIndex = iterator->blockPointers[blockIndex];
                FreeBlock *freedBlock = malloc(sizeof(FreeBlock));
                freedBlock->blockIndex = diskBlockIndex;
                freedBlock->nextBlock = NULL;
                freedBlock->previousBlock = freeBlockTail;
                if (freeBlockTail) freeBlockTail->nextBlock = freedBlock;
                else freeBlockHead = freedBlock;
                freeBlockTail = freedBlock;
                usedBlockCount--;
            }

            if (iterator->nextEntry == iterator) currentDirectory->childEntry = NULL;
            else {
                iterator->previousEntry->nextEntry = iterator->nextEntry;
                iterator->nextEntry->previousEntry = iterator->previousEntry;
                if (currentDirectory->childEntry == iterator)
                    currentDirectory->childEntry = iterator->nextEntry;
            }

            free(iterator);
            printf("File deleted successfully.\n");
            return;
        }
        iterator = iterator->nextEntry;
    } while (iterator != currentDirectory->childEntry);

    printf("File not found.\n");
}

// Removes an empty directory
void removeDirectory(const char *directoryName) {
    FileNode *iterator = currentDirectory->childEntry;
    if (!iterator) {
        printf("Directory not found.\n");
        return;
    }

    do {
        if (iterator->isDirectory && strcmp(iterator->entryName, directoryName) == 0) {
            if (iterator->childEntry) {
                printf("Directory not empty. Remove files first.\n");
                return;
            }

            if (iterator->nextEntry == iterator) currentDirectory->childEntry = NULL;
            else {
                iterator->previousEntry->nextEntry = iterator->nextEntry;
                iterator->nextEntry->previousEntry = iterator->previousEntry;
                if (currentDirectory->childEntry == iterator)
                    currentDirectory->childEntry = iterator->nextEntry;
            }

            free(iterator);
            printf("Directory removed successfully.\n");
            return;
        }
        iterator = iterator->nextEntry;
    } while (iterator != currentDirectory->childEntry);

    printf("Directory not found.\n");
}

// Lists contents of current directory
void listEntries() {
    if (!currentDirectory->childEntry) {
        printf("(empty)\n");
        return;
    }

    FileNode *iterator = currentDirectory->childEntry;
    do {
        printf("%s%s\n", iterator->entryName, iterator->isDirectory ? "/" : "");
        iterator = iterator->nextEntry;
    } while (iterator != currentDirectory->childEntry);
}

// Changes the current directory
void changeDirectory(const char *directoryName) {
    if (strcmp(directoryName, "..") == 0) {
        if (currentDirectory->parentDirectory) {
            currentDirectory = currentDirectory->parentDirectory;
            printf("Moved to %s\n", currentDirectory->entryName);
        } else printf("Already at root directory.\n");
        return;
    }

    FileNode *iterator = currentDirectory->childEntry;
    if (!iterator) {
        printf("Directory not found.\n");
        return;
    }

    do {
        if (iterator->isDirectory && strcmp(iterator->entryName, directoryName) == 0) {
            currentDirectory = iterator;
            printf("Moved to %s\n", directoryName);
            return;
        }
        iterator = iterator->nextEntry;
    } while (iterator != currentDirectory->childEntry);

    printf("Directory not found.\n");
}

// Displays current directory path
void showCurrentPath() {
    FileNode *iterator = currentDirectory;
    char path[1024] = "";
    while (iterator) {
        char temp[1024];
        sprintf(temp, "/%s%s", iterator->parentDirectory ? iterator->entryName : "", path);
        strcpy(path, temp);
        iterator = iterator->parentDirectory;
    }
    printf("%s\n", strlen(path) ? path : "/");
}

// Shows disk usage
void showDiskUsage() {
    int freeBlocks = totalBlockCount - usedBlockCount;
    double usedPercentage = ((double)usedBlockCount / totalBlockCount) * 100.0;
    printf("Total Blocks: %d\nUsed Blocks: %d\nFree Blocks: %d\nDisk Usage: %.2f%%\n",
           totalBlockCount, usedBlockCount, freeBlocks, usedPercentage);
}

// Process user commands
void processCommandLine(char *inputCommand) {
    char *command = strtok(inputCommand, " ");
    char *argument1 = strtok(NULL, " ");
    char *argument2 = strtok(NULL, "\0");

    if (!command) return;

    if (strcmp(command, "mkdir") == 0 && argument1) makeDirectory(argument1);
    else if (strcmp(command, "create") == 0 && argument1) createFile(argument1);
    else if (strcmp(command, "write") == 0 && argument1 && argument2) writeFile(argument1, argument2 + 1);
    else if (strcmp(command, "read") == 0 && argument1) readFile(argument1);
    else if (strcmp(command, "delete") == 0 && argument1) deleteFile(argument1);
    else if (strcmp(command, "rmdir") == 0 && argument1) removeDirectory(argument1);
    else if (strcmp(command, "ls") == 0) listEntries();
    else if (strcmp(command, "cd") == 0 && argument1) changeDirectory(argument1);
    else if (strcmp(command, "pwd") == 0) showCurrentPath();
    else if (strcmp(command, "df") == 0) showDiskUsage();
    else if (strcmp(command, "exit") == 0) {
        printf("Memory released. Exiting program...\n");
        exit(0);
    } else printf("Invalid command or missing argument.\n");
}

// Main Function
int main() {
    int numberOfBlocks = DEFAULT_NUM_BLOCKS;
    initializeFileSystem(numberOfBlocks);
    printf("Compact VFS - ready. Type 'exit' to quit.\n");

    char userInput[4096];
    while (1) {
        if (currentDirectory == rootDirectory) printf("/ > ");
        else printf("%s > ", currentDirectory->entryName);

        if (!fgets(userInput, sizeof(userInput), stdin)) {
            printf("\n");
            continue;
        }

        size_t lineLength = strlen(userInput);
        if (lineLength > 0 && userInput[lineLength - 1] == '\n')
            userInput[lineLength - 1] = '\0';

        processCommandLine(userInput);
    }
    return 0;
}
