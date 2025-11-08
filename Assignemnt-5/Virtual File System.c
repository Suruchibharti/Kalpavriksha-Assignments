#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>

#define BLOCK_SIZE 512
#define DEFAULT_NUM_BLOCKS 1024
#define MAX_NAME_LEN 50

/* -------------------- Free block (doubly linked) -------------------- */
typedef struct FreeBlock {
    int blockIndex;                       // was 'index' - meaningful: index of block on virtual disk
    struct FreeBlock *nextFree;           // was 'next'
    struct FreeBlock *prevFree;           // was 'prev'
} FreeBlock;

/* -------------------- FileNode (file or directory) -------------------- */
typedef struct FileNode {
    char entryName[MAX_NAME_LEN + 1];     // was 'name' - meaningful: name of file or directory
    int isDirectory;                      // 1 = dir, 0 = file
    struct FileNode *parentDir;           // was 'parent'

    // siblings for circular doubly-linked list inside a directory
    struct FileNode *nextSibling;         // was 'next'
    struct FileNode *prevSibling;         // was 'prev'

    // if directory: pointer to first child (NULL if empty)
    struct FileNode *firstChild;          // was 'children'

    // if file:
    int *allocatedBlockArray;             // was 'blockPointers' - list of allocated block indices
    int blocksAllocated;                  // was 'blocksAllocated' - number of blocks allocated
    long fileSizeBytes;                   // was 'fileSizeBytes' - actual byte size of file content
} FileNode;

/* -------------------- Global Disk and Free List state -------------------- */
static unsigned char *virtualDisk = NULL; // single buffer: NUM_BLOCKS * BLOCK_SIZE
static int totalBlocks = DEFAULT_NUM_BLOCKS;
static int freeBlockCount = 0;
static FreeBlock *freeListHead = NULL;
static FreeBlock *freeListTail = NULL;

/* -------------------- FileSystem root and cwd -------------------- */
static FileNode *rootDirectory = NULL;
static FileNode *currentDirectory = NULL;

/* -------------------- Utility: safe strdup replacement -------------------- */
static char *strDupSafe(const char *inputString) {
    if (!inputString) return NULL;
    size_t inputLength = strlen(inputString) + 1;
    char *duplicateString = malloc(inputLength);
    if (!duplicateString) { perror("malloc"); exit(EXIT_FAILURE); }
    memcpy(duplicateString, inputString, inputLength);
    return duplicateString;
}

/* -------------------- Free list helpers -------------------- */
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

