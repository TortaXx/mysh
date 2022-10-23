#include "builtins.h"
#include <stdlib.h>
#include <stdio.h>
#include <pwd.h>
#include <unistd.h>

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
