#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>


#define BUFF_SIZE 8

char *read_line()
{
    char *buffer = malloc(sizeof(char) * BUFF_SIZE);
    if (buffer == NULL) {
        fprintf(stderr, "Allocation error\n");
        return NULL;
    }
    int c = getchar();
    int position = 0;
    int allocated = BUFF_SIZE;

    while (c != EOF && c != '\n') {
        buffer[position] = c;
        position++;

        if (position >= allocated) {
            buffer = realloc(buffer, allocated + BUFF_SIZE); // no need for keeping old pointer in case of error, we don't want to store partial lines;
            if (buffer == NULL) {
                fprintf(stderr, "Allocation error");
                return NULL;
            }
            allocated += BUFF_SIZE;
        }
        c = getchar();
    }
    if  (position == 0 && c == EOF) {
        free(buffer);
        return NULL;
    }
    buffer[position] = '\0';
    return buffer;
}

char **line_split(char *line) // only splits (no quotes...) on whitespace for now
{
    char **words = malloc(sizeof(char*) * BUFF_SIZE);
    if (words == NULL) {
        fprintf(stderr, "Allocation error");
        return NULL;
    }
    int position = 0;
    int allocated = BUFF_SIZE;
    char *word = strtok(line, " \t\n\n\a");

    while (word != NULL) {
        words[position] = word;
        position++;

        if (position >= allocated) {
            words = realloc(words, (allocated + BUFF_SIZE) * sizeof(char*));
            if (words == NULL) {
                fprintf(stderr, "Allocation error");
                return NULL;
            }
            allocated += BUFF_SIZE;
        }
        word = strtok(NULL, " \t\n");
    }
    words[position] = NULL; // exec calls require arg array to end with NULL pointer
    return words;
}

int sh_execute(char **args)
{
    pid_t pid = fork();
    int wait_status = 0;

    if (pid < 0) {
        perror("mysh");
        return 1;
    } else if (pid == 0) {
        if (execvp(args[0], args) == -1) {
            perror("mysh");
            exit(EXIT_FAILURE); // Exit from child process
        }
    } else {
        do {
            waitpid(pid, &wait_status, WUNTRACED);
        } while (WIFEXITED(wait_status) && WIFSTOPPED(wait_status));
    }
    return 0;
}

int sh_loop()
{
    while (true) {
        printf("$ ");
        char *line = read_line();
        if (line == NULL) {
            printf("EOF");
            putchar('\n');
            return 1;
        }
        char **args = line_split(line);
        sh_execute(args);
        free(line);
        free(args);
    }
    return 0;
}


int main()
{
    sh_loop();
}
