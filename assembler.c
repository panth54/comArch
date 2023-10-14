/* Assembler code fragment */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define MAXLINELENGTH 1000

int readAndParse(FILE *, char *, char *, char *, char *, char *);
int isNumber(char *);
void dectobin(int dec, char *bin, int n);
char* bintohex(const char* binary);
int hextodec(const char* hex);

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

    rewind(inFilePtr);
    Code asb_code[count];

    for (int i = 0; i < count; i++) {
        if (readAndParse(inFilePtr, label, opcode, arg0, arg1, arg2)) {

            // Error check : Label size is longer than six
            if (strlen(label) > 6) {
                fprintf(stderr, "Error: Label '%s' is longer than six characters.\n", label);
                exit(1);
            }

            for (int j = 0; j < i; j++) {
                // Error check : Using the same label
                if (asb_code[j].label[0] != '\0' && strcmp(asb_code[j].label, label) == 0) {
                    fprintf(stderr, "Error: Label '%s' is used more than once.\n", label);
                    exit(1);
                }
            }       


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

    // Error check: Using undefined labels
    for (int i = 0; i < count; i++) {
        if (!isNumber(asb_code[i].arg2) && asb_code[i].arg2[0] != '\0'){
            int labelFound = 0;  // Variable to track if the label is found
            for (int j = 0; j < count; j++) {
                if (!strcmp(asb_code[j].label, asb_code[i].arg2)) {
                    labelFound = 1; // Label found, set the flag
                    break;
                }
            }
            
            if (!labelFound) {
                // Handle the case of an undefined label
                // You can print an error message or perform other error-handling actions here
                printf("Error: '%s' is Undefined label", asb_code[i].arg2);
                exit(0);
            }
        }
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
                        dectobin(asb_code[j].address, offset, 16);
                    }
                }
            }
            snprintf(m_code[i], sizeof(m_code[i]), "0000000010%s%s%s", field0, field1, offset);
        }else if (!strcmp(asb_code[i].opcode, "sw"))
        {
            dectobin(atoi(asb_code[i].arg0), field0, 3);
            dectobin(atoi(asb_code[i].arg1), field1, 3);
            if (isNumber(asb_code[i].arg2))
            {
                dectobin(atoi(asb_code[i].arg2), offset, 16);
            }
            else
            {
                for (int j = 0; j < count; j++)
                {
                    if (!strcmp(asb_code[j].label, asb_code[i].arg2))
                    {
                        dectobin(asb_code[j].address, offset, 16);
                    }
                }
            }
            snprintf(m_code[i], sizeof(m_code[i]), "0000000011%s%s%s", field0, field1, offset);
        }else if (!strcmp(asb_code[i].opcode, "beq"))
        {
            dectobin(atoi(asb_code[i].arg0), field0, 3);
            dectobin(atoi(asb_code[i].arg1), field1, 3);
            if (isNumber(asb_code[i].arg2))
            {
                dectobin(atoi(asb_code[i].arg2), offset, 16);
            }
            else
            {
                int relative_offset = 0;

                for (int j = 0; j < count; j++) {
                    if (!strcmp(asb_code[j].label, asb_code[i].arg2)) {
                        relative_offset = j - i - 1;
                        break;
                    }
                }
                
                dectobin(relative_offset, offset, 16);
            }
            snprintf(m_code[i], sizeof(m_code[i]), "0000000100%s%s%s", field0, field1, offset);
        }else if (!strcmp(asb_code[i].opcode, "jalr")) {
            dectobin(atoi(asb_code[i].arg0), field0, 3);
            dectobin(atoi(asb_code[i].arg1), field1, 3);
            snprintf(m_code[i], sizeof(m_code[i]), "0000000101%s%s0000000000000000", field0, field1);
        } else if (!strcmp(asb_code[i].opcode, "halt")) {
            snprintf(m_code[i], sizeof(m_code[i]), "00000001100000000000000000000000");
        } else if (!strcmp(asb_code[i].opcode, "noop")) {
            snprintf(m_code[i], sizeof(m_code[i]), "00000001110000000000000000000000");
        } else if (!strcmp(asb_code[i].opcode, ".fill")) {
            // Check if the argument is a number or a label
            if (isNumber(asb_code[i].arg0)) {
                int value = atoi(asb_code[i].arg0);
                char binaryValue[33];
                dectobin(value, binaryValue, 32); // Convert the value to a 32-bit binary string
                strcpy(m_code[i], binaryValue); // Store the binary value in the machine code array
            } else {
                // Handle the case where the argument is a label
                // Find the label and get its corresponding value
                for (int j = 0; j < count; j++) {
                    if (!strcmp(asb_code[j].label, asb_code[i].arg0)) {
                        int value = asb_code[j].address; // Assuming the address is the value for the label
                        char binaryValue[33];
                        dectobin(value, binaryValue, 32); // Convert the value to a 32-bit binary string
                        strcpy(m_code[i], binaryValue); // Store the binary value in the machine code array
                        break;
                    }
                }
            }
        }  

    }

    for (int i = 0; i < count; i++)
    {
        char* hex = bintohex(m_code[i]);
        int dec = hextodec(hex);
        printf("(address %d): %d (hex 0x%s )\n",i ,dec , hex);
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

char* bintohex(const char* binary) {
    int decimal = 0;
    int binaryLength = strlen(binary);

    // Convert binary to decimal
    for (int i = 0; i < binaryLength; i++) {
        if (binary[i] == '1') {
            decimal += 1 << (binaryLength - 1 - i);
        }
    }

    // Convert decimal to lowercase hexadecimal
    char* hex = (char*)malloc(9);  // Assuming a 32-bit integer, so 8 characters maximum for hexadecimal
    sprintf(hex, "%x", decimal);  // Use %x to format in lowercase

    return hex;
}

int hextodec(const char* hex) {
    int decimal = 0;
    int hexLength = strlen(hex);

    for (int i = 0; i < hexLength; i++) {
        char c = hex[i];
        int digit;
        if (c >= '0' && c <= '9') {
            digit = c - '0';
        } else if (c >= 'A' && c <= 'F') {
            digit = c - 'A' + 10;
        } else if (c >= 'a' && c <= 'f') {
            digit = c - 'a' + 10;
        } else {
            // Invalid character in the hexadecimal string
            return -1;  // You can choose an appropriate error code
        }
        decimal = decimal * 16 + digit;
    }

    return decimal;
}

void dectobin(int dec, char *bin, int n) {
    for (int i = n - 1; i >= 0; i--) {
        int bit = (dec >> i) & 1;  // Extract the i-th bit
        bin[n - 1 - i] = bit ? '1' : '0';
    }
    bin[n] = '\0';  // Null-terminate the string
}
