#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "Players_data.h"

typedef enum { ROLE_BATSMAN = 1, ROLE_BOWLER = 2, ROLE_ALLROUNDER = 3 } RoleEnum;

typedef struct PlayerNode {
    int playerId;
    char playerName[64];
    char teamName[64];
    RoleEnum role;
    int totalRuns;
    float battingAverage;
    float strikeRate;
    int wickets;
    float economyRate;
    double performanceIndex;
    struct PlayerNode* nextPlayer; /* general chain (per team) */
    struct PlayerNode* nextRole;   /* role-specific chain (per team) */
} PlayerNode;

typedef struct TeamNode {
    int teamId;
    char teamName[64];
    PlayerNode* playerListHead;
    PlayerNode* batsmanHead;
    PlayerNode* bowlerHead;
    PlayerNode* allrounderHead;
    int totalPlayers;
    double averageBattingStrikeRate;
} TeamNode;

static inline double computePerformanceIndex(RoleEnum role, float battingAverage, float strikeRate, int wickets, float economyRate) {
    if (role == ROLE_BATSMAN) return (battingAverage * strikeRate) / 100.0;
    if (role == ROLE_BOWLER) return (wickets * 2.0) + (100.0 - economyRate);
    if (role == ROLE_ALLROUNDER) return ((battingAverage * strikeRate) / 100.0) + (wickets * 2.0);
    return 0.0;
}

int roleStringToEnum(const char* roleStr) {
    if (!roleStr) return 0;
    if (strcasecmp(roleStr, "Batsman") == 0 || strcasecmp(roleStr, "Batter") == 0) return ROLE_BATSMAN;
    if (strcasecmp(roleStr, "Bowler") == 0) return ROLE_BOWLER;
    if (strcasecmp(roleStr, "All-rounder") == 0 || strcasecmp(roleStr, "Allrounder") == 0) return ROLE_ALLROUNDER;
    return 0;
}

const char* roleEnumToString(RoleEnum player) {
    if (player == ROLE_BATSMAN) return "Batsman";
    if (player == ROLE_BOWLER) return "Bowler";
    if (player == ROLE_ALLROUNDER) return "All-rounder";
    return "Unknown";
}

PlayerNode* createPlayerNodeFromData(int playerId, const char* playerName, const char* teamName,
    RoleEnum role, int totalRuns, float battingAverage, float strikeRate, int wickets, float economyRate) {
    PlayerNode* node = (PlayerNode*)malloc(sizeof(PlayerNode));
    if (!node) { perror("malloc"); exit(EXIT_FAILURE); }
    node->playerId = playerId;
    strncpy(node->playerName, playerName ? playerName : "", sizeof(node->playerName)-1);
    node->playerName[sizeof(node->playerName)-1] = '\0';
    strncpy(node->teamName, teamName ? teamName : "", sizeof(node->teamName)-1);
    node->teamName[sizeof(node->teamName)-1] = '\0';
    node->role = role;
    node->totalRuns = totalRuns;
    node->battingAverage = battingAverage;
    node->strikeRate = strikeRate;
    node->wickets = wickets;
    node->economyRate = economyRate;
    node->performanceIndex = computePerformanceIndex(role, battingAverage, strikeRate, wickets, economyRate);
    node->nextPlayer = NULL;
    node->nextRole = NULL;
    return node;
}

void appendPlayerToTeamList(TeamNode* team, PlayerNode* playerNode) {
    if (!team->playerListHead) team->playerListHead = playerNode;
    else {
        PlayerNode* it = team->playerListHead;
        while (it->nextPlayer) it = it->nextPlayer;
        it->nextPlayer = playerNode;
    }
    team->totalPlayers++;
}

int cmpPlayerNodePtrDesc(const void* a, const void* b) {
    const PlayerNode* pa = *(const PlayerNode* const*)a;
    const PlayerNode* pb = *(const PlayerNode* const*)b;
    if (pa->performanceIndex < pb->performanceIndex) return 1;
    if (pa->performanceIndex > pb->performanceIndex) return -1;
    return 0;
}

