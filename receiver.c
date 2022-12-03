/*********************************************************************
* Author:     ales
* Created:    29.11.22
*********************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include "main.h"
#include "sha256.h"
#include "communication.h"

int main() {
    int ret = EXIT_SUCCESS;

    int receiver_socket;
    struct sockaddr_in sender_address;
    struct sockaddr_in receiver_address;

    byte *image;
    size_t image_size;
    char *save_path = SAVE_PATH;

    // ------------------ RECEIVER CONFIG ------------------

    printf("INFO: Configuring receiver to localhost:%d \n", RECEIVER_PORT_IN);

    receiver_address.sin_family = AF_INET;
    receiver_address.sin_port = htons(RECEIVER_PORT_IN);
    receiver_address.sin_addr.s_addr = INADDR_ANY;

    if ((receiver_socket = socket(AF_INET, SOCK_DGRAM, 0)) == -1) {
        printf("ERROR: There was an error creating the socket \n\n");
    } else {
        printf("INFO: Socket successfully created \n\n");
    }

    if (bind(receiver_socket, (struct sockaddr *) &receiver_address, sizeof(receiver_address)) == -1) {
        printf("ERROR: There was an error binding the socket to an IP/Port \n\n");
    } else {
        printf("INFO: Socket successfully bound to IP/Port \n\n");
    }

    // ------------------ SENDER CONFIG ------------------

    printf("INFO: Configuring sender to localhost:%d \n\n", RECEIVER_PORT_OUT);

    sender_address.sin_family = AF_INET;
    sender_address.sin_port = htons(RECEIVER_PORT_OUT);
    sender_address.sin_addr.s_addr = INADDR_ANY;

    // ------------------ IMAGE REQUEST ------------------

    printf("INFO: Requesting image...\n");
    while (true) {
        if (request_image(receiver_socket, &image_size,
                          (struct sockaddr *) &sender_address)) {

            printf("INFO: Sender offered an image of size %ld \n", image_size);
            printf("INFO: Waiting for the data...\n");

            image = receive_image(receiver_socket, image_size,
                                  (struct sockaddr *) &sender_address);

            if ((image != NULL) && save_image(save_path, image, image_size)) {
                printf("INFO: Image successfully saved to %s \n", save_path);
                break;
            } else {
                printf("ERROR: Image was not received correctly \n");
                ret = EXIT_FAILURE;
                break;
            }

        } else {
            printf("WARNING: There was an error requesting the image, requesting again... \n\n");
        }
    }
    close(receiver_socket);
    return ret;
}



