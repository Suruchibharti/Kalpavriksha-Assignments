#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_VALUE_LENGTH 100
#define HASH_TABLE_SIZE 1009

typedef struct DoublyNode {
    int key;
    char value[MAX_VALUE_LENGTH];
    struct DoublyNode* prev;
    struct DoublyNode* next;
} DoublyNode;

typedef struct HashNode {
    int key;
    DoublyNode* listNode;
    struct HashNode* next;
} HashNode;

typedef struct LRUCache {
    int capacity;
    int currentSize;
    DoublyNode* head;
    DoublyNode* tail;
    HashNode* hashTable[HASH_TABLE_SIZE];
} LRUCache;

int getHashIndex(int key) {
    long long k = key;
    if (k < 0) k = -k;
    return (int)(k % HASH_TABLE_SIZE);
}

HashNode* findHashNode(LRUCache* cache, int key) {
    int index = getHashIndex(key);
    HashNode* current = cache->hashTable[index];
    while (current) {
        if (current->key == key) return current;
        current = current->next;
    }
    return NULL;
}

void insertHashNode(LRUCache* cache, int key, DoublyNode* listNode) {
    int index = getHashIndex(key);
    HashNode* newNode = malloc(sizeof(HashNode));
    if (!newNode) {
        printf("Memory allocation failed\n");
        exit(1);
    }
    newNode->key = key;
    newNode->listNode = listNode;
    newNode->next = cache->hashTable[index];
    cache->hashTable[index] = newNode;
}

void deleteHashNode(LRUCache* cache, int key) {
    int index = getHashIndex(key);
    HashNode* current = cache->hashTable[index];
    HashNode* prev = NULL;

    while (current) {
        if (current->key == key) {
            if (prev) prev->next = current->next;
            else cache->hashTable[index] = current->next;
            free(current);
            return;
        }
        prev = current;
        current = current->next;
    }
}

DoublyNode* createDoublyNode(int key, char* value) {
    DoublyNode* node = malloc(sizeof(DoublyNode));
    if (!node) {
        printf("Memory allocation failed\n");
        exit(1);
    }
    node->key = key;
    strcpy(node->value, value);
    node->prev = NULL;
    node->next = NULL;
    return node;
}

void moveToFront(LRUCache* cache, DoublyNode* node) {
    if (cache->head == node) return;

    if (node->prev) node->prev->next = node->next;
    if (node->next) node->next->prev = node->prev;

    if (cache->tail == node) cache->tail = node->prev;

    node->prev = NULL;
    node->next = cache->head;
    cache->head->prev = node;
    cache->head = node;
}

void addToFront(LRUCache* cache, DoublyNode* node) {
    if (!cache->head) {
        cache->head = node;
        cache->tail = node;
        return;
    }
    node->next = cache->head;
    cache->head->prev = node;
    cache->head = node;
}

void removeLeastUsed(LRUCache* cache) {
    if (!cache->tail) return;

    DoublyNode* node = cache->tail;
    deleteHashNode(cache, node->key);

    if (node->prev) {
        cache->tail = node->prev;
        cache->tail->next = NULL;
    } else {
        cache->head = NULL;
        cache->tail = NULL;
    }

    free(node);
    cache->currentSize--;
}

LRUCache* createCache(int capacity) {
    LRUCache* cache = malloc(sizeof(LRUCache));
    if (!cache) {
        printf("Memory allocation failed\n");
        exit(1);
    }
    cache->capacity = capacity;
    cache->currentSize = 0;
    cache->head = NULL;
    cache->tail = NULL;
    for (int i = 0; i < HASH_TABLE_SIZE; i++) {
        cache->hashTable[i] = NULL;
    }
    return cache;
}

void cachePut(LRUCache* cache, int key, char* value) {
    HashNode* hashNode = findHashNode(cache, key);

    if (hashNode) {
        strcpy(hashNode->listNode->value, value);
        moveToFront(cache, hashNode->listNode);
        return;
    }

    if (cache->currentSize == cache->capacity) removeLeastUsed(cache);

    DoublyNode* newNode = createDoublyNode(key, value);
    addToFront(cache, newNode);
    insertHashNode(cache, key, newNode);
    cache->currentSize++;
}

char* cacheGet(LRUCache* cache, int key) {
    HashNode* hashNode = findHashNode(cache, key);
    if (!hashNode) return NULL;
    moveToFront(cache, hashNode->listNode);
    return hashNode->listNode->value;
}

int isValidInteger(char* str) {
    if (str[0] == '-' && str[1] != '\0') str++;
    for (int i = 0; str[i]; i++) {
        if (str[i] < '0' || str[i] > '9') return 0;
    }
    return 1;
}

void freeCache(LRUCache* cache) {
    if (!cache) return;

    for (int i = 0; i < HASH_TABLE_SIZE; i++) {
        HashNode* current = cache->hashTable[i];
        while (current) {
            HashNode* nextNode = current->next;
            free(current);
            current = nextNode;
        }
        cache->hashTable[i] = NULL;
    }

    DoublyNode* cur = cache->head;
    while (cur) {
        DoublyNode* next = cur->next;
        free(cur);
        cur = next;
    }

    free(cache);
}

int main() {
    LRUCache* cache = NULL;
    char command[20], value[MAX_VALUE_LENGTH], keyStr[20], capacityStr[20];
    int key, capacity;
    int menuPrinted = 0;

    while (1) {

        if (!menuPrinted) {
            printf("\n=== LRU Cache ===\n");
            printf("Commands:\n");
            printf(" createCache <capacity>   : Initialize cache\n");
            printf(" put <key> <value>        : Insert/Update item\n");
            printf(" get <key>                : Retrieve item\n");
            printf(" exit                     : Close program\n");
            printf("==========================\n\n");
            menuPrinted = 1;
        }

        printf("Command> ");
        if (scanf("%19s", command) != 1) break;

        if (strcmp(command, "createCache") == 0) {
            if (scanf("%19s", capacityStr) != 1) {
                printf("Invalid capacity\n");
                continue;
            }

            if (!isValidInteger(capacityStr)) {
                printf("Invalid capacity\n");
                continue;
            }

            capacity = atoi(capacityStr);

            if (capacity < 1 || capacity > 1000) {
                printf("Capacity must be between 1 and 1000\n");
                continue;
            }

            if (cache) {
                freeCache(cache);
                cache = NULL;
            }

            cache = createCache(capacity);
            printf("Cache created\n");
        }

        else if (strcmp(command, "put") == 0) {
            if (!cache) {
                printf("Cache not created\n");
                continue;
            }

            if (scanf("%19s %99s", keyStr, value) != 2) {
                printf("Invalid put input\n");
                continue;
            }

            if (!isValidInteger(keyStr)) {
                printf("Invalid key\n");
                continue;
            }

            key = atoi(keyStr);
            cachePut(cache, key, value);
            printf("Inserted\n");
        }

        else if (strcmp(command, "get") == 0) {
            if (!cache) {
                printf("Cache not created\n");
                continue;
            }

            if (scanf("%19s", keyStr) != 1) {
                printf("Invalid get input\n");
                continue;
            }

            if (!isValidInteger(keyStr)) {
                printf("Invalid key\n");
                continue;
            }

            key = atoi(keyStr);
            char* result = cacheGet(cache, key);

            if (result) printf("%s\n", result);
            else printf("NULL\n");
        }

        else if (strcmp(command, "exit") == 0) {
            if (cache) freeCache(cache);
            break;
        }

        else {
            printf("Invalid command\n");
        }
    }

    return 0;
}
