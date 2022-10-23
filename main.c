#include "builtins.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#define BUFF_SIZE 512


struct builtin_pair {
    const char *name;
    int (*fun)(char **args);
};

struct builtin_pair builtin_dict[BUILTIN_COUNT] = {
        { "cd", &cd_builtin },
        { "exit", &exit_builtin },
        {"pwd", &pwd_builtin},
};

char *read_line(bool *eof_entered)
{
    char *buffer = malloc(sizeof(char) * BUFF_SIZE);
    if (buffer == NULL) {
        fprintf(stderr, "Allocation error\n");
        return NULL;
    }
    int c = getchar();
    size_t position = 0;
    size_t allocated = BUFF_SIZE;

    while (c != EOF && c != '\n') {
        buffer[position] = c;
        position++;

        if (position >= allocated) {
            char *new_buffer = realloc(buffer, allocated + BUFF_SIZE); // no need for keeping old pointer in case of error, we don't want to store partial lines;
            if (new_buffer == NULL) {
                fprintf(stderr, "Allocation error");
                free(buffer);
                return NULL;
            }
            buffer = new_buffer;
            allocated += BUFF_SIZE;
        }
        c = getchar();
    }
    if (c == EOF) {
        *eof_entered = true;
        if (position == 0) {
            free(buffer);
            return NULL;
        }
    }
    buffer[position] = '\0';
    return buffer;
}

char **line_split(char *line) // only splits on whitespace for now (no quotes...)
{
    char **words = malloc(sizeof(char*) * BUFF_SIZE);
    if (words == NULL) {
        fprintf(stderr, "Allocation error");
        return NULL;
    }
    size_t position = 0;
    size_t allocated = BUFF_SIZE;
    char *word = strtok(line, " \t\n\n\a");

    while (word != NULL) {
        words[position] = word;
        position++;

        if (position >= allocated) {
            char **new_words = realloc(words, (allocated + BUFF_SIZE) * sizeof(char*));
            if (new_words == NULL) {
                fprintf(stderr, "Allocation error");
                free(words);
                return NULL;
            }
            words = new_words;
            allocated += BUFF_SIZE;
        }
        word = strtok(NULL, " \t\n");
    }
    words[position] = NULL; // exec calls require arg array to end with NULL pointer
    return words;
}

int sh_run_command(char **args)
{
    pid_t pid = fork();
    int wait_status = 0;

    if (pid < 0) {
        perror("mysh");
        return 1;
    } else if (pid == 0) {
        if (execvp(args[0], args) == -1) {
            perror("mysh");
        }
        return -1;
    } else {
        do {
            waitpid(pid, &wait_status, WUNTRACED);
        } while (WIFEXITED(wait_status) && WIFSTOPPED(wait_status));
    }
    return 0;
}

int sh_execute(char **args)
{
    for (int i = 0; i < BUILTIN_COUNT; i++) {
        if (strcmp(args[0], builtin_dict[i].name) == 0) {
            return builtin_dict[i].fun(args);
        }
    }
    return sh_run_command(args);
}

void sh_loop()
{
    while (true) {
        printf("$ ");
        bool eof_entered = false;

        char *line = read_line(&eof_entered);
        if (line == NULL) {
            putchar('\n');
            return;
        }
        if (line[0] == '\0') { // so that execvp doesn't cause trouble
            free(line);
            continue;
        }
        if (eof_entered) {
            putchar('\n');
        }
        char **args = line_split(line);
        if (args == NULL) {
            free(line);
            return;
        }
        int exec_ret = sh_execute(args);

        free(line);
        free(args);
        if (eof_entered || exec_ret == -1) {
            return;
        }
    }
}


int main()
{
    sh_loop();
}
