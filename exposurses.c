/* This file based on menu_scroll.c from:
http://tldp.org/HOWTO/NCURSES-Programming-HOWTO/intro.html */
#include <curses.h>
#include <menu.h>

#define ARRAY_SIZE(a) (sizeof(a) / sizeof(a[0]))
#define CTRLD 	4

char *choices[] = {
	"Choice 1",
	"Choice 2",
	"Choice 3",
	"Choice 4",
	"Choice 5",
	"Choice 6",
	"Choice 7",
	"Choice 8",
	"Choice 9",
	"Choice 10",
	"Exit",
	(char *)NULL,
};
void print_in_middle(WINDOW *win, int starty, int startx, int width, char *string, chtype color);

int main(){
	ITEM **my_items1;
	/*Weird, can't just re-use my_items. Must recreate that as well*/
	ITEM **my_items2;
	int c;
	MENU *my_menu1;
	MENU *my_menu2;
	WINDOW *my_menu_win1;
	WINDOW *my_menu_win2;
	int n_choices, i;

	/* Initialize curses */
	initscr();
	start_color();
	cbreak();
	noecho();
	keypad(stdscr, TRUE);
	init_pair(1, COLOR_RED, COLOR_BLACK);
	init_pair(2, COLOR_CYAN, COLOR_BLACK);

	/* Create items */
	n_choices = ARRAY_SIZE(choices);
	my_items1 = (ITEM **)calloc(n_choices, sizeof(ITEM *));
	for(i = 0; i < n_choices; ++i)
		my_items1[i] = new_item(choices[i], choices[i]);
	my_items2 = (ITEM **)calloc(n_choices, sizeof(ITEM *));
	for(i = 0; i < n_choices; ++i)
		my_items2[i] = new_item(choices[i], choices[i]);

	/* Create menu */
	my_menu1 = new_menu((ITEM **)my_items1);
	my_menu2 = new_menu((ITEM **)my_items2);

	/* Create the window to be associated with the menu */
	my_menu_win1 = newwin(10, 40, 4, 4);
	keypad(my_menu_win1, TRUE);
	my_menu_win2 = newwin(10, 40, 4, 45);
	keypad(my_menu_win2, TRUE);

	/* Set main window and sub window */
	set_menu_win(my_menu1, my_menu_win1);
	set_menu_sub(my_menu1, derwin(my_menu_win1, 6, 38, 3, 1));
	set_menu_win(my_menu2, my_menu_win2);
	set_menu_sub(my_menu2, derwin(my_menu_win2, 6, 38, 3, 1));
	set_menu_format(my_menu1, 5, 1);
	set_menu_format(my_menu2, 5, 1);

	/* Set menu mark to the string " * " */
	set_menu_mark(my_menu1, " * ");
	set_menu_mark(my_menu2, " * ");

	/* Print a border around the main window and print a title */
	box(my_menu_win1, 0, 0);
	box(my_menu_win2, 0, 0);
	print_in_middle(my_menu_win1, 1, 0, 40, "My Menu1", COLOR_PAIR(1));
	print_in_middle(my_menu_win2, 1, 0, 40, "My Menu2", COLOR_PAIR(1));
	mvwaddch(my_menu_win1, 2, 0, ACS_LTEE);
	mvwaddch(my_menu_win2, 2, 0, ACS_LTEE);
	mvwhline(my_menu_win1, 2, 1, ACS_HLINE, 38);
	mvwhline(my_menu_win2, 2, 1, ACS_HLINE, 38);
	mvwaddch(my_menu_win1, 2, 39, ACS_RTEE);
	mvwaddch(my_menu_win2, 2, 39, ACS_RTEE);

	/* Post the menu */
	post_menu(my_menu1);
	post_menu(my_menu2);
	wrefresh(my_menu_win1);
	wrefresh(my_menu_win2);

	attron(COLOR_PAIR(2));
	mvprintw(LINES - 2, 0, "Use PageUp and PageDown to scoll down or up a page of items");
	mvprintw(LINES - 1, 0, "Arrow Keys to navigate (F1 to Exit)");
	attroff(COLOR_PAIR(2));
	refresh();

	while((c = wgetch(my_menu_win1)) != KEY_F(1)){
		switch(c){
			case KEY_DOWN:
			menu_driver(my_menu1, REQ_DOWN_ITEM);
			menu_driver(my_menu2, REQ_DOWN_ITEM);
			break;
			case KEY_UP:
			menu_driver(my_menu1, REQ_UP_ITEM);
			menu_driver(my_menu2, REQ_UP_ITEM);
			break;
			case KEY_NPAGE:
			menu_driver(my_menu1, REQ_SCR_DPAGE);
			break;
			case KEY_PPAGE:
			menu_driver(my_menu1, REQ_SCR_UPAGE);
			break;
		}
		wrefresh(my_menu_win1);
		wrefresh(my_menu_win2);
	}	

	/* Unpost and free all the memory taken up */
	unpost_menu(my_menu1);
	free_menu(my_menu1);
	for(i = 0; i < n_choices; ++i)
		free_item(my_items1[i]);
	endwin();
}

void print_in_middle(WINDOW *win, int starty, int startx, int width, char *string, chtype color){
	int length, x, y;
	float temp;

	if(win == NULL)
		win = stdscr;
	getyx(win, y, x);
	if(startx != 0)
		x = startx;
	if(starty != 0)
		y = starty;
	if(width == 0)
		width = 80;

	length = strlen(string);
	temp = (width - length)/ 2;
	x = startx + (int)temp;
	wattron(win, color);
	mvwprintw(win, y, x, "%s", string);
	wattroff(win, color);
	refresh();
}
