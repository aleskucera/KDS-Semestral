//
// Created by ales on 29.11.22.
//

#ifndef KDS_SEMESTRAL_COMMUNICATION_H
#define KDS_SEMESTRAL_COMMUNICATION_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define REQ_MSG "REQ"
#define ACK_MSG "ACK"

#define PACKET_SIZE 1024


size_t *get_packet_sizes(size_t image_size, size_t packet_size);

void req_image(int client_socket);

void off_image(int client_socket, size_t image_size);

void ack_image(int client_socket);

bool send_image(int client_socket, char *image, size_t image_size);

bool receive_image(int client_socket, char *image, size_t image_size);


#endif //KDS_SEMESTRAL_COMMUNICATION_H
