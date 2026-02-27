#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define MAX_LINE 200

int main(int argc, char *argv[]) {
  // Flush after every printf
  setbuf(stdout, NULL);

  while (1) {
    printf("$ ");
    char input[MAX_LINE];
    fgets(input, sizeof(input), stdin);
    input[strcspn(input, "\n")] = 0;
    printf("%s: command not found\n", input);
  }

  return 0;
}
