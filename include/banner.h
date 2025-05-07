/**
 * @file banner.h
 * @brief Colorful ASCII art banner display for shell applications.
 *
 * Includes a function to display a stylized "SHELL" banner with blue and white
 * coloring using ANSI escape sequences. Includes color definitions for terminal
 * text formatting.
 */

#ifndef BANNER_H
#define BANNER_H

#include <stdio.h>

#define BLUE "\033[0;34m"
#define WHITE "\033[1;37m"
#define RESET "\033[0m"
#define WHITE_ON_BLUE "\033[1;37;44m"

/**
 * @file banner.h
 * @brief Colorful ASCII art banner display for shell applications.
 *
 * Provides a function to display a stylized "SHELL" banner with blue and white
 * coloring using ANSI escape sequences. Includes color definitions for terminal
 * text formatting.
 */
void display_colored_banner()
{
    printf(BLUE "██████" WHITE "╗  " BLUE "██████" WHITE "╗ " BLUE "██" WHITE "╗  " BLUE "██" WHITE "╗   " BLUE "██" WHITE "╗     " BLUE "███████" WHITE "╗" BLUE "██" WHITE "╗  " BLUE "██" WHITE "╗" BLUE "███████" WHITE "╗" BLUE "██" WHITE "╗     " BLUE "██" WHITE "╗     \n");
    printf(BLUE "██" WHITE "╔══" BLUE "██" WHITE "╗" BLUE "██" WHITE "╔═══" BLUE "██" WHITE "╗" BLUE "██" WHITE "║  " WHITE "╚" BLUE "██" WHITE "╗ " BLUE "██" WHITE "╔╝     " BLUE "██" WHITE "╔════╝" BLUE "██" WHITE "║  " BLUE "██" WHITE "║" BLUE "██" WHITE "╔════╝" BLUE "██" WHITE "║     " BLUE "██" WHITE "║      \n");
    printf(BLUE "██████" WHITE "╔╝" BLUE "██" WHITE "║   " BLUE "██" WHITE "║" BLUE "██" WHITE "║   " WHITE "╚" BLUE "████" WHITE "╔╝      " BLUE "███████" WHITE "╗" BLUE "███████" WHITE "║" BLUE "█████" WHITE "╗  " BLUE "██" WHITE "║     " BLUE "██" WHITE "║      \n");
    printf(BLUE "██" WHITE "╔═══╝ " BLUE "██" WHITE "║   " BLUE "██" WHITE "║" BLUE "██" WHITE "║    " WHITE "╚" BLUE "██" WHITE "╔╝       " WHITE "╚════" BLUE "██" WHITE "║" BLUE "██" WHITE "╔══" BLUE "██" WHITE "║" BLUE "██" WHITE "╔══╝  " BLUE "██" WHITE "║     " BLUE "██" WHITE "║      \n");
    printf(BLUE "██" WHITE "║     " WHITE "╚" BLUE "██████" WHITE "╔╝" BLUE "███████" WHITE "╗" BLUE "██" WHITE "║" BLUE "███████" WHITE "╗" BLUE "███████" WHITE "║" BLUE "██" WHITE "║  " BLUE "██" WHITE "║" BLUE "███████" WHITE "╗" BLUE "███████" WHITE "╗" BLUE "███████" WHITE "╗ \n");
    printf(WHITE "╚═╝      ╚═════╝ ╚══════╝╚═╝╚══════╝╚══════╝╚═╝  ╚═╝╚══════╝╚══════╝╚══════╝ \n" RESET);
}

#endif // BANNER_H