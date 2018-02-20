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
    	//printf("Failed\n");
      USAGE(*argv, EXIT_FAILURE);
    }
    debug("Options: 0x%X", global_options);
    /*-h flag*/
    if(global_options & 0x1) {
    	//printf("Bingo\n");
      USAGE(*argv, EXIT_SUCCESS);
    }
    /*default endianness is little*/
    /*encode*/
    /*-a with or without address or little endianness*/
    if (!(global_options & 0x7)) {
      //printf("condition%d\n", 0);
      //printf("The global_options is: %x\n", global_options);
      unsigned int base_addr = global_options & 0xfffff000; //clearing out 3 lsb
      //printf("The base_addr is: %x\n", base_addr);
      //stdin
      char mnemonic[120];
      while (fgets(mnemonic, 120, stdin) != NULL) {
        char last_bit = mnemonic[slen(mnemonic)-1];
        if (last_bit == '\n')
          mnemonic[slen(mnemonic)-1] = '\0';
        //printf("The input mnemonic is: %s\n", mnemonic);
        Instruction ip; //initiate a new Instruction structure
        /*search for the right instruction information*/
        int info_index = encode_search(mnemonic, &ip);
        //printf("The info index is: %d\n", info_index);
        if (info_index == -1)
          exit(EXIT_FAILURE);
        //printf("The index of the instr info is: %d\n", info_index);
        //printf("The format is: %s\n", instrTable[info_index].format);
        ip.info = &instrTable[info_index];
        //printf("The instruction is: %s\n", ip.info->format);
        int encode_res = encode(&ip, base_addr);
        base_addr += 4;
        if (!encode_res)
          exit(EXIT_FAILURE);
        //printf("%x", ip.value);
        int bin_code = ip.value;
        ip.value = convert_endian(bin_code);
        //printf("%x", ip.value);
        int out_val = ip.value;
        int byte0, byte1, byte2, byte3;
        byte0 = (out_val >> 24) & 0xff;
        putchar(byte0);
        byte1 = (out_val >> 16) & 0xff;
        putchar(byte1);
        byte2 = (out_val >> 8) & 0xff;
        putchar(byte2);
        byte3 = out_val & 0xff;
        putchar(byte3);
      }
      //stdout
    }
    /*-a with big endianness and with or without address*/
    if ((global_options & 0x7) == 0x4) {
      //printf("condition%d\n", 1);
      //printf("The global_options is: %x\n", global_options);
      unsigned int base_addr = global_options & 0xfffff000; //clearing out 3 lsb
      //printf("The base_addr is: %x\n", base_addr);
      //stdin
      char mnemonic[120];
      while (fgets(mnemonic, 120, stdin) != NULL) {
        char last_bit = mnemonic[slen(mnemonic)-1];
        if (last_bit == '\n')
          mnemonic[slen(mnemonic)-1] = '\0';
        //printf("The input mnemonic is: %s\n", mnemonic);
        Instruction ip; //initiate a new Instruction structure
        /*search for the right instruction information*/
        int info_index = encode_search(mnemonic, &ip);
        if (info_index == -1)
          exit(EXIT_FAILURE);
        //printf("The index of the instr info is: %d\n", info_index);
        //printf("The format is: %s\n", instrTable[info_index].format);
        ip.info = &instrTable[info_index];
        int encode_res = encode(&ip, base_addr);
        base_addr += 4;
        if (!encode_res)
          exit(EXIT_FAILURE);
        int out_val = ip.value;
        int byte0, byte1, byte2, byte3;
        byte0 = (out_val >> 24) & 0xff;
        putchar(byte0);
        byte1 = (out_val >> 16) & 0xff;
        putchar(byte1);
        byte2 = (out_val >> 8) & 0xff;
        putchar(byte2);
        byte3 = out_val & 0xff;
        putchar(byte3);
      }
      //stdout
    }
    /*decode*/
    /*-d with or without address or little endianness*/
    if ((global_options & 0x7) == 0x2) {
      //printf("condition%d\n", 2);
      //printf("The global_options is: %x\n", global_options);
      unsigned int base_addr = global_options & 0xfffff000; //clearing out 3 lsb
      int byte[4];
      int n = 1;
      while (n) {
        for (int i=0; i<4; i++) {
          byte[i] = getchar();
          //printf("The current byte is: %x\n", byte[i]);
          if (byte[i] == -1)
            break;
        }
        if (byte[0] == -1)
          break;
        int bin_instr = byte[0]*0x1000000 + byte[1]*0x10000 + byte[2]*0x100 + byte[3];
        bin_instr = convert_endian(bin_instr);
        //printf("The instruction is: %x\n", bin_instr);
        Instruction ip; //initiate a new Instruction structure
        ip.value = bin_instr;
        int decode_res = decode(&ip, base_addr);
        base_addr += 4;
        if (!decode_res)
          exit(EXIT_FAILURE);
        ip.value = convert_endian(bin_instr);
        print_decode(&ip);
      }
    }
    /*-d with big endianness and with or without address*/
    if ((global_options & 0x7) == 0x6) {
      //printf("condition%d\n", 3);
      //printf("The global_options is: %x\n", global_options);
      unsigned int base_addr = global_options & 0xfffff000; //clearing out 3 lsb
      //printf("The base address is: %x\n", base_addr);
      int byte[4];
      int n = 1;
      while (n) {
        for (int i=0; i<4; i++) {
          byte[i] = getchar();
          //printf("The current byte is: %x\n", byte[i]);
          if (byte[i] == -1)
            break;
        }
        if (byte[0] == -1)
          break;
        int bin_instr = byte[0]*0x1000000 + byte[1]*0x10000 + byte[2]*0x100 + byte[3];
        //bin_instr = convert_endian(bin_instr);
        //printf("The instruction is: %x\n", bin_instr);
        Instruction ip; //initiate a new Instruction structure
        ip.value = bin_instr;
        int decode_res = decode(&ip, base_addr);
        base_addr += 4;
        if (!decode_res)
          exit(EXIT_FAILURE);
        //ip.value = convert_endian(bin_instr);
        print_decode(&ip);
      }
    }

    return EXIT_SUCCESS;
}

/*
 * Just a reminder: All non-main functions should
 * be in another file not named main.c
 */
