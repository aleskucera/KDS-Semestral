//
// Created by ales on 28.11.22.
//

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include "image.h"
#include "communication.h"

int main() {

    // Socket
    int server_socket;
    int client_socket;
    struct sockaddr_in server_address;

    // Image
    unsigned char *buffer;
    unsigned long image_size;
    char *file_name = "/home/ales/School/KDS/KDS-Semestral/images/image.jpeg";

    // Read image
    if (read_image(file_name, &buffer, &image_size)) {
        printf("Image read successfully!\n");
    } else {
        printf("Error reading image!\n");
        return EXIT_FAILURE;
    }

    // Create the server socket
    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(9002);
    server_address.sin_addr.s_addr = INADDR_ANY;

    // Bind the socket to our specified IP and port
    if (bind(server_socket, (struct sockaddr *) &server_address, sizeof(server_address)) == -1) {
        printf("There was an error binding the socket to an IP/Port \n\n");
    } else {
        printf("Socket successfully bound to IP/Port \n\n");
    }

    // Listen for connections
    listen(server_socket, 1);
    client_socket = accept(server_socket, NULL, NULL);

    // Offer image to the client and send it
    if (off_image(client_socket, image_size)) {
        send_image(client_socket, buffer, image_size);
    }
    //wait 1 second
    sleep(10);
    printf("Closing the socket\n");

    // Close the socket
    close(server_socket);

    return EXIT_SUCCESS;
}