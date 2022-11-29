//
// Created by ales on 29.11.22.
//

#ifndef KDS_SEMESTRAL_IMAGE_H
#define KDS_SEMESTRAL_IMAGE_H

#include <stdio.h>
#include <stdlib.h>

#include <stdbool.h>

bool read_image(char *path, unsigned char **buffer, unsigned long *size);

bool write_image(char *path, unsigned char *image, unsigned long size);

#endif //KDS_SEMESTRAL_IMAGE_H