TeamNode* initializeTeamsFromHeader() {
    TeamNode* teamsArray = (TeamNode*)malloc(sizeof(TeamNode) * teamCount);
    if (!teamsArray) { perror("malloc"); exit(EXIT_FAILURE); }
    for (int i = 0; i < teamCount; ++i) {
        teamsArray[i].teamId = i + 1;
        strncpy(teamsArray[i].teamName, teams[i], sizeof(teamsArray[i].teamName)-1);
        teamsArray[i].teamName[sizeof(teamsArray[i].teamName)-1] = '\0';
        teamsArray[i].playerListHead = NULL;
        teamsArray[i].batsmanHead = NULL;
        teamsArray[i].bowlerHead = NULL;
        teamsArray[i].allrounderHead = NULL;
        teamsArray[i].totalPlayers = 0;
        teamsArray[i].averageBattingStrikeRate = 0.0;
    }
    return teamsArray;
}

TeamNode* getTeamById(TeamNode* teamsArray, TeamNode** teamsSortedById, int tcount, int teamId) {
    if (teamId < 1 || teamId > tcount) return NULL;
    return &teamsArray[teamId - 1];
}

void buildPerRoleSortedLists(TeamNode* teamsArray) {
    for (int ti = 0; ti < teamCount; ++ti) {
        PlayerNode* it = teamsArray[ti].playerListHead;
        PlayerNode* batsArr[256]; int bi = 0;
        PlayerNode* bowlsArr[256]; int wi = 0;
        PlayerNode* allsArr[256]; int ai = 0;
        while (it) {
            it->nextRole = NULL;
            if (it->role == ROLE_BATSMAN && bi < 256) batsArr[bi++] = it;
            else if (it->role == ROLE_BOWLER && wi < 256) bowlsArr[wi++] = it;
            else if (it->role == ROLE_ALLROUNDER && ai < 256) allsArr[ai++] = it;
            it = it->nextPlayer;
        }
        if (bi > 0) qsort(batsArr, bi, sizeof(PlayerNode*), cmpPlayerNodePtrDesc);
        if (wi > 0) qsort(bowlsArr, wi, sizeof(PlayerNode*), cmpPlayerNodePtrDesc);
        if (ai > 0) qsort(allsArr, ai, sizeof(PlayerNode*), cmpPlayerNodePtrDesc);

        teamsArray[ti].batsmanHead = NULL;
        if (bi > 0) {
            teamsArray[ti].batsmanHead = batsArr[0];
            PlayerNode* cur = teamsArray[ti].batsmanHead;
            for (int i = 1; i < bi; ++i) { cur->nextRole = batsArr[i]; cur = cur->nextRole; }
            cur->nextRole = NULL;
        }
        teamsArray[ti].bowlerHead = NULL;
        if (wi > 0) {
            teamsArray[ti].bowlerHead = bowlsArr[0];
            PlayerNode* cur = teamsArray[ti].bowlerHead;
            for (int i = 1; i < wi; ++i) { cur->nextRole = bowlsArr[i]; cur = cur->nextRole; }
            cur->nextRole = NULL;
        }
        teamsArray[ti].allrounderHead = NULL;
        if (ai > 0) {
            teamsArray[ti].allrounderHead = allsArr[0];
            PlayerNode* cur = teamsArray[ti].allrounderHead;
            for (int i = 1; i < ai; ++i) { cur->nextRole = allsArr[i]; cur = cur->nextRole; }
            cur->nextRole = NULL;
        }
    }
}

void computeTeamAverageStrikeRates(TeamNode* teamsArray) {
    for (int i = 0; i < teamCount; ++i) {
        double sumStrike = 0.0; int cnt = 0;
        PlayerNode* it = teamsArray[i].playerListHead;
        while (it) {
            if (it->role == ROLE_BATSMAN || it->role == ROLE_ALLROUNDER) { sumStrike += it->strikeRate; ++cnt; }
            it = it->nextPlayer;
        }
        teamsArray[i].averageBattingStrikeRate = (cnt > 0) ? (sumStrike / cnt) : 0.0;
    }
}

