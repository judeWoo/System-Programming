#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <readline/readline.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/wait.h>

#include "sfish.h"
#include "debug.h"

#define MAX_COMMAND 1024
#define MAX_TOKEN MAX_COMMAND
#define MAX_INPUT MAX_COMMAND
#define MAX_OUTPUT MAX_COMMAND

char *inputv[MAX_TOKEN]; //tokenized input

int main(int argc, char *argv[], char *envp[]) {
    int inputc; //tokenized input size
    char *cwd = NULL; //current working dir
    char *input = NULL; //input
    char *home = NULL; //home dir

    if (!isatty(STDIN_FILENO)) //if from file
    {
        // If your shell is reading from a piped file
        // Don't have readline then write anything to that file.
        // Such as the prompt or "user input"
        if ((rl_outstream = fopen("/dev/null", "w")) == NULL){ //making rl_outstream points at nothing
            perror("Failed trying to open DEVNULL");
            exit(EXIT_FAILURE);
        }
    }

    // init home
    if ((home = getenv("HOME")) == NULL)
    {
        perror("Home directory not set"); //TODO
        exit(EXIT_FAILURE);
    }

    // init OLDPWD
    setenv("OLDPWD", "", 1);

    do
    {
        //init inputc
        inputc = 0;
        //init cwd
        cwd = init_cwd(home, cwd);
        //init input
        input = readline(cwd);
        //init to parse
        free(cwd);

        write(1, "\e[s", strlen("\e[s")); //TODO
        write(1, "\e[20;10H", strlen("\e[20;10H"));
        write(1, "SomeText", strlen("SomeText"));
        write(1, "\e[u", strlen("\e[u"));

        // If EOF is read (aka ^D) readline returns NULL
        if (input == NULL)
        {
            free(cwd);
            continue;
        }

        //check ||

        //tokenize
        tokenized(input, &inputc);

        //parse
        // parse(home, cwd, input, inputc, inputv);

        //free
        rl_free(input);

    } while (1);

    return EXIT_SUCCESS;
}

