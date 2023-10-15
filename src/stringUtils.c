/*******************************************************
 * Copyright (C) 2023
 *
 *  Created on: Oct 15, 2023
 *      Author: p.sydow
 *
 *******************************************************/

#include <string.h>

void chomp(char* line) {
    int chars_read = strlen(line);

    /*************************************************************/
    /* Remove characters from end of string                      */
    /*************************************************************/
    if(chars_read > 0 && line[chars_read-1] == '\n') {
        line[chars_read-1] = '\0';
        // special care for windows line endings:
        if(chars_read > 1 && line[chars_read-2] == '\r') line[chars_read-2] = '\0';
    }
}
