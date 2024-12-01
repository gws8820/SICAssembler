#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "main.h"
#include "Instruction.h"

Instruction instructions[INST_MAX];
int instruction_count = 0;

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
    Instruction *inst = malloc(sizeof(Instruction));

    inst->loc = curloc;
    trim_whitespace(srcline, inst->label, 8);
    trim_whitespace(srcline + 9, inst->operation, 6);
    trim_whitespace(srcline + 17, inst->operand, 18);
    trim_whitespace(srcline + 35, inst->comment, 31);

    /* SymTab - Add */
    if (strlen(inst->label) > 0)
        add_symtab(inst->label, curloc);
    else strcpy(inst->label, "-");

    /* Return Start LOCCTR */
    if (strcmp(inst->operation, "start") == 0) {
        inst->loc = (int)strtol(inst->operand, NULL, 16);
        instructions[instruction_count++] = *inst;
        return inst->loc;
    }

    /* Return LOCCTR */
    else if (strcmp(inst->operation, "byte") == 0) {
        int length = 1;
        if (inst->operand[0] == 'c' && inst->operand[1] == '\'' && inst->operand[strlen(inst->operand) - 1] == '\'')
            length = strlen(inst->operand) - 3;
        
        curloc += length;
    }

    else if (strcmp(inst->operation, "word") == 0)
        curloc += 3;

    else if (strcmp(inst->operation, "resb") == 0)
        curloc += atoi(inst->operand);

    else if (strcmp(inst->operation, "resw") == 0) 
        curloc += 3 * atoi(inst->operand);

    else curloc += 3;

    instructions[instruction_count++] = *inst;
    free(inst);

    return curloc;
}

/* Pass 2 */
void pass2(FILE *lisfile) {
    for (int i=0; i<instruction_count; i++) {
        Instruction *inst = &instructions[i];

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
        printf("Loc: %x, Label: %s, Operation: %s, Operand: %s, Opcode: %s\n", inst->loc, inst->label, inst->operation, inst->operand, inst->opcode);

        /* Write LISFILE */
        if (strcmp(inst->opcode, "-") == 0) strcpy(inst->opcode, " ");
        if (strcmp(inst->label, "-") == 0) strcpy(inst->label, " ");
        fprintf(lisfile, "%04x %-6.6s %-8s %-6s  %-49s\n", inst->loc, inst->opcode, inst->label, inst->operation, inst->operand);

        for (int opindex = 1; 6 * opindex < strlen(inst->opcode) ; opindex++) // If Operand length exceeds 6
            fprintf(lisfile, "     %.6s\n", (inst->opcode + 6*opindex));

        /* Handle Error */
        if (flag == 1 || flag == 3) {
            printf(" **** undefined symbol in operand\n");
            fprintf(lisfile, " **** undefined symbol in operand                       \n");
        }
        if (flag == 2 || flag == 3) {
            printf(" **** unrecognized operation code\n");
            fprintf(lisfile, " **** unrecognized operation code                       \n");
        }
    }
}