//
// Created by ales on 29.11.22.
//

#include "communication.h"

uint16_t *get_data_sizes(unsigned long image_size, uint16_t packet_data_size) {
    unsigned long number_of_packets;
    uint16_t last_packet_size;
    uint16_t *packet_sizes;

    // Calculate number of packets
    number_of_packets = image_size / packet_data_size;
    if (image_size % packet_data_size != 0) {
        number_of_packets++;
    }

    // Calculate size of the last packet
    last_packet_size = image_size % packet_data_size;

    // Allocate memory for packet sizes
    packet_sizes = malloc(sizeof(uint16_t) * number_of_packets + 1);

    // Fill packet sizes
    for (int i = 0; i < number_of_packets; i++) {
        packet_sizes[i] = packet_data_size;
    }
    packet_sizes[number_of_packets - 1] = last_packet_size;
    packet_sizes[number_of_packets] = 0;

    return packet_sizes;
}

bool req_image(int client_socket, unsigned long *image_size) {
    ssize_t msg_size;

    // Send request message
    msg_size = send(client_socket, REQ_MSG, sizeof(REQ_MSG), 0);
    if (msg_size == -1) {
        printf("REQUEST: There was an error sending data to the remote socket \n\n");
        return false;
    } else {
        printf("REQUEST: Sent request message to the server: %s\n", REQ_MSG);
    }

    // Wait for response
    msg_size = recv(client_socket, image_size, sizeof(size_t), 0);
    if (msg_size == -1) {
        printf("REQUEST: There was an error receiving data from the remote socket \n\n");
        return false;
    } else {
        printf("REQUEST: Received image size message from the server: %ld\n", *image_size);
    }

    // Acknowledge the image size
    msg_size = send(client_socket, ACK_MSG, sizeof(ACK_MSG), 0);
    if (msg_size == -1) {
        printf("REQUEST: There was an error sending data to the remote socket \n\n");
        return false;
    } else {
        printf("REQUEST: Acknowledged image size with the message: %s\n", ACK_MSG);
    }
    return true;
}

bool off_image(int client_socket, unsigned long image_size) {
    ssize_t msg_size;
    char request[MSG_SIZE];
    char acknowledgement[MSG_SIZE];

    // Wait for the request message from the client
    msg_size = recv(client_socket, request, MSG_SIZE, 0);
    if (msg_size == -1) {
        printf("OFFER: There was an error receiving data from the remote socket \n\n");
        return false;
    } else {
        printf("OFFER: Received message from a client: %s\n", request);
    }

    // Send the image size
    if (strcmp(request, REQ_MSG) == 0) {
        msg_size = send(client_socket, &image_size, sizeof(size_t), 0);
        if (msg_size == -1) {
            printf("OFFER: There was an error sending data to the remote socket \n\n");
            return false;
        } else {
            printf("OFFER: Sent message to the client: %ld\n", image_size);
        }
    }

    // Wait for the confirmation message from the client
    msg_size = recv(client_socket, acknowledgement, MSG_SIZE, 0);
    if (msg_size == -1) {
        printf("OFFER: There was an error receiving data from the remote socket \n\n");
        return false;
    } else {
        printf("OFFER: Received response from the client: %s\n", acknowledgement);
    }

    return (strcmp(acknowledgement, ACK_MSG) == 0);
}

bool send_image(int client_socket, unsigned char *image, unsigned long image_size) {
    ssize_t msg_size;
    uint16_t data_size;
    unsigned char *packet;
    uint16_t *data_sizes = get_data_sizes(image_size, PACKET_DATA_SIZE);

    while (*data_sizes != 0) {
        // Create packet
        data_size = *data_sizes;
        packet = create_data_packet(image, data_size);
        // Send packet
        msg_size = send(client_socket, packet, PACKET_SIZE, 0);
        free(packet);
        if (msg_size == -1) {
            printf("SEND: There was an error sending data to the remote socket \n\n");
            return false;
        } else {
            printf("SEND: Sent packet to the client: %d\n", *data_sizes);
        }
        image += *data_sizes;
        data_sizes++;
    }

    // Send the last packet
    packet = create_end_packet();
    printf("SEND: Sending the last packet %c\n", packet[0]);
    msg_size = send(client_socket, packet, PACKET_SIZE, 0);
    free(packet);
    if (msg_size == -1) {
        printf("SEND: There was an error sending data to the remote socket \n\n");
        return false;
    } else {
        printf("SEND: Sent the LAST packet");
    }


    return true;
}

bool receive_image(int client_socket, unsigned char *image, unsigned long image_size) {
    ssize_t msg_size;
    unsigned char packet[PACKET_SIZE];
    int32_t data_size;

    while (true) {
        msg_size = recv(client_socket, packet, PACKET_SIZE, 0);

        // Decode packet
        if ((msg_size == -1) || (!decode_packet(packet, PACKET_SIZE))) {
            printf("RECEIVE: There was an error decoding the packet \n\n");
            return false;
        } else {
            if ((data_size = unpack_data_packet(packet, image)) > 0) {
                printf("RECEIVE: Received packet from the server: %d\n", data_size);
                image += data_size;
            } else {
                printf("RECEIVE: Received the last packet\n");
                break;
            }
        }
    }
    return true;
}

unsigned char *create_data_packet(unsigned char *data, uint16_t data_size) {
    unsigned char *packet = malloc(PACKET_SIZE);
    packet[0] = DATA_MSG;
    packet[1] = (unsigned char) (data_size >> 8);
    packet[2] = (unsigned char) data_size;
    memcpy(&packet[3], data, (size_t) data_size);
    encode_packet(packet, PACKET_SIZE);
    return packet;
}

unsigned char *create_end_packet(void) {
    unsigned char *packet = malloc(PACKET_SIZE);
    packet[0] = END_MSG;
    memset(&packet[1], 0, PACKET_SIZE - 1);
    encode_packet(packet, PACKET_SIZE);
    return packet;
}

int32_t unpack_data_packet(unsigned char *packet, unsigned char *data) {
    uint16_t data_size;
    if (packet[0] == DATA_MSG) {
        data_size = packet[1] << 8 | packet[2];
        memcpy(data, packet + 3, data_size);
        return (int32_t) data_size;
    } else if (packet[0] == END_MSG) {
        return 0;
    } else {
        return -1;
    }
}





