#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "main.h"
#include "Instruction.h"

/* Define OpTab */
typedef struct {
    char operation[6];
    int hexcode;
} OpTab;

OpTab optabDict[TABLE_MAX];
int optabCount = 0;

/* OpTab - Add */
void add_optab(char *opline) {
    char operation[7];
    int hexcode;

    sscanf(opline, "%s %x", operation, &hexcode);

    strcpy(optabDict[optabCount].operation, operation);
    optabDict[optabCount].hexcode = hexcode;
    optabCount++;
}

/* OpTab - Find */
int find_optab(char *operation) {
    for (int i=0; i<optabCount; i++) {
        if (strncasecmp(optabDict[i].operation, operation, 6) == 0)  // L2U
            return optabDict[i].hexcode;
    } return -1;
}