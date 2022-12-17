/*********************************************************************
* Author:     ales
* Created:    3.12.22
*********************************************************************/

#ifndef MAIN_H
#define MAIN_H

/* Message structure:
 *
 * TYPE | N1 | N2 | D | CRC
 *
 * TYPE: 1 byte (char)
 * N1: 4 bytes (size_t)
 * N2: 2 bytes (uint16_t)
 * D: 0 - 1024 bytes (unsigned char)
 * CRC: 2 bytes (uint16_t)
 *
 * */

#define SENDER_PORT_IN 15001
#define SENDER_PORT_OUT 14000

#define RECEIVER_PORT_IN 15000
#define RECEIVER_PORT_OUT 14001

#define ACK_MSG 'A'
#define NAK_MSG 'N'
#define EOT_MSG 'E'

#define REQ_MSG 'R'
#define OFF_MSG 'O'

#define HASH_MSG 'H'
#define DATA_MSG 'D'

#define DATA_SIZE 1024
#define PACKET_SIZE (1 + sizeof(size_t) \
                    + sizeof(uint16_t)  \
                    + DATA_SIZE + CRC_SIZE)

# define CRC_POLY 0xffff
# define CRC_SIZE 2

#define WINDOW_SIZE 10

#define TIMEOUT 1000000 // 1 second

#define SHA256_BLOCK_SIZE 32

#define IMAGE_PATH "../images/image.jpeg"
#define SAVE_PATH "../images/received_image.jpeg"

typedef unsigned char byte;

#endif //MAIN_H
