#include <fcntl.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

#define MAX_BASH_LINE 200
#define MAX_ARG 200

typedef struct command {
  char **args;
  int arg_count;
} command;

typedef struct redirection {
  bool active;
  bool append;
  int fd;
  char *output_file;
} redirection;

char *FindPath(char *cmd) {
  char *env_path = getenv("PATH");
  if (!env_path || !cmd)
    return NULL;

  char *path_copy = strdup(env_path);
  char *to_free = path_copy;
  char *dir;
  static char path[512];

  while ((dir = strsep(&path_copy, ":")) != NULL) {
    if (*dir == '\0')
      continue;
    snprintf(path, sizeof(path), "%s/%s", dir, cmd);
    if (access(path, X_OK) == 0) {
      free(to_free);
      return path;
    }
  }
  free(to_free);
  return NULL;
}

void ParseInput(char *input, command *cmd, redirection *redir) {
  int arg_count = 0, buf_idx = 0;
  char buffer[MAX_BASH_LINE];
  bool in_single_quotes = false, in_double_quotes = false,
       expecting_file = false;

  for (int i = 0;; i++) {
    char c = input[i];

    if ((c == ' ' || c == '>' || c == '\0') && !in_single_quotes &&
        !in_double_quotes) {

      if (buf_idx > 0) {
        buffer[buf_idx] = '\0';
        if (expecting_file) {
          redir->output_file = strdup(buffer);
          expecting_file = false;
        } else {
          if (c == '>' &&
              (strcmp(buffer, "1") == 0 || strcmp(buffer, "2") == 0)) {
            redir->fd = atoi(buffer);
          } else {
            cmd->args[arg_count++] = strdup(buffer);
          }
        }
        buf_idx = 0;
      }

      if (c == '>') {
        redir->active = true;
        expecting_file = true;
        if (input[i + 1] == '>') {
          redir->append = true;
          i++;
        }
      } else if (c == '\0') {
        break;
      }

    } else if (c == '\\') {
      if (in_double_quotes) {
        char next = input[i + 1];
        if (next == '\"' || next == '\\')
          buffer[buf_idx++] = input[++i];
        else
          buffer[buf_idx++] = c;
      } else if (!in_single_quotes) {
        if (input[i + 1] != '\0')
          buffer[buf_idx++] = input[++i];
      } else {
        buffer[buf_idx++] = c;
      }
    } else if (c == '\"' && !in_single_quotes) {
      in_double_quotes = !in_double_quotes;
    } else if (c == '\'' && !in_double_quotes) {
      in_single_quotes = !in_single_quotes;
    } else {
      buffer[buf_idx++] = c;
    }
  }

  cmd->args[arg_count] = NULL;
  cmd->arg_count = arg_count;
}

bool IsBuiltin(char *cmd) {
  char *builtins[] = {"exit", "echo", "pwd", "cd", "type", NULL};
  for (int i = 0; builtins[i]; i++)
    if (strcmp(cmd, builtins[i]) == 0)
      return true;
  return false;
}

void ExecuteBuiltin(char **args, int arg_count) {
  if (strcmp(args[0], "exit") == 0)
    exit(0);

  if (strcmp(args[0], "echo") == 0) {
    for (int i = 1; i < arg_count; i++)
      printf("%s%s", args[i], (i == arg_count - 1) ? "" : " ");
    printf("\n");
  } else if (strcmp(args[0], "pwd") == 0) {
    printf("%s\n", getcwd(NULL, 0));
  } else if (strcmp(args[0], "cd") == 0) {
    char *path =
        (arg_count > 1 && strcmp(args[1], "~") == 0) ? getenv("HOME") : args[1];
    if (chdir(path) != 0)
      printf("cd: %s: No such file or directory\n", args[1]);
  } else if (strcmp(args[0], "type") == 0) {
    if (arg_count < 2)
      return;
    if (IsBuiltin(args[1]))
      printf("%s is a shell builtin\n", args[1]);
    else {
      char *p = FindPath(args[1]);
      p ? printf("%s is %s\n", args[1], p) : printf("%s: not found\n", args[1]);
    }
  }
}

void ExecuteExternal(char **args) {
  char *path = FindPath(args[0]);
  if (path) {
    if (fork() == 0) {
      execv(path, args);
      exit(1);
    }
    wait(NULL);
  } else {
    fprintf(stderr, "%s: command not found\n", args[0]);
  }
}

void ExecuteCommand(char **args, int arg_count) {
  if (IsBuiltin(args[0])) {
    ExecuteBuiltin(args, arg_count);
  } else {
    ExecuteExternal(args);
  }
}

int main(int argc, char *argv[]) {
  setbuf(stdout, NULL);

  command *cmd = malloc(sizeof(command));
  cmd->args = malloc(sizeof(char *) * MAX_ARG);
  redirection *redir = malloc(sizeof(redirection));

  while (1) {
    redir->active = false;
    redir->output_file = NULL;
    redir->fd = 1;
    redir->append = false;
    cmd->arg_count = 0;

    printf("$ ");
    char input[MAX_BASH_LINE];
    if (!fgets(input, sizeof(input), stdin))
      break;
    input[strcspn(input, "\n")] = 0;

    ParseInput(input, cmd, redir);

    if (cmd->arg_count == 0)
      continue;

    int open_flags = O_WRONLY | O_CREAT;
    if (redir->append) {
      open_flags |= O_APPEND;
    } else {
      open_flags |= O_TRUNC;
    }

    if (redir->active && redir->output_file != NULL) {
      int file_fd = open(redir->output_file, open_flags, 0644);
      if (file_fd < 0) {
        fprintf(stderr, "Erreur d'ouverture du fichier\n");
      } else {
        int target_fd = (redir->fd == 2) ? STDERR_FILENO : STDOUT_FILENO;

        int saved_fd = dup(target_fd);
        dup2(file_fd, target_fd);
        close(file_fd);

        ExecuteCommand(cmd->args, cmd->arg_count);

        fflush(stdout);
        fflush(stderr);

        dup2(saved_fd, target_fd);
        close(saved_fd);
      }
    } else {
      ExecuteCommand(cmd->args, cmd->arg_count);
    }

    for (int k = 0; k < cmd->arg_count; k++) {
      free(cmd->args[k]);
    }
    if (redir->output_file != NULL) {
      free(redir->output_file);
    }
  }

  free(cmd->args);
  free(cmd);
  free(redir);

  return 0;
}