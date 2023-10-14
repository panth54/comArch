/* Assembler code fragment */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define MAXLINELENGTH 1000

int readAndParse(FILE *, char *, char *, char *, char *, char *);
int isNumber(char *);
void dectobin(int dec, char *bin, int n);

typedef struct {
    char label[6];    
    char opcode[6];  
    char arg0[6];   
    char arg1[6];   
    char arg2[6];  
    int address; 
} Code;


int main(int argc, char *argv[])
{
    char *inFileString, *outFileString;
    FILE *inFilePtr, *outFilePtr;
    char label[MAXLINELENGTH], opcode[MAXLINELENGTH], arg0[MAXLINELENGTH],
            arg1[MAXLINELENGTH], arg2[MAXLINELENGTH];

    if (argc != 3) {
        printf("error: usage: %s <assembly-code-file> <machine-code-file>\n",
            argv[0]);
        exit(1);
    }

    inFileString = argv[1];
    outFileString = argv[2];

    inFilePtr = fopen(inFileString, "r");
    if (inFilePtr == NULL) {
        printf("error in opening %s\n", inFileString);
        exit(1);
    }
    outFilePtr = fopen(outFileString, "w");
    if (outFilePtr == NULL) {
        printf("error in opening %s\n", outFileString);
        exit(1);
    }

    int count = 0;
    char line[MAXLINELENGTH];

    // Count lines until the end of the file is reached
    while (fgets(line, MAXLINELENGTH, inFilePtr) != NULL) {
        count++;
    }

    // printf("count da lines %d", count);

    rewind(inFilePtr);
    Code asb_code[count];

    for (int i = 0; i < count; i++) {
    if (readAndParse(inFilePtr, label, opcode, arg0, arg1, arg2)) {
        if (strcmp(opcode, "add") == 0 || strcmp(opcode, "nand") == 0 ||
            strcmp(opcode, "lw") == 0 || strcmp(opcode, "sw") == 0 ||
            strcmp(opcode, "beq") == 0) {
            // Process R-type and I-type instructions.
            strcpy(asb_code[i].label, label);
            strcpy(asb_code[i].opcode, opcode);
            strcpy(asb_code[i].arg0, arg0);
            strcpy(asb_code[i].arg1, arg1);
            strcpy(asb_code[i].arg2, arg2);
            asb_code[i].address = i;
        } else if (strcmp(opcode, "jalr") == 0) {
            // For "jalr" opcode, set arg2 to null character.
            strcpy(asb_code[i].label, label);
            strcpy(asb_code[i].opcode, opcode);
            strcpy(asb_code[i].arg0, arg0);
            strcpy(asb_code[i].arg1, arg1);
            asb_code[i].arg2[0] = '\0'; // Set arg2 to null character
            asb_code[i].address = i;
        } else if (strcmp(opcode, "noop") == 0 || strcmp(opcode, "halt") == 0) {
            // Process O-type instructions.
            strcpy(asb_code[i].label, label);
            strcpy(asb_code[i].opcode, opcode);
            asb_code[i].arg0[0] = '\0'; // Set arg0 to null character
            asb_code[i].arg1[0] = '\0'; // Set arg1 to null character
            asb_code[i].arg2[0] = '\0'; // Set arg2 to null character
            asb_code[i].address = i;
       } else if (strcmp(opcode, ".fill") == 0) {
            // Handle ".fill" instruction separately.
            strcpy(asb_code[i].label, label);
            strcpy(asb_code[i].opcode, opcode);
            strcpy(asb_code[i].arg0, arg0);
            asb_code[i].arg1[0] = '\0'; // Set arg1 to null character
            asb_code[i].arg2[0] = '\0'; // Set arg2 to null character
            asb_code[i].address = i;
        } else {
            // Handle unrecognized opcode.
            fprintf(stderr, "Error: Unrecognized opcode '%s'\n", opcode);
            exit(1);
        }
    }
}
    for (int i = 0; i < count; i++) {
        printf("Instruction %-2d", i );
        printf("Label: %-6s", asb_code[i].label);
        printf("Opcode: %-6s", asb_code[i].opcode);
        printf("Arg0: %-6s", asb_code[i].arg0);
        printf("Arg1: %-6s", asb_code[i].arg1);
        printf("Arg2: %-6s", asb_code[i].arg2);
        printf("Address: %-6d", asb_code[i].address);
        printf("\n");
    }
    
    char m_code[count][33];

    char field0[4];
    char field1[4];
    char field2[4];
    char offset[17];

    for (int i = 0; i < count; i++){
        if (!strcmp(asb_code[i].opcode, "add")){
            dectobin(atoi(asb_code[i].arg0), field0, 3);
            dectobin(atoi(asb_code[i].arg1), field1, 3);
            dectobin(atoi(asb_code[i].arg2), field2, 3);
            snprintf(m_code[i], sizeof(m_code[i]), "0000000000%s%s0000000000000%s", field0, field1, field2);
        }else if (!strcmp(asb_code[i].opcode, "nand")){
           dectobin(atoi(asb_code[i].arg0), field0, 3);
            dectobin(atoi(asb_code[i].arg1), field1, 3);
            dectobin(atoi(asb_code[i].arg2), field2, 3);
            snprintf(m_code[i], sizeof(m_code[i]), "0000000001%s%s0000000000000%s", field0, field1, field2);
        }else if (!strcmp(asb_code[i].opcode, "lw"))
        {
            dectobin(atoi(asb_code[i].arg0), field0, 3);
            dectobin(atoi(asb_code[i].arg1), field1, 3);
            if (isNumber(asb_code[i].arg2))
            {
                dectobin(atoi(asb_code[i].arg2), offset, 16);
            }else{
                for (int j = 0; j < count; j++)
                {
                    if (!strcmp(asb_code[j].label,asb_code[i].arg2))
                    {
                        dectobin(atoi(asb_code[j].arg0), offset, 16);
                    }
                }
            }
            snprintf(m_code[i], sizeof(m_code[i]), "0000000010%s%s%s", field0, field1, offset);
        }else if (!strcmp(asb_code[i].opcode, "sw"))
        {
            /* code */
        }else if (!strcmp(asb_code[i].opcode, "beq"))
        {
            /* code */
        }else if (!strcmp(asb_code[i].opcode, "jalr"))
        {
            /* code */
        }
        else if (!strcmp(asb_code[i].opcode, "halt"))
        {
            /* code */
        }else if (!strcmp(asb_code[i].opcode, "noop"))
        {
            /* code */
        }else if (!strcmp(asb_code[i].opcode, ".fill"))
        {
            /* code */
        }
        
        
        
        
        

        
    }

    for (int i = 0; i < count; i++)
    {
       printf("%s\n",m_code[i]);
    }
    
    

    


    return(0);
}