void tokenized(char *input, int *inputc)
{
    char *chop_a[MAX_COMMAND];
    char *chop_b[MAX_TOKEN];
    char *chop_c[MAX_TOKEN];
    char *chop_d[MAX_TOKEN];

    char *out[MAX_OUTPUT];
    char *out_f[MAX_OUTPUT];
    char *in[MAX_INPUT];
    char *in_f[MAX_OUTPUT];

    char *tmp[1];

    int ac = 0;
    int bc = 0;
    int cc = 0;
    int dc = 0;

    int outc = 0;
    int inc = 0;
    int outfc = 0;
    int infc = 0;

    int flag = 0; //for skipping in&out files

    tmp[0] = input;
    chop_a[ac] = strtok_r(input, "|", &input); //chopping input with 'pipe'
    if (chop_a[ac] == NULL) //return null only when token has only delimiter or NULL
    {
        printf(SYNTAX_ERROR, tmp[0]);
        return;
    }
    while (chop_a[ac] != NULL)
    {
        ac++;
        chop_a[ac] = strtok_r(NULL, "|", &input);
        if (chop_a[ac] == NULL)
        {
            break;
        }
    }

    for (int i = 0; i < ac; ++i) //chopping 'pipe-chopped' input with 'out'
    {
        tmp[0] = chop_a[i];
        chop_b[bc] = strtok_r(chop_a[i], ">", &chop_a[i]);
        if (chop_b[bc] == NULL)
        {
            printf(SYNTAX_ERROR, tmp[0]);
            return;
        }
        while (chop_b[bc] != NULL)
        {
            bc++;
            chop_b[bc] = strtok_r(NULL, ">", &chop_a[i]);
            if (chop_b[bc] == NULL)
            {
                bc++; //inserting NULL between commands
                break;
            }
        }
    }

    for (int i = 0; i < bc; ++i) //chopping 'pipe-out-chopped' input with 'in'
    {
        if (chop_b[i] == NULL)
        {
            chop_c[cc] = NULL;
            cc++;
            continue;
        }
        tmp[0] = chop_b[i];
        chop_c[cc] = strtok_r(chop_b[i], "<", &chop_b[i]);
        if (chop_c[cc] == NULL)
        {
            printf(SYNTAX_ERROR, tmp[0]);
            return;
        }
        if ((cc > 0) && (chop_c[cc - 1] != NULL))
        {
            out[outc] = chop_c[cc]; //if token appeared after prog+arg
            outc++;
        }
        while (chop_c[cc] != NULL)
        {
            cc++;
            chop_c[cc] = strtok_r(NULL, "<", &chop_b[i]);
            if (chop_c[cc] == NULL)
            {
                break;
            }
            in[inc] = chop_c[cc]; //if token appeared after 'in'
            inc++;
        }
    }

    for (int i = 0; i < cc; ++i) //chopping 'pipe-out-in-chopped' input with 'whitespace'
    {
        //if token is not null & not first one & not the very next one to NUll one (prog+arg)
        if ((chop_c[i] != NULL) && (i > 0) && (flag == 0))
        {
            continue;
        }
        if (chop_c[i] == NULL)
        {
            chop_d[dc] = NULL; //keep NULL ones for seperating between commands
            dc++;
            flag = 1;
            continue;
        }
        chop_d[dc] = strtok_r(chop_c[i], " \t\r\v\f\n", &chop_c[i]);
        if (chop_d[dc] == NULL)
        {
            printf(SYNTAX_ERROR, "Only whitespace");
            return;
        }
        while (chop_d[dc] != NULL)
        {
            dc++;
            chop_d[dc] = strtok_r(NULL, " \t\r\v\f\n", &chop_c[i]);
            if (chop_d[dc] == NULL)
            {
                break;
            }
        }
        flag = 0; //indicate next one will not be prog+arg
    }

    for (int i = 0; i < outc; ++i) //chop all the whitespaces and save only very last token
    {
        out_f[outfc] = strtok_r(out[i], " \t\r\v\f\n", &out[i]);
        if (out_f[outfc] == NULL)
        {
            printf(SYNTAX_ERROR, "file name error");
            return;
        }
        while (out_f[outfc] != NULL)
        {
            char *temp = strtok_r(NULL, " \t\r\v\f\n", &out[i]);
            if (temp == NULL)
            {
                outfc++;
                out_f[outfc] = NULL;
                break;
            }
            else
            {
                out_f[outfc] = temp;
            }
        }
    }

    for (int i = 0; i < inc; ++i) //chop all the whitespaces and save only very last token
    {
        in_f[infc] = strtok_r(in[i], " \t\r\v\f\n", &in[i]);
        if (in_f[infc] == NULL)
        {
            printf(SYNTAX_ERROR, "file name error");
            return;
        }
        while (in_f[infc] != NULL)
        {
            char *temp = strtok_r(NULL, " \t\r\v\f\n", &in[i]);
            if (temp == NULL)
            {
                infc++;
                in_f[infc] = NULL;
                break;
            }
            else
            {
                in_f[infc] = temp;
            }
        }
    }

    for (int i = 0; i < dc; ++i)
    {
        debug("chop_d[%d] has now: %s", i, chop_d[i]);
    }

    for (int i = 0; i < outfc; ++i)
    {
        debug("Out[%d] has now: %s", i, out_f[i]);
    }

    for (int i = 0; i < infc; ++i)
    {
        debug("In[%d] has now: %s", i, in_f[i]);
    }

    return;
}

