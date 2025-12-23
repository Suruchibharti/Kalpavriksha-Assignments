#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MAX_NAME_LEN 64
#define HASH_SIZE 41
#define BUF 256

typedef struct Process {
    int pid;
    char name[MAX_NAME_LEN];
    int burstTotal;
    int ioStart;
    int ioTime;

    int burstLeft;
    int ioLeft;
    int cpuExecuted;
    int ioExecuted;
    int finishTick;
    int killTick;
    int killed;

    char state[16];

    struct Process *nextHash;
    struct Process *nextNode;
} Process;

typedef struct {
    Process *head;
    Process *tail;
    int count;
} Queue;

typedef struct KillEvent {
    int pid;
    int tick;
    struct KillEvent *next;
} KillEvent;

typedef struct {
    Process *bucket[HASH_SIZE];
} PidMap;

void initQueue(Queue *q) {
    q->head = q->tail = NULL;
    q->count = 0;
}

void pushQueue(Queue *q, Process *p) {
    p->nextNode = NULL;
    if (!q->tail) q->head = q->tail = p;
    else {
        q->tail->nextNode = p;
        q->tail = p;
    }
    q->count++;
}

Process* popQueue(Queue *q) {
    if (!q->head) return NULL;
    Process *p = q->head;
    q->head = p->nextNode;
    if (!q->head) q->tail = NULL;
    p->nextNode = NULL;
    q->count--;
    return p;
}

int removeFromQueue(Queue *q, int pid) {
    Process *cur = q->head, *prev = NULL;
    while (cur) {
        if (cur->pid == pid) {
            if (!prev) {
                q->head = cur->nextNode;
                if (!q->head) q->tail = NULL;
            } else {
                prev->nextNode = cur->nextNode;
                if (!prev->nextNode) q->tail = prev;
            }
            cur->nextNode = NULL;
            q->count--;
            return 1;
        }
        prev = cur;
        cur = cur->nextNode;
    }
    return 0;
}

void initMap(PidMap *m) {
    for (int i = 0; i < HASH_SIZE; i++) m->bucket[i] = NULL;
}

int hashCode(int pid) { return pid % HASH_SIZE; }

void insertMap(PidMap *m, Process *p) {
    int h = hashCode(p->pid);
    p->nextHash = m->bucket[h];
    m->bucket[h] = p;
}

Process* findMap(PidMap *m, int pid) {
    int h = hashCode(pid);
    Process *cur = m->bucket[h];
    while (cur) {
        if (cur->pid == pid) return cur;
        cur = cur->nextHash;
    }
    return NULL;
}

int validName(const char *s) {
    if (!s || !s[0]) return 0;
    for (int i = 0; s[i]; i++)
        if (!(isalnum(s[i]) || s[i] == '_')) return 0;
    return 1;
}

Process* newProcess(const char *name, int pid, int b, int ioS, int ioT) {
    Process *p = malloc(sizeof(Process));
    p->pid = pid;
    strncpy(p->name, name, MAX_NAME_LEN);
    p->burstTotal = b;
    p->ioStart = ioS;
    p->ioTime = ioT;

    p->burstLeft = b;
    p->ioLeft = ioT;
    p->cpuExecuted = 0;
    p->ioExecuted = 0;
    p->finishTick = -1;
    p->killTick = -1;
    p->killed = 0;
    strcpy(p->state, "OK");
    p->nextHash = NULL;
    p->nextNode = NULL;
    return p;
}

void addKill(KillEvent **head, int pid, int t) {
    KillEvent *k = malloc(sizeof(KillEvent));
    k->pid = pid;
    k->tick = t;
    k->next = *head;
    *head = k;
}

