/*********************************************************************
* Author:     ales
* Created:    29.11.22
*********************************************************************/

#include "communication.h"


void send_packet(int socket, byte msg_type, size_t n1, uint16_t n2, byte *data, struct sockaddr *address) {
    byte packet[PACKET_SIZE];
    socklen_t addr_len = sizeof(*address);

    // Fill packet
    packet[0] = msg_type;
    memcpy(&packet[1], &n1, sizeof(size_t));
    memcpy(&packet[1 + sizeof(size_t)], &n2, sizeof(uint16_t));

    if (data != NULL) {
        memcpy(&packet[1 + sizeof(size_t) + sizeof(uint16_t)], data, n2);
    } else {
        memset(&packet[1 + sizeof(size_t) + sizeof(uint16_t)], 0, n2);
    }

    // Calculate CRC
    encode_packet(packet, PACKET_SIZE);

    // Send packet
    sendto(socket, packet, PACKET_SIZE, 0, address, addr_len);
}

bool receive_packet(int socket, byte *msg_type, size_t *n1, uint16_t *n2, byte *data) {
    fd_set read_fds;
    byte packet[PACKET_SIZE];
    struct timeval timeout = {0, TIMEOUT};

    // Wait for packet, if timeout or error occurs, return false
    FD_ZERO(&read_fds);
    FD_SET(socket, &read_fds);

    switch (select(socket + 1, &read_fds, NULL, NULL, &timeout)) {
        case -1:
            printf("ERROR: select() failed\n");
            return false;
        case 0:
            printf("WARNING: Timeout\n");
            return false;
        default:
            break;
    }

    // Receive packet
    recvfrom(socket, packet, PACKET_SIZE, 0, NULL, NULL);

    // Decode packet, if CRC is invalid, return false
    if (!decode_packet(packet, PACKET_SIZE)) {
        printf("WARNING: CRC error \n");
        return false;
    }

    // Extract packet data
    *msg_type = packet[0];

    if (n1 != NULL) {
        memcpy(n1, &packet[1], sizeof(size_t));
    }

    if (n2 != NULL) {
        memcpy(n2, &packet[1 + sizeof(size_t)], sizeof(uint16_t));
    }

    if (data != NULL) {
        memcpy(data, &packet[1 + sizeof(size_t) + sizeof(uint16_t)], DATA_SIZE);
    }

    return true;
}

bool request_image(int socket, size_t *image_size, struct sockaddr *sender_address) {
    byte msg_type;

    // Send request
    send_packet(socket, REQ_MSG, 0, 0, NULL, sender_address);
    printf("INFO: Request sent\n");

    // Listen for response
    for (int i = 0; i < 10; i++) {
        if (receive_packet(socket, &msg_type, image_size, NULL, NULL)
            && (msg_type == OFF_MSG)) {
            printf("INFO: Received OFF message with image size: %lu\n", *image_size);
            for (int i = 0; i < 10; i++) {
                send_packet(socket, ACK_MSG, 0, 0, NULL, sender_address);
            }
            return true;
        } else {
            printf("WARNING: Sending request again\n");
            send_packet(socket, REQ_MSG, 0, 0, NULL, sender_address);
        }
    }
    return false;
}

void offer_image(int socket, size_t image_size, struct sockaddr *receiver_address) {
    byte msg_type;

    // Listen for request
    while (!receive_packet(socket, &msg_type, NULL, NULL, NULL)
           || msg_type != REQ_MSG) {
    }
    printf("INFO: Received request\n");

    // Send image size
    while (true) {
        send_packet(socket, OFF_MSG, image_size, 0, NULL, receiver_address);

        // Listen for ACK
        if (receive_packet(socket, &msg_type, NULL, NULL, NULL) && msg_type == ACK_MSG) {
            printf("INFO: Received ACK, proceeding to send image\n");
            return;
        } else {
            printf("WARNING: ACK not received, sending OFF again\n");
        }
    }

}

