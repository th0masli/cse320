#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <semaphore.h>

#include "debug.h"
#include "mailbox.h"
#include "csapp.h"

//define a linked list for mailbox entry list
typedef struct entry_list {
    MAILBOX_ENTRY *entry;
    struct entry_list *next;
    struct entry_list *prev;
} ENTRY_LIST;

//define the mailbox struct
typedef struct mailbox {
    char *handle_name; //corresponding handle name
    unsigned int defunct;   //flag for defunct mailbox after shutdown
    unsigned int ref_count; //referencing count for mailbox
    ENTRY_LIST *entry_header; //entry header
    ENTRY_LIST *entry_rear; //entry rear
    sem_t mutex; //protect access to mailbox
    sem_t msg;   //counts available entry(msg)
} MAILBOX;

//insert the entry to the end of mailbox queue
void insert_entry(MAILBOX *mb, MAILBOX *mailbox_entry);
//remove entry from the beginning of the mailbox
MAILBOX_ENTRY *remove_entry(MAILBOX *mb);

/*
 * A mailbox provides the ability for its client to set a hook to be
 * called when an undelivered entry in the mailbox is discarded on
 * mailbox finalization.  For example, the hook might arrange for the
 * sender of an undelivered message to receive a bounce notification.
 * The discard hook is not responsible for actually deallocating the
 * mailbox entry; that is handled by the mailbox finalization procedure.
 * There is one special case that the discard hook must handle, and that
 * is the case that the "from" field of a message is NULL.  This will only
 * occur in a mailbox entry passed to discard hook, and it indicates that
 * the sender of the message was in fact the mailbox that is now being
 * finalized.  Since that mailbox can no longer be used, it does not make
 * sense to do anything further with it (and in fact trying to do so would
 * result in a deadlock because the mailbox is locked), so it has been
 * replaced by NULL.
 *
 * The following is the type of discard hook function.
 */
typedef void (MAILBOX_DISCARD_HOOK)(MAILBOX_ENTRY *);

/*
 * Create a new mailbox for a given handle.
 * The mailbox is returned with a reference count of 1.
 */
MAILBOX *mb_init(char *handle) {

    MAILBOX *new_mb = Malloc(sizeof(MAILBOX));
    //init the handle name corresponding to the mailbox
    int h_len = strlen(handle) + 1;
    char *handle_name = Malloc(h_len);
    memcpy(handle_name, handle, h_len);
    new_mb->handle_name = handle_name;
    new_mb->defunct = 0;
    new_mb->ref_count = 1;
    //install the mutex for the mailbox
    Sem_init(&(new_mb->mutex), 0, 1);
    Sem_init(&(new_mb->msg), 0, 1);
    //init the entry header and rear
    ENTRY_LIST *entry_hdr = Malloc(sizeof(ENTRY_LIST));
    ENTRY_LIST *entry_rear = Malloc(sizeof(ENTRY_LIST));
    entry_hdr->entry = NULL;
    entry_rear->entry = NULL;
    entry_hdr->next = entry_rear;
    entry_hdr->prev = NULL;
    entry_rear->next = NULL;
    entry_rear->prev = entry_hdr;

    new_mb->entry_header = entry_hdr;
    new_mb->entry_rear = entry_rear;

    return new_mb;
}

/*
 * Set the discard hook for a mailbox.
 */
void mb_set_discard_hook(MAILBOX *mb, MAILBOX_DISCARD_HOOK *) {


}

/*
 * Increase the reference count on a mailbox.
 * This must be called whenever a pointer to a mailbox is copied,
 * so that the reference count always matches the number of pointers
 * that exist to the mailbox.
 */
void mb_ref(MAILBOX *mb) {

    if (mb != NULL) {
      P(&(mb->mutex));

      mb->ref_count++;

      V(&(mb->mutex));
    }
}

/*
 * Decrease the reference count on a mailbox.
 * This must be called whenever a pointer to a mailbox is discarded,
 * so that the reference count always matches the number of pointers
 * that exist to the mailbox.  When the reference count reaches zero,
 * the mailbox will be finalized.
 */
void mb_unref(MAILBOX *mb) {

    if (mb != NULL) {
        if ((mb->ref_count) > 0) {
          p(&(mb->mutex));

          mb->ref_count--;

          V(&(mb->mutex));
        }
        //finalize the mailbox when the reference count is 0
        else if ((mb->ref_count) == 0)
          mb_fini(mb);
    }
}

//finalize the mailbox everything
void mb_fini(MAILBOX *mb) {
    /*
    free entry_list including the entry_header and entry_rear
    free the handle_name
    free the mailbox
    discard everything
    */

}

/*
 * Shut down this mailbox.
 * The mailbox is set to the "defunct" state.  A defunct mailbox should
 * not be used to send any more messages or notices, but it continues
 * to exist until the last outstanding reference to it has been
 * discarded.  At that point, the mailbox will be finalized, and any
 * entries that remain in it will be discarded.
 */
void mb_shutdown(MAILBOX *mb) {

    if (mb != NULL) {
        if ((mb->defunct) == 0) {
            mb->defunct = 1;
            //block mb_next_entry() using P(sem_wait)
        }
    }
}

