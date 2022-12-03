/*********************************************************************
* Author:     ales
* Created:    3.12.22
*********************************************************************/

#ifndef UTILS_H
#define UTILS_H

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

#include "main.h"


// ---------------- COMMUNICATION UTILS ----------------

uint16_t *get_segment_sizes(size_t image_size, size_t *number_of_packets);

bool all_segments_sent(const bool *sent_segments, size_t number_of_packets);

size_t get_missing_segment(const bool *sent_segments, size_t number_of_packets);

size_t get_offset(const uint16_t *segment_sizes, size_t index);


// ---------------- IMAGE UTILS ----------------

bool read_image(char *path, byte **buffer, size_t *size);

bool save_image(char *path, byte *image, size_t size);

#endif //UTILS_H
