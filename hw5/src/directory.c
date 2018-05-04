#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

#include "directory.h"
#include "debug.h"
#include "csapp.h"


/*
 * The directory maintains a mapping from handles (i.e. user names)
 * to client info, which includes the client mailbox and the socket
 * file descriptor over which commands from the client are received.
 * The directory is used when sending a message, in order to find
 * the destination mailbox that corresponds to a particular recipient
 * specified by their handle.
 */


//handle struct
typedef struct handle {
    char *usr_name;
    int handle_fd;
    MAILBOX *mailbox;
    struct handle *next;
    struct handle *prev;
} HANDLE;

//directory struct
typedef struct directory {
    int defunct; //defunct flag set to 1 when the dir is "defunct"
    HANDLE *handle_header;
    sem_t mutex;
} DIRECTORY;

DIRECTORY *dir;

//look up if the handle has registered
HANDLE *handle_lookup(char *handle_name);
/* insert the handle to the handle list */
void insert_handle(HANDLE *handle);
//finish the handle
void handle_fini(HANDLE *handle);
/* remove handle from the handle list */
void remove_handle(HANDLE *handle);
//get the length of the handle list
int handles_len(HANDLE *header);


/*
 * Initialize the directory.
 */
void dir_init(void) {
    //init directory
    dir = Malloc(sizeof(DIRECTORY));
    dir->defunct = 0;
    Sem_init(&(dir->mutex), 0, 1);
    //init the handle linked list
    HANDLE *handle_hdr = Malloc(sizeof(HANDLE));
    handle_hdr->usr_name = NULL;
    handle_hdr->handle_fd = -1;
    handle_hdr->mailbox = NULL;
    handle_hdr->next = handle_hdr;
    handle_hdr->prev = handle_hdr;
    //put the handle header in to the directory
    dir->handle_header = handle_hdr;
}

/*
 * Shut down the directory.
 * This marks the directory as "defunct" and shuts down all the client sockets,
 * which triggers the eventual termination of all the server threads.
 */
void dir_shutdown(void) {
    //if it is not defunct then defunc it
    if ((dir->defunct) == 0) {
        //starting from the handle right behind the header
        HANDLE *header = dir->handle_header;
        HANDLE *cur_handle = header->next;
        while (cur_handle != header) {
            //need to check the man page
            shutdown(cur_handle->handle_fd, SHUT_RDWR);
            cur_handle = cur_handle->next;
        }
        //mark the directory as "defunct"
        dir->defunct = 1;
    }

}

/*
 * Finalize the directory.
 *
 * Precondition: the directory must previously have been shut down
 * by a call to dir_shutdown().
 */
void dir_fini(void) {
    if ((dir->defunct) == 1) {

        HANDLE *header = dir->handle_header;
        HANDLE *cur_handle = header->next;
        while (cur_handle != header) {
            /*
            free(cur_handle->usr_name);
            //shut down the mailbox? or free it
            mb_shutdown(cur_handle->mailbox);
            */
            HANDLE *defunt_handle = cur_handle;
            cur_handle = cur_handle->next;
            //free(defunt_handle);
            handle_fini(defunt_handle);
        }

        free(header);
        free(dir);
    }

}

//finish the handle
void handle_fini(HANDLE *handle) {
    free(handle->usr_name);
    //shut down the mailbox? or free it
    mb_shutdown(handle->mailbox);
    free(handle);
}

/*
 * Register a handle in the directory.
 *   handle - the handle to register
 *   sockfd - file descriptor of client socket
 *
 * Returns a new mailbox, if handle was not previously registered.
 * Returns NULL if handle was already registered or if the directory is defunct.
 */
