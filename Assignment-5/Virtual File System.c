#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>

#define BLOCK_SIZE 512
#define DEFAULT_NUM_BLOCKS 1024
#define MAX_NAME_LEN 50

typedef struct FreeBlock {
    int blockIndex;
    struct FreeBlock *nextFree;
    struct FreeBlock *prevFree;
} FreeBlock;

typedef struct FileNode {
    char entryName[MAX_NAME_LEN + 1];
    int isDirectory;
    struct FileNode *parentDir;
    struct FileNode *nextSibling;
    struct FileNode *prevSibling;
    struct FileNode *firstChild;
    int *allocatedBlockArray;
    int blocksAllocated;
    long fileSizeBytes;
} FileNode;

static unsigned char *virtualDisk = NULL;
static int totalBlocks = DEFAULT_NUM_BLOCKS;
static int freeBlockCount = 0;
static FreeBlock *freeListHead = NULL;
static FreeBlock *freeListTail = NULL;
static FileNode *rootDirectory = NULL;
static FileNode *currentDirectory = NULL;

static void appendFreeBlockTail(int blockIndex) {
    FreeBlock *newFreeBlock = malloc(sizeof(FreeBlock));
    if (!newFreeBlock) { perror("malloc"); exit(EXIT_FAILURE); }
    newFreeBlock->blockIndex = blockIndex;
    newFreeBlock->nextFree = NULL;
    newFreeBlock->prevFree = NULL;
    if (!freeListHead) {
        freeListHead = freeListTail = newFreeBlock;
    } else {
        freeListTail->nextFree = newFreeBlock;
        newFreeBlock->prevFree = freeListTail;
        freeListTail = newFreeBlock;
    }
    freeBlockCount++;
}

static int allocateBlockFromHead() {
    if (!freeListHead) return -1;
    FreeBlock *allocatedFreeBlock = freeListHead;
    int allocatedIndex = allocatedFreeBlock->blockIndex;
    freeListHead = allocatedFreeBlock->nextFree;
    if (freeListHead) freeListHead->prevFree = NULL;
    else freeListTail = NULL;
    free(allocatedFreeBlock);
    freeBlockCount--;
    return allocatedIndex;
}

static void initializeFileSystem(int numBlocks) {
    totalBlocks = numBlocks;
    virtualDisk = malloc((size_t)totalBlocks * BLOCK_SIZE);
    if (!virtualDisk) { perror("malloc"); exit(EXIT_FAILURE); }
    memset(virtualDisk, 0, (size_t)totalBlocks * BLOCK_SIZE);
    freeListHead = freeListTail = NULL;
    freeBlockCount = 0;
    for (int blockIndex = 0; blockIndex < totalBlocks; ++blockIndex) appendFreeBlockTail(blockIndex);
    rootDirectory = malloc(sizeof(FileNode));
    if (!rootDirectory) { perror("malloc"); exit(EXIT_FAILURE); }
    memset(rootDirectory, 0, sizeof(FileNode));
    strcpy(rootDirectory->entryName, "/");
    rootDirectory->isDirectory = 1;
    rootDirectory->parentDir = NULL;
    rootDirectory->firstChild = NULL;
    rootDirectory->nextSibling = rootDirectory->prevSibling = NULL;
    rootDirectory->allocatedBlockArray = NULL;
    rootDirectory->blocksAllocated = 0;
    rootDirectory->fileSizeBytes = 0;
    currentDirectory = rootDirectory;
}

static FileNode *findChildByName(FileNode *dirNode, const char *searchName) {
    if (!dirNode || !dirNode->isDirectory) return NULL;
    FileNode *childHead = dirNode->firstChild;
    if (!childHead) return NULL;
    FileNode *currentChild = childHead;
    do {
        if (strcmp(currentChild->entryName, searchName) == 0) return currentChild;
        currentChild = currentChild->nextSibling;
    } while (currentChild != childHead);
    return NULL;
}

static void insertChild(FileNode *dirNode, FileNode *childNode) {
    if (!dirNode->firstChild) {
        dirNode->firstChild = childNode;
        childNode->nextSibling = childNode->prevSibling = childNode;
    } else {
        FileNode *childHead = dirNode->firstChild;
        FileNode *childTail = childHead->prevSibling;
        childTail->nextSibling = childNode;
        childNode->prevSibling = childTail;
        childNode->nextSibling = childHead;
        childHead->prevSibling = childNode;
    }
    childNode->parentDir = dirNode;
}

