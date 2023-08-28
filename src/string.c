#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "string.h"

// handle escape codes
char *parseString(char *in) {
  char *res = (char *) malloc(sizeof(char) * (strlen(in) + 1));

  // the number of charechters lost due to escape codes
  int lost = 0;

  for (int i = 0; i < strlen(in); i++) {
    if (in[i] == '\\') {
      if (in[i + 1] == 'a') res[i - lost] = '\a';
      else if (in[i + 1] == 'b') res[i - lost] = '\b';
      else if (in[i + 1] == 'f') res[i - lost] = '\f';
      else if (in[i + 1] == 'n') res[i - lost] = '\n';
      else if (in[i + 1] == 'r') res[i - lost] = '\r';
      else if (in[i + 1] == 't') res[i - lost] = '\t';
      else if (in[i + 1] == 'v') res[i - lost] = '\v';
      else if (in[i + 1] == 'e') res[i - lost] = '\e';
      else if (in[i + 1] == '\\') res[i - lost] = '\\';
      else if (in[i + 1] == '\"') res[i - lost] = '\"';
      else res[i - lost] = in[i + 1];
      lost++;
      i++;
    } else res[i - lost] = in[i];
  }

  res = (char *) realloc(res, strlen(in) + 1 - lost);
  res[strlen(in) - lost] = '\0';

  return res;
}