void parse(char *home, char *cwd, char *input, int inputc, char **inputv)
{
    struct stat sb;

    if (*(inputv) == NULL || inputc <= 0)
    {
        goto end;
    }
    //buit-in
    if (strcmp(*(inputv), "help") == 0) //help
        {
            printf("help - print a list of all builtins and their basic usage in a single column (CSE320 da best)\n"
                    "exit - exits the shell\n"
                    "cd - changes the current working directory of the shell\n"
                    "pwd - prints the absolute path of the current working directory\n");
            goto end;
        }

        if (strcmp(*(inputv), "exit") == 0) //exit
        {
            exit(EXIT_SUCCESS);
        }

        if (strcmp(*(inputv), "pwd") == 0) //pwd
        {
            if ((cwd = my_getcwd()) == 0)
            {
                printf(BUILTIN_ERROR, input);
                goto end;
            }

            printf("%s\n", cwd);
            goto free;
        }

        if (strcmp(*(inputv), "cd") == 0) //cd to home
        {
            if (inputc == 1)
            {
                if (setenv("OLDPWD", getenv("PWD"), 1) == -1)
                {
                    printf(BUILTIN_ERROR, input);
                    goto end;
                }

                if (chdir(home) == -1)
                {
                    printf(BUILTIN_ERROR, input);
                    goto end;
                }

                cwd = my_getcwd(); //setting PWD
                setenv("PWD", cwd, 1);

                goto free;
            }

            else
            {
                if (strcmp(*(inputv + 1), "-") == 0) //cd to last dir
                {
                    if (chdir(getenv("OLDPWD")) == -1)
                    {
                        printf(BUILTIN_ERROR, input);
                        goto end;
                    }

                    cwd = my_getcwd(); //setting PWD
                    setenv("PWD", cwd, 1);

                    goto free;
                }

                else if (strcmp(*(inputv + 1), ".") == 0) //cd to current dir
                {
                    if (setenv("OLDPWD", getenv("PWD"), 1) == -1)
                    {
                        printf(BUILTIN_ERROR, input);
                        goto end;
                    }

                    if (chdir(".") == -1)
                    {
                        printf(BUILTIN_ERROR, input);
                        goto end;
                    }

                    cwd = my_getcwd(); //setting PWD
                    setenv("PWD", cwd, 1);

                    goto free;
                }

                else if (strcmp(*(inputv + 1), "..") == 0) //cd to previous dir
                {
                    if (setenv("OLDPWD", getenv("PWD"), 1) == -1)
                    {
                        printf(BUILTIN_ERROR, input);
                        goto end;
                    }

                    if (chdir("..") == -1)
                    {
                        printf(BUILTIN_ERROR, input);
                        goto end;
                    }

                    cwd = my_getcwd(); //setting PWD
                    setenv("PWD", cwd, 1);

                    goto free;
                }

                else //cd to path
                {
                    if (setenv("OLDPWD", getenv("PWD"), 1) == -1)
                    {
                        printf(BUILTIN_ERROR, input);
                        goto end;
                    }

                    if (chdir(*(inputv + 1)) == -1)
                    {
                        printf(BUILTIN_ERROR, input);
                        goto end;
                    }

                    cwd = my_getcwd(); //setting PWD
                    setenv("PWD", cwd, 1);

                    goto free;
                }
            }
        }
        //execute
        else
        {
            if (strstr((*inputv), "/") != NULL)
            {
                if (stat(*(inputv), &sb) == -1)
                {
                    printf(EXEC_ERROR, input);
                    exit(EXIT_FAILURE);
                }

                else
                {
                    execute(input, inputv);
                    goto end; //TODO
                }
            }

            else
            {
                execute(input, inputv);
                goto end;
            }
        }

        free:
        free(cwd);

        end:
        return;
}

char *init_cwd(char *home, char *cwd)
{
    // check getcwd()
    if ((cwd = my_getcwd()) == 0)
    {
        perror("getcwd failed"); //TODO
        exit(EXIT_FAILURE);
    }

    // set PWD
    setenv("PWD", cwd, 1);

    //change home dir to ~
    if (strncmp(cwd, home, strlen(home)) == 0)
    {
        if (strcpy(cwd, cwd + strlen(home)) == NULL)
        {
            perror("strcpy failed"); //TODO
            exit(EXIT_FAILURE);
        }
        prepend(cwd, "~");
    }

    // realloc cwd: +1 is for empty char
    cwd = (char *) realloc(cwd, strlen(cwd) + strlen(" :: howoo >> ") + 1);
    if (cwd == NULL)
    {
        perror("realloc failed"); //TODO
        exit(EXIT_FAILURE);
    }

    // concate cwd
    cwd = strcat(cwd, " :: howoo >> ");
    if (cwd == NULL)
    {
        perror("strcat failed");
        exit(EXIT_FAILURE);
    }

    return cwd;
}

void execute(char *input, char **file_array)
{
    pid_t child_pid;
    pid_t tpid;
    int child_status;

    if ((child_pid = fork()) == -1) //fork
    {
        printf(EXEC_ERROR, input);
        exit(EXIT_FAILURE);
    }

    if ((int) child_pid == 0) //child
    {

        if (execvp(*file_array, file_array) == -1) //overide child process
        {
            printf(EXEC_NOT_FOUND, input);
            exit(EXIT_FAILURE);
        }
        exit(EXIT_SUCCESS);
    }

    else //parent
    {
        do
        {
            tpid = wait(&child_status); //wait for child
            if (tpid != child_pid) //if parent has more than one child
            {
                // kill(tpid);
            }
        } while (tpid != child_pid);
    }

    return;
}

void prepend(char* s, const char* t)
{
    if (memmove(s + strlen(t), s, strlen(s) + 1) == NULL) //+1 for empty char
    {
        perror("memmove failed");
        exit(EXIT_FAILURE);
    }
    if (memcpy(s, t, strlen(t)) == NULL) //do not copy empty char of t
    {
        perror("memcpy failed");
        exit(EXIT_FAILURE);
    }

    return;
}

char *my_getcwd()
{
    size_t size = 100;
    char *buffer = NULL;

    while (1)
    {
        buffer = (char *) malloc(size);

        if (buffer == NULL)
        {
            perror("malloc failed");
            exit(EXIT_FAILURE);
        }

        if (getcwd(buffer, size) != NULL)
        {
            return buffer;
        }

        else
        {
            free(buffer);

            if (errno != ERANGE) //If error is not about the range
            {
                return 0;
            }

            size *= 2;
        }
    }

    return buffer;
}
