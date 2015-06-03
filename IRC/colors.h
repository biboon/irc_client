#ifndef __COLORS_H__
#define __COLORS_H__

#include <stdio.h>

/* Clears the screen (may be not safe) */
#define clrscr() printf("\033[H\033[2J")

/* Changes color */
#define setcolor(param) printf("\033[%sm",param)

/* Colors definitions */
#define RESET "0"
#define BLINK "5"

#define BLACK "30"
#define RED "31"
#define GREEN "32"
#define YELLOW "33"
#define BLUE "34"
#define MAGENTA "35"
#define CYAN "36"
#define WHITE "37"

/*Add 10 for background color */

#endif
