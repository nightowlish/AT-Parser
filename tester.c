#include <stdio.h>
#include <stdlib.h>
#include "parser.h"

void print(AT_COMMAND_DATA mydata) {
    if (mydata.line_count == 0)
        printf("Command has no data.\n");
    else 
        printf("Command has %d data lines:\n", mydata.line_count);
        for(int i = 0; i < mydata.line_count; i++)
            printf("\t%s\n", mydata.data[i]);
    printf("\n");
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("Usage: tester.exe <test_file>");
        return -1;
    }

    FILE *filePointer;
    uint8_t ch;
    filePointer = fopen(argv[1], "rb");
    if (filePointer == NULL) 
        printf("Test file could not be open. Maybe it doesn't exist?\n");
    else {
        STATE_MACHINE_RETURN_VALUE return_value = STATE_MACHINE_NOT_READY;
        while (fread (&ch, 1, 1, filePointer)) {
            return_value = at_command_parse(ch);
            if (return_value == STATE_MACHINE_READY_OK) {
                printf("AT command result is syntactically correct!\n");
                print(mydata);

                fseek(filePointer, 1L, SEEK_CUR); // consume last character
                fseek(filePointer, 0L, SEEK_CUR); // flush everything
                at_command_parse(0x0); // reset command

                continue;
            }
            else if (return_value == STATE_MACHINE_READY_WITH_ERROR) {
                printf("AT command result is NOT syntactically correct!\n");
                break;
            }
        }
        if (return_value == STATE_MACHINE_NOT_READY)
            printf("AT command result is NOT syntactically correct!\n");
    }
    fclose(filePointer);
    return 0;
}