/* Initialize the virtual disk and free list */
static void initializeFileSystem(int numBlocks) {
    totalBlocks = numBlocks;
    virtualDisk = malloc((size_t)totalBlocks * BLOCK_SIZE);
    if (!virtualDisk) { perror("malloc"); exit(EXIT_FAILURE); }
    /* Initially zero (optional) */
    memset(virtualDisk, 0, (size_t)totalBlocks * BLOCK_SIZE);

    freeListHead = freeListTail = NULL;
    freeBlockCount = 0;
    for (int blockIndex = 0; blockIndex < totalBlocks; ++blockIndex) appendFreeBlockTail(blockIndex);

    /* Create root directory */
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

/* -------------------- Directory child (circular list) helpers -------------------- */
/* Find child by name (returns pointer or NULL) */
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

/* Insert child at end of circular list */
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

/* Remove child from circular list (but do not free) */
static void detachChild(FileNode *dirNode, FileNode *childNode) {
    if (!dirNode || !dirNode->firstChild || !childNode) return;
    FileNode *childHead = dirNode->firstChild;
    if (childNode->nextSibling == childNode) {
        /* only node */
        dirNode->firstChild = NULL;
    } else {
        if (dirNode->firstChild == childNode) dirNode->firstChild = childNode->nextSibling;
        childNode->prevSibling->nextSibling = childNode->nextSibling;
        childNode->nextSibling->prevSibling = childNode->prevSibling;
    }
    childNode->nextSibling = childNode->prevSibling = NULL;
    childNode->parentDir = NULL;
}

/* -------------------- Block allocation/free helpers for files -------------------- */
/* Allocate n blocks; return array of indices (malloced) or NULL on failure */
static int *allocateBlocks(int numberOfBlocks) {
    if (numberOfBlocks <= 0) return NULL;
    if (freeBlockCount < numberOfBlocks) return NULL;
    int *allocatedBlocks = malloc(sizeof(int) * numberOfBlocks);
    if (!allocatedBlocks) { perror("malloc"); exit(EXIT_FAILURE); }
    for (int blockCounter = 0; blockCounter < numberOfBlocks; ++blockCounter) {
        int allocatedIndex = allocateBlockFromHead();
        if (allocatedIndex < 0) {
            /* should not happen because we checked freeBlockCount */
            free(allocatedBlocks);
            return NULL;
        }
        allocatedBlocks[blockCounter] = allocatedIndex;
    }
    return allocatedBlocks;
}

/* Free an array of blocks by appending each to tail (order preserved) */
static void freeBlocksAppendTail(int *blocks, int count) {
    if (!blocks || count <= 0) return;
    for (int blockCounter = 0; blockCounter < count; ++blockCounter) {
        int freedBlockIndex = blocks[blockCounter];
        /* optional: clear disk content */
        unsigned char *blockAddress = virtualDisk + ((size_t)freedBlockIndex * BLOCK_SIZE);
        memset(blockAddress, 0, BLOCK_SIZE);
        appendFreeBlockTail(freedBlockIndex);
    }
    free(blocks);
}

/* -------------------- File/Directory operations -------------------- */
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

static void printPwdRecursive(FileNode *walkNode) {
    if (!walkNode) return;
    if (walkNode->parentDir == NULL) { /* root */
        printf("/");
        return;
    }
    printPwdRecursive(walkNode->parentDir);
    if (walkNode->parentDir->parentDir != NULL) printf("%s/", walkNode->entryName); /* if parent not root, show parent path? */
}

/* Simplified pwd: build path by walking parents */
static void commandPwd() {
    /* Build stack of names */
    FileNode *cwdNode = currentDirectory;
    if (cwdNode->parentDir == NULL) { /* root */
        printf("/\n");
        return;
    }
    /* Count depth */
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

/* cd command: supports .. and / and single-name child */
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

/* create file (empty) */
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

/* Helper to compute ceil division for blocks */
static int blocksNeededForSize(long sizeBytes) {
    if (sizeBytes <= 0) return 0;
    return (int)((sizeBytes + BLOCK_SIZE - 1) / BLOCK_SIZE);
}

/* Write (append) data to a file. content string is raw bytes; will append to existing file */
static void commandWrite(const char *fileName, const char *content) {
    if (!fileName || !content) { printf("Usage: write <file> \"content\"\n"); return; }
    FileNode *node = findChildByName(currentDirectory, fileName);
    if (!node) { printf("File not found.\n"); return; }
    if (node->isDirectory) { printf("Target is a directory.\n"); return; }

    long newDataLen = (long)strlen(content); /* treat as bytes (no special escapes) */
    long newTotalSize = node->fileSizeBytes + newDataLen;
    int newTotalBlocks = blocksNeededForSize(newTotalSize);
    int existingBlocks = node->blocksAllocated;
    int additionalBlocksNeeded = newTotalBlocks - existingBlocks;

    if (additionalBlocksNeeded > 0) {
        /* allocate additional blocks */
        if (freeBlockCount < additionalBlocksNeeded) {
            printf("Disk full. Cannot write file.\n");
            return;
        }
        int *newBlocks = allocateBlocks(additionalBlocksNeeded);
        if (!newBlocks) { printf("Allocation failed.\n"); return; }
        /* expand node->allocatedBlockArray array */
        int *combinedBlocks = malloc(sizeof(int) * newTotalBlocks);
        if (!combinedBlocks) { perror("malloc"); exit(EXIT_FAILURE); }
        /* copy existing */
        for (int copyIndex = 0; copyIndex < existingBlocks; ++copyIndex) combinedBlocks[copyIndex] = node->allocatedBlockArray[copyIndex];
        /* append new blocks */
        for (int appendIndex = 0; appendIndex < additionalBlocksNeeded; ++appendIndex) combinedBlocks[existingBlocks + appendIndex] = newBlocks[appendIndex];
        free(newBlocks);
        free(node->allocatedBlockArray);
        node->allocatedBlockArray = combinedBlocks;
        node->blocksAllocated = newTotalBlocks;
    } else if (newTotalBlocks == 0 && node->blocksAllocated > 0) {
        /* writing zero bytes? nothing to do */
    }

    /* Write content bytes into blocks, respecting existing content length (append) */
    long offsetInFile = node->fileSizeBytes; /* where to start appending */
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

/* Read file and print its contents (exact fileSizeBytes) */
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
        /* print chunk */
        fwrite(srcPtr, 1, canRead, stdout);
        remaining -= canRead;
        offset += canRead;
    }
    printf("\n");
}

/* Delete file: remove node from directory and return blocks to free list tail */
static void commandDelete(const char *fileName) {
    if (!fileName) { printf("Usage: delete <file>\n"); return; }
    FileNode *node = findChildByName(currentDirectory, fileName);
    if (!node) { printf("File not found.\n"); return; }
    if (node->isDirectory) { printf("Target is a directory.\n"); return; }
    /* detach from parent's children */
    detachChild(currentDirectory, node);
    /* free blocks */
    if (node->allocatedBlockArray && node->blocksAllocated > 0) {
        freeBlocksAppendTail(node->allocatedBlockArray, node->blocksAllocated);
        node->allocatedBlockArray = NULL;
        node->blocksAllocated = 0;
        node->fileSizeBytes = 0;
    }
    free(node);
    printf("File deleted successfully.\n");
}

/* rmdir: remove empty directory only */
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

/* df: print disk usage */
static void commandDf() {
    int used = totalBlocks - freeBlockCount;
    double usagePercent = (totalBlocks == 0) ? 0.0 : ((double)used * 100.0 / (double)totalBlocks);
    printf("Total Blocks: %d\n", totalBlocks);
    printf("Used Blocks: %d\n", used);
    printf("Free Blocks: %d\n", freeBlockCount);
    printf("Disk Usage: %.2f%%\n", usagePercent);
}

/* Cleanup: free all nodes recursively */
static void freeDirectoryRecursive(FileNode *dirNode) {
    if (!dirNode) return;
    /* free children */
    FileNode *child = dirNode->firstChild;
    if (child) {
        FileNode *childHead = child;
        FileNode *iterator = childHead;
        do {
            FileNode *nextChild = iterator->nextSibling;
            if (iterator->isDirectory) freeDirectoryRecursive(iterator);
            else {
                /* free file blocks */
                if (iterator->allocatedBlockArray && iterator->blocksAllocated > 0) {
                    freeBlocksAppendTail(iterator->allocatedBlockArray, iterator->blocksAllocated);
                    iterator->allocatedBlockArray = NULL;
                }
                free(iterator);
            }
            iterator = nextChild;
        } while (iterator != childHead);
    }
    /* free this directory node (except root might be freed outside) */
    free(dirNode);
}

/* Free entire FS resources gracefully */
static void cleanupFileSystem() {
    /* Free children of root */
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
    /* free root */
    free(rootDirectory);
    rootDirectory = currentDirectory = NULL;

    /* free any free list nodes */
    FreeBlock *freeIterator = freeListHead;
    while (freeIterator) {
        FreeBlock *nextFree = freeIterator->nextFree;
        free(freeIterator);
        freeIterator = nextFree;
    }
    freeListHead = freeListTail = NULL;
    freeBlockCount = 0;

    /* free virtual disk */
    free(virtualDisk);
    virtualDisk = NULL;
}

/* -------------------- Input parsing helpers -------------------- */
/* Trim leading/trailing spaces */
static void trimWhitespace(char *str) {
    if (!str) return;
    /* left */
    char *p = str;
    while (isspace((unsigned char)*p)) p++;
    if (p != str) memmove(str, p, strlen(p) + 1);
    /* right */
    size_t strLen = strlen(str);
    while (strLen > 0 && isspace((unsigned char)str[strLen - 1])) str[--strLen] = '\0';
}

/* Parse a command line; handle quoted content for write command */
static void processCommandLine(char *line) {
    trimWhitespace(line);
    if (strlen(line) == 0) return;
    /* tokenization: get first token (command) */
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
        /* expecting: write filename "content possibly with spaces" */
        char *fileName = strtok(NULL, " ");
        if (!fileName) { printf("Usage: write <file> \"content\"\n"); return; }
        char *restOfLine = strtok(NULL, ""); /* get rest of string including quotes if any */
        if (!restOfLine) { printf("Usage: write <file> \"content\"\n"); return; }
        trimWhitespace(restOfLine);
        char *contentStart = restOfLine;
        if (*contentStart == '"') {
            contentStart++;
            /* find closing quote */
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

/* -------------------- REPL -------------------- */
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
        /* Print prompt as current path */
        if (currentDirectory == rootDirectory) printf("/ > ");
        else {
            /* print last component of path */
            printf("%s > ", currentDirectory->entryName);
        }
        if (!fgets(inputLine, sizeof(inputLine), stdin)) {
            printf("\n");
            continue;
        }
        /* remove newline */
        size_t lineLen = strlen(inputLine);
        if (lineLen > 0 && inputLine[lineLen-1] == '\n') inputLine[lineLen-1] = '\0';
        processCommandLine(inputLine);
    }

    return 0;
}
