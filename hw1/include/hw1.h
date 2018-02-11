#ifndef HW_H
#define HW_H

#include "const.h"
#include "instruction.h"

/*search for the right instruction information according to the input mnemonic*/
int encode_search(char *mnemonic, Instruction *ip);

#endif
