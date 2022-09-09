#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#define BUFF_SIZE 8

char *sh_read_line()
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
        c = getchar();

        if (position >= allocated) {
            buffer = realloc(buffer, allocated + BUFF_SIZE); // no need for keeping old pointer in case of error, we don't want to store partial lines;
            if (buffer == NULL) {
                fprintf(stderr, "Allocation error");
                free(buffer);
                return NULL;
            }
            allocated += BUFF_SIZE;
        }
    }
    if  (position == 0 && c == EOF) {
        free(buffer);
        return NULL;
    }
    buffer[position] = '\0';
    return buffer;

}

int sh_loop()
{
    while (true) {
        printf("$ ");
        char *line = sh_read_line();
        if (line == NULL) {
            printf("EOF");
            putchar('\n');
            return 1;
        }
        printf("%s\n", line);
        free(line);
    }
}


int main()
{
    sh_loop();
}
