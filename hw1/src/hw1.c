#include "hw1.h"

#ifdef _STRING_H
#error "Do not #include <string.h>. You will get a ZERO."
#endif

#ifdef _STRINGS_H
#error "Do not #include <strings.h>. You will get a ZERO."
#endif

#ifdef _CTYPE_H
#error "Do not #include <ctype.h>. You will get a ZERO."
#endif

/*
 * You may modify this file and/or move the functions contained here
 * to other source files (except for main.c) as you wish.
 */

/**
 * @brief Validates command line arguments passed to the program.
 * @details This function will validate all the arguments passed to the
 * program, returning 1 if validation succeeds and 0 if validation fails.
 * Upon successful return, the selected program options will be set in the
 * global variable "global_options", where they will be accessible
 * elsewhere in the program.
 *
 * @param argc The number of arguments passed to the program from the CLI.
 * @param argv The argument strings passed to the program from the CLI.
 * @return 1 if validation succeeds and 0 if validation fails.
 * Refer to the homework document for the effects of this function on
 * global variables.
 * @modifies global variable "global_options" to contain a bitmap representing
 * the selected options.
 */

int strcmp(char *str0, char *str1); /*string compare*/
int slen(char *str); /*string length*/
int str_hex(char *str); /*convert str to hex number*/
/*num_converter.c*/
int bin_dec(int bin); /*convert binary number to decimal*/
/*encode_decode_helper.c*/
int search_instr(Opcode target_op); /*linear search instrTable; return the index of the instruction, return -1 if not found*/
/*instruction*/
//Opcode opcodeTable[];
//Opcode specialTable[];
//Instr_info instrTable[];

/*global var for flags*/
char *flag_h = "-h"; /*help menu*/
char *flag_a = "-a"; /*assembly: text-to-binary*/
char *flag_d = "-d"; /*disassembly: binary-to-text*/
char *flag_b = "-b"; /*base address*/
char *flag_e = "-e"; /*endiness*/

int check_flag(char *arg, char *flag); /*checking flags*/
int valid_baddr(char *n); /*checking base address*/
int check_0(char *n); /*check if all digits are 0*/
int valid_hex(char *n); /*check if n is a valid hex number*/

int four_args(int a_first, int d_first, char *opt_flag, char *param_spec); /*validating and setting global options for argc==4*/
int six_args(int a_first, int d_first, char *end_spec, char *addr_spec); /*validating and setting global options for argc==6*/

int validargs(int argc, char **argv)
{
	if (argc == 1) /*no flag provided; too few arguments*/
    	return 0;
    int h_first = check_flag(argv[1], &flag_h[0]);
    if (argc > 6 && h_first) /*too many arguments*/
    	return 0;
    /*starting with -h*/
    if (!h_first){
    	global_options = 0x1; /*change global option*/
    	return 1;
    }
    /*starting with -a -d*/
    int a_first, d_first;
    a_first = check_flag(argv[1], &flag_a[0]);
    d_first = check_flag(argv[1], &flag_d[0]);
    if (!a_first || !d_first){
    	/*only -a or -d*/
    	if (argc == 2) {
            if (!d_first) {
                global_options = 0x2; /*change global option*/
            }
    		/*-a no need change global option*/
    		/*stdin & stdout*/
    		return 1;
    	}
    	/*1 optional argument; it can only be -e or -b*/
    	if (argc == 4) {
    		int fi = argc - 2; /*index of optional arguments*/
            char *opt_flag = argv[fi]; /*address of optional argument*/
            char *param_spec = argv[fi+1]; /*address of optional arguments' specification*/
    		int fourargs_res = four_args(a_first, d_first, opt_flag, param_spec);
            if (!fourargs_res) {
                /*stdin & stdout*/
                return 1;
            }
    	}
    	/*2 optional arguments*/
    	if (argc == 6) {
    		int f0, f1; /*the index of 1st and 2nd optional arguments*/
    		f0 = argc - 4;
    		f1 = argc - 2;
    		/*-e is 1st and -b is 2nd*/
    		if (!check_flag(argv[f0], &flag_e[0]) && !check_flag(argv[f1], &flag_b[0])){
                char *end_spec = argv[f0+1];
                char *addr_spec = argv[f1+1];
                int sixargs_res = six_args(a_first, d_first, end_spec, addr_spec);
                if (!sixargs_res) {
                    /*stdin & stdout*/
                    return 1;
                }
    		}
    		/*-b is first and -e is 2nd*/
    		if (!check_flag(argv[f0], &flag_b[0]) && !check_flag(argv[f1], &flag_e[0])){
    			char *end_spec = argv[f1+1];
                char *addr_spec = argv[f0+1];
                int sixargs_res = six_args(a_first, d_first, end_spec, addr_spec);
                if (!sixargs_res) {
                    /*stdin & stdout*/
                    return 1;
                }
    		}
    	}
    }
    
    return 0;
}

/**
 * @brief Computes the binary code for a MIPS machine instruction.
 * @details This function takes a pointer to an Instruction structure
 * that contains information defining a MIPS machine instruction and
 * computes the binary code for that instruction.  The code is returne
 * in the "value" field of the Instruction structure.
 *
 * @param ip The Instruction structure containing information about the
 * instruction, except for the "value" field.
 * @param addr Address at which the instruction is to appear in memory.
 * The address is used to compute the PC-relative offsets used in branch
 * instructions.
 * @return 1 if the instruction was successfully encoded, 0 otherwise.
 * @modifies the "value" field of the Instruction structure to contain the
 * binary code for the instruction.
 */
