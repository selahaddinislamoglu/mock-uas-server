/**
 * @file network_utils.c
 * @brief Implementation of network utility functions for the SIP server.
 */

#include "network_utils.h"
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

/**
 * @brief Sends a SIP message to the specified destination and port.
 * 
 * @param message The SIP message to be sent.
 * @param destination The IP address of the destination.
 * @param port The port number on the destination.
 */
void send_sip_message(const sip_message_t *message, const char *destination, int port) {
    int sockfd;
    struct sockaddr_in dest_addr;

    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        perror("Socket creation failed");
        return;
    }

    memset(&dest_addr, 0, sizeof(dest_addr));
    dest_addr.sin_family = AF_INET;
    dest_addr.sin_port = htons(port);

    if (inet_pton(AF_INET, destination, &dest_addr.sin_addr) <= 0) {
        perror("Invalid address/ Address not supported");
        close(sockfd);
        return;
    }

    // Send the message
    if (sendto(sockfd, message->buffer, strlen(message->buffer), 0,
               (struct sockaddr *)&dest_addr, sizeof(dest_addr)) < 0) {
        perror("Send failed");
    } else {
        printf("Message sent to %s:%d\n", destination, port);
    }

    close(sockfd);
}
