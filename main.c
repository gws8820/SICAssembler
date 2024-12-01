#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "main.h"
#include "Instruction.h"

int main() {
    int curloc = 0;
    char srcline[SRC_MAX];
    char opline[SRC_MAX];

    FILE *optab, *srcfile, *lisfile, *objfile;
    optab = fopen("OPTAB", "r");
    srcfile = fopen("SRCFILE", "r");
    lisfile = fopen("LISFILE", "w");
    objfile = fopen("OBJFILE", "w");

    if (srcfile == NULL) {
        printf("Failed to read SRCFILE.\n");
        return -1;
    }

    /* Save OpTab */
    while (fgets(opline, LINE_MAX, optab) != NULL)
        add_optab(opline);
    
    /* Run Pass 1 */
    while (fgets(srcline, LINE_MAX, srcfile) != NULL)
        if (srcline[0] != '\n' && srcline[0] != '\0')
            curloc = pass1(curloc, srcline); // Update LOCCTR

    /* Run Pass 2 */
    printf("2-Pass Assembler\n------------------------------------\n");
    pass2(lisfile);

    /* Run Loader */
    printf("\n\nLoader\n------------------------------------\n");
    load(objfile);

    fclose(srcfile);
    fclose(optab);
    fclose(lisfile);
    fclose(objfile);

    return 0;
}