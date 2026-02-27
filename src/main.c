#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define MAX_BASH_LINE 200
#define MAX_ARG_LINE 200

char *path_ok(char *command) {
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

    char *commande, *arg;
    commande = strtok(input, " ");
    arg = strtok(NULL, "\n");

    if (strcmp(commande, "exit") == 0) {
      break;
    } else if (strcmp(commande, "echo") == 0) {
      printf("%s\n", arg);
    } else if (strcmp(commande, "type") == 0) {
      if (arg && (strcmp(arg, "echo") == 0 || strcmp(arg, "exit") == 0 ||
                  strcmp(arg, "type") == 0)) {
        printf("%s is a shell builtin\n", arg);
      } else {
        char *path = path_ok(arg);
        if (path)
          printf("%s is %s\n", arg, path);
        else
          printf("%s: not found\n", arg);
      }
    } else {
      printf("%s: command not found\n", commande);
    }
  }

  return 0;
}