int main() {
    PidMap procMap;
    Queue readyQ, waitQ, doneQ;
    KillEvent *killList = NULL;

    initMap(&procMap);
    initQueue(&readyQ);
    initQueue(&waitQ);
    initQueue(&doneQ);

    char line[BUF];
    int n;

    printf("--- FCFS Process Scheduler ---\n");

    while (1) {
        printf("Enter number of processes (N): ");
        if (!fgets(line, BUF, stdin)) return 0;
        if (sscanf(line, "%d", &n) == 1 && n > 0) break;
        printf("Error: Please enter a positive integer.\n");
    }

    printf("Enter Process Details <process_name> <pid> <burst_time> <io_start_time> <io_duration> :\n");
    printf("Example: vscode 102 8 3 2\n");

    for (int i = 1; i <= n; i++) {
        while (1) {
            printf("Process %d: ", i);
            if (!fgets(line, BUF, stdin)) return 0;

            char nm[MAX_NAME_LEN], t1[32], t2[32], t3[32], t4[32];
            if (sscanf(line, "%63s %31s %31s %31s %31s", nm, t1, t2, t3, t4) != 5) {
                printf("Error: Invalid format. Expected 5 arguments.\n");
                continue;
            }

            if (!validName(nm)) {
                printf("Error: Process name must contain alphabets, digits or underscore.\n");
                continue;
            }

            int pid, br, ios, iod;
            char *end;

            pid = strtol(t1, &end, 10);
            if (*end || pid <= 0) { printf("Error: PID must be positive integer.\n"); continue; }

            br = strtol(t2, &end, 10);
            if (*end || br <= 0) { printf("Error: burst_time must be > 0.\n"); continue; }

            ios = strtol(t3, &end, 10);
            if (*end || ios < -1) { printf("Error: io_start must be -1 or >= 0.\n"); continue; }

            iod = strtol(t4, &end, 10);
            if (*end || iod < 0) { printf("Error: io_duration must be >= 0.\n"); continue; }

            if (findMap(&procMap, pid)) {
                printf("Error: PID %d already exists.\n", pid);
                continue;
            }

            Process *p = newProcess(nm, pid, br, ios, iod);
            insertMap(&procMap, p);
            pushQueue(&readyQ, p);
            break;
        }
    }

    int kills;
    while (1) {
        printf("Enter number of KILL commands (0 if none): ");
        if (!fgets(line, BUF, stdin)) return 0;
        if (sscanf(line, "%d", &kills) == 1 && kills >= 0) break;
        printf("Error: Enter non-negative integer.\n");
    }

    if (kills > 0) {
        printf("Enter Kill Details: <PID> <Time>\n");
        for (int i = 1; i <= kills; i++) {
            while (1) {
                int kp, kt;
                printf("Kill Cmd %d: ", i);
                if (!fgets(line, BUF, stdin)) return 0;
                if (sscanf(line, "%d %d", &kp, &kt) != 2 || kp <= 0 || kt < 0) {
                    printf("Error: Invalid input. Use: PID Time (positive values).\n");
                    continue;
                }
                if (!findMap(&procMap, kp)) {
                    printf("Error: PID %d does not exist in the process list.\n", kp);
                    continue;
                }
                addKill(&killList, kp, kt);
                break;
            }
        }
    }

    printf("\nStarting Simulation...\n");

    int tick = 0, finished = 0;
    Process *running = NULL;

    while (finished < n) {
        for (KillEvent *k = killList; k; k = k->next) {
            if (k->tick == tick) {
                Process *p = findMap(&procMap, k->pid);
                if (p && !p->killed && strcmp(p->state, "OK") == 0) {
                    p->killed = 1;
                    p->killTick = tick;
                    strcpy(p->state, "KILLED");
                    p->finishTick = tick;

                    if (running == p) running = NULL;
                    else if (removeFromQueue(&readyQ, p->pid));
                    else removeFromQueue(&waitQ, p->pid);

                    pushQueue(&doneQ, p);
                    finished++;
                }
            }
        }

        if (waitQ.head) {
            Queue nextW;
            initQueue(&nextW);
            while (waitQ.head) {
                Process *p = popQueue(&waitQ);
                if (!p->killed && p->ioLeft > 0) {
                    p->ioLeft--;
                    p->ioExecuted++;
                }
                if (p->ioLeft <= 0 && !p->killed) pushQueue(&readyQ, p);
                else if (!p->killed) pushQueue(&nextW, p);
            }
            waitQ = nextW;
        }

        if (running) {
            if (running->ioStart != -1 &&
                running->cpuExecuted == running->ioStart &&
                running->burstLeft > 0) {
                pushQueue(&waitQ, running);
                running = NULL;
            } else if (running->burstLeft == 0) {
                running->finishTick = tick;
                pushQueue(&doneQ, running);
                finished++;
                running = NULL;
            }
        }

        if (!running) running = popQueue(&readyQ);

        if (running) {
            running->burstLeft--;
            running->cpuExecuted++;
            if (running->burstLeft == 0) {
                running->finishTick = tick + 1;
                pushQueue(&doneQ, running);
                finished++;
                running = NULL;
            }
        }

        tick++;
    }

    Process *list[300];
    int lc = 0;

    for (int i = 0; i < HASH_SIZE; i++) {
        Process *p = procMap.bucket[i];
        while (p) {
            if (p->finishTick >= 0 || p->killed) list[lc++] = p;
            p = p->nextHash;
        }
    }

    for (int i = 0; i < lc; i++)
        for (int j = i + 1; j < lc; j++)
            if (list[i]->pid > list[j]->pid) {
                Process *t = list[i];
                list[i] = list[j];
                list[j] = t;
            }

    printf("\n--- Execution Summary ---\n");
    printf("%-10s %-15s %-10s %-10s %-15s %-15s %-15s\n",
           "PID", "Name", "CPU", "IO", "Status", "Turnaround", "Waiting");

    for (int i = 0; i < lc; i++) {
        Process *p = list[i];
        if (p->killed) {
            printf("%-10d %-15s %-10d %-10d %-15s %-15s %-15s\n",
                   p->pid, p->name, p->burstTotal, 0, "KILLED", "-", "-");
        } else {
            int tat = p->finishTick;
            int wt = tat - p->burstTotal - p->ioExecuted;
            if (wt < 0) wt = 0;
            printf("%-10d %-15s %-10d %-10d %-15s %-15d %-15d\n",
                   p->pid, p->name, p->burstTotal, p->ioExecuted,
                   "OK", tat, wt);
        }
    }

    for (int i = 0; i < HASH_SIZE; i++) {
        Process *p = procMap.bucket[i];
        while (p) {
            Process *tmp = p;
            p = p->nextHash;
            free(tmp);
        }
    }

    KillEvent *k = killList;
    while (k) {
        KillEvent *tmp = k;
        k = k->next;
        free(tmp);
    }

    return 0;
}
