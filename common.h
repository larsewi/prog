#ifndef COMMON_H
#define COMMON_H

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <assert.h>
#include <arpa/inet.h>
#include <librsync.h>

/** The port the server will be listening on */
#define PORT 5612

/* It's the largest value we can represent with the 12 bits in the SDU Length
 * header field (2^12-1 = 4095). Hence, it's the largest allowed payload. */
#define BUFFER_SIZE 4095


static int send_message(int sock, const char *msg, size_t len, int eof) {
    assert(len <= BUFFER_SIZE);

    /* Make space for EoF flag */
    uint16_t header = (uint16_t)len << 4;

    /* Set EoF flag */
    if (eof != 0) {
        header |= 1;
    }

    /* Send header */
    header = htons(header);
    ssize_t ret = write(sock, &header, sizeof(header));
    if (ret < 0 || (size_t)ret != sizeof(header)) {
        perror("Failed to send message header");
        return -1;
    }

    if (len > 0) {
        /* Send payload */
        ret = write(sock, msg, len);
        if (ret < 0 || (size_t)ret != len) {
            perror("Failed to send message payload");
            return -1;
        }
    }

    return 0;
}

static int recv_message(int sock, char *msg, size_t *len, int *eof) {
    /* Receive header */
    uint16_t header;
    ssize_t ret = read(sock, &header, sizeof(header));
    if (ret < 0 || (size_t)ret != sizeof(header)) {
        perror("Failed to receive message header");
        return -1;
    }
    header = ntohs(header);

    /* Extract EOF flag */
    *eof = header & 1;

    /* Extract message length */
    *len = header >> 4;

    if (*len > 0) {
        /* Read payload */
        ret = read(sock, msg, *len);
        if (ret < 0 || (size_t)ret != *len) {
            perror("Failed to receive message payload");
            return -1;
        }
    }

    return 0;
}

#endif /* COMMON_H */
