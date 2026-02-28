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

int main(int argc, char *argv[]) {
  // Flush after every printf
  setbuf(stdout, NULL);

  while (1) {
    printf("$ ");

    char input[MAX_BASH_LINE];
    fgets(input, sizeof(input), stdin);
    input[strcspn(input, "\n")] = 0;

    char *args[MAX_ARG];
    int i = 0;
    char *token = strtok(input, " ");
    while (token != NULL) {
      args[i++] = token;
      token = strtok(NULL, " ");
    }
    args[i] = NULL;

    if (i == 0)
      continue;

    if (strcmp(args[0], "exit") == 0) {
      break;
    } else if (strcmp(args[0], "echo") == 0) {
      for (int j = 1; j < i; j++)
        printf("%s%c", args[j], (j == i - 1) ? '\n' : ' ');
    } else if (strcmp(args[0], "type") == 0) {
      if (i < 2)
        continue;
      if (strcmp(args[1], "echo") == 0 || strcmp(args[1], "exit") == 0 ||
          strcmp(args[1], "type") == 0)
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
  return 0;
}