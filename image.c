/*********************************************************************
* Author:     ales
* Created:    29.11.22
*********************************************************************/

#include "image.h"

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

bool write_image(char *path, unsigned char *image, unsigned long size) {
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
