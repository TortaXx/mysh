#ifndef MYSH_BUILTINS_H
#define MYSH_BUILTINS_H

#define BUILTIN_COUNT 5

int cd_builtin(char **args);

int exit_builtin(char **args);

int pwd_builtin(char **args);

int true_builtin(char **args);

int false_builtin(char **args);



#endif //MYSH_BUILTINS_H
