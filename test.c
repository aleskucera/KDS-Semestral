//
// Created by ales on 29.11.22.
//

#include <stdio.h>
#include <stdlib.h>
#include

int main() {
    unsigned long image_size = 1025;
    unsigned long number_of_packets = image_size / 1024;
    if (image_size % 1024 != 0) {
        number_of_packets++;
    }
    unsigned long last_packet_size = image_size % 1024;

    printf("Number of packets: %ld\n", number_of_packets);
    printf("Last packet size: %ld\n", last_packet_size);

}