/*
 * Get the handle associated with a mailbox.
 */
char *mb_get_handle(MAILBOX *mb) {
    //mb may be defunct; maybe NULL
    char *handle_name = NULL;
    if (mb != NULL) {
        handle_name = mb->handle_name;
    }

    return handle_name;
}

/*
 * Add a message to the end of the mailbox queue.
 *   msgid - the message ID
 *   from - the sender's mailbox
 *   body - the body of the message, which can be arbitrary data, or NULL
 *   length - number of bytes of data in the body
 *
 * The message body must have been allocated on the heap,
 * but the caller is relieved of the responsibility of ultimately
 * freeing this storage, as it will become the responsibility of
 * whomever removes this message from the mailbox.
 *
 * The reference to the sender's mailbox ("from") is conceptually
 * "transferred" from the caller to the new message, so no increase in
 * the reference count is performed.  However, after the call the
 * caller must discard this pointer which it no longer "owns".
 */
//need to do in a thread safe way
void mb_add_message(MAILBOX *mb, int msgid, MAILBOX *from, void *body, int length) {
    //allocate the message body on the heap
    MAILBOX_ENTRY *new_entry = Malloc(sizeof(MAILBOX_ENTRY));
    new_entry->type = MESSAGE_ENTRY_TYPE;
    //buffer for body
    new_entry->body = body;
    new_entry->length = length;
    //
    MESSAGE *new_msg = Malloc(sizeof(MESSAGE));
    MAILBOX *sender = Malloc(sizeof(MAILBOX));
    memcpy(sender, from, sizeof(MAILBOX));
    new_msg->from = sender;
    new_msg->msgid = msgid
    //add the new message to the new entry
    new_entry->content = new_msg;

    P(&(mb->mutex))
    //insert the newly created new_entry to the target mailbox's entry list
    insert_entry(mb, new_entry);
    V(&(mb->mutex))
}

//insert the entry to the end of mailbox queue
void insert_entry(MAILBOX *mb, MAILBOX *mailbox_entry) {
    ENTRY_LIST *rear = mb->entry_rear;
    (rear->prev)->next = mailbox_entry;
    mailbox_entry->prev = rear->prev;
    mailbox_entry->next = rear;
    rear->prev = mailbox_entry;
}

/*
 * Add a notice to the end of the mailbox queue.
 *   ntype - the notice type
 *   msgid - the ID of the message to which the notice pertains
 *   body - the body of the notice, which can be arbitrary data, or NULL
 *   length - number of bytes of data in the body
 *
 * The notice body must have been allocated on the heap, but the
 * caller is relieved of the responsibility of ultimately freeing this
 * storage, as it will become the responsibility of whomever removes
 * this notice from the mailbox.
 */
//need to do in thread safe way
void mb_add_notice(MAILBOX *mb, NOTICE_TYPE ntype, int msgid, void *body, int length) {
    //The notice body must have been allocated on the heap
    MAILBOX_ENTRY *new_entry = Malloc(sizeof(MAILBOX_ENTRY));
    new_entry->type = NOTICE_ENTRY_TYPE;
    //buffer for body
    new_entry->body = body;
    new_entry->length = length;
    //for the notice itself
    NOTICE *new_notice = Malloc(sizeof(NOTICE));
    new_notice->type = ntype;
    new_notice->msgid = msgid;
    //add the new notice to the new entry
    new_entry->content = new_notice;

    P(&(mb->mutex))
    //use the same insert method to insert the notice to the end of mailbox queue
    insert_entry(mb, new_entry);
    V(&(mb->mutex))
}

/*
 * Remove the first entry from the mailbox, blocking until there is
 * one.  The caller assumes the responsibility of freeing the entry
 * and its body, if present.  In addition, if it is a message entry,
 * the caller must decrease the reference count on the "from" mailbox
 * to account for the destruction of the pointer.
 *
 * This function will return NULL in case the mailbox is defunct.
 * The thread servicing the mailbox should use this as an indication
 * that service should be terminated.
 */
//need to do in item safe way
MAILBOX_ENTRY *mb_next_entry(MAILBOX *mb) {

    MAILBOX_ENTRY *target_entry = NULL;
    P(&(mb->msg));   //wait for available entry
    P(&(mb->mutex)); //lock the queue
    target_entry = remove_entry(mb); //remove the first entry
    V(&(mb->mutex)); //unlock the queue
    //only if there is an entry can be removed then anouce the availability
    if ((mb->entry_header)->next != mb->entry_rear)
      V(&(mb->msg));

    return target_entry;
}

//check if there is an entry can be removed

//remove entry from the beginning of the mailbox
MAILBOX_ENTRY *remove_entry(MAILBOX *mb) {
    //if the tail is the next entry of head then do nothing
    MAILBOX_ENTRY *target_entry;
    target_entry = (mb->entry_header)->next;
    if ((target_entry == mb->entry_rear) || target_entry == NULL)
        return NULL;
    (mb->entry_header)->next = target_entry->next;
    (target_entry->next)->prev = mb->entry_header;
    target_entry->next = NULL;
    target_entry->prev = NULL;

    return target_entry;
}
