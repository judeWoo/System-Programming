#include "wrappers.h"
#include "debug.h"

void*
Malloc(size_t size)
{
  void* ret;
  if ((ret = malloc(size)) == NULL) {
    perror("Out of Memory");
    if(program_state != NULL) {
      free(program_state);
    }
    exit(EXIT_FAILURE);
  }
  return ret;
}

void*
Calloc(size_t nmemb, size_t size)
{
  void* ret;
  if ((ret = calloc(nmemb, size)) == NULL) {
    perror("Out of Memory");
    if(program_state != NULL) {
      free(program_state);
    }
    exit(EXIT_FAILURE);
  }
  return ret;
}

int
Open(char const* pathname, int flags)
{
  int fd;
  if ((fd = open(pathname, flags, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH)) < 0) {
    perror("Could Not Open file");
    fprintf(stderr, "%s\n", pathname);
    if(program_state != NULL) {
      free(program_state);
    }
    exit(EXIT_FAILURE);
  }
  return fd;
}

ssize_t
read_to_bigendian(int fd, void* buf, size_t count)
{
  ssize_t bytes_read;

  bytes_read = read(fd, buf, count);
/*#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
  reverse_bytes(buf, count);
#endif*/
  return bytes_read;
}

ssize_t
write_to_bigendian(int fd, void* buf, size_t count)
{
  ssize_t bytes_read;

// #if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
//   reverse_bytes(buf, count);
// #endif
  bytes_read = write(fd, buf, count);
  return bytes_read;
}

void
reverse_bytes(void* bufp, size_t count)
{
  char* ptr = bufp;
  char temp;
  int i, j;
  for (i = (count - 1), j = 0; j < i; --i, ++j/*, temp=~temp*/) {
    temp = ptr[i];
    ptr[i] = ptr[j];
    ptr[j] = temp;
  }
}

void
*memeset(void *s, int c, size_t n) {
  //TODO
  // register char* stackpointer asm("esp"); //initialize stackpointer pointer with the value of the actual stackpointer
  memset(s, c, n); //From the address pointed by s, this fills n bytes with value of c and returns s
  return s;
};

void
*memecpy(void *dest, const void *src, size_t n) {
  //TODO
  //register char* stackpointer asm("esp"); //initialize stackpointer pointer with the value of the actual stackpointer
  memcpy(dest, src, n); //From the address pointed by s, this fills n bytes with value that is pointed by src and returns s
  return dest;
};