MAILBOX *dir_register(char *handle, int sockfd) {

    debug("Try to register handle: %s", handle);

    P(&(dir->mutex));
    //Returns NULL if handle was already registered or if the directory is defunct
    HANDLE *handle_lookup_res = handle_lookup(handle);
    if ((handle_lookup_res != NULL) || dir->defunct) {

        V(&(dir->mutex));

        return NULL;
    }

    //Returns a new mailbox, if handle was not previously registered
    HANDLE *new_handle = Malloc(sizeof(HANDLE));
    //+1 is for the '\0'
    char *usr_name = Malloc(sizeof(strlen(handle))+1);
    memcpy(usr_name, handle, (strlen(handle)+1));
    new_handle->usr_name = usr_name;
    new_handle->handle_fd = sockfd;
    MAILBOX *new_mailbox = mb_init(handle);
    new_handle->mailbox = new_mailbox;
    //call mb_ref() to increase the reference count on a mailbox before returning a pointer to it
    mb_ref(new_mailbox);
    //insert the new handle to the list
    insert_handle(new_handle);

    V(&(dir->mutex));

    debug("Successfully registered handle: %s", new_handle->usr_name);

    return new_mailbox;

}

//look up if the handle has registered
HANDLE *handle_lookup(char *handle_name) {
    HANDLE *header = dir->handle_header;
    HANDLE *cur_handle = header->next;
    while (cur_handle != header) {
        //there is already a same handle there
        if (!strcmp((cur_handle->usr_name), handle_name))
            return cur_handle;
    }

    return NULL;
}


/* insert the handle to the handle list */
void insert_handle(HANDLE *handle) {
    HANDLE *header = dir->handle_header;
    handle->next = header->next;
    (header->next)->prev = handle;
    header->next = handle;
    handle->prev = header;
}


/*
 * Unregister a handle in the directory.
 * The associated mailbox is removed from the directory and shut down.
 */
void dir_unregister(char *handle) {

    debug("Try to unregisterd handle: %s", handle);

    //P(&(dir->mutex));

    HANDLE *handle_lookup_res = handle_lookup(handle);
    //find that handle
    if (handle_lookup_res != NULL) {
        remove_handle(handle_lookup_res);
        mb_unref(handle_lookup_res->mailbox);
        handle_fini(handle_lookup_res);

        debug("Removed handle: %s", handle);
    }

    //V(&(dir->mutex));

}

/* remove handle from the handle list */
void remove_handle(HANDLE *handle) {

    (handle->prev)->next = handle->next;
    (handle->next)->prev = handle->prev;
    handle->next = NULL;
    handle->prev = NULL;

}

/*
 * Query the directory for a specified handle.
 * If the handle is not registered, NULL is returned.
 * If the handle is registered, the corresponding mailbox is returned.
 * The reference count of the mailbox is increased to account for the
 * pointer that is being returned.  It is the caller's responsibility
 * to decrease the reference count when the pointer is ultimately discarded.
 */
MAILBOX *dir_lookup(char *handle) {

    P(&(dir->mutex));

    MAILBOX *target_mailbox = NULL;
    HANDLE *handle_lookup_res = handle_lookup(handle);
    //if registered
    if (handle_lookup_res != NULL) {
        //increase the reference count
        target_mailbox = handle_lookup_res->mailbox;
        mb_ref(target_mailbox);
    }

    V(&(dir->mutex));

    return target_mailbox;

}

/*
 * Obtain a list of all handles currently registered in the directory.
 * Returns a NULL-terminated array of strings.
 * It is the caller's responsibility to free the array and all the strings
 * that it contains.
 */
char **dir_all_handles(void) {

    P(&(dir->mutex));

    HANDLE *header = dir->handle_header;
    HANDLE *cur_handle = header->next;
    int h_len = handles_len(header);
    int name_len;
    //a NULL-terminated array of strings
    char **handles_array = Malloc(sizeof(char*)*(h_len+1));
    char *handles_array_cur = *handles_array;
    while (cur_handle != header) {
        name_len = strlen(cur_handle->usr_name)+1;
        handles_array_cur = Malloc(name_len);
        memcpy(handles_array_cur, cur_handle->usr_name, name_len);
        cur_handle++;
        handles_array_cur++;
    }

    handles_array_cur = NULL;

    /*
    for (int i=0; i<h_len; i++) {
        printf("|");
        printf("%s", handles_array[i]);
    }
    */

    V(&(dir->mutex));

    return handles_array;

}

//get the length of the handle list
int handles_len(HANDLE *header) {
    int len = 0;
    HANDLE *handle = header->next;
    while (handle != header) {
        handle++;
        len++;
    }

    return len;
}