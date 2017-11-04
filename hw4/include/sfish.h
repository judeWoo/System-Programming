#ifndef SFISH_H
#define SFISH_H

/* Format Strings */
#define EXEC_NOT_FOUND "sfish: %s: command not found\n"
#define JOBS_LIST_ITEM "[%d] %s\n"
#define STRFTIME_RPRMT "%a %b %e, %I:%M%p"
#define BUILTIN_ERROR  "sfish builtin error: %s\n"
#define SYNTAX_ERROR   "sfish syntax error: %s\n"
#define EXEC_ERROR     "sfish exec error: %s\n"
#define MAX_COMMAND 1024
#define MAX_TOKEN MAX_COMMAND
#define MAX_INPUT MAX_COMMAND
#define MAX_OUTPUT MAX_COMMAND

char *my_getcwd(void);
char *init_cwd(char *home, char *cwd);
int syntax_checker(char *input);
void prepend(char* s, const char* t);
void execute(char *input, char **file_array);
void fillbuf(char *input[], char *outfile[], char *infile[], int inputc, int outfilec, int infilec);
void emptybuf(int inputc, int outfilec, int infilec);
void piped(int inputc, int outfilec, int infilec);
void parse(char *home, char *cwd, char *input, int inputc, char *inputv[]);
void tokenized(char *input, int *input_bufc, int *outfile_bufc, int *infile_bufc);

#endif
