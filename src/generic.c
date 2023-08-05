#include <stdio.h>
#include <stdlib.h>
#include "generic.h"

// print generic nicely
void Generic_print(Generic in) {
  if (in.type == TYPE_INT) {
    printf("%i\n", *((int *) in.p_val));
  } else if (in.type == TYPE_FLOAT) {
    printf("%f\n", *((double *) in.p_val));
  } else if (in.type == TYPE_STRING) {
    printf("%s\n", *((char **) in.p_val));
  } else if (in.type == TYPE_VOID) {
    printf("[Void]\n");
  } else if (in.type == TYPE_FUNCTION) {
    printf("[Function]\n");
  }
}