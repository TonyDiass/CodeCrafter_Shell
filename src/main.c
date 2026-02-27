#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define MAX_BASH_LINE 200
#define MAX_ARG_LINE 200

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
    } else {
      printf("%s: command not found\n", input);
    }
  }

  return 0;
}