static void detachChild(FileNode *dirNode, FileNode *childNode) {
    if (!dirNode || !dirNode->firstChild || !childNode) return;
    FileNode *childHead = dirNode->firstChild;
    if (childNode->nextSibling == childNode) {
        dirNode->firstChild = NULL;
    } else {
        if (dirNode->firstChild == childNode) dirNode->firstChild = childNode->nextSibling;
        childNode->prevSibling->nextSibling = childNode->nextSibling;
        childNode->nextSibling->prevSibling = childNode->prevSibling;
    }
    childNode->nextSibling = childNode->prevSibling = NULL;
    childNode->parentDir = NULL;
}

static int *allocateBlocks(int numberOfBlocks) {
    if (numberOfBlocks <= 0) return NULL;
    if (freeBlockCount < numberOfBlocks) return NULL;
    int *allocatedBlocks = malloc(sizeof(int) * numberOfBlocks);
    if (!allocatedBlocks) { perror("malloc"); exit(EXIT_FAILURE); }
    for (int blockCounter = 0; blockCounter < numberOfBlocks; ++blockCounter) {
        int allocatedIndex = allocateBlockFromHead();
        if (allocatedIndex < 0) {
            free(allocatedBlocks);
            return NULL;
        }
        allocatedBlocks[blockCounter] = allocatedIndex;
    }
    return allocatedBlocks;
}

static void freeBlocksAppendTail(int *blocks, int count) {
    if (!blocks || count <= 0) return;
    for (int blockCounter = 0; blockCounter < count; ++blockCounter) {
        int freedBlockIndex = blocks[blockCounter];
        unsigned char *blockAddress = virtualDisk + ((size_t)freedBlockIndex * BLOCK_SIZE);
        memset(blockAddress, 0, BLOCK_SIZE);
        appendFreeBlockTail(freedBlockIndex);
    }
    free(blocks);
}

static void commandMkdir(const char *dirName) {
    if (!dirName || strlen(dirName) == 0) { printf("Invalid name.\n"); return; }
    if (strlen(dirName) > MAX_NAME_LEN) { printf("Name too long (max %d).\n", MAX_NAME_LEN); return; }
    if (findChildByName(currentDirectory, dirName)) {
        printf("Name already exists in current directory.\n");
        return;
    }
    FileNode *newNode = malloc(sizeof(FileNode));
    if (!newNode) { perror("malloc"); exit(EXIT_FAILURE); }
    memset(newNode, 0, sizeof(FileNode));
    strncpy(newNode->entryName, dirName, MAX_NAME_LEN);
    newNode->isDirectory = 1;
    newNode->firstChild = NULL;
    newNode->nextSibling = newNode->prevSibling = NULL;
    newNode->allocatedBlockArray = NULL;
    newNode->blocksAllocated = 0;
    newNode->fileSizeBytes = 0;
    insertChild(currentDirectory, newNode);
    printf("Directory '%s' created successfully.\n", dirName);
}

static void commandLs() {
    FileNode *childHead = currentDirectory->firstChild;
    if (!childHead) { printf("(empty)\n"); return; }
    FileNode *childIterator = childHead;
    do {
        if (childIterator->isDirectory) printf("%s/\n", childIterator->entryName);
        else printf("%s\n", childIterator->entryName);
        childIterator = childIterator->nextSibling;
    } while (childIterator != childHead);
}

static void commandPwd() {
    FileNode *cwdNode = currentDirectory;
    if (cwdNode->parentDir == NULL) {
        printf("/\n");
        return;
    }
    int pathDepth = 0;
    FileNode *walker = cwdNode;
    while (walker && walker->parentDir) { pathDepth++; walker = walker->parentDir; }
    char **pathParts = malloc(sizeof(char*) * pathDepth);
    walker = cwdNode;
    int partIndex = pathDepth - 1;
    while (walker && walker->parentDir) {
        pathParts[partIndex--] = walker->entryName;
        walker = walker->parentDir;
    }
    printf("/");
    for (int iterIndex = 0; iterIndex < pathDepth; ++iterIndex) {
        printf("%s", pathParts[iterIndex]);
        if (iterIndex + 1 < pathDepth) printf("/");
    }
    printf("\n");
    free(pathParts);
}

