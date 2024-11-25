#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define SRC_MAX 1000
#define LINE_MAX 66
#define TABLE_MAX 40

/* Define Instruction */
typedef struct {
    int loc;
    char label[9];      // 8 + 1
    char operation[7];  // 6 + 1
    char operand[19];   // 18 + 1
    char comment[32];   // 31 + 1
    char opcode[20];
} Instruction;

Instruction instructions[LINE_MAX];
int instruction_count = 0;

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

/* Trim Whitespace & Add Nullchar */
void trim_whitespace(const char *src, char *dest, int length) {
    int j = 0;
    for (int i = 0; i < length && src[i] != '\0'; i++) {
        if (!isspace((unsigned char)src[i])) {
            dest[j++] = src[i];
        }
    }
    dest[j] = '\0';
}

/* Pass 1 */
int pass1(int curloc, char *srcline) {
    Instruction inst;

    inst.loc = curloc;
    trim_whitespace(srcline, inst.label, 8);
    trim_whitespace(srcline + 9, inst.operation, 6);
    trim_whitespace(srcline + 17, inst.operand, 18);
    trim_whitespace(srcline + 35, inst.comment, 31);

    /* Return Start LOCCTR */
    if (strcmp(inst.operation, "start") == 0) {
        inst.loc = (int)strtol(inst.operand, NULL, 16);
        instructions[instruction_count++] = inst;
        return inst.loc;
    }

    /* SymTab - Add */
    else if (strlen(inst.label) > 0)
        add_symtab(inst.label, curloc);

    instructions[instruction_count++] = inst;

    /* Calculate Memory Allocation */
    if (strcmp(inst.operation, "byte") == 0) {
        int length = 1;
        if (inst.operand[0] == 'c' && inst.operand[1] == '\'' && inst.operand[strlen(inst.operand) - 1] == '\'')
            length = strlen(inst.operand) - 3;
        
        curloc += length;
    }

    else if (strcmp(inst.operation, "word") == 0)
        curloc += 3;

    else if (strcmp(inst.operation, "resb") == 0)
        curloc += atoi(inst.operand);

    else if (strcmp(inst.operation, "resw") == 0) 
        curloc += 3 * atoi(inst.operand);

    else curloc += 3;
    
    return curloc;
}

/* Pass 2 */
int pass2(Instruction *inst) {
    int flag = 0;
    char hexcode[20] = {'\0'};
    char *ptr = hexcode;

    /* START / END / RESB / RESW */
    if (strcmp(inst->operation, "start") == 0 || strcmp(inst->operation, "end") == 0 || strcmp(inst->operation, "resb") == 0 || strcmp(inst->operation, "resw") == 0)
        strcpy(hexcode, "-");

    /* Memory Constants */
    else if (strcmp(inst->operation, "byte") == 0)  {
        if (inst->operand[0] == 'c' && inst->operand[1] == '\'' && inst->operand[strlen(inst->operand) - 1] == '\'')
            for (int i = 2; i < strlen(inst->operand) - 1; i++)
                ptr += sprintf(ptr, "%02x", inst->operand[i]); // 2 Bits for Each Character ((Hex 2^4)^2 = Char 2^8)
        else sprintf(hexcode, "%06x", atoi(inst->operand));
    }
        
    else if (strcmp(inst->operation, "word") == 0)
        sprintf(hexcode, "%06x", atoi(inst->operand));

    /* Location Counter */
    else {
        int optab = find_optab(inst->operation);
        
        /* Illegal Operation Code */
        if (optab == -1) flag += 2;
        else sprintf(hexcode, "%02x", optab);

        int adrctr;
        if (strstr(inst->operand, ",x")) { // Index Register
            *(hexcode + 2) = '9'; // Third Bit of Opcode
            char symbol[19];
            strcpy (symbol, inst->operand);
            symbol[strlen(symbol)-2] = '\0'; // Remove Index Expression
            adrctr = find_symtab(symbol);
        } 
        else {
            *(hexcode + 2) = '1'; // Third Bit of Opcode
            adrctr = find_symtab(inst->operand);
        }

        /* Illegal Symbol */
        if (adrctr == -1) flag ++;
        else sprintf(hexcode + 3, "%03x", adrctr & 0xFFF); // Extract Least 3 Bits
    }

    strcpy(inst->opcode, hexcode);
    return flag;
}


int main() {
    int curloc = 0;
    char srcline[SRC_MAX];
    char opline[SRC_MAX];

    FILE *srcfile, *optab, *result, *lisfile;
    srcfile = fopen("SRCFILE", "r");
    optab = fopen("optab.txt", "r");
    result = fopen("result.txt", "w");
    lisfile = fopen("LISFILE", "w");

    if (srcfile == NULL) {
        printf("Failed to read SRCFILE.\n");
        return -1;
    }

    /* Save Operation Table */
    for (int i = 0; fgets(opline, LINE_MAX, optab) != NULL; i++)
        add_optab(opline);
    
    /* Run Pass 1 */
    while (fgets(srcline, LINE_MAX, srcfile) != NULL)
        if (srcline[0] != '\n' && srcline[0] != '\0')
            curloc = pass1(curloc, srcline); // Update LOCCTR

    /* Run Pass 2 */
    for (int i=0; i<instruction_count; i++) {
        Instruction inst = instructions[i];
        int flag = pass2(&inst);
        printf("Loc: %x, Label: %s, Operation: %s, Operand: %s, Opcode: %s\n\n", inst.loc, strlen(inst.label) > 0 ? inst.label : "-", inst.operation, inst.operand, inst.opcode);
        fprintf(result, "Loc: %x, Label: %s, Operation: %s, Operand: %s, Opcode: %s\n", inst.loc, strlen(inst.label) > 0 ? inst.label : "-", inst.operation, inst.operand, inst.opcode);

        /* Write LISFILE */
        fprintf(lisfile, "%04x %-6.6s %-8s %-6s  %-49s\n", inst.loc, strcmp(inst.opcode, "-") == 0 ? " " : inst.opcode, strlen(inst.label) > 0 ? inst.label : " ", inst.operation, inst.operand);
        for (int opindex = 1; 6 * opindex < strlen(inst.opcode) ; opindex++) // If Operand length exceeds 6
            fprintf(lisfile, "     %.6s\n", (inst.opcode + 6*opindex));

        if(flag == 1 || flag == 3) {
            printf(" **** undefined symbol in operand\n\n");
            fprintf(result, " **** undefined symbol in operand\n");
            fprintf(lisfile, " **** undefined symbol in operand                       \n");
        }
        if(flag == 2 || flag == 3) {
            printf(" **** unrecognized operation code\n\n");
            fprintf(result, " **** unrecognized operation code\n");
            fprintf(lisfile, " **** unrecognized operation code                       \n");
        }
        
    }

    fclose(srcfile);
    fclose(optab);
    fclose(result);
    fclose(lisfile);

    return 0;
}