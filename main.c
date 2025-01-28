#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "utils.h"

int main(ARG_UNUSED int argc, ARG_UNUSED char *argv[]) {
  printf("Hello World\n");

  const char content[] = "This is an immutable file";
  const char filename[] = "immutable.txt";

  int fd = open(filename, O_CREAT | O_WRONLY, (mode_t)0700);
  if (fd == -1) {
    fprintf(stderr, "Failed to open '%s': %s\n", filename, strerror(errno));
    return EXIT_FAILURE;
  }

  size_t n_bytes = 0;
  while (n_bytes < sizeof(content) - 1) {
    ssize_t ret = write(fd, content + n_bytes,
                        (sizeof(content) - 1 /* null-byte */) - n_bytes);
    if (ret < 0) {
      fprintf(stderr, "Failed to write '%s' to file '%s': %s\n", content,
              filename, strerror(errno));
      return EXIT_FAILURE;
    }
    n_bytes += (size_t)ret;
  }

  close(fd);
  return EXIT_SUCCESS;
}
