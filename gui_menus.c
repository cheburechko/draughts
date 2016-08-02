#include "interface.h"
#include <stdlib.h>

// creates a menu
gui_menu * gui_create_menu(int lines, int cols,
					char * name, char *choices[], char * help, int size,
					gui_menu_style * style, char * keys)
{
	gui_menu * result = (gui_menu *) malloc(sizeof(gui_menu));

	// create items
	ITEM ** items = (ITEM**) calloc(size + 1, sizeof(ITEM *));
	int i, begin_x, begin_y;
	for (i = 0; i < size; i++)
		items[i] = new_item(choices[i], choices[i]);
	items[size] = NULL;
	
	// create menu
	MENU * menu = new_menu((ITEM **) items);
	menu_opts_off(menu, O_SHOWDESC);

	// place in center
	begin_x = (COLS - cols) / 2;
	begin_y = (LINES - lines) / 2;

	// count space for help info
	int help_lines = -1;
	if (help)
	{
		help_lines = 1;
		i = 0;
		while (help[i])
		{
			if (help[i] == '\n')
				help_lines++;
			i++;
		}
	}

	// create window
	WINDOW * win = newwin(lines, cols, begin_y, begin_x);
	WINDOW * subwin = derwin(win, lines - 5 - help_lines, cols - 2, 3, 1);
	WINDOW * helpwin = 0;
	if (help)
		helpwin = derwin(win, help_lines, cols - 2, 
						 lines - help_lines - 1, 1);

	set_menu_win(menu, win);
	set_menu_sub(menu, subwin);
	set_menu_format(menu, lines - 5 - help_lines, 1);
	// write struct
	result -> win = win;
	result -> subwin = subwin;
	result -> helpwin = helpwin;
	result -> menu = menu;
	result -> lines = lines;
	result -> cols = cols;
	result -> name = name;
	result -> help = help;
	result -> help_lines = help_lines;
	result -> style = style;
	result -> keys = keys;
	result -> items = items;
	return result;
}

// show menu
void gui_activate_menu(gui_menu * menu)
{
	int x,y;
	getyx(menu -> win, y, x);
	box(menu -> win, 0, 0);
	print_in_middle(menu -> win, 1, 0, menu -> cols, menu -> name, 
					menu -> style -> name_color);

	mvwaddch(menu -> win, 2, 0, ACS_LTEE);
	mvwhline(menu -> win, 2, 1, ACS_HLINE, menu -> cols - 2);
	mvwaddch(menu -> win, 2, menu -> cols - 1, ACS_RTEE);

	if (menu -> helpwin)
	{
		mvwaddch(menu -> win, menu -> lines - menu -> help_lines -2,
				 0, ACS_LTEE);
		mvwhline(menu -> win, menu -> lines - menu -> help_lines -2, 
				 1, ACS_HLINE, menu -> cols - 2);
		mvwaddch(menu -> win, menu -> lines - menu -> help_lines -2,
				 menu -> cols - 1, ACS_RTEE);

		mvwprintw(menu -> helpwin, 0, 0, "%s", menu -> help);
	}

	keypad(menu -> win, TRUE);
	post_menu(menu -> menu);
	pos_menu_cursor(menu -> menu);

	wrefresh(menu -> win);
}

// hide menu
void gui_hide_menu(gui_menu * menu)
{
	int x,y;
	getyx(menu -> win, y, x);
	keypad(menu -> win, FALSE);
	unpost_menu(menu -> menu);
	wborder(menu -> win, ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ');

	mvwhline(menu -> win, 2, 0, ' ', menu -> cols);
	mvwhline(menu -> win, 1, 0, ' ', menu -> cols);
	if (menu -> helpwin)
	{
		mvwhline(menu -> win, menu -> lines - menu -> help_lines -2,
				 0, ' ', menu -> cols);
		for (y = 0; y < menu -> help_lines; y++)
			mvwhline(menu -> helpwin, y, 0, ' ', menu -> cols - 2);
	}

	wrefresh(menu -> win);
}

// ask user for choise.
int gui_call_menu(gui_menu * menu)
{
	gui_activate_menu(menu);
	int result = 0, i = 0;
	while (!result)
	{
		int c = wgetch(menu -> win);
		switch(c)
		{
			case KEY_DOWN:
				menu_driver(menu -> menu, REQ_DOWN_ITEM);
				break;
			case KEY_UP:
				menu_driver(menu -> menu, REQ_UP_ITEM);
				break;
			case 10:
				result = item_index(current_item(menu -> menu)) + 1;
				//mvprintw(30, 1, "Index = %d", result);
				break;
			default:
				i = 0;
				if (menu -> keys)
					while(menu -> keys[i])
					{
						if (menu -> keys[i] == c)
						{
							result = - (int) c;
							break;
						}
						i++;
					}
				break;
		}
		//mvwprintw(menu -> win, 1, 1, "%c", c);
		wrefresh(menu -> win);
	}
	gui_hide_menu(menu);
	return result;
}

void gui_destroy_menu(gui_menu * menu)
{
	gui_hide_menu(menu);
	int l = item_count(menu -> menu);
	free_menu(menu -> menu);
	delwin(menu -> subwin);
	delwin(menu -> win);

	int i = 0;
	while(menu -> items[i])
	{
		free_item(menu -> items[i]);
		i++;
	}
		
	free(menu -> items);
	free(menu);
}
