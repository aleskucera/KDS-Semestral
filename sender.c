/*********************************************************************
* Author:     ales
* Created:    29.11.22
*********************************************************************/

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include "image.h"
#include "sha256.h"
#include "communication.h"

#define SENDER_PORT 15000
#define RECEIVER_PORT 14001

int main() {
    int sender_socket;
    struct sockaddr_in sender_address;
    struct sockaddr_in receiver_address;

    size_t image_size;
    unsigned char *image;
    char *image_path = "/home/ales/School/KDS/KDS-Semestral/images/image.jpeg";

    SHA256_CTX ctx;
    unsigned char hash[SHA256_BLOCK_SIZE];

    // ------------------ SENDER CONFIG ------------------

    printf("Configuring sender to localhost:%d \n", SENDER_PORT);

    sender_address.sin_family = AF_INET;
    sender_address.sin_port = htons(SENDER_PORT);
    sender_address.sin_addr.s_addr = INADDR_ANY;

    sender_socket = socket(AF_INET, SOCK_DGRAM, 0);

    if (bind(sender_socket, (struct sockaddr *) &sender_address, sizeof(sender_address)) == -1) {
        printf("There was an error binding the socket to an IP/Port \n\n");
    } else {
        printf("Socket successfully bound to IP/Port \n\n");
    }

    // ------------------ RECEIVER CONFIG ------------------

    printf("Configuring receiver to localhost:%d \n\n", RECEIVER_PORT);

    receiver_address.sin_family = AF_INET;
    receiver_address.sin_port = htons(RECEIVER_PORT);
    receiver_address.sin_addr.s_addr = INADDR_ANY;

    // ------------------ IMAGE LOAD ------------------

    printf("Loading image...\n");

    read_image(image_path, &image, &image_size);

    printf("Loaded image of size %ld bytes\n\n", image_size);

    // ------------------ IMAGE HASH ------------------

    printf("Calculating loaded image hash...\n");

    sha256_init(&ctx);
    sha256_update(&ctx, image, image_size);
    sha256_final(&ctx, hash);

    printf("Image hash: ");

    for (int i = 1; i < SHA256_BLOCK_SIZE; i++) {
        printf(" 0x%02x", hash[i]);
    }

    printf("\n\n");

    // ------------------ IMAGE OFFER ------------------

    printf("Waiting for a request from receiver... \n\n");

    offer_image(sender_socket, image_size, (struct sockaddr *) &receiver_address);

    printf("Receiver requested the image, sending... \n\n");

    // ------------------ IMAGE TRANSMISSION ------------------

    send_image(sender_socket, image, image_size, (struct sockaddr *) &receiver_address);

    printf("Image sent successfully \n\n");

    // ------------------ DATA VERIFICATION ------------------

    if (verify_hash(sender_socket, hash, (struct sockaddr *) &receiver_address)) {
        printf("Hashes match, image was successfully sent \n\n");
    } else {
        printf("There was an error verifying the hash \n\n");
    }

    close(sender_socket);

    return EXIT_SUCCESS;
}