#ifndef SFISH_H
#define SFISH_H

/* Format Strings */
#define EXEC_NOT_FOUND "sfish: %s: command not found\n"
#define JOBS_LIST_ITEM "[%d] %s\n"
#define STRFTIME_RPRMT "%a %b %e, %I:%M%p"
#define BUILTIN_ERROR  "sfish builtin error: %s\n"
#define SYNTAX_ERROR   "sfish syntax error: %s\n"
#define EXEC_ERROR     "sfish exec error: %s\n"

char *my_getcwd(void);
void prepend(char* s, const char* t);
// void my_system(char *path, char *input)
char *init(char *home, char *cwd);
void parse(char *home, char *cwd, char *input, int inputc, char **inputv);
char **tokenize(char *inputcpy, int *inputc);
void execute(char *home, char *cwd, char *input, int inputc, char **inputv);


#endif