static void commandCd(const char *argName) {
    if (!argName || strlen(argName) == 0) { printf("Usage: cd <dir>\n"); return; }
    if (strcmp(argName, "/") == 0) {
        currentDirectory = rootDirectory;
        printf("Moved to /\n");
        return;
    }
    if (strcmp(argName, "..") == 0) {
        if (currentDirectory->parentDir) {
            currentDirectory = currentDirectory->parentDir;
            printf("Moved to ");
            commandPwd();
        } else {
            printf("Already at root.\n");
        }
        return;
    }
    FileNode *child = findChildByName(currentDirectory, argName);
    if (!child) {
        printf("Directory not found.\n");
        return;
    }
    if (!child->isDirectory) {
        printf("Target is not a directory.\n");
        return;
    }
    currentDirectory = child;
    printf("Moved to ");
    commandPwd();
}

static void commandCreate(const char *fileName) {
    if (!fileName || strlen(fileName) == 0) { printf("Invalid name.\n"); return; }
    if (strlen(fileName) > MAX_NAME_LEN) { printf("Name too long (max %d).\n", MAX_NAME_LEN); return; }
    if (findChildByName(currentDirectory, fileName)) {
        printf("Name already exists in current directory.\n");
        return;
    }
    FileNode *newNode = malloc(sizeof(FileNode));
    if (!newNode) { perror("malloc"); exit(EXIT_FAILURE); }
    memset(newNode, 0, sizeof(FileNode));
    strncpy(newNode->entryName, fileName, MAX_NAME_LEN);
    newNode->isDirectory = 0;
    newNode->firstChild = NULL;
    newNode->nextSibling = newNode->prevSibling = NULL;
    newNode->allocatedBlockArray = NULL;
    newNode->blocksAllocated = 0;
    newNode->fileSizeBytes = 0;
    insertChild(currentDirectory, newNode);
    printf("File '%s' created successfully.\n", fileName);
}

static int blocksNeededForSize(long sizeBytes) {
    if (sizeBytes <= 0) return 0;
    return (int)((sizeBytes + BLOCK_SIZE - 1) / BLOCK_SIZE);
}

static void commandWrite(const char *fileName, const char *content) {
    if (!fileName || !content) { printf("Usage: write <file> \"content\"\n"); return; }
    FileNode *node = findChildByName(currentDirectory, fileName);
    if (!node) { printf("File not found.\n"); return; }
    if (node->isDirectory) { printf("Target is a directory.\n"); return; }
    long newDataLen = (long)strlen(content);
    long newTotalSize = node->fileSizeBytes + newDataLen;
    int newTotalBlocks = blocksNeededForSize(newTotalSize);
    int existingBlocks = node->blocksAllocated;
    int additionalBlocksNeeded = newTotalBlocks - existingBlocks;
    if (additionalBlocksNeeded > 0) {
        if (freeBlockCount < additionalBlocksNeeded) {
            printf("Disk full. Cannot write file.\n");
            return;
        }
        int *newBlocks = allocateBlocks(additionalBlocksNeeded);
        if (!newBlocks) { printf("Allocation failed.\n"); return; }
        int *combinedBlocks = malloc(sizeof(int) * newTotalBlocks);
        if (!combinedBlocks) { perror("malloc"); exit(EXIT_FAILURE); }
        for (int copyIndex = 0; copyIndex < existingBlocks; ++copyIndex) combinedBlocks[copyIndex] = node->allocatedBlockArray[copyIndex];
        for (int appendIndex = 0; appendIndex < additionalBlocksNeeded; ++appendIndex) combinedBlocks[existingBlocks + appendIndex] = newBlocks[appendIndex];
        free(newBlocks);
        free(node->allocatedBlockArray);
        node->allocatedBlockArray = combinedBlocks;
        node->blocksAllocated = newTotalBlocks;
    } else if (newTotalBlocks == 0 && node->blocksAllocated > 0) {
    }
    long offsetInFile = node->fileSizeBytes;
    long remainingToWrite = newDataLen;
    const unsigned char *srcPtr = (const unsigned char *)content;
    while (remainingToWrite > 0) {
        int blockIndexInFile = (int)(offsetInFile / BLOCK_SIZE);
        int offsetInBlock = (int)(offsetInFile % BLOCK_SIZE);
        int diskBlockIndex = node->allocatedBlockArray[blockIndexInFile];
        unsigned char *dstPtr = virtualDisk + ((size_t)diskBlockIndex * BLOCK_SIZE) + offsetInBlock;
        int canWrite = (int) (BLOCK_SIZE - offsetInBlock);
        if (canWrite > remainingToWrite) canWrite = (int)remainingToWrite;
        memcpy(dstPtr, srcPtr, canWrite);
        srcPtr += canWrite;
        remainingToWrite -= canWrite;
        offsetInFile += canWrite;
    }
    node->fileSizeBytes = newTotalSize;
    printf("Data written successfully (size=%ld bytes).\n", newDataLen);
}

