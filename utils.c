/*********************************************************************
* Author:     ales
* Created:    3.12.22
*********************************************************************/

#include "main.h"
#include "utils.h"

uint16_t *get_segment_sizes(size_t image_size, size_t *number_of_packets) {
    uint16_t *packet_sizes;

    // Calculate number of packets
    *number_of_packets = image_size / DATA_SIZE;
    if (image_size % DATA_SIZE != 0) {
        (*number_of_packets)++;
    }

    // Allocate memory for packet sizes
    packet_sizes = malloc(sizeof(uint16_t) * *number_of_packets);

    // Fill packet sizes
    for (int i = 0; i < *number_of_packets; i++) {
        packet_sizes[i] = DATA_SIZE;
    }

    // Set last packet size
    packet_sizes[*number_of_packets - 1] = image_size % DATA_SIZE;

    return packet_sizes;
}

size_t get_missing_segment(const bool *acks, size_t number_of_packets) {
    for (int i = 0; i < number_of_packets; ++i) {
        if (!acks[i]) {
            return i;
        }
    }
    return number_of_packets;
}

size_t get_last_segment(const bool *acks, size_t number_of_packets) {
    for (int i = 0; i < number_of_packets; ++i) {
        if (!acks[i]) {
            return i - 1;
        }
    }
    return number_of_packets - 1;
}

size_t get_offset(const uint16_t *segment_sizes, size_t index) {
    size_t offset = 0;
    for (size_t i = 0; i < index; i++) {
        offset += segment_sizes[i];
    }
    return offset;
}

bool read_image(char *path, unsigned char **buffer, unsigned long *size) {
    bool ret = false;
    FILE *file = fopen(path, "rb");

    if (file != NULL) {

        fseek(file, 0, SEEK_END);
        *size = ftell(file);
        fseek(file, 0, SEEK_SET);

        *buffer = malloc(*size);
        fread(*buffer, *size, 1, file);
        fclose(file);
        ret = true;
    }
    return ret;
}

bool save_image(char *path, unsigned char *image, unsigned long size) {
    bool ret = false;
    FILE *fp;
    fp = fopen(path, "wb");
    if (fp != NULL) {
        fwrite(image, size, 1, fp);
        fclose(fp);
        ret = true;
    }
    return ret;
}