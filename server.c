#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <librsync.h>

#include "common.h"

static char in_buf[BUFFER_SIZE * 2], out_buf[BUFFER_SIZE * 2];

static int accept_connection(void);
static int recv_signature(int sock, rs_signature_t **sig);

int main(int argc, char *argv[])
{
    puts("Waiting for connection...");
    int sock = accept_connection();
    if (sock == -1) {
        return EXIT_FAILURE;
    }

    puts("Receiving signature...");
    rs_signature_t *sig;
    int ret = recv_signature(sock, &sig);
    if (ret == -1) {
        close(sock);
        return EXIT_FAILURE;
    }

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

static int recv_signature(int sock, rs_signature_t **sig) {
    rs_job_t *job = rs_loadsig_begin(sig);
    assert(job != NULL);

    /* Setup buffers */
    rs_buffers_t bufs = { 0 };
    bufs.next_in = in_buf;

    rs_result res;
    do {
        if (bufs.eof_in == 0) {
            if (bufs.avail_in > BUFFER_SIZE) {
                /* The job requires more data, but we cannot fit another
                 * message into the input buffer */
                fputs("Insufficient buffer capacity", stderr);
                rs_job_free(job);
                return -1;
            }

            if (bufs.avail_in > 0) {
                /* Leftover tail data, move it to front */
                memmove(in_buf, bufs.next_in, bufs.avail_in);
            }

            size_t n_bytes;
            int ret = recv_message(sock, in_buf + bufs.avail_in, &n_bytes, &bufs.eof_in);
            if (ret == -1) {
                rs_job_free(job);
                return -1;
            }

            bufs.next_in = in_buf;
            bufs.avail_in += n_bytes;
        }

        res = rs_job_iter(job, &bufs);
        if (res != RS_DONE && res != RS_BLOCKED) {
            rs_job_free(job);
            return -1;
        }

        /* The job should take care of draining the buffers */
    } while (res != RS_DONE);

    rs_job_free(job);
    return 0;
}
