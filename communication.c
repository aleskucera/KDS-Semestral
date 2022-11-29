//
// Created by ales on 29.11.22.
//

#include "communication.h"

uint16_t *get_segment_sizes(unsigned long image_size, uint16_t packet_data_size) {
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

    // Send request message
    if (send_msg_packet(client_socket, REQ_MSG) == -1) {
        printf("Error sending request message!\n");
        return false;
    }

    // Receive image size
    if (receive_off_packet(client_socket, image_size) == false) {
        printf("Error receiving image size!\n");
        return false;
    }

    // Send acknowledge message
    if (send_msg_packet(client_socket, ACK_MSG) == -1) {
        printf("Error sending ACK message!\n");
        return false;
    }

    return true;
}

bool off_image(int client_socket, unsigned long image_size) {

    // Wait for request message
    if (receive_msg_packet(client_socket, REQ_MSG) == false) {
        printf("Error receiving request message!\n");
        return false;
    }

    // Send offer message
    if (send_off_packet(client_socket, image_size) == -1) {
        printf("Error sending image size!\n");
        return false;
    }

    // Wait for acknowledge message
    if (receive_msg_packet(client_socket, ACK_MSG) == false) {
        printf("Error receiving ACK message!\n");
        return false;
    }
    return true;
}

bool send_image(int client_socket, unsigned char *image, unsigned long image_size) {
    uint16_t segment_size;

    // Split the image into segments
    uint16_t *segment_sizes = get_segment_sizes(image_size, PACKET_DATA_SIZE);

    while ((segment_size = *segment_sizes) != 0) {

        // Send the image segment
        if (send_data_packet(client_socket, image, segment_size) == PACKET_SIZE) {
            printf("SEND IMAGE: Sent data packet with %d amount of data bytes\n", segment_size);
            image += segment_size;
            segment_sizes++;
        } else {
            return false;
        }
    }

    // Send the last packet
    if (send_msg_packet(client_socket, END_MSG) == PACKET_SIZE) {
        printf("SEND IMAGE: Sent the END message \n");
    } else {
        return false;
    }

    return true;
}

bool receive_image(int client_socket, unsigned char *image) {
    int32_t data_size;

    while (true) {
        if (receive_data_packet(client_socket, image, &data_size)) {
            printf("RECEIVE IMAGE: Received data packet with %d amount of data bytes\n", data_size);
            image += data_size;
        } else {
            return false;
        }
    }
    return true;
}

ssize_t send_data_packet(int client_socket, unsigned char *data, uint16_t data_size) {
    ssize_t msg_size;
    unsigned char *packet = malloc(PACKET_SIZE);

    // Add msg type prefix
    packet[0] = DATA_MSG;

    // Add data size
    packet[1] = (unsigned char) (data_size >> 8);
    packet[2] = (unsigned char) data_size;

    // Add data
    memcpy(&packet[3], data, (size_t) data_size);

    // Calculate CRC
    encode_packet(packet, PACKET_SIZE);

    // Send packet
    msg_size = send(client_socket, packet, PACKET_SIZE, 0);

    // Free memory
    free(packet);

    return msg_size;
}

ssize_t send_msg_packet(int client_socket, unsigned char msg) {
    ssize_t msg_size;
    unsigned char *packet = malloc(PACKET_SIZE);

    // Add msg type prefix
    packet[0] = msg;

    // Zero the rest of the packet
    memset(&packet[1], 0, PACKET_SIZE - 1);

    // Calculate CRC
    encode_packet(packet, PACKET_SIZE);

    // Send packet
    msg_size = send(client_socket, packet, PACKET_SIZE, 0);

    // Free memory
    free(packet);

    return msg_size;
}

ssize_t send_off_packet(int client_socket, unsigned long image_size) {
    ssize_t msg_size;
    unsigned char *packet = malloc(PACKET_SIZE);

    // Add msg type prefix
    packet[0] = OFF_MSG;

    // Add image size
    packet[1] = (unsigned char) (image_size >> 24);
    packet[2] = (unsigned char) (image_size >> 16);
    packet[3] = (unsigned char) (image_size >> 8);
    packet[4] = (unsigned char) image_size;

    // Calculate CRC
    encode_packet(packet, PACKET_SIZE);

    // Send packet
    msg_size = send(client_socket, packet, PACKET_SIZE, 0);

    // Free memory
    free(packet);

    return msg_size;
}

unsigned long unpack_off_packet(unsigned char *packet) {
    unsigned long image_size;

    // Unpack image size
    image_size = packet[1] << 24;
    image_size |= packet[2] << 16;
    image_size |= packet[3] << 8;
    image_size |= packet[4];

    return image_size;
}

bool receive_off_packet(int client_socket, unsigned long *image_size) {
    unsigned char packet[PACKET_SIZE];

    // Receive packet
    if (recv(client_socket, packet, PACKET_SIZE, 0) == -1) {
        printf("RECEIVE OFFER: There was an error receiving the packet \n\n");
        return false;
    }

    // Decode packet
    if ((!decode_packet(packet, PACKET_SIZE)) || (packet[0] != OFF_MSG)) {
        printf("RECEIVE OFFER: There was an error decoding the packet \n\n");
        return false;
    } else {
        // Get image size
        *image_size = unpack_off_packet(packet);
        printf("RECEIVE OFF: Received OFF packet from the server: %ld\n", *image_size);
    }
    return true;
}


int32_t unpack_data_packet(unsigned char *packet, unsigned char *buffer) {
    uint16_t data_size;

    // Check if the packet is a DATA packet
    if (packet[0] != DATA_MSG) {
        return -1;
    }

    // Get data size
    data_size = packet[1] << 8 | packet[2];

    // Copy data to the buffer
    memcpy(buffer, packet + 3, data_size);

    return (int32_t) data_size;
}

bool receive_data_packet(int client_socket, unsigned char *buffer, int32_t *data_size) {
    unsigned char packet[PACKET_SIZE];

    // Receive packet
    if (recv(client_socket, packet, PACKET_SIZE, 0) == -1) {
        printf("RECEIVE DATA: There was an error receiving the packet \n\n");
        return false;
    }

    // Decode packet
    if ((!decode_packet(packet, PACKET_SIZE)) || (*data_size = unpack_data_packet(packet, buffer)) == -1) {
        printf("RECEIVE DATA: There was an error decoding the packet \n\n");
        return false;
    }
    return true;
}

bool receive_msg_packet(int client_socket, unsigned char msg) {
    unsigned char packet[PACKET_SIZE];

    // Receive packet
    if (recv(client_socket, packet, PACKET_SIZE, 0) == -1) {
        printf("RECEIVE MSG: There was an error receiving the packet \n\n");
        return false;
    }

    // Check if the packet is a MSG packet
    if ((!decode_packet(packet, PACKET_SIZE)) || (packet[0] != msg)) {
        return false;
    }

    return true;
}





