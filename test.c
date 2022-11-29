//
// Created by ales on 29.11.22.
//

#include <stdio.h>
#include <stdlib.h>

#include "communication.h"
#include "crc.h"

int main() {
    unsigned char packet[PACKET_SIZE];
    unsigned char *data = (unsigned char *) malloc(PACKET_DATA_SIZE);
    uint16_t packet_size = PACKET_DATA_SIZE;


    for (int i = 0; i < PACKET_DATA_SIZE; i++) {
        data[i] = i;
    }

    // Create packet
    memcpy(packet, &packet_size, sizeof(uint16_t));
    memcpy(packet + sizeof(uint16_t), data, PACKET_DATA_SIZE);
    encode_packet(packet, PACKET_SIZE);

    // Print first two bytes of packet as uint16_t
    uint16_t *packet_size_ptr = (uint16_t *) packet;
    printf("Packet size: %d\n", *packet_size_ptr);

    // Print data
    printf("Data: ");
    for (int i = sizeof(uint16_t); i < PACKET_SIZE - 2; i++) {
        printf("%d ", packet[i]);
    }
    printf("\n");

    // Print last two bytes of packet as hexademical
    printf("CRC: %x\n", packet[PACKET_SIZE - 2] << 8 | packet[PACKET_SIZE - 1]);

    // Decode packet
    if (decode_packet(packet, PACKET_SIZE)) {
        printf("Packet is valid\n");
    } else {
        printf("Packet is invalid\n");
    }

    return EXIT_SUCCESS;
}