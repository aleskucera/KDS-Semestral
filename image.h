//
// Created by ales on 29.11.22.
//

#ifndef KDS_SEMESTRAL_IMAGE_H
#define KDS_SEMESTRAL_IMAGE_H

#include <stdio.h>
#include <stdlib.h>

#include <stdbool.h>

bool read_image(char *path, char **buffer, long *size);

bool write_image(char *path, char *image, long size);

#endif //KDS_SEMESTRAL_IMAGE_H