/*
 * Read and parse a line of the assembly-language file.  Fields are returned
 * in label, opcode, arg0, arg1, arg2 (these strings must have memory already
 * allocated to them).
 *
 * Return values:
 *     0 if reached end of file
 *     1 if all went well
 *
 * exit(1) if line is too long.
 */
int readAndParse(FILE *inFilePtr, char *label, char *opcode, char *arg0,
    char *arg1, char *arg2)
{
    char line[MAXLINELENGTH];
    char *ptr = line;

    /* delete prior values */
    label[0] = opcode[0] = arg0[0] = arg1[0] = arg2[0] = '\0';

    /* read the line from the assembly-language file */
    if (fgets(line, MAXLINELENGTH, inFilePtr) == NULL) {
	/* reached end of file */
        return(0);
    }

    /* check for line too long (by looking for a \n) */
    if (strchr(line, '\n') == NULL) {
        /* line too long */
	printf("error: line too long\n");
	exit(1);
    }

    /* is there a label? */
    ptr = line;
    if (sscanf(ptr, "%[^\t\n ]", label)) {
	/* successfully read label; advance pointer over the label */
        ptr += strlen(label);
    }

    /*
     * Parse the rest of the line.  Would be nice to have real regular
     * expressions, but scanf will suffice.
     */
    sscanf(ptr, "%*[\t\n ]%[^\t\n ]%*[\t\n ]%[^\t\n ]%*[\t\n ]%[^\t\n ]%*[\t\n ]%[^\t\n ]",
        opcode, arg0, arg1, arg2);
    return(1);
}

int isNumber(char *string)
{
    /* return 1 if string is a number */
    int i;
    return( (sscanf(string, "%d", &i)) == 1);
}

int sort(){
    return 0;
}

void dectobin(int dec, char *bin, int n) {
    for (int i = n - 1; i >= 0; i--) {
        bin[i] = (dec & 1) + '0'; 
        dec >>= 1; 
    }
    bin[n] = '\0';
}

