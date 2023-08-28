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
      switch (in[i + 1]) {
      case 'a' :
        res[i - lost] = '\a';
        break;
      case 'b' :
        res[i - lost] = '\b';
        break;
      case'f' :
        res[i - lost] = '\f';
        break;
      case 'n' :
        res[i - lost] = '\n';
        break;
      case 'r' :
        res[i - lost] = '\r';
        break;
      case 't' :
        res[i - lost] = '\t';
        break;
      case 'v' :
        res[i - lost] = '\v';
        break;
      case 'e' :
        res[i - lost] = '\e';
        break;
      case '\\' :
        res[i - lost] = '\\';
        break;
      case '\"':
        res[i - lost] = '\"';
        break;
      default :
        res[i - lost] = in[i + 1];
        break;
      }
      lost++;
      i++;
    } else res[i - lost] = in[i];

  }

  res = (char *) realloc(res, strlen(in) + 1 - lost);
  res[strlen(in) - lost] = '\0';

  return res;
}
