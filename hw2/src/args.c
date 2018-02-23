#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
//Added unistd for getopt
//#include <unistd.h>

#include "debug.h"
#include "snarf.h"

// these shall be initialized by the system? At least for optind for now
int opterr;
int optopt;
int optind;
char *optarg;

char *url_to_snarf;
char *output_file;

void
parse_args(int argc, char *argv[], char *origin_keywords[])
{
  int i;
  char option;
  char **keywords = origin_keywords;

  for (i = 0; optind < argc; i++) {
    debug("%d opterr: %d", i, opterr);
    debug("%d optind: %d", i, optind);
    debug("%d optopt: %d", i, optopt);
    debug("%d argv[optind]: %s", i, argv[optind]);
    //opterr = 0; // prevent system error message in order to print self-defined error message
    //if ((option = getopt(argc, argv, "+q:o")) != -1) {
    if ((option = getopt(argc, argv, "+q:o:")) != -1) {
      //printf("The opterr is: %d\n", opterr);
      switch (option) {
        case 'q': {
          printf("The q is good\n");
          if (url_to_snarf == NULL) {
            if (!strcasecmp(optarg, "-o")) {
            //free(keywords);
            free(origin_keywords);
            exit(-1);
            }
            //printf("The header is: %s\n", optarg);
            *keywords = optarg;
            keywords++;
            info("Query header: %s", optarg);
            break;
          } else {
            /* URL come before flags */
            //free(keywords);
            free(origin_keywords);
            exit(-1);
          }
        }
        case 'o': {
          printf("The o is good\n");
          if (url_to_snarf == NULL) {
            if (!strcasecmp(optarg, "-q")) {
            //free(keywords);
            free(origin_keywords);
            exit(-1);
          }
            info("Output file: %s", optarg);
            if (output_file == NULL) {
              output_file = optarg;
            } else {
              //free(keywords);
              free(origin_keywords);
              exit(-1);
            }
            break;
          } else {
            /* URL come before flags */
            //free(keywords);
            free(origin_keywords);
            exit(-1);
          }
        }
        case '?': {
          //printf("The flag char is: %c\n", optopt);
          //free(keywords);
          free(origin_keywords);
          if (optopt != 'h') {
            //printf("The opterr is: %d\n", opterr);
            fprintf(stderr, KRED "-%c is not a supported argument\n" KNRM,
                    optopt);
            USAGE(argv[0]);
            exit(-1); // exit status should be -1
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
    	//url_to_snarf = argv[optind];
      if (url_to_snarf == NULL) {
            url_to_snarf = argv[optind];
      } else {
        //free(keywords);
        free(origin_keywords);
        exit(-1);
      }
      //printf("The url is: %s\n", url_to_snarf);
    	optind++;
    }
  }
}
