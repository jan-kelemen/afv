#include <curses.h>

namespace afv
{
    int run([[maybe_unused]] int argc, [[maybe_unused]] char** argv)
    {
        initscr(); /* Start curses mode 		  */
        printw("Hello World !!!"); /* Print Hello World		  */
        refresh(); /* Print it on to the real screen */
        getch(); /* Wait for user input */
        endwin(); /* End curses mode		  */

        return 0;
    }
} // namespace afv