void send_image(int socket, byte *image, size_t image_size, const byte *hash, struct sockaddr *receiver_address) {
    size_t i = 0;
    byte msg_type;
    byte data[DATA_SIZE];
    size_t packet_number;

    bool *acks;
    bool *sents;
    size_t n_packets;
    uint16_t segment_size;
    uint16_t *segment_sizes;

    // Get segment sizes
    segment_sizes = get_segment_sizes(image_size, &n_packets);
    printf("INFO: Number of packets: %lu\n", n_packets);

    // Allocate memory for ACKs
    acks = malloc(sizeof(bool) * n_packets);
    memset(acks, false, sizeof(bool) * n_packets);

    sents = malloc(sizeof(bool) * n_packets);
    memset(sents, false, sizeof(bool) * n_packets);

    while (true) {
        // Send segment
        if (!sents[i]) {
            send_packet(socket, DATA_MSG,
                        i, segment_sizes[i],
                        &image[get_offset(segment_sizes, i)],
                        receiver_address);
            sents[i] = true;
        }

        // Listen for response
        if (receive_packet(socket, &msg_type, &packet_number, &segment_size, data)
            && packet_number < n_packets) {
            switch (msg_type) {
                case ACK_MSG: // Acknowledge segment
                    printf("INFO: Received ACK for segment %lu\n", packet_number);

                    // Mark segment as acknowledged
                    if (segment_size == segment_sizes[packet_number]) {
                        acks[packet_number] = true;
                    } else {
                        printf("WARNING: Segment size mismatch\n");
                    }
                    break;
                case NAK_MSG: // Negative acknowledge segment, resend
                    printf("WARNING: Received NAK for segment %lu\n", packet_number);
                    //Fill ACKs
                    for (int i = 0; i < packet_number; i++) {
                        acks[i] = true;
                    }
                    // Resend packet
                    send_packet(socket, DATA_MSG,
                                packet_number, segment_sizes[packet_number],
                                &image[get_offset(segment_sizes, packet_number)],
                                receiver_address);
                    break;
                case HASH_MSG: // Hash received, send ack or nak
                    printf("INFO: Received hash\n");

                    // Check if hash is correct
                    if (memcmp(data, hash, SHA256_BLOCK_SIZE) == 0) {
                        printf("INFO: Hash is correct\n");
                        send_packet(socket, ACK_MSG, 0, 0, NULL, receiver_address);
                    } else {
                        printf("ERROR: Hash is incorrect\n");
                        send_packet(socket, NAK_MSG, 0, 0, NULL, receiver_address);
                    }
                    break;
                case EOT_MSG: // End of transmission, exit
                    printf("INFO: Received EOT\n");
                    free(segment_sizes);
                    free(acks);
                    free(sents);
                    return;
                default:
                    printf("WARNING: Received unknown message type\n");
                    break;
            }
        } else {
            printf("WARNING: Received invalid packet\n");
        }

        // Increment segment index
        if (i < (WINDOW_SIZE + get_missing_segment(acks, n_packets)) && i < n_packets-1) {
            i++;
        } else {
            i = get_missing_segment(acks, n_packets);
        }
    }
}

byte *receive_image(int socket, size_t image_size, struct sockaddr *sender_address) {
    size_t i;

    byte msg_type;
    uint16_t data_size;
    size_t packet_number;

    byte data[DATA_SIZE];
    byte *image = malloc(image_size);

    bool *acks;
    size_t n_packets;
    uint16_t *segment_sizes;

    SHA256_CTX ctx;
    byte hash[SHA256_BLOCK_SIZE];

    // Get segment sizes
    segment_sizes = get_segment_sizes(image_size, &n_packets);
    printf("INFO: Number of expected packets: %lu\n", n_packets);

    // Allocate memory for ACKs
    acks = malloc(sizeof(bool) * n_packets);
    memset(acks, false, sizeof(bool) * n_packets);

    // Receive image
    while ((i = get_missing_segment(acks, n_packets)) < n_packets) {

        // Listen for data segment
        if (receive_packet(socket, &msg_type, &packet_number, &data_size, data)
            && msg_type == DATA_MSG && (packet_number < n_packets)) {
            printf("INFO: Received segment %lu\n", packet_number);
            acks[packet_number] = true;
            memcpy(&image[get_offset(segment_sizes, packet_number)], data, data_size);
            //send_packet(socket, ACK_MSG, packet_number, 0, NULL, sender_address);
        } else {
            printf("WARNING: Received invalid packet\n");
        }

        // Inform sender of missing segment
        if (acks[i] == 0) {
            printf("WARNING: Missing segment %lu\n", i);
            send_packet(socket, NAK_MSG, i, 0, NULL, sender_address);
        }
    }

    // Send hash
    sha256_init(&ctx);
    sha256_update(&ctx, image, image_size);
    sha256_final(&ctx, hash);

    // Print hash
    printf("INFO: Hash: ");
    for (int j = 0; j < SHA256_BLOCK_SIZE; j++) {
        printf(" 0x%02x", hash[j]);
    }
    printf("\n");

    while (true) {
        printf("INFO: Sending hash for verification\n");
        send_packet(socket, HASH_MSG, 0, SHA256_BLOCK_SIZE, hash, sender_address);

        // Listen for ACK
        if (receive_packet(socket, &msg_type, NULL, NULL, NULL)) {
            if (msg_type == ACK_MSG) {
                printf("INFO: Received ACK, image received successfully\n");
                break;
            } else if (msg_type == NAK_MSG) {
                printf("ERROR: Received NAK, image received unsuccessfully\n");
                return NULL;
            } else {
                printf("WARNING: Received unknown message type, sending hash again\n");
            }
        }
    }

    // Send EOT
    printf("INFO: Sending EOT messages\n");
    for (i = 0; i < 3; i++) {
        printf("INFO: %lu/3 EOT\n", i + 1);
        send_packet(socket, EOT_MSG, 0, 0, NULL, sender_address);
    }

    free(segment_sizes);
    free(acks);

    return image;
}





