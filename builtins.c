#include "builtins.h"
#include <stdlib.h>
#include <stdio.h>
#include <pwd.h>
#include <unistd.h>

#define PATH_MAX 4096
#define UNUSED(x) (void)(x)


int cd_builtin(char **args)
{
    char *target_dir = args[1];
    if (target_dir == NULL) { // cd without argument changes dir to user's home dir ($HOME)
        target_dir= getenv("HOME");
        if (target_dir == NULL) {
            struct passwd *pwd = getpwuid(getuid());
            if (pwd == NULL) {
                perror("mysh: cd");
                return 1;
            }
            target_dir = pwd->pw_dir;
        }
    }
    if (chdir(target_dir) != 0) {
        perror("mysh: cd");
        return 1;
    }
    return 0;
}

int exit_builtin(char **args)
{
    UNUSED(args);
    return -1;
}

int pwd_builtin(char **args)
{
    UNUSED(args);
    char *buffer = malloc(sizeof(char) * PATH_MAX);
    if (buffer == NULL) {
        perror("mysh: cd");
        return 1;
    }
    if (getcwd(buffer, PATH_MAX) == NULL) {
        perror("mysh: cd");
        free(buffer);
        return 1;
    }
    printf("%s\n", buffer);
    free(buffer);
    return 0;
}

int true_builtin(char **args)
{
    UNUSED(args);
    return 0;
}

int false_builtin(char **args)
{
    UNUSED(args);
    return 1;
}
