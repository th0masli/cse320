#include <stdio.h>
#include "hw1.h"

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