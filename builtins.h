#ifndef MYSH_BUILTINS_H
#define MYSH_BUILTINS_H

#define BUILTIN_COUNT 5

int cd_builtin(char **args);

int exit_builtin(char **args);

int pwd_builtin(char **args);

int true_builtin(char **args);

int false_builtin(char **args);


struct builtin_pair {
    const char *name;
    int (*fun)(char **args);
};

struct builtin_pair builtin_dict[BUILTIN_COUNT] = {
        { "cd", &cd_builtin },
        { "exit", &exit_builtin },
        {"pwd", &pwd_builtin},
        {"true", &true_builtin},
        {"false", &false_builtin},
};



#endif //MYSH_BUILTINS_H
