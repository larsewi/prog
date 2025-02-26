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

static int connect_to_server(const char *ip_addr);

int main(int argc, char *argv[]) {
    puts("Connecting to server...");
    int sock = connect_to_server(IP_ADDRESS);
    if (sock == -1) {
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
