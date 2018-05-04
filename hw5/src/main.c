#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

#include "debug.h"
#include "server.h"
#include "directory.h"
#include "thread_counter.h"

#include "csapp.h"

static void terminate();


void *client_handler(void *argp);
//install sigaction handler
handler_t *install_sig_handler(int signum, handler_t *handler);
//sighup handler just kill the whole process
void sighup_handler(int sig);

/*
typedef struct thread_counter {
    unsigned int num;
    sem_t mutex;
} THREAD_COUNTER;
*/


THREAD_COUNTER *thread_counter;

int main(int argc, char* argv[]) {
    // Option processing should be performed here.
    // Option '-p <port>' is required in order to specify the port number
    // on which the server should listen.
    int option;
    char *port;
    //socket variables
    int listenfd, *connfd; // listen file descriptor and connection file descriptor
    socklen_t clientlen;
    struct sockaddr_storage clientaddr;
    pthread_t tid; //thread id
    //
    //get the port number from command line
    while ((option = getopt(argc, argv, "p:")) != -1) {
        switch (option) {
            case 'p':
                //port = atoi(optarg);
                port = optarg;
                //printf("The port number is: %s\n", optarg);
                break;
            /*
            case '?':
                if (optopt == 'p')
                    fprintf (stderr, "Option -%c requires an argument.\n", optopt);
            */
        }
    }
    //printf("The port number is: %s\n", port);

    // Perform required initializations of the thread counter and directory.
    thread_counter = tcnt_init();
    dir_init();

    // TODO: Set up the server socket and enter a loop to accept connections
    // on this socket.  For each connection, a thread should be started to
    // run function bvd_client_service().  In addition, you should install
    // a SIGHUP handler, so that receipt of SIGHUP will perform a clean
    // shutdown of the server.

    //install a signal handler using sigaction
    install_sig_handler(SIGHUP, sighup_handler);
    /*
    open a listen file descriptor
    wrapped with socket bind
    */
    listenfd = Open_listenfd(port);
    //main loop
    while (1) {
        clientlen = sizeof(struct sockaddr_storage);
        connfd = Malloc(sizeof(int)); //this file descriptor is freed in bvd_client_service()
        *connfd = Accept(listenfd, (SA *) &clientaddr, &clientlen);
        //Pthread_create(&tid, NULL, bvd_client_service, connfd);
        Pthread_create(&tid, NULL, client_handler, connfd);
    }

    fprintf(stderr, "You have to finish implementing main() "
	        "before the Bavarde server will function.\n");

    terminate();
}


//create worker thread to handle client requests
void *client_handler(void *arg) {

    Pthread_detach(pthread_self());

    //debug("The thread number is: %d\n", thread_counter->num);

    bvd_client_service(arg);


    return NULL;
}

//install sigaction handler
handler_t *install_sig_handler(int signum, handler_t *handler)
{
    struct sigaction action, old_action;

    action.sa_handler = handler;
    sigemptyset(&action.sa_mask); /* Block sigs of type being handled */
    action.sa_flags = SA_RESTART; /* Restart syscalls if possible */

    if (sigaction(signum, &action, &old_action) < 0)
         unix_error("Signal error");
    return (old_action.sa_handler);
}

//sighup handler just kill the whole process
void sighup_handler(int sig) {
    //terminate the program
    terminate(sig);
}


/*
 * Function called to cleanly shut down the server.
 */
void terminate(int sig) {
    // Shut down the directory.
    // This will trigger the eventual termination of service threads.
    dir_shutdown();

    debug("Waiting for service threads to terminate...");
    tcnt_wait_for_zero(thread_counter);
    debug("All service threads terminated.");

    tcnt_fini(thread_counter);
    dir_fini();
    exit(EXIT_SUCCESS);
}
