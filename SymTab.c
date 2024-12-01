#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "main.h"
#include "Instruction.h"

/* Define SymTab */
typedef struct {
    char symbol[9];
    int locctr;
} SymTab;

SymTab symtabDict[TABLE_MAX];
int symtabCount = 0;

/* SymTab - Add */
void add_symtab(char *symbol, int locctr) {
    strcpy(symtabDict[symtabCount].symbol, symbol);
    symtabDict[symtabCount].locctr = locctr;
    symtabCount++;
}

/* SymTab - Find */
int find_symtab(char *symbol) {
    for (int i=0; i<symtabCount; i++) {
        if (strcmp(symtabDict[i].symbol, symbol) == 0)
            return symtabDict[i].locctr;
    } return -1;
}