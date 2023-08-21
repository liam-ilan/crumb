#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <termios.h>
#include <ctype.h>
#include <string.h>
#include "events.h"

void disableRaw() {
  // reset
  tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios);
}

void enableRaw() {
  // enable mouse
  printf("\e[?1003h\e[?1006h");

  // flush stdout
  fflush(stdout);

  // get current settings
  struct termios raw = orig_termios;

  // turn off flags
  raw.c_iflag &= ~(IXON);
  raw.c_lflag &= ~(ECHO | ICANON | IEXTEN | ISIG );
  raw.c_oflag &= ~(OPOST);
  raw.c_cc[VMIN] = 0;
  raw.c_cc[VTIME] = 1;

  // write settings back into terminal
  tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
}

// reads a single char from stdin, assuming that raw is enabled
char readChar() {
  char c = '\0';
  read(STDIN_FILENO, &c, 1);

  // printf("%c\r\n", c);
  // handle exit
  if (iscntrl(c) && (c == 26 || c == 3)) exit(0);
  return c;
}

// returns the current event as a string, where the result is malloc
char *event() {
  // enable mouse events
  

  enableRaw();

  // string to return
  char *res = malloc(sizeof(char) * 1);
  res[0] = '\0';

  // read first char
  char c = readChar();

  if (c == '\e') {
    // escape code case
    // increase memory, and add \e
    res = realloc(res, sizeof(char) * (strlen(res) + 1 + 2));
    strncat(res, &c, 1);

    // read char again and add to result
    c = readChar();
    strncat(res, &c, 1);
    
    if (c == '[') {
      // control sequence introducer case
      // read more at https://en.wikipedia.org/wiki/ANSI_escape_code#CSIsection
      
      // all escape sequences are terminated by a char in the range of 0x40 – 0x7E (64 - 126)
      // we loop until we find a terminator, and append to res
      do {
        res = realloc(res, sizeof(char) * (strlen(res) + 1 + 1));
        c = readChar();
        strncat(res, &c, 1);
      } while (!(64 <= c && c <= 126));
    }
  } else if (c != '\0') {
    // case where just a key was pressed, add memory, insert, and return
    res = realloc(res, sizeof(char) * (strlen(res) + 1 + 1));
    strncat(res, &c, 1);
  }
  
  // disable mouse events
  printf("\e[?1000l");
  disableRaw();
  return res;
}

void exitEvents() {
  disableRaw();
  // \e[?1000l disables mouse events
  // \e[?25h shows cursor
  // \e[m resets color
  printf("\e[?1000l\e[?25h\e[m");
}

void initEvents() {
  // \e[?1003h, \e[?1006h annd \n start mouse events
  // \e[A] and \r undos last \n
  printf("\n\e[A\r");
  tcgetattr(STDIN_FILENO, &orig_termios);
  atexit(disableRaw);
}