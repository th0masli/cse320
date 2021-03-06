/*
 * Routines for making a HTTP connection to a server on the Internet,
 * sending a simple HTTP GET request, and interpreting the response headers
 * that come back.
 *
 * E. Stark, 11/18/97 for CSE 230
 */
#define _GNU_SOURCE //for fgetline

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <signal.h>
#include <sys/socket.h>

#include "url.h"
#include "http.h"

typedef struct HDRNODE *HEADERS;
HEADERS http_parse_headers(HTTP *http);
void http_free_headers(HEADERS env);

/*
 * Routines to manage HTTP connections
 */

typedef enum { ST_REQ, ST_HDRS, ST_BODY, ST_DONE } HTTP_STATE;

struct http {
  FILE *file;			// Stream to remote server
  HTTP_STATE state;		// State of the connection
  int code;			// Response code
  char version[4];		// HTTP version from the response
  char *response;		// Response string with message
  HEADERS headers;		// Reply headers
};


/*
 * Open an HTTP connection for a specified IP address and port number
 */

HTTP *
http_open(IPADDR *addr, int port)
{
  HTTP *http;
  struct sockaddr_in sa;
  int sock;

  if(addr == NULL) {
    //printf("The address is NULL.\n");
    return(NULL);
  }
  if((http = malloc(sizeof(*http))) == NULL) {
    //printf("Malloc for http is NULL.\n");
    return(NULL);
  }
  bzero(http, sizeof(*http));
  if((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
    //printf("The socket is below 0.\n");
    free(http);
    return(NULL);
  }
  bzero(&sa, sizeof(sa));
  sa.sin_family = AF_INET;
  sa.sin_port = htons(port);
  bcopy(addr, &sa.sin_addr.s_addr, sizeof(struct in_addr));
  if(connect(sock, (struct sockaddr *)(&sa), sizeof(sa)) < 0
     || (http->file = fdopen(sock, "w+")) == NULL) {
    //printf("Connection failed.\n");
    free(http);
    close(sock);
    return(NULL);
  }
  http->state = ST_REQ;
  return(http);
}

/*
 * Close an HTTP connection that was previously opened.
 */

int
http_close(HTTP *http)
{
  int err;

  err = fclose(http->file);
  free(http->response);
  http_free_headers(http->headers); // Addded to free the header
  free(http);
  return(err);
}

/*
 * Obtain the underlying FILE in an HTTP connection.
 * This can be used to issue additional headers after the request.
 */

FILE *
http_file(HTTP *http)
{
  return(http->file);
}

/*
 * Issue an HTTP GET request for a URL on a connection.
 * After calling this function, the caller may send additional
 * headers, if desired, then must call http_response() before
 * attempting to read any document returned on the connection.
 */

int http_request(HTTP *http, URL *up)
{
  void *prev;

  if(http->state != ST_REQ)
    return(1);
  /* Ignore SIGPIPE so we don't die while doing this */
  prev = signal(SIGPIPE, SIG_IGN);
  if(fprintf(http->file, "GET %s://%s:%d%s HTTP/1.0\r\nHost: %s\r\n",
	     url_method(up), url_hostname(up), url_port(up),
	     url_path(up), url_hostname(up)) == -1) {
    signal(SIGPIPE, prev);
    return(1);
  }
  http->state = ST_HDRS;
  signal(SIGPIPE, prev);
  return(0);
}

/*
 * Finish outputting an HTTP request and read the reply
 * headers from the response.  After calling this, http_getc()
 * may be used to collect any document returned as part of the
 * response.
 */

int
http_response(HTTP *http)
{
  void *prev;
  char *response = NULL;
  //int len; // changed to size_t
  size_t len = 0; // why?

  if(http->state != ST_HDRS)
    return(1);
  /* Ignore SIGPIPE so we don't die while doing this */
  prev = signal(SIGPIPE, SIG_IGN);
  if(fprintf(http->file, "\r\n") == -1
     || fflush(http->file) == EOF) {
    signal(SIGPIPE, prev);
    return(1);
  }
  rewind(http->file);
  signal(SIGPIPE, prev);
  //response = fgetln(http->file, &len);
  //response = malloc(len+1);
  if(!getline(&response, &len, http->file)) {
    free(response);
    return(1);
  }
  if(response == NULL
     || (http->response = malloc(len+1)) == NULL) {
    //free(response);
    free(http->response);
    return(1);
  }
  strncpy(http->response, response, len);
  free(response);
  do
    http->response[len--] = '\0';
  //while(len >= 0 && //comparison of unsigned expression >= 0 is always true
  while(len > 0 &&
	(http->response[len] == '\r'
	 || http->response[len] == '\n'));
  if(sscanf(http->response, "HTTP/%3s %d ", http->version, &(http->code)) != 2)
    return(1);
  http->headers = http_parse_headers(http);
  //http->headers = NULL;
  http->state = ST_BODY;
  return(0);
}

/*
 * Retrieve the HTTP status line and code returned as the
 * first line of the response from the server
 */

char *
http_status(HTTP *http, int *code)
{
  if(http->state != ST_BODY)
    return(NULL);
  if(code != NULL)
    *code = http->code;
  //int code_len = strlen(http->response);
  //printf("The response code length is: %d\n", code_len);
  http->response = remove_space(http->response);
  //code_len = strlen(http->response);
  //printf("Now the response code length is: %d\n", code_len);
  return(http->response);
}

/*
 * Read the next character of a document from an HTTP connection
 */

int
http_getc(HTTP *http)
{
  if(http->state != ST_BODY)
    return(EOF);
  return(fgetc(http->file));
}

/*
 * Routines for parsing the RFC822-style headers that come back
 * as part of the response to an HTTP request.
 */

typedef struct HDRNODE {
    char *key;
    char *value;
    struct HDRNODE *next;
} HDRNODE;


/*
 * Function for parsing RFC 822 header lines directly from input stream.
 */

HEADERS
http_parse_headers(HTTP *http)
{
    FILE *f = http->file;
    HEADERS env = NULL, last = NULL;
    HDRNODE *node;
    // maybe?
    //HEADERS node;
    //int len; //changed to type size_t
    size_t len = 0; //why?
    char *line, *l, *ll, *cp;
    ll = NULL; // initiate the buffer to NULL
    ssize_t l_size; // Added for the return value of getline

    //while((ll = fgetln(f, &len)) != NULL) {
    while((l_size = getline(&ll, &len, f)) != -1) {
      if (!strcasecmp(ll, "\r\n")) {
        //printf("It's over!\n");
        break;
      }
      /*
      printf("The getline result is: %lu\n", l_size);
      printf("The buffer now is: %s\n", ll);
      */
      /*
      printf("The header is :%s\n", ll);
      printf("=====================\n");
      */
      //printf("The len is: %lu\n", sizeof(len));
      line = l = malloc(len+1);
    	l[len] = '\0';
    	strncpy(l, ll, len); //make a copy of read line buffer
      //free(ll); // free the buffer when reading ends
      // get rid of \r and \n
    	while(len > 0 && (l[len-1] == '\n' || l[len-1] == '\r'))
    	    l[--len] = '\0';
      // if len is 0 after eliminating \r and \n then it should terminate
    	if(len == 0) {
    	   free(line);
    	   break;
    	}
    	node = malloc(sizeof(HDRNODE));
    	node->next = NULL;
      // maybe get rid of the front space in l?
    	for(cp = l; *cp == ' '; cp++)
      ;
    	l = cp;
    	for( ; *cp != ':' && *cp != '\0'; cp++)
      ;
  	  if(*cp == '\0' || *(cp+1) != ' ') {
  	     free(line);
  	     free(node);
  	     continue;
  	  }
    	*cp++ = '\0';
    	node->key = strdup(l);
      char *tmp = node->key;
      while (*tmp != '\0') { // || tmp != '\n' || tmp != '\r') {
        if (isupper(*tmp))
          *tmp = tolower(*tmp);
        tmp++;
      }
    	while(*cp == ' ')
    	    cp++;
    	node->value = strdup(cp);
    	//for(cp = node->key; *cp != NULL; cp++) // comparison between pointer and integer(char) ('int' and 'void *')
      /*
      for(cp = node->key; cp != NULL; cp++) {
          if(isupper(*cp))
            *cp = tolower(*cp);
      }
      */
      if (env != NULL) {
        last->next = node;
        last = node;
      }
      if (env == NULL) {
        env = node;
        last = env;
      }
      /*
      last->next = node;
      last = node;
      */
      //free(l);
      //free(node);
      free(line); // free line will free both l and cp
      node = NULL;
    }
    free(node);
    free(ll);
    //printf("The second header is: %s\n", (env->next)->key);

    return(env);
}

/*
 * Free headers previously created by http_parse_headers()
 */

void
http_free_headers(HEADERS env)
{
    HEADERS next;

    while(env != NULL) {
    	free(env->key);
    	free(env->value);
    	next = env->next;
    	free(env);
    	env = next;
    }
}

/*
 * Find the value corresponding to a given key in the headers
 */

char *
http_headers_lookup(HTTP *http, char *key)
{
    HEADERS env = http->headers;
    while(env != NULL) {
      //printf("%s: ", env->key);
      //printf("%s\n", env->value);
    	if(!strcmp(env->key, key)) {
        env->value = remove_space(env->value);
        return(env->value);
      }
    	env = env->next;
    }
    return(NULL);
}

/*
 * removing the response status end spaces
 */
char *remove_space(char* status) {
    char *tmp = status;
    /*
    while (*tmp != '\0')
      tmp++;
    while (*(tmp-1) == ' ' || *(tmp-1) == '\n' || *(tmp-1) == '\r') {
      tmp--;
    }

    *tmp = '\0';
    */
    while (*tmp != '\r')
      tmp++;
    *tmp = '\0';

    return status;
}

/* search for multiple keywords in response headers */
/*
HEADERS search_headers(HEADERS env, char *keywords[]) {
    HEADERS new_env = NULL;
    HEADERS tmp = NULL;
    while (keywords) {
      tmp = env;
      while (tmp) {
        // compare the header node's key with the keyword argument
        if (!strcasecmp(tmp->key, *keywords)) {
          if (new_env == NULL) {
            new_env = tmp;
          } else {
            new_env->next = tmp;
          }
        }
        tmp++; // linear search the response header list
      }
      keyword++;
    }

    return new_env;
}
*/
