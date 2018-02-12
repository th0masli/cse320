#ifndef HW_H
#define HW_H

#include "const.h"
#include "instruction.h"

/*search for the right instruction information according to the input mnemonic*/
int encode_search(char *mnemonic, Instruction *ip);
/*convert string to hex int*/
int str_hex(char *str);
/*endianness converter*/
int convert_endian(int value);
/*stdout when decoding*/
void print_decode(Instruction *ip);
/*convert binary string to int*/
int str_bin(char *str);
/*return the length of the string*/
int slen(char *str);

#endif
