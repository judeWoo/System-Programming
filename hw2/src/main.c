#include "debug.h"
#include "wrappers.h"
#include <stdlib.h>

int
main(int argc, char *argv[])
{
  int infile, outfile, in_flags, out_flags;
  int parse_args_ret, check_bom_ret, print_state_ret, get_encoding_function_ret;
  parse_args_ret = parse_args(argc, argv);
  check_bom_ret = check_bom();
  print_state_ret = print_state();
  in_flags = O_RDONLY;
  out_flags = O_WRONLY | O_CREAT | O_TRUNC;
  infile = Open(program_state->in_file, in_flags);
  outfile = Open(program_state->out_file, out_flags);
  lseek(infile, program_state->bom_length, SEEK_SET); /* Discard BOM */
  //lseek() function allows the file offset to be set beyond the end of the file (but this does not change the size of the file).
  get_encoding_function_ret = get_encoding_function()(infile, outfile); //return function pointer and assign parameters
  if(program_state != NULL) {
    // close((int)program_state);
    free(program_state);
  }
  close(outfile);
  close(infile);
  if ((parse_args_ret == 1) | (check_bom_ret == 1) | (print_state_ret == 1) | (get_encoding_function_ret == -1))
  {
    return EXIT_FAILURE;
  }
  return EXIT_SUCCESS;
}
