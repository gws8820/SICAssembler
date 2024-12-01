#ifndef SICASM_H
#define SICASM_H

#define SRC_MAX 1000
#define TABLE_MAX 100
#define LINE_MAX 66

/* Assembler.c */
int pass1(int curloc, char *srcline);
void pass2(FILE *lisfile);

/* Loader.c */
void load(FILE *objfile);

/* OpTab.c */
void add_optab(char *opline);
int find_optab(char *operation);

/* SymTab.c */
void add_symtab(char *symbol, int locctr);
int find_symtab(char *symbol);

#endif