#include "interface.h"
#include <stdlib.h>

gui_status_line * gui_create_status_line(int cols, int y, int x)
{
	gui_status_line * result = (gui_status_line *) malloc
											(sizeof(gui_status_line));
	result -> win = newwin(1, cols, y, x);
	result -> cols = cols;
	return result;
}

void gui_clear_status_line(gui_status_line * line)
{
	mvwhline(line -> win, 0, 0, ' ', line -> cols);
	wrefresh(line -> win);
}

void gui_print_status_line(gui_status_line * line, char * message)
{
	gui_clear_status_line(line);
	mvwprintw(line -> win, 0, 0, message);
	wrefresh(line -> win);
}

void gui_destroy_status_line(gui_status_line * line)
{
	gui_clear_status_line(line);
	delwin(line -> win);
	free(line);
}
