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
int syntax_checker_helper(char *input);
int builtin_execvp(char *home, char *cwd, char **command);
void prepend(char* s, const char* t);
void execute(char *home, char *cwd, char **command, int in, int out, int outfile_bufc, int infile_bufc);
void parse(char *home, char *cwd, int input_bufc, int outfile_bufc, int infile_bufc);
void fillbuf(char *input[], char *outfile[], char *infile[], int input_bufc, int outfile_bufc, int infile_bufc);
void emptybuf(int input_bufc, int outfile_bufc, int infile_bufc);
void tokenized(char *input, int *input_bufc, int *outfile_bufc, int *infile_bufc);
void signal_handle(int signal);

#endif
