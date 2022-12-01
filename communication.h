/*********************************************************************
* Author:     ales
* Created:    29.11.22
*********************************************************************/

#ifndef COMMUNICATION_H
#define COMMUNICATION_H

#include <stdio.h>
#include <stdlib.h>

#include <unistd.h>
#include <string.h>
#include <stdbool.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include "crc.h"

#define REQ_MSG 'R'
#define OFF_MSG 'O'
#define ACK_MSG 'A'
#define NAK_MSG 'N'
#define HASH_MSG 'H'
#define DATA_MSG 'D'


#define DATA_SIZE 512
#define HEADER_SIZE (1 +  sizeof(size_t) + sizeof(uint16_t) + CRC_SIZE) // TYPE | PN | DS | D | CRC
#define PACKET_SIZE (DATA_SIZE + HEADER_SIZE)


uint16_t *get_segment_sizes(size_t image_size, size_t *number_of_packets);

// ---------------- CONNECTION ESTABLISHMENT ----------------

bool request_image(int socket, size_t *image_size, struct sockaddr *sender_address);

bool offer_image(int socket, size_t image_size, struct sockaddr *receiver_address);


// ---------------- DATA TRANSFER ----------------

void send_image(int socket, unsigned char *image, size_t image_size, struct sockaddr *receiver_address);

void send_data_packet(int socket, unsigned char *data, uint16_t segment_size,
                      size_t packet_number, struct sockaddr *receiver_address);

unsigned char *receive_image(int socket, size_t image_size, struct sockaddr *sender_address);

bool receive_data_packet(int socket, unsigned char *buffer, uint16_t *segment_size,
                         size_t *packet_number, struct sockaddr *sender_address);

// ---------------- DATA VERIFICATION ----------------

bool send_hash(int socket, unsigned char *hash, struct sockaddr *address);

bool verify_hash(int socket, unsigned char *hash, struct sockaddr *address);

#endif //COMMUNICATION_H
