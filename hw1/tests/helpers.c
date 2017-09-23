#include "__helpers.h"

typedef unsigned char byte;
#define MSB 0x80
char *binary_string(void* number, size_t size) {
    int i;
    int bits;
    char bit;
    char* bit_string;
    byte* copied_number_space;
    byte* tmp_number_ptr;

    bits = size * 8;
    bit_string = malloc(bits);
    bit_string[bits] = '\0';

    copied_number_space = malloc(size);
    memcpy(copied_number_space, number, size);
    tmp_number_ptr = copied_number_space + (size-1);

    for (i = 0;i < bits; ++i, *tmp_number_ptr<<=1) {

        if(i % 8 == 0 && (i != bits && i != 0))
            tmp_number_ptr--;

        bit = (*tmp_number_ptr & MSB) == 0 ? '0' : '1';
        bit_string[i] = bit;
    }
    free(copied_number_space);

    return bit_string;
}
#undef byte
#undef MSB

char *contents_of(const char *file_name) {
    size_t buf_size = 10000;
    char *buf = calloc(buf_size, 1);
    size_t i;

    errno = 0;
    FILE *file = fopen(file_name, "r");
    if(errno) {
	   perror("contents_of::fopen:");
	   exit(EXIT_FAILURE);
    }

    errno = 0;
    for(i = 0; i < buf_size && (buf[i] = fgetc(file)) != EOF; i++, errno = 0) {
	   if(errno) {
	       perror("contents_of::fgetc:");
	       exit(EXIT_FAILURE);
	   }
    }
    buf[i] = 0;
    return buf;
}
