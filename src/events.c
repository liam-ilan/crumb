#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <termios.h>
#include <ctype.h>
#include <string.h>
#include "events.h"

// stores original terminal settings
struct termios orig_termios;

// stores terminal settings for while the program is running (echo off)
struct termios run_termios;

void disableRaw() {
  // reset
  printf("\e[?1000l");
  fflush(stdout);

  tcsetattr(STDIN_FILENO, TCSAFLUSH, &run_termios);
}

void enableRaw() {

  // get standard settings
  struct termios raw = orig_termios;

  // set flags
  raw.c_iflag &= ~(IXON);
  raw.c_lflag &= ~(ECHO | ICANON | IEXTEN | ISIG);
  raw.c_oflag &= ~(OPOST);
  raw.c_cc[VMIN] = 0;
  raw.c_cc[VTIME] = 1;

  // write settings back into terminal
  tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);

  // enable mouse
  printf("\e[?1003h\e[?1006h");

  // flush stdout
  fflush(stdout);
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

    } else if (c == 'O') {
      // \e O is used under certain cases to access function keys
      // this escape code accepts the next char, thus this case is needed
      res = realloc(res, sizeof(char) * (strlen(res) + 1 + 1));
      c = readChar();
      strncat(res, &c, 1);
    }

  } else if (c != '\0') {
    // case where just a key was pressed, add memory, insert, and return
    res = realloc(res, sizeof(char) * (strlen(res) + 1 + 1));
    strncat(res, &c, 1);
  }
  
  // disable mouse events
  disableRaw();
  return res;
}

void exitEvents() {
  tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios);
  // \e[?1000l disables mouse events
  printf("\e[?1000l");
}

void initEvents() {
  tcgetattr(STDIN_FILENO, &orig_termios);
  run_termios = orig_termios;
  run_termios.c_lflag &= ~(ECHO);
  atexit(exitEvents);
}