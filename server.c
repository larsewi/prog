#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <librsync.h>

#include "common.h"

static int accept_connection(void);

int main(int argc, char *argv[])
{
    puts("Waiting for connection...");
    int sock = accept_connection();
    if (sock == -1) {
        return EXIT_FAILURE;
    }

    int eof;
    size_t len;
    char buffer[BUFFER_SIZE + 1 /* to add terminating null-byte */];
    do {
        int ret = recv_message(sock, buffer, &len, &eof);
        if (ret == -1) {
            return EXIT_FAILURE;
        }
        buffer[len] = '\0';
        printf("%s", buffer);
    } while (!eof);

    close(sock);
    return EXIT_SUCCESS;
}

static int accept_connection(void) {
    /* Create socket */
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == -1) {
        perror("Failed to create socket");
        return -1;
    }

    /* Enable reuse address */
    int opt = 1;
    setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    /* Assign IP address and port */
    struct sockaddr_in server_addr = { 0 };
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htons(INADDR_ANY); /* Use local IP */
    server_addr.sin_port = htons(PORT);

    /* Bind socket to given IP address */
    int ret = bind(sock, (struct sockaddr *)&server_addr, sizeof(server_addr));
    if (ret == -1) {
        perror("Failed to bind socket");
        close(sock);
        return -1;
    }

    /* Listen for incoming connections. In a real world application you would
       probably have a larger "connection request" queue. */
    ret = listen(sock, 1);
    if (ret == -1) {
        perror("Failed to listen");
        close(sock);
        return -1;
    }

    /* Accept incoming connection */
    struct sockaddr_in client_addr; socklen_t addr_len;
    int conn = accept(sock, (struct sockaddr *)&client_addr, &addr_len);

    /* We don't expect any more connections in this example. In a real world
       application you would probably keep this socket open to accept more
       connections. */
    close(sock);

    if (conn == -1) {
        perror("Failed to accept");
        return -1;
    }

    return conn;
}
