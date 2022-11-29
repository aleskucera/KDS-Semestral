//
// Created by ales on 29.11.22.
//


#include "communication.h"

size_t *get_packet_sizes(size_t image_size, size_t packet_size) {
    size_t number_of_packets, last_packet_size;
    size_t *packet_sizes;

    // Calculate number of packets
    number_of_packets = image_size / packet_size;
    if (image_size % packet_size != 0) {
        number_of_packets++;
    }

    // Calculate size of the last packet
    last_packet_size = image_size % packet_size;

    // Allocate memory for packet sizes
    packet_sizes = malloc(sizeof(size_t) * number_of_packets + 1);

    // Fill packet sizes
    for (int i = 0; i < number_of_packets; i++) {
        packet_sizes[i] = packet_size;
    }
    packet_sizes[number_of_packets - 1] = last_packet_size;
    packet_sizes[number_of_packets] = 0;

    return packet_sizes;
}

void req_image(int client_socket) {
    size_t msg_size;
    msg_size = send(client_socket, REQ_MSG, sizeof(REQ_MSG), 0);

    // Check for error with the connection
    if (msg_size == -1) {
        printf("REQUEST: There was an error sending data to the remote socket \n\n");
        exit(1);
    } else {
        printf("REQUEST: Sent message to the server: %s\n", REQ_MSG);
    }
}

void off_image(int client_socket, size_t image_size) {
    size_t msg_size;
    msg_size = send(client_socket, &image_size, sizeof(image_size), 0);

    // Check for error with the connection
    if (msg_size == -1) {
        printf("OFFER: There was an error sending data to the remote socket \n\n");
        exit(1);
    } else {
        printf("OFFER: Sent message to the server: %ld\n", image_size);
    }
}

void ack_image(int client_socket) {
    size_t msg_size;
    msg_size = send(client_socket, ACK_MSG, sizeof(ACK_MSG), 0);

    // Check for error with the connection
    if (msg_size == -1) {
        printf("ACK: There was an error sending data to the remote socket \n\n");
        exit(1);
    } else {
        printf("ACK: Sent message to the server: %s\n", ACK_MSG);
    }
}

bool send_image(int client_socket, char *image, size_t image_size) {
    size_t *packet_sizes = get_packet_sizes(image_size, PACKET_SIZE);

    while (*packet_sizes != 0) {
        size_t msg_size = send(client_socket, image, *packet_sizes, 0);
        if (msg_size == -1) {
            printf("SEND: There was an error sending data to the remote socket \n\n");
            return false;
        } else {
            image += *packet_sizes;
            printf("SEND: Sent packet to the server: %ld\n", *packet_sizes);
        }
        packet_sizes++;
    }
    return true;
}

bool receive_image(int client_socket, char *image, size_t image_size) {
    size_t *packet_sizes = get_packet_sizes(image_size, PACKET_SIZE);

    while (*packet_sizes != 0) {
        size_t msg_size = recv(client_socket, image, *packet_sizes, 0);
        if (msg_size == -1) {
            printf("RECEIVE: There was an error receiving data from the remote socket \n\n");
            return false;
        } else {
            image += *packet_sizes;
            printf("RECEIVE: Received packet from the server: %ld\n", *packet_sizes);
        }
        packet_sizes++;
    }
    return true;
}

