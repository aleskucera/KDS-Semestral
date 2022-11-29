//
// Created by ales on 29.11.22.
//


#include "crc.h"

void encode_packet(unsigned char *buffer, long size) {
    size_t word_size = size - CRC_SIZE;
    unsigned short residue = crc_ccitt(buffer, word_size, CRC_POLY, 0);
    buffer[size - CRC_SIZE] = (unsigned char) ((residue >> 8) & 0xff);
    buffer[size - CRC_SIZE + 1] = (unsigned char) (residue & 0xff);
}

bool decode_packet(unsigned char *buffer, long size) {
    unsigned short residue = crc_ccitt(buffer, size, CRC_POLY, 0);
    return residue == 0;
}


unsigned short crc_ccitt(unsigned char *data, size_t length, unsigned short seed, unsigned short final) {

    unsigned int crc = seed;
    unsigned int temp;

    for (size_t count = 0; count < length; ++count) {
        temp = (*data++ ^ (crc >> 8)) & 0xff;
        crc = crc_table[temp] ^ (crc << 8);
    }

    return (unsigned short) (crc ^ final);

}