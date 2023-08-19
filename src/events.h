#ifndef EVENTS_H
#define EVENTS_H
#include <termios.h>

// stores original terminal settings
struct termios orig_termios;

// prototypes
void initEvents();
void exitEvents();
char *event();

#endif