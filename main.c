#include <errno.h>
#include <fcntl.h>
#include <linux/fs.h> /* Definition of FS_* constants */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <unistd.h>

#include "utils.h"

static const char filename[] = "immutable.txt";
static const char content[] = "This is an immutable file";

int main(ARG_UNUSED int argc, ARG_UNUSED char *argv[]) {
  printf("Creating file '%s'\n", filename);
  int fd = open(filename, O_CREAT | O_WRONLY, (mode_t)0700);
  if (fd == -1) {
    fprintf(stderr, "Failed to open '%s': %s\n", filename, strerror(errno));
    return EXIT_FAILURE;
  }

  printf("Writing '%s' to file '%s'\n", content, filename);
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

  printf("Getting inode flags for file '%s'\n", filename);
  int attr;
  int ret = ioctl(fd, FS_IOC_GETFLAGS, &attr);
  if (ret < 0) {
    fprintf(stderr, "Failed to get inode flags for file '%s': %s\n", filename,
            strerror(errno));
    return EXIT_FAILURE;
  }

  printf("Setting immutable flag for file '%s'\n", filename);
  attr |= FS_IMMUTABLE_FL;

  printf("Updating inode flags for file '%s'\n", filename);
  ret = ioctl(fd, FS_IOC_SETFLAGS, &attr);
  if (ret < 0) {
    fprintf(stderr, "Failed to get inode flags for file '%s': %s", filename,
            strerror(errno));
    return EXIT_FAILURE;
  }

  close(fd);
  return EXIT_SUCCESS;
}
