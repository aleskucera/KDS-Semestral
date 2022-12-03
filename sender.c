/*********************************************************************
* Author:     ales
* Created:    29.11.22
*********************************************************************/

#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include "main.h"
#include "sha256.h"
#include "communication.h"

int main() {
    int sender_socket;
    struct sockaddr_in sender_address;
    struct sockaddr_in receiver_address;

    byte *image;
    size_t image_size;
    char *image_path = "/home/ales/School/KDS/KDS-Semestral/images/image.jpeg";

    SHA256_CTX ctx;
    byte hash[SHA256_BLOCK_SIZE];

    // ------------------ SENDER CONFIG ------------------

    printf("INFO: Configuring sender to localhost:%d \n", SENDER_PORT_IN);

    sender_address.sin_family = AF_INET;
    sender_address.sin_port = htons(SENDER_PORT_IN);
    sender_address.sin_addr.s_addr = INADDR_ANY;

    if ((sender_socket = socket(AF_INET, SOCK_DGRAM, 0)) == -1) {
        printf("ERROR: There was an error creating the socket \n\n");
        return EXIT_FAILURE;
    } else {
        printf("INFO: Socket successfully created \n");
    }

    if (bind(sender_socket, (struct sockaddr *) &sender_address, sizeof(sender_address)) == -1) {
        printf("ERROR: There was an error binding the socket to an IP/Port \n\n");
        return EXIT_FAILURE;
    } else {
        printf("INFO: Socket successfully bound to IP/Port \n\n");
    }

    // ------------------ RECEIVER CONFIG ------------------

    printf("INFO: Configuring receiver to localhost:%d \n\n", SENDER_PORT_OUT);

    receiver_address.sin_family = AF_INET;
    receiver_address.sin_port = htons(SENDER_PORT_OUT);
    receiver_address.sin_addr.s_addr = INADDR_ANY;

    // ------------------ IMAGE LOAD ------------------

    printf("INFO: Loading image...\n");

    if (read_image(image_path, &image, &image_size)) {
        printf("INFO: Image successfully loaded \n\n");
    } else {
        printf("ERROR: There was an error loading the image \n\n");
        return EXIT_FAILURE;
    }

    // ------------------ IMAGE HASH ------------------

    printf("INFO: Calculating loaded image hash...\n");

    sha256_init(&ctx);
    sha256_update(&ctx, image, image_size);
    sha256_final(&ctx, hash);

    printf("INFO: Image hash: ");

    for (int i = 0; i < SHA256_BLOCK_SIZE; i++) {
        printf(" 0x%02x", hash[i]);
    }

    printf("\n\n");

    // ------------------ IMAGE OFFER ------------------

    printf("INFO: Waiting for a request from receiver... \n\n");

    offer_image(sender_socket, image_size, (struct sockaddr *) &receiver_address);

    printf("INFO: Receiver requested the image, sending... \n\n");

    // ------------------ IMAGE TRANSMISSION ------------------

    send_image(sender_socket, image, image_size, hash, (struct sockaddr *) &receiver_address);

    printf("INFO: Image sent successfully \n\n");

    return EXIT_SUCCESS;

}