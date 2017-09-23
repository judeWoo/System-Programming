#include "debug.h"
#include "utf.h"
#include "wrappers.h"
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int opterr;
int optopt;
int optind;
char *optarg;

state_t *program_state;

int
parse_args(int argc, char *argv[])
{
  int i;
  int opt_flag = 0;
  char option;
  char *joined_argv;

  //check for valid argument
  if (argc > 6)
  {
    USAGE(argv[0]);
    exit(EXIT_FAILURE); //TODO
  }

  joined_argv = join_string_array(argc, argv); //joining command+argument
  info("argc: %d argv: %s", argc, joined_argv);
  free(joined_argv); //free joined_argv pointer, which means it no longer points at sth

  program_state = Calloc(1, sizeof(state_t)); //save space for a pointer that points 1 element of state_t size and assign to program_state
  while ((option = getopt(argc, argv, "he:")) != -1) { //int getopt (int argc, char *const *argv, const char *options) when return -1, sets optind to first element of the argument that is not option
    switch (option) {
      case 'e': {
        opt_flag = 1;
        info("Encoding Argument: %s", optarg);
          //optarg points at the value of the option argument, for those options that accept arguments.
          if ((program_state->encoding_to = determine_format(optarg)) == 0) //assign the value of encdoing_to with format_t
            print_state(); //print error msg if user-typed encoding format is wrong
          break;
        }
        case '?': {
          opt_flag = 1;
          if (optopt != 'h')
          {
            fprintf(stderr, KRED "-%c is not a supported argument\n" KNRM, optopt);
            USAGE(argv[0]);
            exit(EXIT_FAILURE);
          }
        // case "errorcase"[0]:
        //   USAGE(argv[0]);
        //   exit(0);
        }
        case 'h': {
          opt_flag = 1;
          USAGE(argv[0]);
          exit(0);
        }
        default: {
          opt_flag = 1;
          break;
        }
      }
    }
    if (opt_flag == 0)
    {
      USAGE(argv[0]);
      exit(EXIT_FAILURE);
    }
  for (i = 0; optind < argc; ++i) {
    debug("%d opterr: %d", i, opterr); // opterr's value not zero, prints error
    debug("%d optind: %d", i, optind); // the index of the next element of the argv array. 1 is set for init
    debug("%d optopt: %d", i, optopt); // stores an unknown option character || an option with a missing required argument
    debug("%d argv[optind]: %s", i, argv[optind]);
    //invalid case: the argument of the length 5 without -h
    if(optind == 5)
    {
      USAGE(argv[0]);
      exit(EXIT_FAILURE);
    }
    else
    {
      if (program_state->in_file == NULL) {
        program_state->in_file = argv[optind];
      }
      else if(program_state->out_file == NULL)
      {
        program_state->out_file = argv[optind];
      }
      optind++;
    }
    //TODO EDGE case: bin/utf +sdasd...
  }
  // free(joined_argv);
  return 0;
}

format_t
determine_format(char *argument)
{
  if (strcmp(argument, STR_UTF16LE) == 0)
    return UTF16LE;
  if (strcmp(argument, STR_UTF16BE) == 0)
    return UTF16BE;
  if (strcmp(argument, STR_UTF8) == 0)
    return UTF8;
  return 0;
}

const char*
bom_to_string(format_t bom){
  switch(bom){
    case UTF8: return STR_UTF8;
    case UTF16BE: return STR_UTF16BE;
    case UTF16LE: return STR_UTF16LE;
  }
  return "UNKNOWN";
}

char*
join_string_array(int count, char *array[])
{ //count == argc, array == argv;
  char *ret;
  // char charArray[count];
  int i;
  int len = 0, str_len, cur_str_len;
  // TODO
  str_len = array_size(count, array); // the length of joint string of command+argument
  ret = (char *) malloc(str_len * sizeof(char)); // check this it might be wrong

  for (i = 0; i < count; ++i) {
    cur_str_len = strlen(array[i]);
    memecpy(ret + len, array[i], cur_str_len);
    len += cur_str_len;
    memecpy(ret + len, " ", 1);
    len += 1;
  }

  return ret;
}

int
array_size(int count, char *array[])
{ //count == argc, array == argv;
  int i, sum = 1; /* NULL terminator */
  for (i = 0; i < count; ++i) {
    sum += strlen(array[i]); //array[i] == "-h", array[i+1] == "-p" ...
    ++sum; /* For the spaces */
  }
  return sum-1;
}

int
print_state()
{
// errorcase:
  if (program_state->encoding_to == 0) {
    error("program_state is %p", (void*)program_state);
    exit(EXIT_FAILURE);
  }
  info("program_state {\n"
         "  format_t encoding_to = 0x%X;\n"
         "  format_t encoding_from = 0x%X;\n"
         "  char *in_file = '%s';\n"
         "  char *out_file = '%s';\n"
         "};\n",
         program_state->encoding_to, program_state->encoding_from,
         program_state->in_file, program_state->out_file);
  return 0;
}
