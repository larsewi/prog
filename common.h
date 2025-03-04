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

    uint16_t header = (uint16_t)len << 4;

    /* Set EoF flag */
    if (eof != 0) {
        header |= 1;
    }

    /* Send header */
    header = htons(header);
    size_t n_bytes = 0;
    do {
        ssize_t ret = write(sock, &header + n_bytes, sizeof(header) - n_bytes);
        if (ret < 0) {
            perror("Failed to send message header");
            return -1;
        }
        n_bytes += (size_t)ret;
    } while (n_bytes < sizeof(header));

    if (len > 0) {
        /* Send payload */
        n_bytes = 0;
        do {
            ssize_t ret = write(sock, msg + n_bytes, len - n_bytes);
            if (ret < 0) {
                perror("Failed to send message payload");
                return -1;
            }
            n_bytes += (size_t)ret;
        } while (n_bytes < len);
    }

    return 0;
}

static int recv_message(int sock, char *msg, size_t *len, int *eof) {
    /* Receive header */
    uint16_t header;
    size_t n_bytes = 0;
    do {
        ssize_t ret = read(sock, &header + n_bytes, sizeof(header) - n_bytes);
        if (ret < 0) {
            perror("Failed to receive message header");
            return -1;
        }
        n_bytes += (size_t)ret;
    } while (n_bytes < sizeof(header));
    header = ntohs(header);

    /* Extract EOF flag */
    *eof = header & 1;

    /* Extract message length */
    *len = header >> 4;

    if (*len > 0) {
        /* Read payload */
        n_bytes = 0;
        do {
            ssize_t ret = read(sock, msg + n_bytes, *len - n_bytes);
            if (ret < 0) {
                perror("Failed to receive message payload");
                return -1;
            }
            n_bytes += (size_t)ret;
        } while (n_bytes < *len);
    }

    return 0;
}

#endif /* COMMON_H */