int encode(Instruction *ip, unsigned int addr) {
    
    return 0;
}

/**
 * @brief Decodes the binary code for a MIPS machine instruction.
 * @details This function takes a pointer to an Instruction structure
 * whose "value" field has been initialized to the binary code for
 * MIPS machine instruction and it decodes the instruction to obtain
 * details about the type of instruction and its arguments.
 * The decoded information is returned by setting the other fields
 * of the Instruction structure.
 *
 * @param ip The Instruction structure containing the binary code for
 * a MIPS instruction in its "value" field.
 * @param addr Address at which the instruction appears in memory.
 * The address is used to compute absolute branch addresses from the
 * the PC-relative offsets that occur in the instruction.
 * @return 1 if the instruction was successfully decoded, 0 otherwise.
 * @modifies the fields other than the "value" field to contain the
 * decoded information about the instruction.
 */
int decode(Instruction *ip, unsigned int addr) {
    int bi_word = ip->value; /*binary instruction word*/
    int bi_op_index = bi_word >> 26; /*move right for 26 bits to get the bits 31:26*/
    //int op_index = bin_dec(bi_op_index); /*no need to convert?*/
    int op_index = bi_op_index;
    //printf("The converted decimal result is: %d\n", op_index);
    /*opcode is special*/
    if (op_index == 0) {
        int bi_spec_index = bi_word << 26; /*5:0*/
        bi_spec_index = bi_spec_index >> 26;
        int spec_index = bin_dec(bi_spec_index);
        Opcode spec_op = specialTable[spec_index];

        return 1;
    }
    /*opcode is bcond*/
    else if (op_index == 1) {
        int bi_bcond_index = bi_word << 11; /*20:16*/
        bi_bcond_index = bi_bcond_index >> 26;
        //int bcond_index = bin_dec(bi_bcond_index);

        return 1;
    }
    else {
        printf("The opcode index is: %d\n", op_index);
        Opcode op = opcodeTable[op_index];
        /*linear search instrTable*/
        int instr_index = search_instr(op);
        //int instr_index = 3; /*for test*/
        Instr_info instr = instrTable[instr_index];
        ip->info = &instr;
        printf("The instruction is: %s\n", instr.format);
    }

    return 0;
}



/*checking what flag provided*/
int check_flag(char *arg, char *flag) {
	int res = strcmp(arg, &flag[0]);
	//printf("The flag is: %s\n", flag);
	//printf("The result is: %d\n", res);
	if (!res)
		return 0;
	return 1;
}


/*checking if it is a hex number & length <8 or multiple of 4096*/
int valid_baddr(char *n) {
	int len = slen(n); /*check length*/
    int hex_res = valid_hex(n); /*check if hex*/
    /*0 is also valid*/
    if (len < 4 && !check_0(n))
        return 0;
    if (len > 3 && len < 9 && !hex_res) {
        char *last3 = n + len - 3; /*starting pointer of the last 3 digits*/
        if (!check_0(last3))
            return 0;
    }

    return 1;
}


/*checking if it is a valid hex number*/
int valid_hex(char *n) {
	while (*n) {
		if ((*n >= '0' && *n <= '9') || (*n >= 'a' && *n <= 'f') || (*n >= 'A' && *n <= 'F'))
			n++;
		else
			return 1; /*not a valid hex number*/
	}

	return 0;
}


/*check if all zero*/
int check_0(char *n) {
    while (*n) {
        if (*n == '0')
            n++;
        else
            return 1;
    }

    return 0;
}


/*validating and setting global options for argc == 4*/
int four_args(int a_first, int d_first, char *opt_flag, char *param_spec) {
    /*-e is the optional argument; check its next valid or not*/
    if (!check_flag(opt_flag, &flag_e[0])) {
        if (*param_spec == 'b' || *param_spec == 'l') {
            /*change global option*/
            if (!a_first && *param_spec == 'b')
                global_options = 0x4;
            else if (!d_first && *param_spec == 'b')
                global_options = 0x6;
            else if (!d_first && *param_spec == 'l')
                global_options = 0x2;
            /*stdin & stdout*/
            return 0;
        }
    }
    /*-b is the optional argument; check its next valid or not*/
    if (!check_flag(opt_flag, &flag_b[0])) {
        /*check if argv[fi+1] is a valid base address*/
        //printf("The argv[fi+1] is: %p\n", argv[fi+1]);
        int baddr_res = valid_baddr(param_spec); /*get the int format base address*/
        //printf("The base address check result is: %d\n", baddr_res);
        if (!baddr_res) {
            /*change global option*/
            int addr_int = str_hex(param_spec);
            if (!d_first)
                global_options = addr_int + 0x2;
            else
                global_options = addr_int;
            /*stdin & stdout*/
            return 0;
        }
    }

    return 1;
}



/*validating and setting global options for argc == 6*/
int six_args(int a_first, int d_first, char *end_spec, char *addr_spec) {
    int baddr_res = valid_baddr(addr_spec); /*need check if -b followed by a valid argument*/
    if ((*end_spec == 'b' || *end_spec == 'l') && !baddr_res) {
        /*change global option*/
        int addr_int = str_hex(addr_spec); /*get the int format base address*/
        if (!a_first && *end_spec == 'b')
            global_options = addr_int + 0x4;
        else if (!a_first && *end_spec == 'l')
            global_options = addr_int;
        else if (!d_first && *end_spec == 'b')
            global_options = addr_int + 0x6;
        else if (!d_first && *end_spec == 'l')
            global_options = addr_int + 0x2;
        /*stdin & stdout*/
        return 0;
    }

    return 1;
}


