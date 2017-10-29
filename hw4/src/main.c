#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <readline/readline.h>
#include <errno.h>

#include "sfish.h"
#include "debug.h"

int main(int argc, char *argv[], char *envp[]) {
    int inputc; //tokenized input size
    char *cwd = NULL; //current working dir
    char *input = NULL; //input
    char **inputv = NULL; //tokenized input
    char *home = NULL; //home dir

    if (!isatty(STDIN_FILENO)) //if from file
    {
        // If your shell is reading from a piped file
        // Don't have readline write anything to that file.
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

    do
    {
        //init inputc
        inputc = 0;

        //init cwd
        cwd = init(home, cwd);

        //init input
        input = readline(cwd);

        write(1, "\e[s", strlen("\e[s")); //TODO
        write(1, "\e[20;10H", strlen("\e[20;10H"));
        write(1, "SomeText", strlen("SomeText"));
        write(1, "\e[u", strlen("\e[u"));

        // If EOF is read (aka ^D) readline returns NULL
        if (input == NULL)
        {
            if (cwd != NULL)
            {
                free(cwd);
            }

            continue;
        }

        //tokenize
        inputv = tokenize(input, &inputc);

        //execute
        // execute();

        //parse
        parse(home, cwd, input, inputc, inputv);

        //free
        if (cwd != NULL)
        {
            free(cwd);
        }

        if (input != NULL)
        {
            rl_free(input);
        }

    } while (1);

    return EXIT_SUCCESS;
}

void execute(char *home, char *cwd, char *input, int inputc, char **inputv)
{
    //Search file using *inputv
    if ()
    {
        /* code */
    }

    return;
}

char **tokenize(char *input, int *inputc)
{
    int i = 0; //counter for string
    char *temp_input = NULL; //temp input
    char *inputcpy = NULL; //copied input
    char **inputv = NULL; //tokenzied input

    //copy input
    inputcpy = malloc(sizeof(char) * strlen(input));
    if (inputcpy == NULL)
    {
        perror("malloc failed");
        exit(EXIT_FAILURE);
    }
    strcpy(inputcpy, input);

    temp_input = malloc(sizeof(char) * strlen(inputcpy)); //allocate space to hold input temporarily
    if (temp_input == NULL)
    {
        perror("malloc failed");
        exit(EXIT_FAILURE);
    }

    temp_input = strtok(inputcpy, " \r\n\t\v\f"); //tokenize input

    do
    {
        if (i > 0)
        {
            temp_input = strtok(NULL, " \r\n\t\v\f"); //tokenize the same input
        }

        if (temp_input == NULL)
        {
            break;
        }

        ++i; //update counter

        if (i <= 1)
        {
            inputv = malloc(sizeof(char *)); //allocate space to hold input
            if (inputv == NULL)
            {
                perror("malloc failed");
                exit(EXIT_FAILURE);
            }
        }

        else
        {
            inputv = realloc(inputv, sizeof(char *) * i); //allocate space to hold input
            if (inputv == NULL)
            {
                perror("realloc failed");
                exit(EXIT_FAILURE);
            }
        }

        inputv[i - 1] = malloc(sizeof(char *));
        if (inputv[i - 1] == NULL)
        {
            perror("malloc failed");
            exit(EXIT_FAILURE);
        }
        inputv[i - 1] = temp_input;

    } while (temp_input != NULL);

    *inputc = i;

    free(temp_input);

    return inputv; // TODO - free?
}

void parse(char *home, char *cwd, char *input, int inputc, char **inputv)
{
    if (strcmp(*(inputv), "help") == 0) //help
        {
            printf("help - print a list of all builtins and their basic usage in a single column (CSE320)\n"
                    "exit - exits the shell\n"
                    "cd - changes the current working directory of the shell\n"
                    "pwd - prints the absolute path of the current working directory\n");
            goto free;
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
                goto free;
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
                    goto free;
                }

                if (chdir(home) == -1)
                {
                    printf(BUILTIN_ERROR, input);
                    goto free;
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
                        goto free;
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
                        goto free;
                    }

                    if (chdir(".") == -1)
                    {
                        printf(BUILTIN_ERROR, input);
                        goto free;
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
                        goto free;
                    }

                    if (chdir("..") == -1)
                    {
                        printf(BUILTIN_ERROR, input);
                        goto free;
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
                        goto free;
                    }

                    if (chdir(*(inputv + 1)) == -1)
                    {
                        printf(BUILTIN_ERROR, input);
                        goto free;
                    }

                    cwd = my_getcwd(); //setting PWD
                    setenv("PWD", cwd, 1);

                    goto free;
                }
            }

        }

        else
        {
            printf(EXEC_NOT_FOUND, input);
        }

        free:
        return;
}

char *init(char *home, char *cwd)
{
    // check getcwd()
    if ((cwd = my_getcwd()) == 0)
    {
        perror("Failed using getcwd()"); //TODO
        exit(EXIT_FAILURE);
    }

    // set PWD
    setenv("PWD", cwd, 1);

    //change home dir to ~
    if (strncmp(cwd, home, strlen(home)) == 0)
    {
        if (strcpy(cwd, cwd + strlen(home)) == NULL)
        {
            perror("Failed using strcpy()"); //TODO
            exit(EXIT_FAILURE);
        }
        prepend(cwd, "~");

    }

    // realloc cwd: +1 is for empty char
    cwd = realloc(cwd, strlen(cwd) + strlen(" :: howoo >> ") + 1);

    // concate cwd
    cwd = strcat(cwd, " :: howoo >> ");

    if (cwd == NULL)
    {
        perror("realloc failed");
        exit(EXIT_FAILURE);
    }

    return cwd;
}

// void my_system(char *path, char *input)
// {
//     pid_t child_pid;
//     char *chopped_input;

//     strsep();



//     child_pid = fork(); //fork

//     if ((int) child_pid == 0)
//     {
//         execv(path, chopped_input);
//     }
//     else
//     {

//     }

// }

void prepend(char* s, const char* t)
{
    if (memmove(s + strlen(t), s, strlen(s) + 1) == NULL)
    {
        perror("memmove failed\n");
        exit(EXIT_FAILURE);
    }
    if (memcpy(s, t, strlen(t)) == NULL)
    {
        perror("memmove failed\n");
        exit(EXIT_FAILURE);
    }

    return;
}

char *my_getcwd()
{
  size_t size = 100;

  while (1)
    {
      char *buffer = (char *) malloc(size);

      if (getcwd(buffer, size) == buffer)
      {
        return buffer;
      }

      free(buffer);

      if (errno != ERANGE)
      {
        return 0;
      }

      size *= 2; //TODO
    }
}
