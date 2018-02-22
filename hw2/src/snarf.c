/*
 * Example of the use of the HTTP package for "Internet Bumbler"
 * E. Stark, 11/18/97 for CSE 230
 *
 * This program takes a single argument, which is interpreted as the
 * URL of a document to be retrieved from the Web.  It uses the HTTP
 * package to parse the URL, make an HTTP connection to the remote
 * server, retrieve the document, and display the result code and string,
 * the response headers, and the data portion of the document.
 */

#include <stdio.h>
#include <string.h>
// Added stdlib
#include <stdlib.h>

#include "debug.h"
#include "http.h"
#include "url.h"
#include "snarf.h"
#include <ctype.h>

int
main(int argc, char *argv[])
{
  URL *up;
  HTTP *http;
  IPADDR *addr;
  int port, c, code;
  char *status, *method;
  //list for recording keywords
  char *keywords[argc];

  /*
  printf("The number of arguments are: %d\n", argc);
  printf("With args: %s\n", argv[2]);
  */
  //printf("The url get is: %s\n", url_to_snarf);

  //parse_args(argc, argv);
  parse_args(argc, argv, keywords);
  if((up = url_parse(url_to_snarf)) == NULL) {
    fprintf(stderr, "Illegal URL: '%s'\n", argv[1]);
    //fprintf(stderr, "Illegal URL: '%s'\n", url_to_snarf); // that is the right way to print out!
    exit(1);
  }
  method = url_method(up);
  addr = url_address(up);
  port = url_port(up);
  if(method == NULL || strcasecmp(method, "http")) {
    fprintf(stderr, "Only HTTP access method is supported\n");
    url_free(up); // added free url
    exit(1);
  }
  if((http = http_open(addr, port)) == NULL) {
    fprintf(stderr, "Unable to contact host '%s', port %d\n",
	    url_hostname(up) != NULL ? url_hostname(up) : "(NULL)", port);
    url_free(up); // added free url
    exit(1);
  }
  http_request(http, up);
  /*
   * Additional RFC822-style headers can be sent at this point,
   * if desired, by outputting to url_file(up).  For example:
   *
   *     fprintf(url_file(up), "If-modified-since: 10 Jul 1997\r\n");
   *
   * would activate "Conditional GET" on most HTTP servers.
   */
  http_response(http);
  /*
   * At this point, response status and headers are available for querying.
   *
   * Some of the possible HTTP response status codes are as follows:
   *
   *	200	Success -- document follows
   *	302	Document moved
   *	400	Request error
   *	401	Authentication required
   *	403	Access denied
   *	404	Document not found
   *	500	Server error
   *
   * You probably want to examine the "Content-type" header.
   * Two possibilities are:
   *    text/html	The body of the document is HTML code
   *	text/plain	The body of the document is plain ASCII text
   */
  status = http_status(http, &code);
  //printf("The status is: %s\n", status);
  //printf("The code is: %d\n", code);
#ifdef DEBUG
  debug("%s", status);
#else
  (void) status;
#endif
  /*for loop for searching the keywords list*/
  char *key, *value;
  for (int i = 0; keywords[i] != NULL; i++) {
    key = keywords[i];
    // headers is not case sensitive
    char *key_to_lower = strdup(key);
    char *tmp = key_to_lower;
    while (*tmp != '\0') {
      if (isupper(*tmp))
        *tmp = tolower(*tmp);
      tmp++;
    }
    value = http_headers_lookup(http, key_to_lower);
    if (value) {
      // should print to stderr
      /*
      printf("%s: ", key);
      printf("%s", value);
      */
      fprintf(stderr, "%s: ", key);
      fprintf(stderr, "%s\n", value);
    }
  }

  //printf("The 2nd search result is: %s\n", (search_res->next)->value);
  /*
   * At this point, we can retrieve the body of the document,
   * character by character, using http_getc()
   */
  while((c = http_getc(http)) != EOF)
    putchar(c);
  //need to free http->headers
  //http_free_headers(http->headers);
  http_close(http);
  url_free(up);
  return(0);
}