void printTeamPlayers(TeamNode* team) {
    printf("\nPlayers of Team %s (TeamID=%d):\n", team->teamName, team->teamId);
    printf("====================================================================================\n");
    printf("ID\tName\t\tRole\tRuns\tAvg\tSR\tWkts\tER\tPerf.Index\n");
    printf("====================================================================================\n");
    PlayerNode* it = team->playerListHead;
    while (it) {
        printf("%d\t%-16s\t%-11s\t%d\t%.2f\t%.2f\t%d\t%.2f\t%.2f\n",
               it->playerId, it->playerName, roleEnumToString(it->role),
               it->totalRuns, it->battingAverage, it->strikeRate, it->wickets, it->economyRate, it->performanceIndex);
        it = it->nextPlayer;
    }
    printf("====================================================================================\n");
    printf("Total Players: %d\n", team->totalPlayers);
    printf("Average Batting Strike Rate: %.2f\n", team->averageBattingStrikeRate);
}

void displayTopKPlayersOfTeamByRole(TeamNode* team, RoleEnum role, int K) {
    if (!team) return;
    PlayerNode* it = (role == ROLE_BATSMAN) ? team->batsmanHead : (role == ROLE_BOWLER ? team->bowlerHead : team->allrounderHead);
    if (!it) { printf("No players of role %s in team %s\n", roleEnumToString(role), team->teamName); return; }
    printf("\nTop %d %s of Team %s (TeamID=%d):\n", K, roleEnumToString(role), team->teamName, team->teamId);
    printf("====================================================================================\n");
    printf("ID\tName\t\tRole\tRuns\tAvg\tSR\tWkts\tER\tPerf.Index\n");
    printf("====================================================================================\n");
    int cnt = 0;
    while (it && cnt < K) {
        printf("%d\t%-16s\t%-11s\t%d\t%.2f\t%.2f\t%d\t%.2f\t%.2f\n",
               it->playerId, it->playerName, roleEnumToString(it->role),
               it->totalRuns, it->battingAverage, it->strikeRate, it->wickets, it->economyRate, it->performanceIndex);
        it = it->nextRole; ++cnt;
    }
}

typedef struct HeapNode {
    PlayerNode* node;
    int teamIndex;
} HeapNode;

void heapSwap(HeapNode* a, HeapNode* b) { HeapNode tmp = *a; *a = *b; *b = tmp; }

void heapifyDown(HeapNode heap[], int size, int idx) {
    int largest = idx;
    int l = 2*idx + 1, r = 2*idx + 2;
    if (l < size && heap[l].node->performanceIndex > heap[largest].node->performanceIndex) largest = l;
    if (r < size && heap[r].node->performanceIndex > heap[largest].node->performanceIndex) largest = r;
    if (largest != idx) { heapSwap(&heap[idx], &heap[largest]); heapifyDown(heap, size, largest); }
}

void heapifyUp(HeapNode heap[], int idx) {
    while (idx > 0) {
        int p = (idx - 1) / 2;
        if (heap[p].node->performanceIndex < heap[idx].node->performanceIndex) { heapSwap(&heap[p], &heap[idx]); idx = p; }
        else break;
    }
}

void heapPush(HeapNode heap[], int* size, HeapNode val) { heap[*size] = val; heapifyUp(heap, *size); (*size)++; }

HeapNode heapPop(HeapNode heap[], int* size) {
    HeapNode top = heap[0];
    (*size)--;
    if (*size > 0) { heap[0] = heap[*size]; heapifyDown(heap, *size, 0); }
    return top;
}

void displayAllPlayersByRoleAcrossTeams(TeamNode* teamsArray, RoleEnum role) {
    HeapNode heap[128]; int heapSize = 0;
    for (int i = 0; i < teamCount; ++i) {
        PlayerNode* head = (role == ROLE_BATSMAN) ? teamsArray[i].batsmanHead : (role == ROLE_BOWLER ? teamsArray[i].bowlerHead : teamsArray[i].allrounderHead);
        if (head) { HeapNode hn = { .node = head, .teamIndex = i }; heapPush(heap, &heapSize, hn); }
    }
    if (heapSize == 0) { printf("No players of role %s across teams\n", roleEnumToString(role)); return; }
    printf("\nAll players (Role: %s) across teams sorted by Performance Index:\n", roleEnumToString(role));
    printf("====================================================================================\n");
    printf("ID\tName\tTeam\tRole\tRuns\tAvg\tSR\tWkts\tER\tPerf.Index\n");
    printf("====================================================================================\n");
    while (heapSize > 0) {
        HeapNode top = heapPop(heap, &heapSize);
        PlayerNode* p = top.node;
        printf("%d\t%-16s\t%-12s\t%-11s\t%d\t%.2f\t%.2f\t%d\t%.2f\t%.2f\n",
               p->playerId, p->playerName, p->teamName, roleEnumToString(p->role),
               p->totalRuns, p->battingAverage, p->strikeRate, p->wickets, p->economyRate, p->performanceIndex);
        PlayerNode* next = p->nextRole;
        if (next) { HeapNode hn = { .node = next, .teamIndex = top.teamIndex }; heapPush(heap, &heapSize, hn); }
    }
}

