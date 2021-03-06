#include <stdio.h>
#include "hw1.h"

/*the table for bcond opcodes*/
Opcode bcondTable[4] = {
  OP_BLTZ, OP_BGEZ, OP_BLTZAL, OP_BGEZAL
};

int optlen(Opcode *table); /*return the length of the table*/
//int check_extra(Source srcs[3]); /*check if extra in source*/
int check_branch(Instr_info instr); /*check if it is a branch instruction*/

/*linear search for the opcode and special table
 *return the index of the opcode in that table
 */
int search_op(Opcode op, Opcode table[]) {
    Opcode *table_pointer = &table[0]; //get the 1st element's address
    int tlen = optlen(table_pointer);
    //printf("The length of the table is: %d\n", tlen);
    for (int i=0; i<tlen; i++) {
      if (op == table[i])
        return i; //return the index of the opcode
    }

    return -1; //nothing found
}

/*return the length of the table*/
int optlen(Opcode *table) {
    int len = 0;
    while(*table) {
      len++;
      table++;
    }

    return len;
}

/*check if the opcode is a bcond*/
int check_bcond(Opcode op) {
    for (int i=0; i<4; i++) {
      if (op == bcondTable[i])
        return 0;
    }

    return 1;
}

/*reverse calculate extra*/
int reverse_extra(int extra, Instr_info instr, unsigned int addr) {
  int origin_extra = 0, extra_tmp;
  Type instr_type = instr.type;
  if (instr_type == RTYP) {
    origin_extra = extra; //5 bits from 10:6
    //origin_extra = (origin_extra | 0x400) << 6;
    //origin_extra = origin_extra * 0x400;
    origin_extra = origin_extra << 6;
    //printf("The original extra in decimal is: %d\n", origin_extra);
    //printf("The original extra in hex is: %x\n", origin_extra);
  }
  else if (instr_type == ITYP) {
    /*not branch instruction, get 15:0*/
    if (check_branch(instr) == -1) {
      origin_extra = extra & 0xffff; //16 bits from 15:0
      //printf("The extra value is: %x\n", extra);
    }
    /*branch instruction*/
    else if (check_branch(instr) != -1) {
      extra_tmp = extra - (addr + 4); //minus base addr and 4
      //printf("Extra value without base address is: %x\n", extra_tmp);
      extra_tmp = extra_tmp >> 2; //right shift by 2 bits
      //printf("Extra value before left shift is: %x\n", extra_tmp);
      origin_extra = extra_tmp & 0xffff; //get the 15:0 bits
      //printf("The original 16-bit extra value is: %x\n", origin_extra);
    }
  }
  else if (instr_type == JTYP) {
    unsigned int modified_addr = (addr + 4) & 0xf0000000; //get the modified address
    //printf("The base address is: %x\n", addr);
    //printf("The modified_addr is: %x\n", modified_addr);
    extra_tmp = extra - modified_addr; //extra after shifted
    //printf("The extra after shifted is: %x\n", extra_tmp);
    origin_extra = extra_tmp >> 2; //original extra before shifted; 25:0 26 bits
    //printf("The original extra from 25:0 is: %x\n", origin_extra);
  }
  /*OP_BREAK*/
  else if (instr.opcode == OP_BREAK) {
    origin_extra = extra; //25:6
    //origin_extra = (origin_extra | 0x400) << 6;
    origin_extra = origin_extra * 0x400;
  }

  return origin_extra;
}

/*calculate the original rs rt rd and extra value for opcode other than bcond*/
void args_val(Instruction *instr, int *rs, int *rt, int *rd, int *origin_extra, unsigned int addr) {
  Instr_info instr_detail = *(instr->info);
  for (int i=0; i<3; i++) {
    Source src_val = instr_detail.srcs[i];
    if (src_val == RS) {
      *rs = instr->args[i];
      //printf("The rs is: %x\n", *rs);
      //*rs = *rs * 0x200000;
      *rs = *rs << 21;
      //printf("The instruction only containing rs is: %x\n", *rs);
    }
    else if (src_val == RT) {
      *rt = instr->args[i];
      //printf("The rt is: %x\n", *rt);
      //*rt = *rt * 0x10000;
      *rt = *rt << 16;
      //printf("The instruction only containing rt is: %x\n", *rt);
    }
    else if (src_val == RD) {
      *rd = instr->args[i];
      //printf("The rd is: %x\n", *rd);
      //*rd = *rd * 0x800;
      *rd = *rd << 11;
      //printf("The instruction only containing rd is: %x\n", *rd);
    }
    else if (src_val == EXTRA) {
      int extra = instr->args[i];
      //printf("Extra in args is: %d\n", extra);
      instr->extra = extra;
      *origin_extra = reverse_extra(extra, instr_detail, addr);
      //printf("The original extra value is: %x\n", *origin_extra);
    }
  }
}

/*fill up the regs*/
void fill_regs(Instruction *instr, int value) {
  instr->regs[0] = value >> 21 & 0x1f; /*RS 25:21*/
  //printf("The rs is: %d\n", rs);
  instr->regs[1] = value >> 16 & 0x1f; /*RT 20:16*/
  //printf("The rt is: %d\n", rt);
  instr->regs[2] = value >> 11 & 0x1f; /*RD 15:11*/
}
