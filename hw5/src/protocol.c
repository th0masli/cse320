#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>

#include "protocol.h"
#include "csapp.h"


//batch converting
void convert_hdr(bvd_packet_header *hdr, uint32_t (*converter)(uint32_t));
//set time when sending
void set_time(bvd_packet_header *hdr);


//sent packet to the client
int proto_send_packet(int fd, bvd_packet_header *hdr, void *payload) {

    size_t hdr_size = sizeof(bvd_packet_header);
    uint32_t p_len = hdr->payload_length;

    if (hdr->type == BVD_NO_PKT) {
        return -1;
    }
    //set time to real time
    set_time(hdr);
    //convert to net format
    convert_hdr(hdr, htonl);
    //undefined type header
    if (rio_writen(fd, hdr, hdr_size) < 0) {
        //set errno
        //exit(-1);
        return -1;
    }

    if ((payload != NULL) && (p_len > 0)) {
        if (rio_writen(fd, payload, p_len) < 0) {
            //set errno
            //exit(-1);
            return -1;
        }
    }
    return 0;
}


//recieve packet from the client
int proto_recv_packet(int fd, bvd_packet_header *hdr, void **payload) {

    //read in first
    size_t hdr_size = sizeof(bvd_packet_header);
    //uint32_t p_len = hdr->payload_length;
    if (rio_readn(fd, hdr, hdr_size) < 0) {
        //set errno
        //exit(-1);
        return -1;
    }

    //convert to host byte
    convert_hdr(hdr, ntohl);
    //undefined type header
    if (hdr->type == BVD_NO_PKT) {
        //exit(-1);
        return -1;
    }

    //check payload lenght to write
    uint32_t p_len = hdr->payload_length;
    if (p_len > 0) {
        char *data = Malloc(p_len);
        *payload = data;
        if (rio_readn(fd, data, p_len) < 0) {
            //set errno
            //exit(-1);
            return -1;
        }
    }

    return 0;
}


//batch converting
void convert_hdr(bvd_packet_header *hdr, uint32_t (*converter)(uint32_t)) {
    //dont convert b/c it is uint_8
    hdr->payload_length = converter(hdr->payload_length);
    hdr->msgid = converter(hdr->msgid);
    hdr->timestamp_sec = converter(hdr->timestamp_sec);
    hdr->timestamp_nsec = converter(hdr->timestamp_nsec);

}


//set time when sending
void set_time(bvd_packet_header *hdr) {
    struct timespec time_stamp;

    clock_gettime(CLOCK_MONOTONIC, &time_stamp);

    hdr->timestamp_sec = (uint32_t)time_stamp.tv_sec;
    hdr->timestamp_nsec = (uint32_t)time_stamp.tv_nsec;
}



