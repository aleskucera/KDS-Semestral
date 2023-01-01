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

bool receive_packet(int socket, byte *msg_type, size_t *n1, uint16_t *n2, byte *data, const struct timeval *timeout) {
    fd_set read_fds;
    byte packet[PACKET_SIZE];


    // Wait for packet, if timeout or error occurs, return false
    FD_ZERO(&read_fds);
    FD_SET(socket, &read_fds);

    int select_result;
    if (timeout != NULL) {
        struct timeval timeout_copy = *timeout;
        select_result = select(socket + 1, &read_fds, NULL, NULL, &timeout_copy);
    } else {
        select_result = select(socket + 1, &read_fds, NULL, NULL, NULL);
    }

    switch (select_result) {
        case -1:
            printf("ERROR: select() failed\n");
            return false;
        case 0:
            if (timeout != NULL) {
                printf("ERROR: select() timed out\n");
            }
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
    struct timeval timeout = {0, TIMEOUT};

    // Send request
    send_packet(socket, REQ_MSG, 0, 0, NULL, sender_address);
    printf("INFO: Request sent\n");

    // Listen for response
    for (int i = 0; i < 10; i++) {
        if (receive_packet(socket, &msg_type, image_size, NULL, NULL, &timeout)
            && (msg_type == OFF_MSG)) {
            printf("INFO: Received OFF message with image size: %lu\n", *image_size);
            for (int j = 0; j < 10; j++) {
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
    while (!receive_packet(socket, &msg_type, NULL, NULL, NULL, NULL)
           || msg_type != REQ_MSG) {
    }
    printf("INFO: Received request\n");

    // Send image size
    while (true) {
        send_packet(socket, OFF_MSG, image_size, 0, NULL, receiver_address);

        // Listen for ACK
        if (receive_packet(socket, &msg_type, NULL, NULL, NULL, NULL) && msg_type == ACK_MSG) {
            printf("INFO: Received ACK, proceeding to send image\n");
            return;
        } else {
            printf("WARNING: ACK not received, sending OFF again\n");
        }
    }

}

void send_image(int socket, byte *image, size_t image_size, const byte *hash, struct sockaddr *receiver_address) {
    size_t window_start;
    size_t window_end;
    byte msg_type;
    byte data[DATA_SIZE];
    size_t packet_number;
    struct timeval timeout = {0, TIMEOUT};
    struct timespec now;

    bool *acks;
    struct timespec *sent;
    size_t n_packets;
    uint16_t segment_size;
    uint16_t *segment_sizes;
    bool received_hash = false;

    // Get segment sizes
    segment_sizes = get_segment_sizes(image_size, &n_packets);
    printf("INFO: Number of packets: %lu\n", n_packets);

    // Allocate memory for ACKs
    acks = malloc(sizeof(bool) * n_packets);
    memset(acks, false, sizeof(bool) * n_packets);

    sent = malloc(sizeof(struct timespec) * n_packets);
    for (int i = 0; i < n_packets; i++) {
        sent[i].tv_sec = 0;
        sent[i].tv_nsec = 0;
    }

    while (true) {
        window_start = get_missing_segment(acks, n_packets);
        window_end = window_start + WINDOW_SIZE;

        // Check if sent packets have timed out
        if (!received_hash) {
            for (size_t i = window_start; i < window_end && i < n_packets; i++) {

                // Skip packets that has been acknowledged
                if (acks[i]) {
                    continue;
                }

                // Get time since packet was sent
                clock_gettime(CLOCK_REALTIME, &now);
                double elapsed = (now.tv_sec - sent[i].tv_sec) + (now.tv_nsec - sent[i].tv_nsec) * 1e-9;

                if ((sent[i].tv_sec == 0) && (sent[i].tv_nsec == 0)){
                    printf("INFO: Packet %lu has not been sent yet\n", i);
                } else {
                    printf("INFO: Packet %lu has been sent %.3f seconds ago\n", i, elapsed);
                }

                // If packet has timed out or has not been sent yet, send it
                if ((elapsed > PACKET_TIMEOUT) || ((sent[i].tv_sec == 0) && (sent[i].tv_nsec == 0))) {
                    clock_gettime(CLOCK_REALTIME, &sent[i]);
                    send_packet(socket, DATA_MSG, i, segment_sizes[i],
                                &image[get_offset(segment_sizes, i)], receiver_address);
                }
            }
        }

        // Listen for response
        if (receive_packet(socket, &msg_type, &packet_number, &segment_size, data, &timeout)
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
                    received_hash = true;
                    break;
                case EOT_MSG: // End of transmission, exit
                    printf("INFO: Received EOT\n");
                    free(segment_sizes);
                    free(acks);
                    free(sent);
                    return;
                default:
                    printf("WARNING: Received unknown message type\n");
                    break;
            }
        } else {
            printf("WARNING: Received invalid packet\n");

        }
    }
}

byte *receive_image(int socket, size_t image_size, struct sockaddr *sender_address) {
    byte msg_type;
    uint16_t data_size;
    size_t packet_number;
    struct timeval timeout = {0, TIMEOUT};

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
    while (get_missing_segment(acks, n_packets) < n_packets) {

        // Listen for data segment until something is received
        if (receive_packet(socket, &msg_type, &packet_number, &data_size, data, NULL)
            && msg_type == DATA_MSG && (packet_number < n_packets)) {
            printf("INFO: Received segment %lu\n", packet_number);
            acks[packet_number] = true;
            memcpy(&image[get_offset(segment_sizes, packet_number)], data, data_size);
            send_packet(socket, ACK_MSG, packet_number, segment_sizes[packet_number], NULL, sender_address);
        } else {
            printf("WARNING: Received invalid packet\n");
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

    printf("INFO: Sending hash for verification\n");
    send_packet(socket, HASH_MSG, 0, SHA256_BLOCK_SIZE, hash, sender_address);
    while (true) {
        if (receive_packet(socket, &msg_type, NULL, NULL, NULL, &timeout)) {
            if (msg_type == ACK_MSG) {
                printf("INFO: Received ACK, image received successfully\n");
                break;
            } else if (msg_type == NAK_MSG) {
                printf("ERROR: Received NAK, image received unsuccessfully\n");
                return NULL;
            } else if (msg_type == DATA_MSG){
                printf("WARNING: Received DATA, ignoring\n");
            } else {
                printf("WARNING: Received unknown message type, ignoring\n");
            }
        } else {
            printf("WARNING: No response received, sending hash again\n");
            send_packet(socket, HASH_MSG, 0, SHA256_BLOCK_SIZE, hash, sender_address);
        }
    }

    // Send EOT
    printf("INFO: Sending EOT messages\n");
    for (int i = 0; i < 10; i++) {
        printf("INFO: %d/10 EOT\n", i + 1);
        send_packet(socket, EOT_MSG, 0, 0, NULL, sender_address);
    }

    free(segment_sizes);
    free(acks);

    return image;
}





