#include "builtins.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#define BUFF_SIZE 512


struct cmd_sequence {
    char **commands;
    char *separators;
};

void free_cmd_sequence(struct cmd_sequence *sequence)
{
    free(sequence->commands);
    free(sequence->separators);
    free(sequence);
}

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

struct cmd_sequence *line_split(char *line) // splits line on `;`, `&&` and `||`
{
    char **commands = malloc(sizeof(char*) * 2);
    if (commands == NULL) {
        fprintf(stderr, "Allocation error");
        return NULL;
    }
    char *cmd_separators = malloc(sizeof(char) * 2);
    if (cmd_separators == NULL) {
        fprintf(stderr, "Allocation error");
        free(commands);
        return NULL;
    }
    char *curr_cmd_start = line;
    commands[0] = curr_cmd_start;
    size_t i = 0;
    size_t cmd_count = 1;
    size_t allocated = 2;
    while (line[i] != '\0') {
        if (((line[i] == '&' || line[i] == '|') && line[i] == line[i + 1]) ||
            line[i] == ';') {
            if (line[i] != ';') {
                line[i + 1] = ' '; // replaces the second character of logical operator
            }
            cmd_separators[cmd_count - 1] = line[i];
            line[i] = '\0';
            curr_cmd_start = line + i + 1;
            commands[cmd_count] = curr_cmd_start;
            cmd_count++;
        }
        if (cmd_count >= allocated) {
            char **new_commands = realloc(commands, (allocated * 2) * sizeof(char *));
            if (new_commands == NULL) {
                free(commands);
                free(cmd_separators);
                fprintf(stderr, "Allocation error");
                return NULL;
            }
            commands = new_commands;
            char *new_cmd_separators = realloc(cmd_separators, (allocated * 2) * sizeof(char));
            if (new_cmd_separators == NULL) {
                free(new_commands);
                free(cmd_separators);
                fprintf(stderr, "Allocation error");
                return NULL;
            }
            cmd_separators = new_cmd_separators;
            allocated *= 2;
        }
        i++;
    }
    struct cmd_sequence *sequence = malloc(sizeof(struct cmd_sequence));
    if (sequence == NULL) {
        free(commands);
        free(cmd_separators);
        return NULL;
    }
    commands[cmd_count] = NULL;
    cmd_separators[cmd_count] = '\0';
    sequence->commands = commands;
    sequence->separators = cmd_separators;
    return sequence;
}

char **split_command(char *line) // only splits on whitespace for now (no quotes...)
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

    return (words[0] == NULL) ? NULL : words;
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
        return -1; // How did we get here?
    } else {
        waitpid(pid, &wait_status, WUNTRACED);
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
        struct cmd_sequence *sequence = line_split(line);
        if (sequence == NULL) {
            free(line);
            return;
        }

        bool skip_next_cmd = false;

        for (size_t i = 0; sequence->commands[i] != NULL; i++) {
            if (skip_next_cmd) {
                skip_next_cmd = false;
                continue;
            }
            char **args = split_command(sequence->commands[i]);
            if (args == NULL) {
                free_cmd_sequence(sequence);
                free(line);
                return;
            }
            int exec_ret = sh_execute(args);
            free(args);

            if (exec_ret == -1) {
                free(line);
                free_cmd_sequence(sequence);
                return;
            }
            if (sequence->commands[i + 1] != NULL &&     // if next command is null, separators[i] would be out of bounds
                ((exec_ret == 1 && sequence->separators[i] == '&') || (exec_ret == 0 && sequence->separators[i] == '|'))) {
                skip_next_cmd = true;
            }
        }

        free(line);
        free_cmd_sequence(sequence);

        if (eof_entered) {
            return;
        }
    }
}


int main()
{
    sh_loop();
}
