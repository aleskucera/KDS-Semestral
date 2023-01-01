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

#include <time.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include "crc.h"
#include "main.h"
#include "utils.h"
#include "sha256.h"

// ---------------- CONNECTION ESTABLISHMENT ----------------

bool request_image(int socket, size_t *image_size, struct sockaddr *sender_address);

void offer_image(int socket, size_t image_size, struct sockaddr *receiver_address);


// ---------------- DATA TRANSFER ----------------

void send_image(int socket, byte *image, size_t image_size, const byte *hash, struct sockaddr *receiver_address);

unsigned char *receive_image(int socket, size_t image_size, struct sockaddr *sender_address);

// ---------------- UTILITY ----------------

void send_packet(int socket, byte msg_type, size_t n1, uint16_t n2, byte *data, struct sockaddr *address);

bool receive_packet(int socket, byte *msg_type, size_t *n1, uint16_t *n2, byte *data, const struct timeval *timeout);


#endif //COMMUNICATION_H
