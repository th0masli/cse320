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
    MAILBOX_ENTRY *mb_entry;
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
int insert_entry(MAILBOX *mb, ENTRY_LIST *mailbox_entry);
//remove entry from the beginning of the mailbox
MAILBOX_ENTRY *remove_entry(MAILBOX *mb);
//finalize the mailbox everything
void mb_fini(MAILBOX *mb);

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
//typedef void (MAILBOX_DISCARD_HOOK)(MAILBOX_ENTRY *);

/*
 * Create a new mailbox for a given handle.
 * The mailbox is returned with a reference count of 1.
 */
MAILBOX *mb_init(char *handle) {

    MAILBOX *new_mb = Malloc(sizeof(MAILBOX));
    //init the handle name corresponding to the mailbox
    int h_len = strlen(handle);// + 1;
    char *handle_name = Malloc(h_len);
    memcpy(handle_name, handle, h_len);
    new_mb->handle_name = handle_name;
    new_mb->defunct = 0;
    new_mb->ref_count = 1;
    //install the mutex for the mailbox
    Sem_init(&(new_mb->mutex), 0, 1);
    Sem_init(&(new_mb->msg), 0, 0);
    //init the entry header and rear
    ENTRY_LIST *entry_hdr = NULL;
    ENTRY_LIST *entry_rear = NULL;

    new_mb->entry_header = entry_hdr;
    new_mb->entry_rear = entry_rear;

    return new_mb;
}

/*
 * Set the discard hook for a mailbox.
 */
void mb_set_discard_hook(MAILBOX *mb, MAILBOX_DISCARD_HOOK *hook) {
    //to do
    //set a hook to deal with the discarded mailbox
    //debug("Using hook");
    MAILBOX_ENTRY *hook_entry = (MAILBOX_ENTRY*)(mb->entry_header);
    if ((mb->defunct) == 1 && (hook_entry->content.message.from) != NULL) {
        hook(hook_entry);
        mb_unref(hook_entry->content.message.from);
    }
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
          P(&(mb->mutex));

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
    free(mb->handle_name);
    ENTRY_LIST *cur_entry = mb->entry_header;
    while (cur_entry != NULL) {
        ENTRY_LIST *tmp_entry = cur_entry;
        cur_entry = cur_entry->next;
        free((tmp_entry->mb_entry)->body);
        free(tmp_entry->mb_entry);
        free(tmp_entry);
    }
    free(mb);

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
    if (mb != NULL) {

        return mb->handle_name;

    }

    return NULL;
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

    if (mb != NULL && (mb->defunct) != 1) {
        //allocate the message body on the heap
        MAILBOX_ENTRY *new_mb_entry = Malloc(sizeof(MAILBOX_ENTRY));
        new_mb_entry->type = MESSAGE_ENTRY_TYPE;
        void *msg_body = Malloc(length);
        memcpy(msg_body, body, length);
        new_mb_entry->body = msg_body;
        new_mb_entry->length = length;
        (new_mb_entry->content).message.from = from;
        (new_mb_entry->content).message.msgid = msgid;
        //cast it to the ENTRY_LIST type
        ENTRY_LIST *new_entry = Malloc(sizeof(ENTRY_LIST));
        new_entry->mb_entry = new_mb_entry;

        P(&(mb->mutex));
        //insert the newly created new_entry to the target mailbox's entry list
        int insert_res = insert_entry(mb, new_entry);
        V(&(mb->mutex));
        //anounce available msg
        if (insert_res == 0) {
            debug("Insert successfully");
            V(&(mb->msg));
        }
    }

}

//insert the entry to the end of mailbox queue
int insert_entry(MAILBOX *mb, ENTRY_LIST *new_entry) {
    //check if the head is empty
    if (mb->entry_header == NULL) {

        mb->entry_header = new_entry;
        mb->entry_rear = new_entry;

    }else {
        (mb->entry_rear)->next = new_entry;
        new_entry->prev = mb->entry_rear;
        mb->entry_rear = new_entry;
        new_entry->next = NULL;
    }
    //check if header or rear is still null
    if (mb->entry_header == NULL || mb->entry_rear == NULL) {
        debug("Insert failed");
        return -1;
    }

    return 0;

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

    if (mb != NULL && (mb->defunct) != 1) {
        //The notice body must have been allocated on the heap
        MAILBOX_ENTRY *new_mb_entry = Malloc(sizeof(MAILBOX_ENTRY));
        new_mb_entry->type = NOTICE_ENTRY_TYPE;
        void *notice_body = Malloc(length);
        memcpy(notice_body, body, length);
        new_mb_entry->body = notice_body;
        new_mb_entry->length = length;
        //type notice
        new_mb_entry->content.notice.type = ntype;
        new_mb_entry->content.notice.msgid = msgid;
        //cast to entry list type
        ENTRY_LIST *new_entry = Malloc(sizeof(ENTRY_LIST));
        new_entry->mb_entry = new_mb_entry;

        P(&(mb->mutex));
        //use the same insert method to insert the notice to the end of mailbox queue
        int insert_res = insert_entry(mb, new_entry);
        V(&(mb->mutex));
        //anounce available msg
        if (insert_res == 0) {
            debug("Insert successfully");
            V(&(mb->msg));
        }
    }

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
    //return NULL if the mailbox is defunct
    if ((mb->defunct) == 1) {
        debug("The defunct flag is: %d", mb->defunct);
        return NULL;
    }
    debug("Getting the entries");
    MAILBOX_ENTRY *target_entry;
    P(&(mb->msg));   //wait for available entry
    P(&(mb->mutex)); //lock the queue
    debug("Trying to remove");
    target_entry = remove_entry(mb); //remove the first entry
    V(&(mb->mutex)); //unlock the queue

    //only if there is an entry can be removed then anouce the availability
    if (target_entry != NULL) {
      debug("removed successfully");
      debug("The entry length is: %d", target_entry->length);
    } else {
        debug("No entry at this time");
    }

    return target_entry;
}

//check if there is an entry can be removed

//remove entry from the beginning of the mailbox
MAILBOX_ENTRY *remove_entry(MAILBOX *mb) {
    //if the tail is the next entry of head then do nothing
    ENTRY_LIST *removed_hdr = mb->entry_header;
    if (removed_hdr == NULL)
        return NULL;
    else {
        mb->entry_header = removed_hdr->next;
        removed_hdr->next = NULL;
        //(mb->entry_header)->prev = NULL;

        return removed_hdr->mb_entry;
    }

}
