#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <limits.h>
#include <librsync.h>

#include "common.h"

#define IP_ADDRESS "127.0.0.1"


static char in_buf[BUFFER_SIZE * 2], out_buf[BUFFER_SIZE * 2];

static int connect_to_server(const char *ip_addr);
static int send_signature(int sock, const char *fname);

int main(int argc, char *argv[]) {
    /* Parse arguments */
    if (argc < 2) {
        printf("USAGE: %s <FILENAME>", argv[0]);
        return EXIT_FAILURE;
    }
    const char *fname = argv[1];

    puts("Connecting to server...");
    int sock = connect_to_server(IP_ADDRESS);
    if (sock == -1) {
        return EXIT_FAILURE;
    }

    puts("Sending signature...");
    int ret = send_signature(sock, fname);
    if (ret == -1) {
        close(sock);
        return EXIT_FAILURE;
    }

    close(sock);
    return EXIT_SUCCESS;
}

static int connect_to_server(const char *ip_addr) {
    /* Create socket */
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == -1) {
        perror("Failed to create socket");
        return -1;
    }

    /* Assign IP address and port */
    struct sockaddr_in addr = { 0 };
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr(ip_addr);
    addr.sin_port = htons(PORT);

    /* Connect to server */
    int ret = connect(sock, (struct sockaddr *)&addr, sizeof(addr));
    if (ret == -1) {
        perror("Failed to connect");
        close(sock);
        return -1;
    }

    return sock;
}
