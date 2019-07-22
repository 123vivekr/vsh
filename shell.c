/*
 * description: simple shell program with a few builtins and features
 *
 * author: Vivek R
 * email: 123vivekr@gmail.com
 */

#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>
#include <string.h>
#include <limits.h>

#define VSH_RL_BUFSIZE 1024

/*
 * reads a line and adds to a buffer (char array)
 * automatically resizes the char array if needed (on-the-go)
 */

char* shell_read_line(void) {
    char *line = NULL;
    ssize_t bufsize = 0; // bufsize is given 0. getline creates a new malloc and assigns it to line
    getline(&line, &bufsize, stdin);
    return line;
}

/*
 * This function splits the string into tokens. Delimiters: space, tab, carriage return, newline or ascii bell
 * tokens[0...n] points to the token strings
 * tokens is automatically resized on-the-go
 */

#define VSH_TOK_BUFSIZE 2
#define VSH_TOK_DELIM " \t\r\n\a"
char **shell_split_lines(char *line, int *pipe_present)
{
    int bufsize = VSH_TOK_BUFSIZE, position = 0;
    char **tokens = malloc(bufsize * sizeof(char*));
    char *token;

    if (!tokens) {
        fprintf(stderr, "vsh: allocation error");
        exit(EXIT_FAILURE);
    }
    // tokensize the string
    token = strtok(line, VSH_TOK_DELIM);
    while (token != NULL) {
        tokens[position] = token;
        if(strcmp(token, "|") == 0)
            *pipe_present = 1;
        position++;

        // increase token array size
        if(position >= bufsize) {
            bufsize += VSH_TOK_BUFSIZE;
            tokens = realloc(tokens, bufsize * sizeof(char*));

            if(!tokens) {
                fprintf(stderr, "vsh: allocation error");
                exit(EXIT_FAILURE);
            }
        }
        // get next token
        token = strtok(NULL, VSH_TOK_DELIM);
    }
    tokens[position] = NULL;
    return tokens;
}

/*
 * Function launches commands if not found in built-ins
 */

int vsh_launch(char **args)
{
    pid_t pid, wpid;
    int status;

    if ((pid = fork()) == 0) {
        // child process
        if (execvp(args[0], args)== -1) {
            perror("vsh: execution error");
        }
        exit(EXIT_FAILURE);
    } else if (pid < 0) {
        perror("vsh: forking error");
    } else {
        do {
            wpid = waitpid(pid, &status, WUNTRACED);
        } while (!WIFEXITED(status) && !WIFSIGNALED(status));
    }

    return 0;
}

// function for builtin shell commands
int vsh_cd(char **args);
int vsh_help(char **args);
int vsh_exit(char **args);
int vsh_pwd(char **args);
int vsh_tofile(char **args);
int vsh_fromfile(char **args);

// list of builtin commands
char *builtin_str[] = {
    "cd",
    "exit",
    "help",
    "pwd",
    // TODO
    "to_file",
    "from_file"
};

// an array of pointers, to functions all having the parameter type: char** and return type: int
int (*builtin_func[]) (char**) = {
    &vsh_cd,
    &vsh_exit,
    &vsh_help,
    &vsh_pwd,
    &vsh_tofile,
    &vsh_fromfile
};

// number of builtins
int vsh_num_builtins() {
    return sizeof(builtin_str) / sizeof(char *);
}

int vsh_cd(char **args) {
    if (args[1] == NULL) {
        fprintf(stderr, "vsh: expected argument to \"cd\"\n");
    } else {
        if (chdir(args[1]) != 0) {
            // change directory failed
            perror("vsh: cd failed");
        }
    }
    return 1;
}

int vsh_help(char **args) {
    int i;
    printf("\t --VSH--\n");
    printf("\t A simple shell\n");
    printf("\t Shell builtins:\n");

    for (int i = 0; i < vsh_num_builtins(); ++i) {
        printf("\t %s\n", builtin_str[i]);
    }
    return 1;
}

int vsh_exit(char **args) {
    return 0;
}

int shell_execute(char **args) {
    int i;

    // error handling code
    if (args[0] == NULL) {
        return 1;
    }

    // checks if any built-ins match and calls the appropriate one
    for (i = 0; i < vsh_num_builtins(); ++i) {
        if(strcmp(args[0], builtin_str[i]) == 0) {
                return (*builtin_func[i])(args);
        }
    }

    // this code is called if no built-ins are matched. CUSTOM COMMANDS
    return vsh_launch(args);
}

/*
 * Pipes in shell
 * tofile: mimics the > operator in shell
 * fromfile: mimics the < operator in shell
 */

/*
 * Create a pipe.
 * Read input from program output and write to file using C function
 */

