//
// Created by ales on 29.11.22.
//

#include <stdio.h>
#include <stdlib.h>

#include "crc.h"

void code_word(unsigned char *buffer, long size) {
    size_t word_size = size - CRC_SIZE;
    unsigned short residue = crc_ccitt(buffer, word_size, CRC_POLY, 0);
    printf("Residue when coding the word: 0x%x\n", residue);
    buffer[size - CRC_SIZE] = (unsigned char) ((residue >> 8) & 0xff);
    buffer[size - CRC_SIZE + 1] = (unsigned char) (residue & 0xff);
}

bool decode_word(unsigned char *buffer, long size) {
    unsigned short residue = crc_ccitt(buffer, size, CRC_POLY, 0);
    printf("Residue when decoding the word: 0x%x\n", residue);
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

int main() {

    FILE *file;
    size_t size;
    unsigned char buff[TEST_SIZE + CRC_SIZE];

    file = fopen("/home/ales/School/KDS/KDS-Semestral/file.txt", "rb");

    size = fread(buff, 1, TEST_SIZE, file);
    printf("read %ld bytes\n", size);

    fclose(file);

    code_word(buff, TEST_SIZE + CRC_SIZE);
    if (decode_word(buff, TEST_SIZE + CRC_SIZE) == true) {
        printf("CRC is correct\n");
    } else {
        printf("CRC is incorrect\n");
    }

    return EXIT_SUCCESS;
}