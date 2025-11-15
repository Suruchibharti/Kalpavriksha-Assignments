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
    struct PlayerNode* nextPlayer;
} PlayerNode;

typedef struct TeamNode {
    int teamId;
    char teamName[64];
    PlayerNode* playerListHead;
    int totalPlayers;
    double averageBattingStrikeRate;
} TeamNode;

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

double computePerformanceIndex(RoleEnum role, float battingAverage, float strikeRate, int wickets, float economyRate) {
    if (role == ROLE_BATSMAN) return (battingAverage * strikeRate) / 100.0;
    if (role == ROLE_BOWLER) return (wickets * 2.0) + (100.0 - economyRate);
    if (role == ROLE_ALLROUNDER) return ((battingAverage * strikeRate) / 100.0) + (wickets * 2.0);
    return 0.0;
}

PlayerNode* createPlayerNodeFromData(int playerId, const char* playerName, const char* teamName,
    RoleEnum role, int totalRuns, float battingAverage, float strikeRate, int wickets, float economyRate) {
    PlayerNode* node = (PlayerNode*)malloc(sizeof(PlayerNode));
    if (!node) { perror("malloc"); exit(EXIT_FAILURE); }
    node->playerId = playerId;
    strncpy(node->playerName, playerName, sizeof(node->playerName) - 1); node->playerName[sizeof(node->playerName) - 1] = '\0';
    strncpy(node->teamName, teamName, sizeof(node->teamName) - 1); node->teamName[sizeof(node->teamName) - 1] = '\0';
    node->role = role;
    node->totalRuns = totalRuns;
    node->battingAverage = battingAverage;
    node->strikeRate = strikeRate;
    node->wickets = wickets;
    node->economyRate = economyRate;
    node->performanceIndex = computePerformanceIndex(role, battingAverage, strikeRate, wickets, economyRate);
    node->nextPlayer = NULL;
    return node;
}

void appendPlayerToTeamList(TeamNode* team, PlayerNode* playerNode) {
    if (!team->playerListHead) {
        team->playerListHead = playerNode;
    } else {
        PlayerNode* it = team->playerListHead;
        while (it->nextPlayer) it = it->nextPlayer;
        it->nextPlayer = playerNode;
    }
    team->totalPlayers++;
}

TeamNode* initializeTeamsFromHeader() {
    TeamNode* teamsArray = (TeamNode*)malloc(sizeof(TeamNode) * teamCount);
    if (!teamsArray) { perror("malloc"); exit(EXIT_FAILURE); }
    for (int teamIndex = 0; teamIndex < teamCount; ++teamIndex) {
        teamsArray[teamIndex].teamId = teamIndex;
        strncpy(teamsArray[teamIndex].teamName, teams[teamIndex], sizeof(teamsArray[teamIndex].teamName) - 1);
        teamsArray[teamIndex].teamName[sizeof(teamsArray[teamIndex].teamName) - 1] = '\0';
        teamsArray[teamIndex].playerListHead = NULL;
        teamsArray[teamIndex].totalPlayers = 0;
        teamsArray[teamIndex].averageBattingStrikeRate = 0.0;
    }
    return teamsArray;
}

void loadStaticPlayersIntoTeams(TeamNode* teamsArray) {
    for (int playerIndex = 0; playerIndex < playerCount; ++playerIndex) {
        const Player* p = &players[playerIndex];
        RoleEnum roleEnum = roleStringToEnum(p->role ? p->role : "");
        int foundTeamIndex = -1;
        for (int searchTeamIndex = 0; searchTeamIndex < teamCount; ++searchTeamIndex) {
            if (strcmp(teamsArray[searchTeamIndex].teamName, p->team) == 0) { foundTeamIndex = searchTeamIndex; break; }
        }
        if (foundTeamIndex == -1) continue;
        PlayerNode* newNode = createPlayerNodeFromData(p->id, p->name, p->team, roleEnum, p->totalRuns, p->battingAverage, p->strikeRate, p->wickets, p->economyRate);
        appendPlayerToTeamList(&teamsArray[foundTeamIndex], newNode);
    }
}

