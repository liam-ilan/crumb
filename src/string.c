#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "string.h"

// handle escape codes
char *parseString(char *in) {
  char *res = (char *) malloc(sizeof(char) * (strlen(in) + 1));
  int length = strlen(in);

  // the number of charechters lost due to escape codes
  int lost = 0;

  for (int i = 0; i < length; i++) {
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
      else if (in[i + 1] == 'x') {

        // get char
        char hexStr[3] = {in[i + 2], in[i + 3], '\0'};
        char c = (char) strtol(hexStr, NULL, 16);

        // invalid inputs return 0 (the null char)
        // we do not allow null chars in crumb, if we find one, simply ignore the full escape code
        if (c != '\0') {
          res[i - lost] = c;
        } else {lost += 1;}

        // hexadecimal escape codes are two chars longer than other escape codes
        lost += 2;
        i += 2;

      } else res[i - lost] = in[i + 1];

      lost++;
      i++;
    } else res[i - lost] = in[i];
  }

  res = (char *) realloc(res, strlen(in) + 1 - lost);
  res[strlen(in) - lost] = '\0';

  return res;
}