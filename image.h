/*********************************************************************
* Author:     ales
* Created:    29.11.22
*********************************************************************/

#ifndef IMAGE_H
#define IMAGE_H

#include <stdio.h>
#include <stdlib.h>

#include <stdbool.h>

bool read_image(char *path, unsigned char **buffer, unsigned long *size);

bool write_image(char *path, unsigned char *image, unsigned long size);

#endif //IMAGE_H
