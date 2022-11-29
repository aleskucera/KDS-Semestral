//
// Created by ales on 28.11.22.
//

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include "image.h"

int main() {

    // Socket
    long msg_size;
    int client_socket;
    int connection_status;
    struct sockaddr_in server_address;

    // Image
    char *buffer;
    long image_size;
    bool write_status;
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


    // Receive message from the server about size of the image
    msg_size = recv(client_socket, &image_size, sizeof(image_size), 0);

    // Check for error with the connection
    if (msg_size == -1) {
        printf("There was an error receiving data from the remote socket \n\n");
    } else {
        printf("Received message from the server: %ld\n", image_size);
    }

    // Send message to the server to start sending the image
    msg_size = send(client_socket, "start", 5, 0);

    // Check for error with the connection
    if (msg_size == -1) {
        printf("There was an error sending data to the remote socket \n\n");
    } else {
        printf("Sent message to the server: start\n");
    }

    buffer = malloc(image_size);
    // Receive the image
    msg_size = recv(client_socket, buffer, image_size, 0);

    // Check for error with the connection
    if (msg_size == -1) {
        printf("There was an error receiving data from the remote socket \n\n");
    } else {
        printf("Received image from the server!\n");
    }

    // Save the image
    write_status = write_image(file_name, buffer, image_size);

    if (!write_status) {
        printf("Error writing image!\n");
    } else {
        printf("Image written successfully!\n");
    }

    // Close the socket
    close(client_socket);

    return 0;
}



