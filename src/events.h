#ifndef EVENTS_H
#define EVENTS_H
#include <termios.h>

// prototypes
void initEvents();
void exitEvents();
char *event();
char readChar();
void disableRaw();
void enableRaw();

// stores original terminal settings
extern struct termios orig_termios;

// stores terminal settings for while the program is running (echo off)
extern struct termios run_termios;


#endif