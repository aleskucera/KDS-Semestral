//
// Created by ales on 29.11.22.
//

#ifndef KDS_SEMESTRAL_COMMUNICATION_H
#define KDS_SEMESTRAL_COMMUNICATION_H

#include <stdio.h>
#include <stdlib.h>

#include <unistd.h>
#include <string.h>
#include <stdbool.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include "crc.h"

//#define MSG_SIZE 4
//#define REQ_MSG "REQ"
//#define ACK_MSG "ACK"
////#define END_MSG "END"

#define REQ_MSG 'R'
#define OFF_MSG 'O'
#define ACK_MSG 'A'
#define END_MSG 'E'
#define DATA_MSG 'D'


#define PACKET_DATA_SIZE 512
#define PACKET_HEADER_SIZE (CRC_SIZE + sizeof(uint16_t) + 1)
#define PACKET_SIZE (PACKET_DATA_SIZE + PACKET_HEADER_SIZE)


uint16_t *get_segment_sizes(unsigned long image_size, uint16_t packet_size);

bool req_image(int client_socket, unsigned long *image_size);

bool off_image(int client_socket, unsigned long image_size);

bool send_image(int client_socket, unsigned char *image, unsigned long image_size);

bool receive_image(int client_socket, unsigned char *image);

ssize_t send_data_packet(int client_socket, unsigned char *data, uint16_t data_size);

ssize_t send_msg_packet(int client_socket, unsigned char msg);

ssize_t send_off_packet(int client_socket, unsigned long image_size);

unsigned long unpack_off_packet(unsigned char *packet);

bool receive_off_packet(int client_socket, unsigned long *image_size);

int32_t unpack_data_packet(unsigned char *packet, unsigned char *buffer);

bool receive_data_packet(int client_socket, unsigned char *buffer, int32_t *data_size);

bool receive_msg_packet(int client_socket, unsigned char msg);

#endif //KDS_SEMESTRAL_COMMUNICATION_H
