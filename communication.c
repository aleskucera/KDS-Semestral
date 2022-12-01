/*********************************************************************
* Author:     ales
* Created:    29.11.22
*********************************************************************/

#include "communication.h"

uint16_t *get_segment_sizes(size_t image_size, size_t *number_of_packets) {
    uint16_t *packet_sizes;

    // Calculate number of packets
    *number_of_packets = image_size / DATA_SIZE;
    if (image_size % DATA_SIZE != 0) {
        (*number_of_packets)++;
    }

    // Allocate memory for packet sizes
    packet_sizes = malloc(sizeof(uint16_t) * *number_of_packets);

    // Fill packet sizes
    for (int i = 0; i < *number_of_packets; i++) {
        packet_sizes[i] = DATA_SIZE;
    }

    // Set last packet size
    packet_sizes[*number_of_packets - 1] = image_size % DATA_SIZE;

    return packet_sizes;
}

bool request_image(int socket, size_t *image_size, struct sockaddr *sender_address) {
    unsigned char packet[PACKET_SIZE];
    socklen_t addr_len = sizeof(*sender_address);

    // Send request
    packet[0] = REQ_MSG;
    memset(&packet[1], 0, PACKET_SIZE - 1);
    encode_packet(packet, PACKET_SIZE);
    sendto(socket, packet, PACKET_SIZE, 0, sender_address, addr_len);

    // Listen for response
    recvfrom(socket, packet, PACKET_SIZE, 0, NULL, NULL);
    if ((packet[0] == OFF_MSG) && (decode_packet(packet, PACKET_SIZE))) {
        *image_size = packet[1] << 24;
        *image_size |= packet[2] << 16;
        *image_size |= packet[3] << 8;
        *image_size |= packet[4];
    } else {
        return false;
    }

    // Send acknowledgement
    packet[0] = ACK_MSG;
    memset(&packet[1], 0, PACKET_SIZE - 1);
    encode_packet(packet, PACKET_SIZE);
    sendto(socket, packet, PACKET_SIZE, 0, sender_address, addr_len);

    return true;
}

bool offer_image(int socket, size_t image_size, struct sockaddr *receiver_address) {
    unsigned char packet[PACKET_SIZE];
    socklen_t addr_len = sizeof(*receiver_address);

    // Listen for request
    recvfrom(socket, packet, PACKET_SIZE, 0, NULL, NULL);

    // Send image size
    packet[0] = OFF_MSG;
    packet[1] = (unsigned char) (image_size >> 24);
    packet[2] = (unsigned char) (image_size >> 16);
    packet[3] = (unsigned char) (image_size >> 8);
    packet[4] = (unsigned char) image_size;
    memset(&packet[5], 0, PACKET_SIZE - 5);
    encode_packet(packet, PACKET_SIZE);
    sendto(socket, packet, PACKET_SIZE, 0, receiver_address, addr_len);

    // Listen for acknowledgement
    recvfrom(socket, packet, PACKET_SIZE, 0, NULL, NULL);
    if ((packet[0] == ACK_MSG) && (decode_packet(packet, PACKET_SIZE))) {
        return true;
    }
    return false;
}

void send_image(int socket, unsigned char *image, size_t image_size, struct sockaddr *receiver_address) {
    uint16_t *segment_sizes;
    size_t number_of_packets;
    size_t offset = 0;

    // Get segment sizes
    segment_sizes = get_segment_sizes(image_size, &number_of_packets);

    // Send data packets
    for (size_t i = 0; i < number_of_packets; i++) {
        send_data_packet(socket, image + offset, segment_sizes[i], i, receiver_address);
        offset += segment_sizes[i];
    }

    free(segment_sizes);
}

