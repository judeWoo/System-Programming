#include "utf.h"
#include "debug.h"
#include "wrappers.h"
#include <unistd.h>

int
from_utf16be_to_utf16le(int infile, int outfile)
{
  int ret = -1;
  int bom;
  utf16_glyph_t buf;
  ssize_t bytes_read;
  ssize_t bytes_to_write;

  bom = UTF16LE;
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
  reverse_bytes(&bom, 2);
#endif
  write_to_bigendian(outfile, &bom, 2);

  while ((bytes_read = read_to_bigendian(infile, &(buf.upper_bytes), 2)) > 0) {
  #if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
    reverse_bytes(&buf.upper_bytes, 2);
  #endif
    bytes_to_write = 2; /* utf-32 future compatibility, make it 4 if possible */
    if(is_upper_surrogate_pair(buf)) {
      if((bytes_read = read_to_bigendian(infile, &buf.lower_bytes, 2) <= 0)) {
        break;
      }
    #if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
      reverse_bytes(&(buf.lower_bytes), 2);
    #endif
      bytes_to_write += 2;
    }
    write_to_bigendian(outfile, &buf, bytes_to_write);
  }
  ret = bytes_read;
  return ret;
}



int
from_utf16be_to_utf8(int infile, int outfile)
{
  int ret = -1;
  int bom;
  utf16_glyph_t buf;
  ssize_t bytes_read;
  size_t bytes_to_write;
  size_t size_of_glyph;
  code_point_t code_point;
  utf8_glyph_t utf8_buf;

  bom = UTF8;

#if __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
  reverse_bytes(&bom, 3);
#endif
  write_to_bigendian(outfile, &bom, 3); //write bom inside outfile

  while ((bytes_read = read_to_bigendian(infile, &(buf.upper_bytes), 2)) > 0) {
  #if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
    reverse_bytes(&(buf.upper_bytes), 2);
  #endif
    bytes_to_write = 2;
    if (buf.upper_bytes > 0xDBFF)
    {
      fprintf(stderr, KRED "UNKNOWN CHARACTER\n" KNRM);
      exit(EXIT_FAILURE);
    }
    if(is_upper_surrogate_pair(buf)) {
      if((bytes_read = read_to_bigendian(infile, &(buf.lower_bytes), 2)) <= 0) {
        if (is_upper_surrogate_pair(buf) && !is_lower_surrogate_pair(buf))
        {
          fprintf(stderr, KRED "UNKNOWN CHARACTER\n" KNRM);
          exit(EXIT_FAILURE);
        }
        break;
      }
    #if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
      reverse_bytes(&(buf.lower_bytes), 2);
    #endif
      bytes_to_write += 2;
    }
    code_point = utf16_glyph_to_code_point(buf);
    utf8_buf = code_point_to_utf8_glyph(code_point, &size_of_glyph);
    write_to_bigendian(outfile, &utf8_buf, size_of_glyph);
  }
  ret = bytes_read;
  return ret;
}

utf16_glyph_t
code_point_to_utf16be_glyph(code_point_t code_point, size_t *size_of_glyph)
{
  utf16_glyph_t ret;

  memeset(&ret, 0, sizeof ret);
  if(is_code_point_surrogate(code_point)) {
    code_point -= 0x10000;
    ret.upper_bytes = (code_point >> 10) + 0xD800;
    ret.lower_bytes = (code_point & 0x3FF) + 0xDC00;
  #if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
    reverse_bytes(&ret.upper_bytes, 2);
    reverse_bytes(&ret.lower_bytes, 2);
  #endif
    *size_of_glyph = 4;
  }
  else {
    ret.upper_bytes |= code_point;
  #if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
    reverse_bytes(&ret.upper_bytes, 2);
  #endif
    *size_of_glyph = 2;
  }
  return ret;
}
