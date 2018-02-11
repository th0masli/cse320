#include <stdio.h>
#include <stdlib.h>
#include "hw1.h"

int tlen(Instr_info *table); /*return the length of the table*/

/*convert big endianness to little endianness*/
int big_to_little(int value) {
    return 0;
}

/*convert little endianness to big*/
int little_to_big(int value) {
    return 0;
}

/*search for the right instruction information according to the input mnemonic*/
int encode_search(char *mnemonic, Instruction *ip) {
    //printf("I got the mnemonic: %s\n", mnemonic);
    Instr_info *instrTable_pointer = &instrTable[0];
    int table_len = tlen(instrTable_pointer);
    //printf("The table's length is: %d\n", table_len);
    for (int i=0; i<table_len; i++) {
      char *instr_format = instrTable[i].format;
      //printf("The current format is: %s\n", instr_format);
      int map_res = sscanf(mnemonic, instr_format, &(ip->args[0]), &(ip->args[1]), &(ip->args[2]));
      if (map_res > 0)
        return i;
    }

    return -1;
}
