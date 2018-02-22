#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
//Added unistd for getopt
//#include <unistd.h>

#include "debug.h"
#include "snarf.h"

/* these shall be initialized by the system? At least for optind for now
int opterr;
int optopt;
int optind;
char *optarg;
*/

char *url_to_snarf;
char *output_file;

void
parse_args(int argc, char *argv[], char *keywords[])
{
  int i;
  char option;

  for (i = 0; optind < argc; i++) {
    debug("%d opterr: %d", i, opterr);
    debug("%d optind: %d", i, optind);
    debug("%d optopt: %d", i, optopt);
    debug("%d argv[optind]: %s", i, argv[optind]);
    opterr = 0; // prevent system error message in order to print self-defined error message
    //if ((option = getopt(argc, argv, "+q:o")) != -1) {
    if ((option = getopt(argc, argv, "+q:o:")) != -1) {
      //printf("The opterr is: %d\n", opterr);
      switch (option) {
        case 'q': {
          //printf("With args optarg: %s\n", optarg);
          //printf("The q is good\n");
          *keywords = optarg;
          keywords++;
          info("Query header: %s", optarg);
          break;
        }
        case 'o': {
          //printf("The o is good\n");
          info("Output file: %s", optarg);
	        output_file = optarg;
          break;
        }
        case '?': {
          //printf("The flag char is: %c\n", optopt);
          if (optopt != 'h') {
            //printf("The opterr is: %d\n", opterr);
            fprintf(stderr, KRED "-%c is not a supported argument\n" KNRM,
                    optopt);
          }
          USAGE(argv[0]);
          exit(0);
          break;
        }
        default: {
          break;
        }
      }
    } else if(argv[optind] != NULL) {
    	info("URL to snarf: %s", argv[optind]);
    	url_to_snarf = argv[optind];
    	optind++;
    }
  }
}
