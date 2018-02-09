#include <stdio.h>
#include "hw1.h"

int check_branch(Instr_info instr); /*check if it is a branch instruction*/
int check_extra(Source srcs[3]); /*check if extra in source*/

Opcode branchTable[10] = {
	OP_BEQ, OP_BGEZ, OP_BGEZAL, OP_BGTZ, OP_BLEZ, OP_BLTZ, OP_BLTZAL, OP_BNE
};

int tlen(Instr_info *table); /*return the length of the table*/

/*linear search instrTable*/
int search_instr(Opcode target_op) {
	Instr_info *instrTable_pointer = &instrTable[0];
	int table_len = tlen(instrTable_pointer);
	printf("The table's length is: %d\n", table_len);
	for (int i=0; i<table_len; i++) {
		Opcode cur_op = instrTable[i].opcode;
		if (cur_op == target_op)
			return i;
	}

	return -1;
}


/*return the length of the table*/
int tlen(Instr_info *table) {
	int len = 0;
	while (table->opcode) {
		len++;
		table++;
	}

	return len;
}

/*get the extra value according to different instruction type*/
int get_extra(int bi_word, Instr_info instr, unsigned int addr) {
	int extra;
	unsigned int tmp;
	if (check_extra(instr.srcs))
		return -50000;
	Type instr_type = instr.type;
	if (instr_type == RTYP) {
		/*need a unsigned tmp*/
		tmp = bi_word << 21;
		tmp = tmp >> 27;
		extra = tmp; /*bit moves to get the 10:6*/
	}
	else if (instr_type == ITYP) {
		/*not branch instruction, get 15:0*/
		if (check_branch(instr) == -1) {
			tmp = bi_word << 16;
			tmp = tmp >> 16;
			extra = tmp;
		}
		/*branch instruction*/
		else if (check_branch(instr) != -1) {
			tmp = bi_word << 16;
			printf("The tmp is: %x\n", tmp);
			tmp = tmp >> 16;
			printf("The tmp is: %x\n", tmp);
			int sign = tmp >> 15;
			printf("The sign is: %x\n", sign);
			/*positive number*/
			if (sign == 0) {
				tmp = tmp << 2; /*shift 2 bits*/
				printf("The temp hex of extra is: %x\n", tmp);
			}
			/*negative number*/
			else if (sign == 1) {
				tmp = tmp | 0xFFFF0000; /*extends to 32 bit*/
				tmp = tmp << 2; /*shift 2 bits*/
				printf("The temp hex of extra is: %x\n", tmp);
				tmp = -1 * (~tmp + 1);
			}
			extra = addr + 4 + tmp;
			printf("The branch extra is: %d\n", extra);
		}
	}
	else if (instr_type == JTYP) {
		unsigned int unsigned_extra;
		unsigned_extra = bi_word << 8;
		unsigned_extra = unsigned_extra >> 6; /*get the 25:0 26 bits and left shift 2 bits*/
		printf("The j type 25:0 is: %x\n", unsigned_extra);
		unsigned int modified_addr = (addr + 4);// & 0x10000000; /*clearing out 28 lsb*/
		unsigned int least_28bits = modified_addr & 0xfffffff; /*get the 28 least most significant bits*/
		modified_addr = modified_addr - least_28bits;
		printf("The base address is: %x\n", addr);
		printf("The modified_addr is: %x\n", modified_addr);
		unsigned_extra = unsigned_extra + modified_addr;
		return unsigned_extra;
	}

	return extra;
}

/*check if it is a branch instruction*/
int check_branch(Instr_info instr) {
	for (int i=0; i<8; i++) {
		if (instr.opcode == branchTable[i])
			return i;
	}
	return -1;
}

/*check if extra in source*/
int check_extra(Source srcs[3]) {
	for (int i=0; i<3; i++) {
		if (srcs[i] == EXTRA)
			return 0;
	}
	return 1;
}