#include <stdlib.h>

#ifdef _STRING_H
#error "Do not #include <string.h>. You will get a ZERO."
#endif

#ifdef _STRINGS_H
#error "Do not #include <strings.h>. You will get a ZERO."
#endif

#ifdef _CTYPE_H
#error "Do not #include <ctype.h>. You will get a ZERO."
#endif

#include "hw1.h"
#include "debug.h"

/*search for the right instruction information according to the input mnemonic*/
//int encode_search(char *mnemonic, Instruction *ip);

int main(int argc, char **argv)
{
    if(!validargs(argc, argv)) {
    	printf("Failed\n");
        USAGE(*argv, EXIT_FAILURE);
    }
    /*test validargs result*/
    //if (validargs(argc, argv)) {
    	//printf("Success\n");
    	//printf("The global option is: %x\n", global_options);
    	//Instruction ip;
      /*decode*/
      /*
      unsigned int addr = 0;
      //ip.value = 0x00c72820; //e.g 1
      //ip.value = 0x8cc50007; //e.g 2
  	  //ip.value = 0x10effc1f; //e.g 3
      //addr = 0x1000; //e.g 3
      ip.value = 0x08000400; //e.g 4
      addr = 0x40000000; //e.g 4
    	decode(&ip, addr);
      */
      /*encode*/
      //unsigned int addr = 0;
      /*bcond test case*/
      /*
      Instr_info bcond_info = {OP_BLTZAL,  ITYP, {RS, EXTRA, NSRC},   "bltzal $%d,%d"    };
      ip.info = &bcond_info;
      ip.extra = 0x80;
      ip.args[0] = 0x1;
      */
      /*special test case e.g1*/
      /*
      Instr_info spec_info = {OP_ADD,     RTYP, {RD, RS, RT},        "add $%d,$%d,$%d"  };
      ip.info = &spec_info;
      ip.args[0] = 5;
      ip.args[1] = 6;
      ip.args[2] = 7;
      */
      /*e.g2*/
      /*
      Instr_info info = {OP_LW,      ITYP, {RT, EXTRA, RS},     "lw $%d,%d($%d)"   };
      ip.info = &info;
      ip.args[0] = 5;
      ip.args[1] = 7;
      ip.args[2] = 6;
      ip.extra = 7;
      */
      /*e.g3*/
      /*
      addr = 0x1000;
      Instr_info info = {OP_BEQ,     ITYP, {RS, RT, EXTRA},     "beq $%d,$%d,%d"   };
      ip.info = &info;
      ip.args[0] = 7;
      ip.args[1] = 15;
      ip.args[2] = 128;
      ip.extra = 128;
      */
      /*e.g4*/
      /*
      addr = 0x40000000;
      Instr_info info = {OP_J,       JTYP, {EXTRA, NSRC, NSRC}, "j 0x%x"           };
      ip.info = &info;
      ip.args[0] = 0x40001000;
      ip.extra = 0x40001000;

      encode(&ip, addr);
    }*/
    debug("Options: 0x%X", global_options);
    /*-h flag*/
    if(global_options & 0x1) {
    	printf("Bingo\n");
      USAGE(*argv, EXIT_SUCCESS);
    }
    /*default endianness is little*/
    /*encode*/
    /*-a with or without address or little endianness*/
    if (!(global_options & 0x7)) {
      printf("The global_options is: %x\n", global_options);
      unsigned int base_addr = global_options & 0xfffff000; //clearing out 3 lsb
      printf("The base_addr is: %x\n", base_addr);
      //stdin
      char mnemonic[120];
      while (fgets(mnemonic, 120, stdin) != NULL) {
        printf("The input mnemonic is: %s\n", mnemonic);
        Instruction ip; //initiate a new Instruction structure
        /*search for the right instruction information*/
        int info_index = encode_search(mnemonic, &ip);
        //printf("The index of the instr info is: %d\n", info_index);
        printf("The format is: %s\n", instrTable[info_index].format);
        ip.info = &instrTable[info_index];
        int encode_res = encode(&ip, base_addr);
        if (!encode_res)
          exit(EXIT_FAILURE);
      }
      //stdout
    }
    /*-a with big endianness and with or without address*/
    if ((global_options & 0x4) == 0x4) {

    }
    /*decode*/
    /*-d with or without address or little endianness*/
    if ((global_options & 0x2) == 0x2) {
      unsigned int base_addr = global_options & 0xfffff000; //clearing out 3 lsb

    }
    /*-d with big endianness and with or without address*/
    if ((global_options & 0x6) == 0x6) {

    }

    return EXIT_SUCCESS;
}

/*
 * Just a reminder: All non-main functions should
 * be in another file not named main.c
 */