void insertIntoRoleListSorted(PlayerNode** headRef, PlayerNode* node) {
    if (!headRef || !node) return;
    node->nextRole = NULL;
    if (!(*headRef) || node->performanceIndex > (*headRef)->performanceIndex) {
        node->nextRole = *headRef; *headRef = node; return;
    }
    PlayerNode* it = *headRef;
    while (it->nextRole && it->nextRole->performanceIndex >= node->performanceIndex) it = it->nextRole;
    node->nextRole = it->nextRole; it->nextRole = node;
}

int addPlayerInteractive(TeamNode* teamsArray, TeamNode** teamsSortedById, int* idValid) {
    char buffer[256], nameBuf[128]; int teamId, playerId, roleSel, totalRuns; float battingAverage, strikeRate, economyRate; int wickets; char extra;
    while (1) {
        printf("Enter Team ID to add player (1-%d): ", teamCount);
        if (!fgets(buffer, sizeof(buffer), stdin)) return -1;
        if (sscanf(buffer, "%d %c", &teamId, &extra) == 1 && teamId >= 1 && teamId <= teamCount) break;
        fprintf(stderr, "Please enter a valid Team ID (1-%d)\n", teamCount);
    }
    TeamNode* targetTeam = getTeamById(teamsArray, teamsSortedById, teamCount, teamId);
    if (!targetTeam) { fprintf(stderr, "Invalid team selected\n"); return -1; }

    while (1) {
        printf("Player ID (1-1000): ");
        if (!fgets(buffer, sizeof(buffer), stdin)) return -1;
        if (sscanf(buffer, "%d %c", &playerId, &extra) == 1 && playerId >= 1 && playerId <= 1000) {
            if (!idValid[playerId]) break; else fprintf(stderr, "Entered ID is already taken\n");
        } else fprintf(stderr, "Please enter a valid Player ID (1-1000)\n");
    }
    idValid[playerId] = 1;

    while (1) {
        printf("Name (1-50 chars): ");
        if (!fgets(nameBuf, sizeof(nameBuf), stdin)) return -1;
        size_t ln = strlen(nameBuf);
        if (ln > 0 && nameBuf[ln-1] == '\n') { nameBuf[ln-1] = '\0'; ln--; }
        if (ln >= 1 && ln <= 50) break;
        fprintf(stderr, "Please enter a valid name (1-50 characters)\n");
    }

    while (1) {
        printf("Role (1-Batsman, 2-Bowler, 3-All-rounder): ");
        if (!fgets(buffer, sizeof(buffer), stdin)) return -1;
        if (sscanf(buffer, "%d %c", &roleSel, &extra) == 1 && roleSel >= 1 && roleSel <= 3) break;
        fprintf(stderr, "Please enter 1, 2 or 3 for role\n");
    }
    while (1) {
        printf("Total Runs: ");
        if (!fgets(buffer, sizeof(buffer), stdin)) return -1;
        if (sscanf(buffer, "%d %c", &totalRuns, &extra) == 1 && totalRuns >= 0) break;
        fprintf(stderr, "Please enter a non-negative integer for Total Runs\n");
    }
    while (1) {
        printf("Batting Average: ");
        if (!fgets(buffer, sizeof(buffer), stdin)) return -1;
        if (sscanf(buffer, "%f %c", &battingAverage, &extra) == 1 && battingAverage >= 0.0f) break;
        fprintf(stderr, "Please enter a non-negative number for Batting Average\n");
    }
    while (1) {
        printf("Strike Rate: ");
        if (!fgets(buffer, sizeof(buffer), stdin)) return -1;
        if (sscanf(buffer, "%f %c", &strikeRate, &extra) == 1 && strikeRate >= 0.0f) break;
        fprintf(stderr, "Please enter a non-negative number for Strike Rate\n");
    }
    while (1) {
        printf("Wickets: ");
        if (!fgets(buffer, sizeof(buffer), stdin)) return -1;
        if (sscanf(buffer, "%d %c", &wickets, &extra) == 1 && wickets >= 0) break;
        fprintf(stderr, "Please enter a non-negative integer for Wickets\n");
    }
    while (1) {
        printf("Economy Rate: ");
        if (!fgets(buffer, sizeof(buffer), stdin)) return -1;
        if (sscanf(buffer, "%f %c", &economyRate, &extra) == 1 && economyRate >= 0.0f) break;
        fprintf(stderr, "Please enter a non-negative number for Economy Rate\n");
    }

    RoleEnum chosenRole = (roleSel == 1 ? ROLE_BATSMAN : (roleSel == 2 ? ROLE_BOWLER : ROLE_ALLROUNDER));
    PlayerNode* newNode = createPlayerNodeFromData(playerId, nameBuf, targetTeam->teamName, chosenRole, totalRuns, battingAverage, strikeRate, wickets, economyRate);

    appendPlayerToTeamList(targetTeam, newNode);

    if (chosenRole == ROLE_BATSMAN) insertIntoRoleListSorted(&targetTeam->batsmanHead, newNode);
    else if (chosenRole == ROLE_BOWLER) insertIntoRoleListSorted(&targetTeam->bowlerHead, newNode);
    else insertIntoRoleListSorted(&targetTeam->allrounderHead, newNode);

    double sumStrike = 0.0; int cnt = 0; PlayerNode* it = targetTeam->playerListHead;
    while (it) { if (it->role == ROLE_BATSMAN || it->role == ROLE_ALLROUNDER) { sumStrike += it->strikeRate; ++cnt; } it = it->nextPlayer; }
    targetTeam->averageBattingStrikeRate = (cnt > 0) ? (sumStrike / cnt) : 0.0;

    printf("Player added successfully to Team %s (TeamID=%d)!\n", targetTeam->teamName, targetTeam->teamId);
    return 0;
}

