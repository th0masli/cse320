#include <stdio.h>
#include <stdlib.h>
#include "hw1.h"

int tlen(Instr_info *table); /*return the length of the table*/
int str_cmp(char *str0, char *str1); /*compare 2 strings*/

/*endianness converter*/
int convert_endian(int value) {
    int byte0, byte1, byte2, byte3;
    byte0 = (value >> 24) & 0xff;
    byte1 = (value >> 16) & 0xff;
    byte2 = (value >> 8) & 0xff;
    byte3 = value & 0xff;

    int endian_reversed = byte3 * 0x1000000 + byte2 * 0x10000 + byte1 * 0x100 + byte0;

    return endian_reversed;

}

/*search for the right instruction information according to the input mnemonic*/
int encode_search(char *mnemonic, Instruction *ip) {
    //printf("I got the mnemonic: %s\n", mnemonic);
    Instr_info *instrTable_pointer = &instrTable[0];
    int table_len = tlen(instrTable_pointer);
    table_len = 64;
    //printf("The table's length is: %d\n", table_len);
    for (int i=0; i<table_len; i++) {
      char *instr_format = instrTable[i].format;
      //printf("The index is: %d\n", i);
      //printf("The current format is: %s\n", instr_format);
      int map_res = sscanf(mnemonic, instr_format, &(ip->args[0]), &(ip->args[1]), &(ip->args[2]));
      //printf("Mapping result is: %d\n", map_res);
      if (map_res > 0 || (!str_cmp(mnemonic, instr_format))) {
        //printf("The register from sccanf is: %d\n", ip->args[1]);
        //printf("The index is: %d\n", i);
        return i;
      }
    }

    return -1;
}
