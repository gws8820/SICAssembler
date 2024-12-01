#ifndef INSTRUCTION_H
#define INSTRUCTION_H

#define INST_MAX 100

/* Define Instruction */
typedef struct {
    int loc;
    char label[9];      // 8 + 1
    char operation[7];  // 6 + 1
    char operand[19];   // 18 + 1
    char comment[32];   // 31 + 1
    char opcode[20];
} Instruction;

extern Instruction instructions[INST_MAX];
extern int instruction_count;

#endif