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
    /* Parse arguments (use stdin and stdout if no argument) */
    const char *fname = (argc >= 2) ? argv[1] : NULL;

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

static int send_signature(int sock, const char *fname) {
    /* Make sure the basis file exists, unless it is stdin */
    const int use_io_stream = (fname == NULL) || (strcmp(fname, "-") == 0);
    if (!use_io_stream) {
        FILE *file = rs_file_open(fname, "a", 0);
        if (file != NULL) {
            rs_file_close(file);
        }
    }

    /* Open basis file */
    FILE *file = rs_file_open(fname, "rb", 0);
    assert(file != NULL);

    /* Get file size */
    rs_long_t fsize = rs_file_size(file);

    /* Get recommended arguments */
    rs_magic_number sig_magic = 0;
    size_t block_len = 0, strong_len = 0;
    rs_result res = rs_sig_args(fsize, &sig_magic, &block_len, &strong_len);
    if (res != RS_DONE) {
        rs_file_close(file);
        return -1;
    }

    /* Start generating signature */
    rs_job_t *job = rs_sig_begin(block_len, strong_len, sig_magic);
    assert(job != NULL);

    /* Setup buffers */
    rs_buffers_t bufs = { 0 };
    bufs.next_in = in_buf;
    bufs.next_out = out_buf;
    bufs.avail_out = BUFFER_SIZE; /* We cannot send more in one message */

    /* Generate signature */
    do {
        if (bufs.eof_in == 0) {
            if (bufs.avail_in >= sizeof(in_buf)) {
                /* The job requires more data, but the input buffer is full */
                fputs("Insufficient buffer capacity", stderr);
                rs_file_close(file);
                rs_job_free(job);
                return -1;
            }

            if (bufs.avail_in > 0) {
                /* Leftover tail data, move it to front */
                memmove(in_buf, bufs.next_in, bufs.avail_in);
            }

            /* Fill input buffer */
            size_t n_bytes = fread(in_buf + bufs.avail_in, 1, sizeof(in_buf) - bufs.avail_in, file);
            if (n_bytes == 0) {
                if (ferror(file)) {
                    perror("Failed to read file");
                    rs_file_close(file);
                    rs_job_free(job);
                    return -1;
                }

                /* End-of-File reached */
                bufs.eof_in = feof(file);
                assert(bufs.eof_in != 0);
            }

            bufs.next_in = in_buf;
            bufs.avail_in += n_bytes;
        }

        /* Iterate job */
        res = rs_job_iter(job, &bufs);
        if (res != RS_DONE && res != RS_BLOCKED) {
            rs_file_close(file);
            rs_job_free(job);
            return -1;
        }

        size_t present = bufs.next_out - out_buf;
        if (present > 0 || res == RS_DONE) {
            /* Drain output buffer */
            assert(present <= BUFFER_SIZE);
            int ret = send_message(sock, out_buf, present, (res == RS_DONE) ? 1 : 0);
            if (ret == -1) {
                rs_file_close(file);
                rs_job_free(job);
                return -1;
            }

            bufs.next_out = out_buf;
            bufs.avail_out = BUFFER_SIZE;
        }
    } while (res != RS_DONE);

    rs_job_free(job);
    rs_file_close(file);
    return 0;
}