void send_data_packet(int socket, unsigned char *data, uint16_t segment_size, size_t packet_number,
                      struct sockaddr *receiver_address) {
    unsigned char packet[PACKET_SIZE];
    socklen_t addr_len = sizeof(*receiver_address);

    // Add msg type prefix
    packet[0] = DATA_MSG;

    // Add packet number
    packet[1] = (unsigned char) (packet_number >> 24);
    packet[2] = (unsigned char) (packet_number >> 16);
    packet[3] = (unsigned char) (packet_number >> 8);
    packet[4] = (unsigned char) packet_number;

    // Add data size
    packet[5] = (unsigned char) (segment_size >> 8);
    packet[6] = (unsigned char) segment_size;

    // Add data
    memcpy(&packet[7], data, segment_size);

    // Calculate CRC
    encode_packet(packet, PACKET_SIZE);

    // Send packet
    sendto(socket, packet, PACKET_SIZE, 0, receiver_address, addr_len);
}

unsigned char *receive_image(int socket, size_t image_size, struct sockaddr *sender_address) {
    unsigned char *buffer = malloc(image_size);
    size_t packet_number;
    size_t offset = 0;
    uint16_t *segment_sizes;
    size_t n_packets;
    uint16_t segment_size;

    // Get segment sizes
    segment_sizes = get_segment_sizes(image_size, &n_packets);

    // Receive packets
    for (size_t i = 0; i < n_packets; i++) {
        receive_data_packet(socket, buffer + offset, &segment_size, &packet_number, sender_address);
        offset += segment_size;
    }

    free(segment_sizes);

    return buffer;
}

bool receive_data_packet(int socket, unsigned char *buffer, uint16_t *segment_size, size_t *packet_number,
                         struct sockaddr *sender_address) {
    unsigned char packet[PACKET_SIZE];
    socklen_t addr_len = sizeof(*sender_address);

    // Listen for packet
    recvfrom(socket, packet, PACKET_SIZE, 0, NULL, NULL);

    // Check if packet is valid
    if ((packet[0] == DATA_MSG) && (decode_packet(packet, PACKET_SIZE))) {

        // Get packet number
        *packet_number = packet[1] << 24;
        *packet_number |= packet[2] << 16;
        *packet_number |= packet[3] << 8;
        *packet_number |= packet[4];

        // Get data size
        *segment_size = packet[5] << 8;
        *segment_size |= packet[6];

        // Get data
        memcpy(buffer, &packet[7], *segment_size);

        return true;
    } else {
        return false;
    }
}

bool send_hash(int socket, unsigned char *hash, struct sockaddr *address) {
    unsigned char packet[PACKET_SIZE];
    socklen_t addr_len = sizeof(*address);

    // Add msg type prefix
    packet[0] = HASH_MSG;

    // Add hash
    memcpy(&packet[1], hash, 32);

    // Calculate CRC
    encode_packet(packet, PACKET_SIZE);

    // Send packet
    sendto(socket, packet, PACKET_SIZE, 0, address, addr_len);

    // Listen for acknowledgement
    recvfrom(socket, packet, PACKET_SIZE, 0, NULL, NULL);
    if ((packet[0] == ACK_MSG) && (decode_packet(packet, PACKET_SIZE))) {
        return true;
    }
    return false;
}

bool verify_hash(int socket, unsigned char *hash, struct sockaddr *address) {
    unsigned char packet[PACKET_SIZE];
    socklen_t addr_len = sizeof(*address);
    unsigned char received_hash[32];

    // Listen for packet
    recvfrom(socket, packet, PACKET_SIZE, 0, NULL, NULL);

    // Check if packet is valid
    if ((packet[0] == HASH_MSG) && (decode_packet(packet, PACKET_SIZE))) {

        // Get hash
        memcpy(received_hash, &packet[1], 32);

        if (!memcmp(received_hash, hash, 32)) {
            // Send acknowledgement
            packet[0] = ACK_MSG;
            memset(&packet[1], 0, PACKET_SIZE - 1);
            encode_packet(packet, PACKET_SIZE);
            sendto(socket, packet, PACKET_SIZE, 0, address, addr_len);
            return true;
        }
    }
    return false;
}





