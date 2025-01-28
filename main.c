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

  printf("Writing '%s' to file\n", content);
  {
    size_t n_bytes = 0;
    while (n_bytes < sizeof(content) - 1) {
      ssize_t ret = write(fd, content + n_bytes,
                          (sizeof(content) - 1 /* null-byte */) - n_bytes);
      if (ret < 0) {
        fprintf(stderr, "Failed to write '%s' to file: %s\n", content,
                strerror(errno));
        return EXIT_FAILURE;
      }
      n_bytes += (size_t)ret;
    }
  }

  printf("Getting inode flags for file\n");
  int attr;
  {
    int ret = ioctl(fd, FS_IOC_GETFLAGS, &attr);
    if (ret < 0) {
      fprintf(stderr, "Failed to get inode flags for file: %s\n",
              strerror(errno));
      return EXIT_FAILURE;
    }
  }

  printf("Setting immutable flag\n");
  attr |= FS_IMMUTABLE_FL;

  printf("Updating inode flags for file\n");
  {
    int ret = ioctl(fd, FS_IOC_SETFLAGS, &attr);
    if (ret < 0) {
      fprintf(stderr, "Failed to get inode flags for file: %s",
              strerror(errno));
      return EXIT_FAILURE;
    }
  }

  printf("Opening file '%s' in read only mode\n", filename);
  fd = open(filename, O_RDONLY);
  if (fd == -1) {
    fprintf(stderr, "Failed to open '%s': %s\n", filename, strerror(errno));
    return EXIT_FAILURE;
  }

  char buffer[1024];
  {
    size_t n_bytes = 0;
    ssize_t ret;
    do {
      ret = read(fd, buffer + n_bytes, sizeof(buffer) - n_bytes);
      if (ret < 0) {
        fprintf(stderr, "Failed to read file '%s': %s\n", filename,
                strerror(errno));
        return EXIT_FAILURE;
      }
      n_bytes += (size_t)ret;
    } while (ret > 0);
    buffer[n_bytes] = '\0'; /* Add null-byte */
  }
  printf("Read '%s' from file\n", content);

  close(fd);

  printf("Opening file '%s' in read write mode\n", filename);
  fd = open(filename, O_WRONLY);
  if (fd == -1) {
    printf("Failed to open '%s': %s\n", filename, strerror(errno));
  } else {
    printf("Successfully opened file '%s' in write only mode...\n", filename);
    close(fd);
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}
