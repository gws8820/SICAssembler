#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define SRC_MAX 1000
#define LINE_MAX 66

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

void l2u(char* string) {
    for (int i = 0; string[i] != '\0'; i++)
        string[i] = toupper(string[i]);
}

void trim_comma(char *string) {
    size_t length = strlen(string);
    if (length > 0 && string[length - 1] == ',')
        string[length - 1] = '\0';
}

void decode(char* resline) {
    Instruction *inst = malloc(sizeof(Instruction));
    if (inst == NULL) {
        fprintf(stderr, "Memory allocation failed\n");
        return;
    }

    char *loc_ptr = strstr(resline, "Loc: ");
    char *label_ptr = strstr(resline, "Label: ");
    char *operation_ptr = strstr(resline, "Operation: ");
    char *operand_ptr = strstr(resline, "Operand: ");
    char *opcode_ptr = strstr(resline, "Opcode: ");

    if (loc_ptr) sscanf(loc_ptr, "Loc: %x", &inst->loc);

    if (label_ptr) {
        sscanf(label_ptr, "Label: %31[^,]", inst->label);
        trim_comma(inst->label);
    }

    if (operation_ptr) {
        sscanf(operation_ptr, "Operation: %31[^,]", inst->operation);
        trim_comma(inst->operation);
    }

    if (operand_ptr) {
        sscanf(operand_ptr, "Operand: %63[^,]", inst->operand);
        trim_comma(inst->operand);
    }

    if (opcode_ptr) sscanf(opcode_ptr, "Opcode: %31s", inst->opcode);

    l2u(inst->label);
    l2u(inst->opcode);

    instructions[instruction_count++] = *inst;
    free(inst);
}

void write_record(FILE *objfile, char *text_buffer, int *line_loc, int *line_length) {
    printf("T%.6X%.2X%s\n", *line_loc, *line_length, text_buffer);
    fprintf(objfile, "T%.6X%.2X%s\n", *line_loc, *line_length, text_buffer);

    text_buffer[0] = '\0';
    *line_length = 0;
}

void run(FILE *objfile) {
    char program_name[9];
    strcpy(program_name, instructions[0].label);
    int program_size = instructions[instruction_count - 1].loc - instructions[0].loc;
    int start_loc = instructions[0].loc;
    int line_loc = start_loc;

    printf("H%-6s%.6X%.6X\n", program_name, line_loc, program_size);
    fprintf(objfile, "H%-6s%.6X%.6X\n", program_name, line_loc, program_size);

    char text_buffer[LINE_MAX] = "";
    int line_length = 0;
    
    for (int i = 1; i < instruction_count; i++) {
        if (!strcmp(instructions[i].opcode, "-") || 
            (instructions[i].loc - line_loc > 0x1E)) {
            write_record(objfile, text_buffer, &line_loc, &line_length);

            if (!strcmp(instructions[i].opcode, "-")) {
                line_loc = instructions[i + 1].loc;
                continue;
            } else line_loc = instructions[i].loc;
        }

        strcat(text_buffer, instructions[i].opcode);
        line_length += strlen(instructions[i].opcode) / 2;
    }

    printf("E%.6X\n", start_loc);
    fprintf(objfile, "E%.6X\n", start_loc);
}

int main() {
    FILE *result, *objfile;
    result = fopen("result.txt", "r");
    objfile = fopen("OBJFILE", "w");

    char resline[SRC_MAX];

    if (result == NULL) {
        fprintf(stderr, "Failed to read result.txt.\n");
        return -1;
    }

    while (fgets(resline, SRC_MAX, result) != NULL)
        decode(resline);

    run(objfile);

    fclose(result);
    fclose(objfile);

    return 0;
}