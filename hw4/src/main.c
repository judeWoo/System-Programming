#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <readline/readline.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <fcntl.h>

#include "sfish.h"
#include "debug.h"

char *input_buf[MAX_TOKEN];
char *outfile_buf[MAX_OUTPUT];
char *infile_buf[MAX_INPUT];

int child_kill;

int main(int argc, char *argv[], char *envp[]) {
    int input_bufc; //tokenized input buf size
    int outfile_bufc; //output file buf size, also indicates # of commands
    int infile_bufc; //input file buf size, also indicates # of commands
    char *cwd = NULL; //current working dir
    char *input = NULL; //input
    char *home = NULL; //home dir
    char *input_holder[1]; //temporary input holder

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
        perror("Home directory not set");
        exit(EXIT_FAILURE);
    }

    // init OLDPWD
    setenv("OLDPWD", "", 1);

    do
    {
        //init sizes
        input_bufc = outfile_bufc = infile_bufc = child_kill = 0;
        //init cwd
        cwd = init_cwd(home, cwd);
        //init input
        input = readline(cwd);
        //init holder
        input_holder[0] = input;
        //init to parse
        free(cwd);

        write(1, "\e[s", strlen("\e[s")); //TODO
        write(1, "\e[20;10H", strlen("\e[20;10H"));
        write(1, "SomeText", strlen("SomeText"));
        write(1, "\e[u", strlen("\e[u"));

        // If EOF is read (aka ^D) readline returns NULL
        if (input == NULL)
        {
            continue;
        }

        if (syntax_checker(input) == 1) //TODO
        {
            goto free;
        }

        //tokenize
        tokenized(input, &input_bufc, &outfile_bufc, &infile_bufc);

        //parse
        parse(home, cwd, input_holder[0], input_bufc, outfile_bufc, infile_bufc);

        //empty buf
        emptybuf(input_bufc, outfile_bufc, infile_bufc);

        free:
        rl_free(input);

    } while (1);

    return EXIT_SUCCESS;
}

