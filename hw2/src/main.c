#include "debug.h"
#include "wrappers.h"
#include <stdlib.h>

int
main(int argc, char *argv[])
{
  int infile, outfile, in_flags, out_flags;
  int get_encoding_function_ret;
  struct stat infile_stat;
  struct stat outfile_stat;

  parse_args(argc, argv);
  check_bom();
  print_state();
  in_flags = O_RDONLY;
  out_flags = O_WRONLY | O_CREAT;
  infile = Open(program_state->in_file, in_flags);
  outfile = Open(program_state->out_file, out_flags);

  if (fstat(infile, &infile_stat) < 0 || fstat(outfile, &outfile_stat) < 0)
  {
    if(program_state != NULL) {
      free(program_state);
    }
    exit(EXIT_FAILURE);
  }
  if (infile_stat.st_ino == outfile_stat.st_ino)
  {
    if(program_state != NULL) {
      free(program_state);
    }
    exit(EXIT_FAILURE);
  }
  if ((lseek(infile, program_state->bom_length, SEEK_SET) < 0))
  {
    if(program_state != NULL) {
      free(program_state);
    }
    exit(EXIT_FAILURE);
  }

  get_encoding_function_ret = get_encoding_function()(infile, outfile); //return function pointer and assign parameters

  if(program_state != NULL) {
    free(program_state);
  }
  if (get_encoding_function_ret < 0)
  {
    return EXIT_FAILURE;
  }

  close(outfile);
  close(infile);

  return EXIT_SUCCESS;
}
