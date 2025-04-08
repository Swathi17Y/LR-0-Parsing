#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_PRODUCTIONS 10
#define MAX_SYMBOLS 10
#define MAX_STATES 50

typedef struct {
    char lhs;
    char rhs[MAX_SYMBOLS];
} Production;

typedef struct {
    int numItems;
    char items[MAX_PRODUCTIONS][MAX_SYMBOLS];
} State;

typedef struct {
    char symbol;
    int nextState;
} Transition;

Production productions[MAX_PRODUCTIONS];
int numProductions;
State states[MAX_STATES];
Transition transitions[MAX_STATES][MAX_SYMBOLS];
int numStates = 0;

void printState(State state, int stateNum) {
    printf("State %d:\n", stateNum);
    for (int i = 0; i < state.numItems; i++) {
        printf("  %s\n", state.items[i]);
    }
    printf("\n");
}

void closure(State *state) {
    int changed;
    do {
        changed = 0;
        for (int i = 0; i < state->numItems; i++) {
            char *item = state->items[i];
            char *dotPos = strchr(item, '.');
            if (dotPos && *(dotPos + 1) != '\0') {
                char nextSymbol = *(dotPos + 1);
                if (strncmp(dotPos + 1, "id", 2) == 0) nextSymbol = 'I'; // Treat id as a unit
                for (int j = 0; j < numProductions; j++) {
                    if (productions[j].lhs == nextSymbol) {
                        char newItem[MAX_SYMBOLS];
                        sprintf(newItem, "%c->.%s", productions[j].lhs, productions[j].rhs);
                        int exists = 0;
                        for (int k = 0; k < state->numItems; k++) {
                            if (strcmp(state->items[k], newItem) == 0) {
                                exists = 1;
                                break;
                            }
                        }
                        if (!exists) {
                            strcpy(state->items[state->numItems++], newItem);
                            changed = 1;
                        }
                    }
                }
            }
        }
    } while (changed);
}

State gotoFunction(State state, char symbol) {
    State newState;
    newState.numItems = 0;
    for (int i = 0; i < state.numItems; i++) {
        char *item = state.items[i];
        char *dotPos = strchr(item, '.');
        if (dotPos && *(dotPos + 1) == symbol) {
            char newItem[MAX_SYMBOLS];
            strcpy(newItem, item);
            char *newDotPos = strchr(newItem, '.');
            char temp = *(newDotPos + 1);
            *(newDotPos + 1) = '.';
            *newDotPos = temp;
            strcpy(newState.items[newState.numItems++], newItem);
        }
    }
    closure(&newState);
    return newState;
}

int stateExists(State newState) {
    for (int i = 0; i < numStates; i++) {
        if (newState.numItems == states[i].numItems) {
            int match = 1;
            for (int j = 0; j < newState.numItems; j++) {
                if (strcmp(newState.items[j], states[i].items[j]) != 0) {
                    match = 0;
                    break;
                }
            }
            if (match) return i;
        }
    }
    return -1;
}

void computeLR0Items() {
    State initialState;
    initialState.numItems = 1;
    sprintf(initialState.items[0], "%c->.%s", productions[0].lhs, productions[0].rhs);
    closure(&initialState);
    states[numStates++] = initialState;
   
    for (int i = 0; i < numStates; i++) {
        printState(states[i], i);
       
        char seenSymbols[MAX_SYMBOLS];
        int seenCount = 0;
        for (int j = 0; j < states[i].numItems; j++) {
            char *item = states[i].items[j];
            char *dotPos = strchr(item, '.');
            if (dotPos && *(dotPos + 1) != '\0') {
                char nextSymbol = *(dotPos + 1);
                if (strncmp(dotPos + 1, "id", 2) == 0) nextSymbol = 'I'; // Treat id as a unit
                int alreadySeen = 0;
                for (int k = 0; k < seenCount; k++) {
                    if (seenSymbols[k] == nextSymbol) {
                        alreadySeen = 1;
                        break;
                    }
                }
                if (!alreadySeen) {
                    seenSymbols[seenCount++] = nextSymbol;
                    State newState = gotoFunction(states[i], nextSymbol);
                    int existingState = stateExists(newState);
                    if (existingState == -1) {
                        states[numStates] = newState;
                        transitions[i][seenCount - 1] = (Transition){nextSymbol, numStates};
                        printf("Goto(State %d, %c) -> State %d\n", i, nextSymbol, numStates);
                        numStates++;
                    } else {
                        transitions[i][seenCount - 1] = (Transition){nextSymbol, existingState};
                        printf("Goto(State %d, %c) -> State %d\n", i, nextSymbol, existingState);
                    }
                }
            }
        }
    }
}

int main() {
    printf("Enter number of productions: ");
    scanf("%d", &numProductions);
    getchar();
   
    for (int i = 0; i < numProductions; i++) {
        printf("Enter production %d (e.g., S->AB): ", i + 1);
        scanf("%c->%s", &productions[i].lhs, productions[i].rhs);
        getchar();
    }
   
    computeLR0Items();
    return 0;
}
