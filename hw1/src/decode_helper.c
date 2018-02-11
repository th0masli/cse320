#include <stdio.h>
#include "hw1.h"

int k_5_bit = 0x1f;

int check_branch(Instr_info instr); /*check if it is a branch instruction*/
int check_extra(Source srcs[3]); /*check if extra in source*/
/*stdout when decoding*/
void print_decode(Instruction *ip);

Opcode branchTable[10] = {
	OP_BEQ, OP_BGEZ, OP_BGEZAL, OP_BGTZ, OP_BLEZ, OP_BLTZ, OP_BLTZAL, OP_BNE
};

int tlen(Instr_info *table); /*return the length of the table*/

/*linear search instrTable*/
int search_instr(Opcode target_op) {
	Instr_info *instrTable_pointer = &instrTable[0];
	int table_len = tlen(instrTable_pointer);
	//printf("The table's length is: %d\n", table_len);
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
	int extra = 0;
	unsigned int tmp;
	Type instr_type = instr.type;
	if (instr_type == RTYP) {
		/*need a unsigned tmp*/
		//tmp = bi_word << 21;
		//tmp = tmp >> 27;
		//extra = tmp; /*bit moves to get the 10:6*/
		extra = bi_word >> 6 & 0x1f; /*bit moves to get the 10:6*/
	}
	else if (instr_type == ITYP) {
		/*not branch instruction, get 15:0*/
		if (check_branch(instr) == -1) {
			/*
			tmp = bi_word << 16;
			tmp = tmp >> 16;
			extra = tmp;
			*/
			extra = bi_word & 0xffff; /*bit moves to get the 15:0*/
		}
		/*branch instruction*/
		else if (check_branch(instr) != -1) {
			/*
			tmp = bi_word << 16;
			printf("The tmp is: %x\n", tmp);
			tmp = tmp >> 16;
			printf("The tmp is: %x\n", tmp);
			int sign = tmp >> 15;
			printf("The sign is: %x\n", sign);
			//positive number
			if (sign == 0) {
				tmp = tmp << 2; //shift 2 bits
				printf("The temp hex of extra is: %x\n", tmp);
			}
			//negative number
			else if (sign == 1) {
				tmp = tmp | 0xFFFF0000; //extends to 32 bit
				tmp = tmp << 2; //shift 2 bits
				printf("The temp hex of extra is: %x\n", tmp);
				//tmp = -1 * (~tmp + 1);
			}
			extra = (addr + 4) + tmp;
			printf("The branch extra is: %d\n", extra);
			*/
			short extra_16 = (short) bi_word & 0xffff; //get the 15:0 bits
			//printf("The 15:0 bits are: %x\n", extra_16);
			int extra_tmp = extra_16;
			//printf("The 32 length extra tmp is: %X\n", extra_tmp);
			extra_tmp = extra_tmp << 2; //left shift 2 bits
			extra = addr + 4 + extra_tmp;
			//printf("The branch extra is: %d\n", extra);
		}
	}
	else if (instr_type == JTYP) {
		unsigned int unsigned_extra;
		//unsigned_extra = bi_word << 8;
		//unsigned_extra = unsigned_extra >> 6; /*get the 25:0 26 bits and left shift 2 bits*/
		unsigned_extra = bi_word & 0x3ffffff; /*get the 25:0 26 bits*/
		//printf("The extra before left shift is: %x\n", unsigned_extra);
		unsigned_extra = unsigned_extra << 2;
		//printf("The j type 25:0 is: %x\n", unsigned_extra);
		unsigned int modified_addr = (addr + 4) & 0xf0000000; /*clearing out 28 lsb*/
		//printf("The base address is: %x\n", addr);
		//printf("The modified_addr is: %x\n", modified_addr);
		unsigned_extra = unsigned_extra + modified_addr;
		//printf("The unsigned_extra of type J in hex is: %x\n", unsigned_extra);
		//if (modified_addr != (unsigned_extra & 0xf0000000))
			//return 0;
		return unsigned_extra;
	}
	/*OP_BREAK*/
	else if (instr.opcode == OP_BREAK) {
		extra = bi_word >> 6 & 0xfffff;
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

/*fill ip when decoding*/
void fill_ip_decoding(Instruction *ip, Instr_info instr, int bi_word, unsigned int addr) {
		ip->info = &instr;
    //printf("The instruction is: %s\n", instr.format);
    //printf("The ip's info is: %s\n", ip->info->format);
    /*extra value R, I, J; if use then set*/
    if (!check_extra(instr.srcs)) {
        int extra_value = get_extra(bi_word, instr, addr);
        //printf("The extra_value in decimal is: %d\n", extra_value);
        /*will do extra instruction*/
        ip->extra = extra_value;
    }
    /*args instruction arguments*/
    for (int i=0; i < 3; i++) {
        Source src_val = instr.srcs[i];
        if (src_val == NSRC)
            ip->args[i] = 0;
        else if (src_val == RS)
            ip->args[i] = ip->regs[0];
        else if (src_val == RT)
            ip->args[i] = ip->regs[1];
        else if (src_val == RD)
            ip->args[i] = ip->regs[2];
        else if (src_val == EXTRA)
            ip->args[i] = ip->extra;
    }
    //printf("The 3rd argument is: %d\n", ip->args[2]);
		//print_decode(ip);
}

/*stdout when decoding*/
void print_decode(Instruction *ip) {
		char *format = ip->info->format;
		int *args;
		args = ip->args;
		printf(format, args[0], args[1], args[2]);
		printf("\n");
}