void tokenized(char *input, int *input_bufc, int *outfile_bufc, int *infile_bufc)
{
    char *chop_a[MAX_COMMAND];
    char *chop_b[MAX_TOKEN];
    char *chop_c[MAX_TOKEN];
    char *chop_d[MAX_TOKEN];

    char *out[MAX_OUTPUT];
    char *out_f[MAX_OUTPUT];
    char *in[MAX_INPUT];
    char *in_f[MAX_OUTPUT];

    int ac = 0;
    int bc = 0;
    int cc = 0;
    int dc = 0;

    int outc = 0;
    int inc = 0;
    int outfc = 0;
    int infc = 0;

    int flag = 0; //for skipping in&out files
    int out_flag = 0; //for adding null to output file buf
    int in_flag = 0; //for adding null to input file buf

    chop_a[ac] = strtok_r(input, "|", &input); //chopping input with 'pipe'
    if (chop_a[ac] == NULL) //return null only when token has only delimiter or NULL
    {
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
        chop_b[bc] = strtok_r(chop_a[i], ">", &chop_a[i]);
        if (chop_b[bc] == NULL)
        {
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
            if (out_flag == 0)
            {
                out[outc] = NULL; //if token appeared after prog+arg
                outc++;
            }
            if (in_flag == 0)
            {
                in[inc] = NULL; //if token appeared after prog+arg
                inc++;
            }
            out_flag = 0;
            in_flag = 0;
            continue;
        }

        chop_c[cc] = strtok_r(chop_b[i], "<", &chop_b[i]);
        if (chop_c[cc] == NULL)
        {
            return;
        }
        if ((cc > 0) && (chop_c[cc - 1] != NULL))
        {
            out[outc] = chop_c[cc]; //if token appeared after prog+arg
            outc++;
            out_flag = 1;
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
            in_flag = 1;
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
            outfc++;
            continue;
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
            infc++;
            continue;
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
    //update sizes
    *input_bufc = dc;
    *outfile_bufc = outfc;
    *infile_bufc = infc;

    //save token/out/in to global buffers
    fillbuf(chop_d, out_f, in_f, dc, outfc, infc);

    return;
}

void emptybuf(int input_bufc, int outfile_bufc, int infile_bufc)
{
    for (int i = 0; i < input_bufc; ++i)
    {
        input_buf[i] = NULL;
    }

    for (int i = 0; i < outfile_bufc; ++i)
    {
        outfile_buf[i] = NULL;
    }

    for (int i = 0; i < infile_bufc; ++i)
    {
        infile_buf[i] = NULL;
    }

    return;
}

void fillbuf(char *input[], char *outfile[], char *infile[], int input_bufc, int outfile_bufc, int infile_bufc)
{
    for (int i = 0; i < input_bufc; ++i)
    {
        input_buf[i] = input[i];
    }

    for (int i = 0; i < outfile_bufc; ++i)
    {
        outfile_buf[i] = outfile[i];
    }

    for (int i = 0; i < infile_bufc; ++i)
    {
        infile_buf[i] = infile[i];
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

void parse(char *home, char *cwd, char *input, int input_bufc, int outfile_bufc, int infile_bufc)
{
    char *command[MAX_TOKEN]; //takes one command at a time
    int input_index = 0;

    int pipefd[2];
    int prev_pipefd[2]; //has previous pipefd

    int a = 0;
    int b = 0;

    for ( ; a < outfile_bufc || b < infile_bufc; a++, b++)
    {
        for (int i = 0; input_index < input_bufc; ++i, ++input_index) //populate command with input buf
        {
            command[i] = input_buf[input_index];
            if (input_buf[input_index] == NULL)
            {
                if (input_index < input_bufc)
                {
                    input_index += 1;
                }
                break;
            }
        }

        if (input_index == input_bufc)
        {
            child_kill = 1;
        }

        if (pipe(pipefd) == -1)
        {
            perror("pipe failed");
            exit(EXIT_FAILURE);
        }

        if ((a == 0) && (a == outfile_bufc - 1)) //indicates just one command
        {
            execute(home, cwd, input, command, 0, 1, a, b);
            close(pipefd[0]);
            close(pipefd[1]);
        }

        else if (a == 0) //indicates first command
        {
            prev_pipefd[0] = pipefd[0];
            execute(home, cwd, input, command, 0, pipefd[1], a, b);
            prev_pipefd[1] = pipefd[1];
        }
        else if (a == outfile_bufc - 1) //indicates last command
        {
            execute(home, cwd, input, command, prev_pipefd[0], 1, a, b);
            close(pipefd[0]);
            close(pipefd[1]);
            close(prev_pipefd[0]);
            close(prev_pipefd[1]);
        }
        else //indicates commands in between
        {
            execute(home, cwd, input, command, prev_pipefd[0], pipefd[1], a, b);
            close(prev_pipefd[0]);
            close(prev_pipefd[1]);
            prev_pipefd[0] = pipefd[0];
            prev_pipefd[1] = pipefd[1];
        }

    }

    return;
}

void execute(char *home, char *cwd, char *input, char **command, int in, int out, int outfile_bufc, int infile_bufc)
{
    pid_t child_pid;
    pid_t pid;

    int child_status;

    struct stat sb;

    if (command[0] == NULL)
    {
        return;
    }

    if (strstr(command[0], "/") != NULL)
    {
        if (stat(command[0], &sb) == -1)
        {
            printf(EXEC_ERROR, input);
            exit(EXIT_FAILURE);
        }

    }

    if (strcmp(command[0], "exit") == 0) //exit
    {
        exit(EXIT_SUCCESS);
    }

    if (strcmp(command[0], "cd") == 0)
    {
        if (command[1] == NULL) //cd to home
        {
            if (setenv("OLDPWD", getenv("PWD"), 1) == -1)
            {
                printf(BUILTIN_ERROR, input);
                return;
            }

            if (chdir(home) == -1)
            {
                printf(BUILTIN_ERROR, input);
                return;
            }

            cwd = my_getcwd(); //setting PWD
            setenv("PWD", cwd, 1);
            free(cwd);
            return;
        }

        else
        {
            if (strcmp(command[1], "-") == 0) //cd to last dir
            {
                if (chdir(getenv("OLDPWD")) == -1)
                {
                    printf(BUILTIN_ERROR, input);
                    return;
                }

                cwd = my_getcwd(); //setting PWD
                setenv("PWD", cwd, 1);
                free(cwd);
                return;

            }

            else if (strcmp(command[1], ".") == 0) //cd to current dir
            {
                if (setenv("OLDPWD", getenv("PWD"), 1) == -1)
                {
                    printf(BUILTIN_ERROR, input);
                    return;
                }

                if (chdir(".") == -1)
                {
                    printf(BUILTIN_ERROR, input);
                    return;
                }

                cwd = my_getcwd(); //setting PWD
                setenv("PWD", cwd, 1);
                free(cwd);
                return;

            }

            else if (strcmp(command[1], "..") == 0) //cd to previous dir
            {
                if (setenv("OLDPWD", getenv("PWD"), 1) == -1)
                {
                    printf(BUILTIN_ERROR, input);
                    return;
                }

                if (chdir("..") == -1)
                {
                    printf(BUILTIN_ERROR, input);
                    return;
                }

                cwd = my_getcwd(); //setting PWD
                setenv("PWD", cwd, 1);
                free(cwd);
                return;
            }

            else //cd to path
            {
                if (setenv("OLDPWD", getenv("PWD"), 1) == -1)
                {
                    printf(BUILTIN_ERROR, input);
                    return;
                }

                if (chdir(command[1]) == -1)
                {
                    printf(BUILTIN_ERROR, input);
                    return;
                }

                cwd = my_getcwd(); //setting PWD
                setenv("PWD", cwd, 1);
                free(cwd);
                return;
            }
        }
    }

    if (infile_buf[infile_bufc] != NULL)
    {
        in = open(infile_buf[infile_bufc], O_RDONLY);
    }

    if (outfile_buf[outfile_bufc] != NULL)
    {
        out = open(outfile_buf[outfile_bufc], O_WRONLY | O_TRUNC | O_CREAT, S_IRUSR | S_IRGRP | S_IWGRP | S_IWUSR);
    }

    if ((child_pid = fork()) == -1) //fork
    {
        printf(EXEC_ERROR, "NO fork");
        exit(EXIT_FAILURE);
    }

    if ((int) child_pid == 0) //child
    {
        if (dup2(in, STDIN_FILENO) == -1)
        {
            printf(EXEC_ERROR, "NO dup2");
            exit(EXIT_FAILURE);
        }

        if (dup2(out, STDOUT_FILENO) == -1)
        {
            printf(EXEC_ERROR, "NO dup2");
            exit(EXIT_FAILURE);
        }

        if (builtin_execvp(home, cwd, input, command) == -1)
        {
            exit(EXIT_FAILURE);
        }

        if (execvp(*command, command) == -1) //overide child process + add NUll to last of command
        {
            printf(EXEC_NOT_FOUND, "NO execvp");
            exit(EXIT_FAILURE);
        }
        exit(EXIT_SUCCESS); //for safety
    }

    else //parent
    {
        if (dup2(STDIN_FILENO, in) == -1)
        {
            printf(EXEC_ERROR, "NO dup2");
            exit(EXIT_FAILURE);
        }

        if (dup2(STDOUT_FILENO, out) == -1)
        {
            printf(EXEC_ERROR, "NO dup2");
            exit(EXIT_FAILURE);
        }

        if (child_kill == 1)
        {
            do
            {
                pid = wait(&child_status); //wait for child

                if (pid != child_pid) //if parent has more than one child
                {
                    debug("NOOOOO %s", "Sad");
                    //kill all child
                }
            } while (pid != child_pid);

        }
    }

    return;
}

int builtin_execvp(char *home, char *cwd, char *input, char *command[])
{
    if (command[0] == NULL)
    {
        goto end;
    }
    //buit-in
    if (strcmp(command[0], "help") == 0) //help
    {
        printf("help - print a list of all builtins and their basic usage in a single column\n"
                "exit - exits the shell\n"
                "cd - changes the current working directory of the shell\n"
                "pwd - prints the absolute path of the current working directory\n");
        goto end;
    }

    if (strcmp(command[0], "pwd") == 0) //pwd
    {
        if ((cwd = my_getcwd()) == 0)
        {
            printf(BUILTIN_ERROR, input);
            return -1;
        }

        printf("%s\n", cwd);
        goto free;
    }

    else
    {
        return 0;
    }

    free:
    free(cwd);

    end:
    exit(EXIT_SUCCESS);

    return 0;
}

int syntax_checker(char *input)
{
    //check either side of | > <
    //check consecutive | > <
    //check argument
    //check filenames & argument
    return 0;
}

char *init_cwd(char *home, char *cwd)
{
    // check getcwd()
    if ((cwd = my_getcwd()) == 0)
    {
        perror("getcwd failed");
        exit(EXIT_FAILURE);
    }

    // set PWD
    setenv("PWD", cwd, 1);

    //change home dir to ~
    if (strncmp(cwd, home, strlen(home)) == 0)
    {
        if (strcpy(cwd, cwd + strlen(home)) == NULL)
        {
            perror("strcpy failed");
            exit(EXIT_FAILURE);
        }
        prepend(cwd, "~");
    }

    // realloc cwd: +1 is for empty char
    cwd = (char *) realloc(cwd, strlen(cwd) + strlen(" :: howoo >> ") + 1);
    if (cwd == NULL)
    {
        perror("realloc failed");
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
