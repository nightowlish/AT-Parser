#include <stdio.h>
#include "parser.h"

STATE_MACHINE_RETURN_VALUE check(uint8_t current_character, uint8_t expected_character, uint16_t *next_state, uint16_t next_state_value) {
    if (expected_character == '*' && current_character != 0x0A && current_character != 0x0D && current_character != '\n')  {        
        *next_state = next_state_value;
        return STATE_MACHINE_NOT_READY;
    }
    if (current_character == expected_character)  {        
        *next_state = next_state_value;
        if (*next_state == 5 || *next_state == 12)            
            return STATE_MACHINE_READY_OK;
        return STATE_MACHINE_NOT_READY;
    }
    return STATE_MACHINE_READY_WITH_ERROR;    
}

STATE_MACHINE_RETURN_VALUE at_command_parse(uint8_t current_character) {
    static uint16_t single_line = 0;
    static uint16_t sms_command = 0;
    static uint16_t state = 0;
    static uint16_t col = 0;
    static uint16_t first_crlf_sms = 0;
    if (current_character == 0x0) {
        mydata.line_count = 0;
        col = 0;
        state = 0;
        single_line = 0;
        first_crlf_sms = 0;
        return STATE_MACHINE_NOT_READY;
    }
    switch(state) {
        case 0: 
            mydata.line_count = 0;
        case 4:
        case 11:
            return check(current_character, 0x0D, &state, state + 1);
        case 1:
        case 5:
            if (current_character == 0x0A)
                mydata.ok = 1;
        case 12:
            if (current_character == 0x0A)
                mydata.ok = 0;
        case 15:
            col = 0;
            return check(current_character, 0x0A, &state, state + 1);
        case 17:
            return check(current_character, 0x0A, &state, state + 1);
        case 2:
            if (current_character == 'O') 
                return check(current_character, 'O', &state, state + 1);
            else if (current_character == 'E') 
                return check(current_character, 'E', &state, 7);
            else if (current_character == '+') 
                return check(current_character, '+', &state, 14);
            else {
                single_line = 1;
                state = 14;
            }
            return STATE_MACHINE_NOT_READY;
        case 3:
            return check(current_character, 'K', &state, state + 1);
        case 6:
        case 13:
            return STATE_MACHINE_READY_OK;
        case 7:
        case 8:
        case 10:
            return check(current_character, 'R', &state, state + 1);
        case 9:
            return check(current_character, 'O', &state, state + 1);
        case 14:
            if (current_character != 0x0D) {
                if ((mydata.line_count < AT_COMMAND_MAX_LINES) && (col < AT_COMMAND_MAX_LINE_SIZE - 1)){
                    mydata.data[mydata.line_count][col] = current_character;
                    col++;
                }
                return STATE_MACHINE_NOT_READY;
            }
            if ((col > 0 && mydata.data[mydata.line_count][col - 1] != '\0') && (mydata.line_count < AT_COMMAND_MAX_LINES)){
                mydata.data[mydata.line_count][col] = '\0';
            }
            return check(current_character, 0x0D, &state, state + 1);
        case 16:
            if (mydata.line_count < AT_COMMAND_MAX_LINES)
                mydata.line_count++;
            if (current_character == '+' && single_line == 0) {
                col = 0;
                state = 14;
                return STATE_MACHINE_NOT_READY;
            }
            else if (current_character == 0x0D) {
                return check(current_character, 0x0D, &state, state + 1);
            }
            sms_command = 1;
            if (first_crlf_sms == 1)
                first_crlf_sms = 0;
            return check(current_character, '*', &state, 14); // * represents any char except cr, lf and new line
        case 18:
            if (current_character == 'O') 
                return check(current_character, 'O', &state, 3);
            if (current_character == 'E') 
                return check(current_character, 'E', &state, 7);
            if (current_character == '+')  {        
                state = 14;
                col = 0;
                return STATE_MACHINE_NOT_READY;
            }
            return STATE_MACHINE_READY_WITH_ERROR;    

        default:
            return STATE_MACHINE_READY_WITH_ERROR;
    }
}