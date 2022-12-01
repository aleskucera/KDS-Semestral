/*********************************************************************
* Author:     ales
* Created:    29.11.22
*********************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include "image.h"
#include "sha256.h"
#include "communication.h"

#define SENDER_PORT 14000
#define RECEIVER_PORT 15001

int main() {
    int receiver_socket;
    struct sockaddr_in sender_address;
    struct sockaddr_in receiver_address;

    size_t image_size;
    unsigned char *image;
    char *save_path = "/home/ales/School/KDS/KDS-Semestral/images/received_image.jpeg";

    SHA256_CTX ctx;
    unsigned char hash[SHA256_BLOCK_SIZE];

    // ------------------ RECEIVER CONFIG ------------------

    printf("Configuring receiver to localhost:%d \n", RECEIVER_PORT);

    receiver_address.sin_family = AF_INET;
    receiver_address.sin_port = htons(RECEIVER_PORT);
    receiver_address.sin_addr.s_addr = INADDR_ANY;

    receiver_socket = socket(AF_INET, SOCK_DGRAM, 0);

    if (bind(receiver_socket, (struct sockaddr *) &receiver_address, sizeof(receiver_address)) == -1) {
        printf("There was an error binding the socket to an IP/Port \n\n");
    } else {
        printf("Socket successfully bound to IP/Port \n\n");
    }

    // ------------------ SENDER CONFIG ------------------

    printf("Configuring sender to localhost:%d \n\n", SENDER_PORT);

    sender_address.sin_family = AF_INET;
    sender_address.sin_port = htons(SENDER_PORT);
    sender_address.sin_addr.s_addr = INADDR_ANY;

    // ------------------ IMAGE REQUEST ------------------

    printf("Requesting an image... \n");

    request_image(receiver_socket, &image_size, (struct sockaddr *) &sender_address);

    printf("Accepted an image offer of size %ld \n", image_size);

    // ------------------ IMAGE TRANSMISSION ------------------

    printf("Waiting for the image data... \n\n");

    image = receive_image(receiver_socket, image_size, (struct sockaddr *) &sender_address);

    printf("Received image data \n\n");

    // ------------------ IMAGE HASH ------------------

    printf("Calculating the hash of the received image... \n");

    sha256_init(&ctx);
    sha256_update(&ctx, image, image_size);
    sha256_final(&ctx, hash);

    printf("Image hash: ");

    for (int i = 1; i < SHA256_BLOCK_SIZE; i++) {
        printf(" 0x%02x", hash[i]);
    }

    printf("\n\n");

    // ------------------ DATA VERIFICATION ------------------

    if (send_hash(receiver_socket, hash, (struct sockaddr *) &sender_address)) {
        printf("Hashes match, image was successfully received \n\n");
        write_image(save_path, image, image_size);
        printf("Image saved to %s \n\n", save_path);
    } else {
        printf("There was an error verifying the hash \n\n");
    }

    close(receiver_socket);

    return EXIT_SUCCESS;
}



