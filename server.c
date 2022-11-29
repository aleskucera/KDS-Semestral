//
// Created by ales on 28.11.22.
//

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include "image.h"

int main() {

    // Socket
    long msg_size;
    int server_socket;
    int client_socket;
    char confirmation[5];
    struct sockaddr_in server_address;

    // Image
    unsigned char *buffer;
    unsigned long image_size;
    bool read_status;
    unsigned long number_of_packets;
    unsigned long last_packet_size;
    char *file_name = "/home/ales/School/KDS/KDS-Semestral/images/image.jpeg";

    // Read image
    read_status = read_image(file_name, &buffer, &image_size);
    if (!read_status) {
        printf("Error reading image!\n");
        return 1;
    } else {
        printf("Image read successfully!\n");
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
        printf("Socket successfully binded to IP/Port \n\n");
    }

    listen(server_socket, 1);
    client_socket = accept(server_socket, NULL, NULL);

    // Send the message about size of the image
    msg_size = send(client_socket, &image_size, sizeof(image_size), 0);

    // Check for error with the connection
    if (msg_size == -1) {
        printf("There was an error sending data to the remote socket \n\n");
    } else {
        printf("Sent message to the client: %ld\n", image_size);
    }

    // Receive message from the server to start sending the image
    msg_size = recv(client_socket, confirmation, 5, 0);

    // Check for error with the connection
    if (msg_size == -1) {
        printf("There was an error receiving data from the remote socket \n\n");
    } else {
        printf("Received message from the client: %s\n", confirmation);
    }

    // Compare confirmation message
    if (strcmp(confirmation, "start") == 0) {
        printf("Confirmation message received!\n");
    } else {
        printf("Confirmation message not received!\n");
    }


    // Send the image
    number_of_packets = (int) (image_size / 1024) + 1;
    last_packet_size = image_size % 1024;

    for (int i = 0; i < number_of_packets; i++) {
        if (i == number_of_packets - 1) {
            printf("Sending last packet of size %ld\n", last_packet_size);
            msg_size = send(client_socket, buffer + i * 1024, last_packet_size, 0);
        } else {
            msg_size = send(client_socket, buffer + i * 1024, 1024, 0);
        }

        // Check for error with the connection
        if (msg_size == -1) {
            printf("There was an error sending data to the remote socket \n\n");
        } else {
            printf("Sent packet %d to the client\n", i);
        }
    }

    // Close the socket
    close(server_socket);

    return 0;
}