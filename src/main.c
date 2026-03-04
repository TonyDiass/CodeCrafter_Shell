#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

#define MAX_BASH_LINE 200
#define MAX_ARG 200

char *find_in(char *command) {
  char *env_path = getenv("PATH");
  if (!env_path || !command)
    return NULL;

  char *path_copy = strdup(env_path);
  char *to_free = path_copy;
  char *dir;
  static char chemin_complet[512];

  while ((dir = strsep(&path_copy, ":")) != NULL) {
    if (*dir == '\0')
      continue;
    snprintf(chemin_complet, sizeof(chemin_complet), "%s/%s", dir, command);
    if (access(chemin_complet, X_OK) == 0) {
      free(to_free);
      return chemin_complet;
    }
  }
  free(to_free);
  return NULL;
}

int parse_input(char *input, char **args) {
  int arg_count = 0;
  char buffer[MAX_BASH_LINE];
  int buf_idx = 0;
  bool in_single_quotes = false;
<<<<<<< HEAD
=======
  bool in_double_quotes = false;
>>>>>>> d5f6f1f (codecrafters submit [skip ci])

  for (int i = 0; input[i] != '\0'; i++) {
    char c = input[i];

<<<<<<< HEAD
    if (c == '\'') {
      in_single_quotes = !in_single_quotes;
    } else if (c == ' ' && !in_single_quotes) {
=======
    if (c == '\"' && !in_single_quotes) {
      in_double_quotes = !in_double_quotes;
    } else if (c == '\'' && !in_double_quotes) {
      in_single_quotes = !in_single_quotes;
    } else if (c == ' ' && !in_single_quotes && !in_double_quotes) {
>>>>>>> d5f6f1f (codecrafters submit [skip ci])
      if (buf_idx > 0) {
        buffer[buf_idx] = '\0';
        args[arg_count++] = strdup(buffer);
        buf_idx = 0;
      }
    } else if (c == '\\' && !in_single_quotes && !in_double_quotes) {
      buffer[buf_idx++] = input[++i];
    } else {
      buffer[buf_idx++] = c;
    }
  }

  if (buf_idx > 0) {
    buffer[buf_idx] = '\0';
    args[arg_count++] = strdup(buffer);
  }
  args[arg_count] = NULL;
  return arg_count;
}

void execute_command(char **args, int arg_count) {
  if (strcmp(args[0], "exit") == 0) {
    exit(0);
  } else if (strcmp(args[0], "echo") == 0) {
    for (int j = 1; j < arg_count; j++) {
      printf("%s%c", args[j], (j == arg_count - 1) ? '\n' : ' ');
    }
  } else if (strcmp(args[0], "pwd") == 0) {
    char cwd[1024];
    if (getcwd(cwd, sizeof(cwd)) != NULL)
      printf("%s\n", cwd);
  } else if (strcmp(args[0], "cd") == 0) {
    char *path =
        (arg_count > 1 && strcmp(args[1], "~") == 0) ? getenv("HOME") : args[1];
    if (chdir(path) != 0)
      printf("cd: %s: No such file or directory\n", args[1]);
  } else if (strcmp(args[0], "type") == 0) {
    if (arg_count < 2)
      return;
    if (strcmp(args[1], "echo") == 0 || strcmp(args[1], "exit") == 0 ||
        strcmp(args[1], "type") == 0 || strcmp(args[1], "pwd") == 0 ||
        strcmp(args[1], "cd") == 0)
      printf("%s is a shell builtin\n", args[1]);
    else {
      char *p = find_in(args[1]);
      if (p)
        printf("%s is %s\n", args[1], p);
      else
        printf("%s: not found\n", args[1]);
    }
  } else {
    char *full_path = find_in(args[0]);
    if (full_path) {
      if (fork() == 0) {
        execv(full_path, args);
        exit(1);
      } else {
        wait(NULL);
      }
    } else {
      printf("%s: command not found\n", args[0]);
    }
  }
}

int main(int argc, char *argv[]) {
  setbuf(stdout, NULL);

  while (1) {
    printf("$ ");
    char input[MAX_BASH_LINE];
    if (!fgets(input, sizeof(input), stdin))
      break;
    input[strcspn(input, "\n")] = 0;

    char *args[MAX_ARG];
    int arg_count = parse_input(input, args);

    if (arg_count == 0)
      continue;

    execute_command(args, arg_count);

    for (int k = 0; k < arg_count; k++)
      free(args[k]);
  }
  return 0;
}