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

int main(int argc, char **argv)
{
    if(!validargs(argc, argv)) {
    	printf("Failed\n");
        USAGE(*argv, EXIT_FAILURE);
    }
    /*test validargs result*/
    if (validargs(argc, argv)) {
    	printf("Success\n");
    	printf("The global option is: %x\n", global_options);
    	Instruction ip;
        unsigned int addr = 0;
        //ip.value = 0x00c72820; /*e.g 1*/
        //ip.value = 0x8cc50007; /*e.g 2*/
    	//ip.value = 0x10effc1f; /*e.g 3*/
        //addr = 0x1000; /*e.g 3*/
        ip.value = 0x08000400; /*e.g 4*/
        addr = 0x40000000; /*e.g 4*/
    	decode(&ip, addr);
    }
    debug("Options: 0x%X", global_options);
    if(global_options & 0x1) {
    	printf("Bingo\n");
        USAGE(*argv, EXIT_SUCCESS);
    }

    return EXIT_SUCCESS;
}

/*
 * Just a reminder: All non-main functions should
 * be in another file not named main.c
 */