static void commandRead(const char *fileName) {
    if (!fileName) { printf("Usage: read <file>\n"); return; }
    FileNode *node = findChildByName(currentDirectory, fileName);
    if (!node) { printf("File not found.\n"); return; }
    if (node->isDirectory) { printf("Target is a directory.\n"); return; }
    if (node->fileSizeBytes == 0) { printf("(empty)\n"); return; }
    long remaining = node->fileSizeBytes;
    long offset = 0;
    while (remaining > 0) {
        int blockIndexInFile = (int)(offset / BLOCK_SIZE);
        int offsetInBlock = (int)(offset % BLOCK_SIZE);
        int diskBlockIndex = node->allocatedBlockArray[blockIndexInFile];
        unsigned char *srcPtr = virtualDisk + ((size_t)diskBlockIndex * BLOCK_SIZE) + offsetInBlock;
        int canRead = (int)(BLOCK_SIZE - offsetInBlock);
        if (canRead > remaining) canRead = (int)remaining;
        fwrite(srcPtr, 1, canRead, stdout);
        remaining -= canRead;
        offset += canRead;
    }
    printf("\n");
}

static void commandDelete(const char *fileName) {
    if (!fileName) { printf("Usage: delete <file>\n"); return; }
    FileNode *node = findChildByName(currentDirectory, fileName);
    if (!node) { printf("File not found.\n"); return; }
    if (node->isDirectory) { printf("Target is a directory.\n"); return; }
    detachChild(currentDirectory, node);
    if (node->allocatedBlockArray && node->blocksAllocated > 0) {
        freeBlocksAppendTail(node->allocatedBlockArray, node->blocksAllocated);
        node->allocatedBlockArray = NULL;
        node->blocksAllocated = 0;
        node->fileSizeBytes = 0;
    }
    free(node);
    printf("File deleted successfully.\n");
}

static void commandRmdir(const char *dirName) {
    if (!dirName) { printf("Usage: rmdir <dir>\n"); return; }
    FileNode *node = findChildByName(currentDirectory, dirName);
    if (!node) { printf("Directory not found.\n"); return; }
    if (!node->isDirectory) { printf("Target is not a directory.\n"); return; }
    if (node->firstChild) { printf("Directory not empty. Remove files first.\n"); return; }
    detachChild(currentDirectory, node);
    free(node);
    printf("Directory removed successfully.\n");
}

static void commandDf() {
    int used = totalBlocks - freeBlockCount;
    double usagePercent = (totalBlocks == 0) ? 0.0 : ((double)used * 100.0 / (double)totalBlocks);
    printf("Total Blocks: %d\n", totalBlocks);
    printf("Used Blocks: %d\n", used);
    printf("Free Blocks: %d\n", freeBlockCount);
    printf("Disk Usage: %.2f%%\n", usagePercent);
}

static void freeDirectoryRecursive(FileNode *dirNode) {
    if (!dirNode) return;
    FileNode *child = dirNode->firstChild;
    if (child) {
        FileNode *childHead = child;
        FileNode *iterator = childHead;
        do {
            FileNode *nextChild = iterator->nextSibling;
            if (iterator->isDirectory) freeDirectoryRecursive(iterator);
            else {
                if (iterator->allocatedBlockArray && iterator->blocksAllocated > 0) {
                    freeBlocksAppendTail(iterator->allocatedBlockArray, iterator->blocksAllocated);
                    iterator->allocatedBlockArray = NULL;
                }
                free(iterator);
            }
            iterator = nextChild;
        } while (iterator != childHead);
    }
    free(dirNode);
}

static void cleanupFileSystem() {
    FileNode *head = rootDirectory->firstChild;
    if (head) {
        FileNode *iterator = head;
        do {
            FileNode *nextChild = iterator->nextSibling;
            if (iterator->isDirectory) freeDirectoryRecursive(iterator);
            else {
                if (iterator->allocatedBlockArray && iterator->blocksAllocated > 0) {
                    freeBlocksAppendTail(iterator->allocatedBlockArray, iterator->blocksAllocated);
                }
                free(iterator);
            }
            iterator = nextChild;
        } while (iterator != head);
    }
    free(rootDirectory);
    rootDirectory = currentDirectory = NULL;
    FreeBlock *freeIterator = freeListHead;
    while (freeIterator) {
        FreeBlock *nextFree = freeIterator->nextFree;
        free(freeIterator);
        freeIterator = nextFree;
    }
    freeListHead = freeListTail = NULL;
    freeBlockCount = 0;
    free(virtualDisk);
    virtualDisk = NULL;
}

