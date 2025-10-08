#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "main.h"
#include "Instruction.h"

/* To Uppercase */
void to_uppercase(char *str) {
    for (int i = 0; str[i]; i++) 
        str[i] = toupper((unsigned char)str[i]);
}

/* Write Record */
void write_record(FILE *objfile, char *text_buffer, int *line_loc, int *line_length) {
    to_uppercase(text_buffer);
    printf("T%.6X%.2X%s\n", *line_loc, *line_length, text_buffer);
    fprintf(objfile, "T%.6X%.2X%s\n", *line_loc, *line_length, text_buffer);

    text_buffer[0] = '\0';
    *line_length = 0;
}

/* Write Object File (H/T/E) */
void write_objfile(FILE *objfile) {
    char program_name[9];
    strcpy(program_name, instructions[0].label);
    to_uppercase(program_name);

    int program_size = instructions[instruction_count - 1].loc - instructions[0].loc;
    int start_loc = instructions[0].loc;
    int line_loc = start_loc;

    /* Header Record */
    printf("H%-6s%.6X%.6X\n", program_name, line_loc, program_size);
    fprintf(objfile, "H%-6s%.6X%.6X\n", program_name, line_loc, program_size);

    char text_buffer[LINE_MAX] = "";
    int line_length = 0;

    for (int i = 1; i < instruction_count; i++) {
        if (!strcmp(instructions[i].opcode, " ") || (instructions[i].loc - line_loc > 0x1E)) {
            write_record(objfile, text_buffer, &line_loc, &line_length);

            if (!strcmp(instructions[i].opcode, " ")) {
                line_loc = instructions[i + 1].loc;
                continue;
            } else line_loc = instructions[i].loc;
        }

        strcat(text_buffer, instructions[i].opcode);
        to_uppercase(instructions[i].opcode);
        line_length += (int)strlen(instructions[i].opcode) / 2;
    }

    /* End Record */
    printf("E%.6X\n", start_loc);
    fprintf(objfile, "E%.6X\n", start_loc);
}