void computeTeamAverageStrikeRates(TeamNode* teamsArray) {
    for (int teamIndex = 0; teamIndex < teamCount; ++teamIndex) {
        double sumStrike = 0.0;
        int countBatters = 0;
        PlayerNode* iterator = teamsArray[teamIndex].playerListHead;
        while (iterator) {
            if (iterator->role == ROLE_BATSMAN || iterator->role == ROLE_ALLROUNDER) { sumStrike += iterator->strikeRate; countBatters++; }
            iterator = iterator->nextPlayer;
        }
        if (countBatters > 0) teamsArray[teamIndex].averageBattingStrikeRate = sumStrike / countBatters;
        else teamsArray[teamIndex].averageBattingStrikeRate = 0.0;
    }
}

void printTeamPlayers(TeamNode* team) {
    printf("\nPlayers of Team %s:\n", team->teamName);
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

int compareTeamsByAvgStrikeDesc(const void* a, const void* b) {
    const TeamNode* ta = *(const TeamNode* const*)a;
    const TeamNode* tb = *(const TeamNode* const*)b;
    if (ta->averageBattingStrikeRate < tb->averageBattingStrikeRate) return 1;
    if (ta->averageBattingStrikeRate > tb->averageBattingStrikeRate) return -1;
    return 0;
}

TeamNode* getTeamById(TeamNode* teamsArray, int teamId) {
    if (teamId < 0 || teamId >= teamCount) return NULL;
    return &teamsArray[teamId];
}

void displayTopKPlayersOfTeamByRole(TeamNode* team, RoleEnum requestedRole, int K) {
    if (!team) return;
    PlayerNode* bufferArr[1000];
    int bufferCount = 0;
    PlayerNode* it = team->playerListHead;
    while (it) { if (it->role == requestedRole) bufferArr[bufferCount++] = it; it = it->nextPlayer; }
    if (bufferCount == 0) { printf("No players of role %s in team %s\n", roleEnumToString(requestedRole), team->teamName); return; }
    if (K > bufferCount) K = bufferCount;
    for (int i = 0; i < K; ++i) {
        int bestIdx = i;
        for (int j = i + 1; j < bufferCount; ++j) if (bufferArr[j]->performanceIndex > bufferArr[bestIdx]->performanceIndex) bestIdx = j;
        if (bestIdx != i) { PlayerNode* tmp = bufferArr[i]; bufferArr[i] = bufferArr[bestIdx]; bufferArr[bestIdx] = tmp; }
    }
    printf("\nTop %d %s of Team %s:\n", K, roleEnumToString(requestedRole), team->teamName);
    printf("====================================================================================\n");
    printf("ID\tName\t\tRole\tRuns\tAvg\tSR\tWkts\tER\tPerf.Index\n");
    printf("====================================================================================\n");
    for (int i = 0; i < K; ++i) {
        PlayerNode* p = bufferArr[i];
        printf("%d\t%-16s\t%-11s\t%d\t%.2f\t%.2f\t%d\t%.2f\t%.2f\n",
               p->playerId, p->playerName, roleEnumToString(p->role),
               p->totalRuns, p->battingAverage, p->strikeRate, p->wickets, p->economyRate, p->performanceIndex);
    }
}

int comparePlayerNodeByPerformanceDesc(const void* a, const void* b) {
    const PlayerNode* pa = *(const PlayerNode* const*)a;
    const PlayerNode* pb = *(const PlayerNode* const*)b;
    if (pa->performanceIndex < pb->performanceIndex) return 1;
    if (pa->performanceIndex > pb->performanceIndex) return -1;
    return 0;
}

void displayAllPlayersByRoleAcrossTeams(TeamNode* teamsArray, RoleEnum requestedRole) {
    PlayerNode* collected[2000];
    int collectedCount = 0;
    for (int teamIndex = 0; teamIndex < teamCount; ++teamIndex) {
        PlayerNode* it = teamsArray[teamIndex].playerListHead;
        while (it) { if (it->role == requestedRole) collected[collectedCount++] = it; it = it->nextPlayer; }
    }
    if (collectedCount == 0) { printf("No players found for role %s across all teams.\n", roleEnumToString(requestedRole)); return; }
    qsort(collected, collectedCount, sizeof(PlayerNode*), comparePlayerNodeByPerformanceDesc);
    printf("\nAll players (Role: %s) across teams sorted by Performance Index:\n", roleEnumToString(requestedRole));
    printf("====================================================================================\n");
    printf("ID\tName\tTeam\tRole\tRuns\tAvg\tSR\tWkts\tER\tPerf.Index\n");
    printf("====================================================================================\n");
    for (int i = 0; i < collectedCount; ++i) {
        PlayerNode* p = collected[i];
        printf("%d\t%-16s\t%-12s\t%-11s\t%d\t%.2f\t%.2f\t%d\t%.2f\t%.2f\n",
               p->playerId, p->playerName, p->teamName, roleEnumToString(p->role),
               p->totalRuns, p->battingAverage, p->strikeRate, p->wickets, p->economyRate, p->performanceIndex);
    }
}

int addPlayerInteractive(TeamNode* teamsArray, int* idValid) {
    char buffer[256];
    char nameBuf[128];
    int teamId;
    int playerId;
    int roleSel;
    int totalRuns;
    float battingAverage;
    float strikeRate;
    int wickets;
    float economyRate;
    char extra;

    while (1) {
        printf("Enter Team ID to add player: ");
        if (!fgets(buffer, sizeof(buffer), stdin)) return -1;
        if (sscanf(buffer, "%d %c", &teamId, &extra) == 1 && teamId >= 0 && teamId <= teamCount - 1) break;
        fprintf(stderr, "Please enter a valid Team ID (0-%d)\n", teamCount - 1);
    }

    TeamNode* targetTeam = getTeamById(teamsArray, teamId);
    if (!targetTeam) { fprintf(stderr, "Invalid team selected\n"); return -1; }

    printf("Enter Player Details:\n");

    while (1) {
        printf("Player ID: ");
        if (!fgets(buffer, sizeof(buffer), stdin)) return -1;
        if (sscanf(buffer, "%d %c", &playerId, &extra) == 1 && playerId >= 1 && playerId <= 1500) {
            if (idValid[playerId] == 0) break;
            else fprintf(stderr, "Entered ID is already taken\n");
        } else fprintf(stderr, "Please enter a valid Player ID (1-1500)\n");
    }
    idValid[playerId] = 1;

    while (1) {
        printf("Name: ");
        if (!fgets(nameBuf, sizeof(nameBuf), stdin)) return -1;
        size_t ln = strlen(nameBuf);
        if (ln > 0 && nameBuf[ln - 1] == '\n') { nameBuf[ln - 1] = '\0'; ln--; }
        if (ln >= 1 && ln <= 50) break;
        fprintf(stderr, "Please enter a valid name (1-50 characters)\n");
    }

    while (1) {
        printf("Role (1-Batsman, 2-Bowler, 3-All-rounder): ");
        if (!fgets(buffer, sizeof(buffer), stdin)) return -1;
        if (sscanf(buffer, "%d %c", &roleSel, &extra) == 1 && (roleSel >= 1 && roleSel <= 3)) break;
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

    double sumStrike = 0.0; int countBatters = 0; PlayerNode* iterator = targetTeam->playerListHead;
    while (iterator) { if (iterator->role == ROLE_BATSMAN || iterator->role == ROLE_ALLROUNDER) { sumStrike += iterator->strikeRate; ++countBatters; } iterator = iterator->nextPlayer; }
    if (countBatters > 0) targetTeam->averageBattingStrikeRate = sumStrike / countBatters; else targetTeam->averageBattingStrikeRate = 0.0;

    printf("Player added successfully to Team %s!\n", targetTeam->teamName);
    return 0;
}

//
// FIXED VALIDATION FUNCTION FOR TEAM ID
//
int getIntegerInputValidatedSimple(const char* prompt, int minAllowed, int maxAllowed) {
    char buf[100];
    int value;

    while (1) {
        printf("%s", prompt);

        if (!fgets(buf, sizeof(buf), stdin)) continue;

        char extra;
        if (sscanf(buf, "%d %c", &value, &extra) == 1) {
            if (value >= minAllowed && value <= maxAllowed)
                return value;
        }

        printf("Input must be an integer between %d and %d\n", minAllowed, maxAllowed);
    }
}

int main() {
    TeamNode* teamsArray = initializeTeamsFromHeader();
    loadStaticPlayersIntoTeams(teamsArray);
    computeTeamAverageStrikeRates(teamsArray);

    int idValid[2001]; memset(idValid, 0, sizeof(idValid));
    for (int playerIndex = 0; playerIndex < playerCount && playerIndex < 2000; ++playerIndex) {
        int pid = players[playerIndex].id;
        if (pid >= 1 && pid <= 2000) idValid[pid] = 1;
    }

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
        int mainChoice;

        if (scanf("%d", &mainChoice) != 1) { while (getchar() != '\n'); printf("Invalid choice\n"); continue; }
        while (getchar() != '\n');

        if (mainChoice == 1) {
            addPlayerInteractive(teamsArray, idValid);
        } 
        else if (mainChoice == 2) {
            int teamId = getIntegerInputValidatedSimple("Enter Team ID: ", 0, teamCount - 1);
            TeamNode* team = getTeamById(teamsArray, teamId);
            if (!team) { printf("Team not found\n"); continue; }
            computeTeamAverageStrikeRates(teamsArray);
            printTeamPlayers(team);
        } 
        else if (mainChoice == 3) {
            computeTeamAverageStrikeRates(teamsArray);
            TeamNode* teamPtrArray[20];
            for (int i = 0; i < teamCount; ++i) teamPtrArray[i] = &teamsArray[i];
            qsort(teamPtrArray, teamCount, sizeof(TeamNode*), compareTeamsByAvgStrikeDesc);

            printf("\nTeams Sorted by Average Batting Strike Rate\n");
            printf("=========================================================\n");
            printf("ID\tTeam Name\t\tAvg Bat SR\tTotal Players\n");
            printf("=========================================================\n");

            for (int i = 0; i < teamCount; ++i) {
                printf("%d\t%-16s\t%.2f\t\t%d\n",
                       teamPtrArray[i]->teamId, teamPtrArray[i]->teamName,
                       teamPtrArray[i]->averageBattingStrikeRate, teamPtrArray[i]->totalPlayers);
            }

            printf("=========================================================\n");
        } 
        else if (mainChoice == 4) {
            int teamId = getIntegerInputValidatedSimple("Enter Team ID: ", 0, teamCount - 1);
            TeamNode* team = getTeamById(teamsArray, teamId);
            if (!team) { printf("Team not found\n"); continue; }

            int roleSel = getIntegerInputValidatedSimple("Enter Role (1-Batsman,2-Bowler,3-All-rounder): ", 1, 3);
            int kVal = getIntegerInputValidatedSimple("Enter number of players: ", 1, 1000);

            displayTopKPlayersOfTeamByRole(team, (RoleEnum)roleSel, kVal);
        } 
        else if (mainChoice == 5) {
            int roleSel = getIntegerInputValidatedSimple("Enter Role (1-Batsman,2-Bowler,3-All-rounder): ", 1, 3);
            displayAllPlayersByRoleAcrossTeams(teamsArray, (RoleEnum)roleSel);
        } 
        else if (mainChoice == 6) {
            break;
        } 
        else {
            printf("Invalid choice\n");
        }
    }

    for (int teamIndex = 0; teamIndex < teamCount; ++teamIndex) {
        PlayerNode* it = teamsArray[teamIndex].playerListHead;
        while (it) { PlayerNode* tmp = it; it = it->nextPlayer; free(tmp); }
    }

    free(teamsArray);
    return 0;
}