static void trimWhitespace(char *str) {
    if (!str) return;
    char *p = str;
    while (isspace((unsigned char)*p)) p++;
    if (p != str) memmove(str, p, strlen(p) + 1);
    size_t strLen = strlen(str);
    while (strLen > 0 && isspace((unsigned char)str[strLen - 1])) str[--strLen] = '\0';
}

static void processCommandLine(char *line) {
    trimWhitespace(line);
    if (strlen(line) == 0) return;
    char *cmd = strtok(line, " ");
    if (!cmd) return;
    if (strcmp(cmd, "mkdir") == 0) {
        char *dirName = strtok(NULL, " ");
        if (!dirName) { printf("Usage: mkdir <name>\n"); return; }
        commandMkdir(dirName);
    } else if (strcmp(cmd, "ls") == 0) {
        commandLs();
    } else if (strcmp(cmd, "pwd") == 0) {
        commandPwd();
    } else if (strcmp(cmd, "cd") == 0) {
        char *dirArg = strtok(NULL, " ");
        if (!dirArg) { printf("Usage: cd <dir>\n"); return; }
        commandCd(dirArg);
    } else if (strcmp(cmd, "create") == 0) {
        char *fileName = strtok(NULL, " ");
        if (!fileName) { printf("Usage: create <name>\n"); return; }
        commandCreate(fileName);
    } else if (strcmp(cmd, "write") == 0) {
        char *fileName = strtok(NULL, " ");
        if (!fileName) { printf("Usage: write <file> \"content\"\n"); return; }
        char *restOfLine = strtok(NULL, "");
        if (!restOfLine) { printf("Usage: write <file> \"content\"\n"); return; }
        trimWhitespace(restOfLine);
        char *contentStart = restOfLine;
        if (*contentStart == '"') {
            contentStart++;
            char *closingQuote = strrchr(contentStart, '"');
            if (closingQuote) *closingQuote = '\0';
        }
        commandWrite(fileName, contentStart);
    } else if (strcmp(cmd, "read") == 0) {
        char *fileName = strtok(NULL, " ");
        if (!fileName) { printf("Usage: read <file>\n"); return; }
        commandRead(fileName);
    } else if (strcmp(cmd, "delete") == 0) {
        char *fileName = strtok(NULL, " ");
        if (!fileName) { printf("Usage: delete <file>\n"); return; }
        commandDelete(fileName);
    } else if (strcmp(cmd, "rmdir") == 0) {
        char *dirName = strtok(NULL, " ");
        if (!dirName) { printf("Usage: rmdir <dir>\n"); return; }
        commandRmdir(dirName);
    } else if (strcmp(cmd, "df") == 0) {
        commandDf();
    } else if (strcmp(cmd, "exit") == 0) {
        cleanupFileSystem();
        printf("Memory released. Exiting program...\n");
        exit(EXIT_SUCCESS);
    } else {
        printf("Unknown command: %s\n", cmd);
    }
}

int main(int argc, char *argv[]) {
    int numBlocks = DEFAULT_NUM_BLOCKS;
    if (argc >= 2) {
        int parsedBlocks = atoi(argv[1]);
        if (parsedBlocks > 0 && parsedBlocks <= 5000) numBlocks = parsedBlocks;
        else {
            printf("Invalid NUM_BLOCKS argument, using default %d.\n", DEFAULT_NUM_BLOCKS);
        }
    }
    initializeFileSystem(numBlocks);
    printf("Compact VFS - ready. Type 'exit' to quit.\n");
    char inputLine[4096];
    while (1) {
        if (currentDirectory == rootDirectory) printf("/ > ");
        else {
            printf("%s > ", currentDirectory->entryName);
        }
        if (!fgets(inputLine, sizeof(inputLine), stdin)) {
            printf("\n");
            continue;
        }
        size_t lineLen = strlen(inputLine);
        if (lineLen > 0 && inputLine[lineLen-1] == '\n') inputLine[lineLen-1] = '\0';
        processCommandLine(inputLine);
    }
    return 0;
}
