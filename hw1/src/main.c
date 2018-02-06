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