int vsh_tofile(char **args)
{
    /*//separate filename and command*/
    /*int operator_pos, i = 0;*/
    /*while (args[i] != NULL) {*/
    /*    if(strcmp(args[i], ">") == 0)*/
    /*        operator_pos = i;*/
    /*    ++i;*/
    /*}*/

    /*char **temp = (char **) malloc(sizeof(int*) * 100);*/
    /*for(int i = 0; i < n_arg-1; ++i) {*/
    /*    temp[i] = (char*) malloc(sizeof(char) * 100);*/
    /*    strcpy(temp[i], args[i]);*/
    /*}*/
    /*temp[n_arg-1] = (char*) malloc(sizeof(char) * 100);*/
    /*temp[n_arg-1] = NULL;*/

    /*/**/
    /* * command: 0 to operator_pos*/
    /* * filename: operator_pos + 1*/
    /* */

    /*int file_out = open(args[operator_pos+1], O_CREAT);*/
    /*if(file_out == -1) {*/
    /*    perror("error opening file");*/
    /*}*/

    /*pid_t pid = fork();*/
    /*if(pid == 0) {*/
    /*    dup2(out, 1);*/
    /*    execvp(temp[0], temp);*/
    /*}*/

    /*close(file_out);*/
    /*//run command and wr*/
    return 1;
}

int vsh_fromfile(char **args)
{
     return 1;
}

/*
 * Prints working directory
 */

int vsh_pwd(char **args)
{
    char cwd[PATH_MAX];
    if(getcwd(cwd, sizeof(cwd)) != NULL) {
        printf("%s\n", cwd);
    }
    else {
        perror("getcwd() error");
    }
    return 1;
}

#define COMMAND_BUF 2

/*
 * input: arguments
 *
 * p (int) contains number of pipes in argument
 * pos (int array) contains position of pipes
 * size of pos = pipe
 */

int parse_pipe(char **args)
{
    int i = 0, p = 0;
    //first assume only 2 commands are there
    int *pos = (int *) malloc(COMMAND_BUF * sizeof(int));
    int bufsize = COMMAND_BUF;
    //check for pipes
    while(args[i] != NULL) {
        /* printf("%d: %s\n", i, args[i]); */
        if(strcmp(args[i], "|") == 0) {
            pos[p++] = i;
        }
        //resize pos array
        if (p == bufsize) {
            bufsize *= 2;
            pos = (int *) realloc(pos, bufsize);
        }
        i++;
    }

    // 1 pipe; 2 processes
    pid_t pid;
    int pipefd[2];
    /*
     * pipefd[0]: read end
     * pipefd[1]: write end
     */

    pipe(pipefd);

    pid = fork();
    if(pid == -1) {
        perror("error fork");
        fprintf(stderr, "vsh: fork error");
        return 1;
    } else if (pid == 0) {
        //second -- child process
        dup2(pipefd[0], 0);
        close(pipefd[1]);

        char **temp = (char**) malloc(sizeof(char*) * 100);
        int i = pos[0] + 1;
        int f = 0;
        while(args[i] != NULL) {
            temp[f] = (char*) malloc(sizeof(char) * 100);
            strcpy(temp[f++], args[i++]);
        }
        temp[f] = (char*) malloc(sizeof(char) * 100);
        temp[f] = NULL;

        execvp(temp[0], temp); //insert command and arguments
        perror("exec");
        return 1;
    } else {
        //first -- parent process
        dup2(pipefd[1], 1);
        close(pipefd[0]);

        int n_arg = pos[0] + 1; // the extra one for NULL
        char **temp = (char**) malloc(sizeof(char*) * n_arg);
        for(int i = 0; i < n_arg-1; ++i) {
            temp[i] = (char*) malloc(sizeof(char) * 100);
            strcpy(temp[i], args[i]);
        }
        temp[n_arg-1] = (char*) malloc(sizeof(char) * 100);
        temp[n_arg-1] = NULL;

        execvp(temp[0], temp);
        perror("exec");
        return 1;
    }
    //TODO: shell automatically closes after execution of commands
    return 1;
}

void vsh_loop(void)
{
    char *line;
    char **args;
    int status;

    do {
        printf(">");
        line = shell_read_line();
        int pipe_present = 0;
        args = shell_split_lines(line, &pipe_present);
        if(pipe_present == 1)
            status = parse_pipe(args);
        else
            status = shell_execute(args);

        free(line);
        free(args);
    } while (status);
}

int main(int argc, char *argv[])
{
    // load config files
    // run command loop
    vsh_loop();
    // perform any shutdown/cleanup
    return EXIT_SUCCESS;
}
