//
// Created by ales on 28.11.22.
//

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include "image.h"
#include "communication.h"

int main() {

    // Socket
    int client_socket;
    int connection_status;
    struct sockaddr_in server_address;

    // Image
    size_t image_size;
    unsigned char *buffer;
    char *file_name = "/home/ales/School/KDS/KDS-Semestral/images/received_image.jpeg";



    // Create a socket
    client_socket = socket(AF_INET, SOCK_STREAM, 0);

    // Define the server address
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(9002);
    server_address.sin_addr.s_addr = INADDR_ANY;

    // Connect to the server
    connection_status = connect(client_socket,
                                (struct sockaddr *) &server_address,
                                sizeof(server_address));

    // Check for error with the connection
    if (connection_status == -1) {
        printf("There was an error making a connection to the remote socket \n\n");
    } else {
        printf("Connected to the server successfully!\n");
    }


    if (req_image(client_socket, &image_size)) {
        buffer = (unsigned char *) malloc(image_size);
        receive_image(client_socket, buffer);
        if (write_image(file_name, buffer, image_size)) {
            printf("Image written successfully!\n");
        } else {
            printf("Error writing image!\n");
        }
        free(buffer);
    } else {
        printf("Error receiving image!\n");
    }

    close(client_socket);

    return EXIT_SUCCESS;
}