int getIntegerInputValidatedSimple(const char* prompt, int minAllowed, int maxAllowed) {
    char buf[128]; int value; char extra;
    while (1) {
        printf("%s", prompt);
        if (!fgets(buf, sizeof(buf), stdin)) continue;
        if (sscanf(buf, "%d %c", &value, &extra) == 1) {
            if (value >= minAllowed && value <= maxAllowed) return value;
        }
        printf("Input must be an integer between %d and %d\n", minAllowed, maxAllowed);
    }
}

int compareTeamsByAvgStrikeDescPtr(const void* a, const void* b) {
    const TeamNode* ta = *(const TeamNode* const*)a;
    const TeamNode* tb = *(const TeamNode* const*)b;
    if (ta->averageBattingStrikeRate < tb->averageBattingStrikeRate) return 1;
    if (ta->averageBattingStrikeRate > tb->averageBattingStrikeRate) return -1;
    return 0;
}

int main(void) {
    TeamNode* teamsArray = initializeTeamsFromHeader();
    TeamNode** teamsSortedById = (TeamNode**)malloc(sizeof(TeamNode*) * teamCount);
    if (!teamsSortedById) { perror("malloc"); exit(EXIT_FAILURE); }
    for (int i = 0; i < teamCount; ++i) teamsSortedById[i] = &teamsArray[i];

    for (int i = 0; i < teamCount - 1; ++i) for (int j = i + 1; j < teamCount; ++j)
        if (teamsSortedById[i]->teamId > teamsSortedById[j]->teamId) { TeamNode* t = teamsSortedById[i]; teamsSortedById[i] = teamsSortedById[j]; teamsSortedById[j] = t; }

    for (int i = 0; i < playerCount; ++i) {
        const Player* p = &players[i];
        RoleEnum r = roleStringToEnum(p->role ? p->role : "");
        PlayerNode* node = createPlayerNodeFromData(p->id, p->name, p->team, r, p->totalRuns, p->battingAverage, p->strikeRate, p->wickets, p->economyRate);
        int found = -1;
        for (int t = 0; t < teamCount; ++t) if (strcmp(teamsArray[t].teamName, p->team) == 0) { found = t; break; }
        if (found >= 0) appendPlayerToTeamList(&teamsArray[found], node);
        else free(node);
    }

    buildPerRoleSortedLists(teamsArray);
    computeTeamAverageStrikeRates(teamsArray);

    int idValid[1001]; memset(idValid, 0, sizeof(idValid));
    for (int i = 0; i < playerCount; ++i) { int pid = players[i].id; if (pid >= 1 && pid <= 1000) idValid[pid] = 1; }

    while (1) {
        printf("========================================\n");
        printf("  ICC ODI Player Performance Analyzer\n");
        printf("========================================\n");
        printf("1. Add Player to team\n");
        printf("2. Display Players of a Specific Team\n");
        printf("3. Display Teams by Average Batting Strike Rate\n");
        printf("4. Display Top K Players of a Specific Team by Role\n");
        printf("5. Display all Players of specific role Across All Teams by performance index\n");
        printf("6. Exit\n");
        printf("Enter Your Choice: ");
        int choice;
        if (scanf("%d", &choice) != 1) { while (getchar() != '\n'); printf("Invalid choice\n"); continue; }
        while (getchar() != '\n');

        if (choice == 1) { addPlayerInteractive(teamsArray, teamsSortedById, idValid); }
        else if (choice == 2) {
            int teamId = getIntegerInputValidatedSimple("Enter Team ID: ", 1, teamCount);
            TeamNode* t = getTeamById(teamsArray, teamsSortedById, teamCount, teamId);
            if (!t) { printf("Team not found\n"); continue; }
            computeTeamAverageStrikeRates(teamsArray);
            printTeamPlayers(t);
        }
        else if (choice == 3) {
            computeTeamAverageStrikeRates(teamsArray);
            TeamNode* arr[128];
            for (int i = 0; i < teamCount; ++i) arr[i] = &teamsArray[i];
            qsort(arr, teamCount, sizeof(TeamNode*), compareTeamsByAvgStrikeDescPtr);
            printf("\nTeams Sorted by Average Batting Strike Rate\n");
            printf("=========================================================\n");
            printf("ID\tTeam Name\t\tAvg Bat SR\tTotal Players\n");
            printf("=========================================================\n");
            for (int i = 0; i < teamCount; ++i) {
                printf("%d\t%-16s\t%.2f\t\t%d\n", arr[i]->teamId, arr[i]->teamName, arr[i]->averageBattingStrikeRate, arr[i]->totalPlayers);
            }
            printf("=========================================================\n");
        }
        else if (choice == 4) {
            int teamId = getIntegerInputValidatedSimple("Enter Team ID: ", 1, teamCount);
            TeamNode* t = getTeamById(teamsArray, teamsSortedById, teamCount, teamId);
            if (!t) { printf("Team not found\n"); continue; }
            int roleSel = getIntegerInputValidatedSimple("Enter Role (1-Batsman,2-Bowler,3-All-rounder): ", 1, 3);
            int kVal = getIntegerInputValidatedSimple("Enter number of players (K): ", 1, 1000);
            displayTopKPlayersOfTeamByRole(t, (RoleEnum)roleSel, kVal);
        }
        else if (choice == 5) {
            int roleSel = getIntegerInputValidatedSimple("Enter Role (1-Batsman,2-Bowler,3-All-rounder): ", 1, 3);
            displayAllPlayersByRoleAcrossTeams(teamsArray, (RoleEnum)roleSel);
        }
        else if (choice == 6) break;
        else printf("Invalid choice\n");
    }

    for (int ti = 0; ti < teamCount; ++ti) {
        PlayerNode* it = teamsArray[ti].playerListHead;
        while (it) { PlayerNode* tmp = it; it = it->nextPlayer; free(tmp); }
    }
    free(teamsArray); free(teamsSortedById);
    return 0;
}
