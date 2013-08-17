/* This file based on menu_scroll.c from:
http://tldp.org/HOWTO/NCURSES-Programming-HOWTO/intro.html */
#include <curses.h>
#include <menu.h>
#include <math.h>

#define ARRAY_SIZE(a) (sizeof(a) / sizeof(a[0]))
#define CTRLD 	4

char *iso_array[] = {
	"50",
	"100",
	"200",
	"400",
	"800",
	"1600",
	"3200"
};

char *shutter_array[] = {
	"1/1000",
	"1/500",
	"1/250",
	"1/125",
	"1/60",
	"1/30",
	"1/15",
	"1/8",
	"1/4",
	"1/2",
	"1"
};

char *aperture_array[] = {
	"f/1.4",
	"f/2",
	"f/2.8",
	"f/4",
	"f/5.6",
	"f/8",
	"f/11",
	"f/16"
};

ITEM **iso_items;
ITEM **shutter_items;
ITEM **aperture_items;
MENU *iso_menu;
MENU *shutter_menu;
MENU *aperture_menu;
WINDOW *iso_win;
WINDOW *shutter_win;
WINDOW *aperture_win;

void selection(char *name);
void print_in_middle(WINDOW *win, int starty, int startx, int width, char *string, chtype color);
int exposure(int iso);
double shutter(int exposure, int aperture);
double aperture(int exposure, int shutter);

int main(){
	int c;
	MENU **menu;
	WINDOW **win;
	int n_iso, i;
	int n_shutter, j;
	int n_aperture, k;

	/* Initialize curses */
	initscr();
	start_color();
	cbreak();
	noecho();
	keypad(stdscr, TRUE);
	init_pair(1, COLOR_RED, COLOR_BLACK);
	init_pair(2, COLOR_CYAN, COLOR_BLACK);

	/* Create items */
	n_iso = ARRAY_SIZE(iso_array);
	n_shutter = ARRAY_SIZE(shutter_array);
	n_aperture = ARRAY_SIZE(aperture_array);
	iso_items = (ITEM **)calloc(n_iso, sizeof(ITEM *));
	for(i = 0; i < n_iso; ++i) {
		iso_items[i] = new_item(iso_array[i], iso_array[i]);
		set_item_userptr(iso_items[i], selection);
	}
	shutter_items = (ITEM **)calloc(n_shutter, sizeof(ITEM *));
	for(i = 0; i < n_shutter; ++i){
		shutter_items[i] = new_item(shutter_array[i], shutter_array[i]);
		set_item_userptr(shutter_items[i], selection);
	}
	aperture_items = (ITEM **)calloc(n_aperture, sizeof(ITEM *));
	for(i = 0; i < n_aperture; ++i){
		aperture_items[i] = new_item(aperture_array[i], aperture_array[i]);
		set_item_userptr(aperture_items[i], selection);
	}

	/* Create menu */
	iso_menu = new_menu((ITEM **)iso_items);
	shutter_menu = new_menu((ITEM **)shutter_items);
	aperture_menu = new_menu((ITEM **)aperture_items);

	/* Create the window to be associated with the menu */
	iso_win = newwin(10, 40, 4, 4);
	keypad(iso_win, TRUE);
	shutter_win = newwin(10, 40, 4, 45);
	keypad(shutter_win, TRUE);
	aperture_win = newwin(10, 40, 4, 86);
	keypad(aperture_win, TRUE);

	/* Set main window and sub window */
	set_menu_win(iso_menu, iso_win);
	set_menu_sub(iso_menu, derwin(iso_win, 6, 38, 3, 1));
	set_menu_win(shutter_menu, shutter_win);
	set_menu_sub(shutter_menu, derwin(shutter_win, 6, 38, 3, 1));
	set_menu_win(aperture_menu, aperture_win);
	set_menu_sub(aperture_menu, derwin(aperture_win, 6, 38, 3, 1));
	set_menu_format(iso_menu, 5, 1);
	set_menu_format(shutter_menu, 5, 1);
	set_menu_format(aperture_menu, 5, 1);

	/* Set menu mark to the string " * " */
	set_menu_mark(iso_menu, " * ");
	set_menu_mark(shutter_menu, " * ");
	set_menu_mark(aperture_menu, " * ");

	/* Print a border around the main window and print a title */
	box(iso_win, 0, 0);
	box(shutter_win, 0, 0);
	box(aperture_win, 0, 0);
	print_in_middle(iso_win, 1, 0, 40, "ISO", COLOR_PAIR(1));
	print_in_middle(shutter_win, 1, 0, 40, "Shutter", COLOR_PAIR(1));
	print_in_middle(aperture_win, 1, 0, 40, "Aperture", COLOR_PAIR(1));
	mvwaddch(iso_win, 2, 0, ACS_LTEE);
	mvwaddch(shutter_win, 2, 0, ACS_LTEE);
	mvwaddch(aperture_win, 2, 0, ACS_LTEE);
	mvwhline(iso_win, 2, 1, ACS_HLINE, 38);
	mvwhline(shutter_win, 2, 1, ACS_HLINE, 38);
	mvwhline(aperture_win, 2, 1, ACS_HLINE, 38);
	mvwaddch(iso_win, 2, 39, ACS_RTEE);
	mvwaddch(shutter_win, 2, 39, ACS_RTEE);
	mvwaddch(aperture_win, 2, 39, ACS_RTEE);

	/* Post the menu */
	post_menu(iso_menu);
	post_menu(shutter_menu);
	post_menu(aperture_menu);
	wrefresh(iso_win);
	wrefresh(shutter_win);
	wrefresh(aperture_win);

	attron(COLOR_PAIR(2));
	mvprintw(LINES - 2, 0, "Use PageUp and PageDown to scoll down or up a page of items");
	mvprintw(LINES - 1, 0, "Arrow Keys to navigate (F1 to Exit)");
	attroff(COLOR_PAIR(2));
	refresh();


	while((c = getch())){
		switch(c){
			case KEY_LEFT:
				menu = &iso_menu;
				win = &iso_win;
					break;
			case KEY_RIGHT:
				menu = &shutter_menu;
				win = &shutter_win;
				 break;
			case KEY_DOWN:
				menu_driver(*menu, REQ_DOWN_ITEM);
			break;
			case KEY_UP:
				menu_driver(*menu, REQ_UP_ITEM);
			break;
			case KEY_NPAGE:
				menu_driver(*menu, REQ_SCR_DPAGE);
			break;
			case KEY_PPAGE:
				menu_driver(*menu, REQ_SCR_UPAGE);
			break;
			case 10: /* ENTER */
			{
				ITEM *cur;
				void (*p)(char *);

				cur = current_item(*menu);
				p = item_userptr(cur);
				p((char *)item_name(cur));
				pos_menu_cursor(*menu);
				break;
			}
			break;
								 
		}
		wrefresh(*win);
	}	
	/* Unpost and free all the memory taken up */
	unpost_menu(iso_menu);
	unpost_menu(shutter_menu);
	unpost_menu(aperture_menu);
	free_menu(iso_menu);
	for(i = 0; i < n_iso; ++i)
		free_item(iso_items[i]);
	for(i = 0; i < n_shutter; ++i)
		free_item(shutter_items[i]);
	for(i = 0; i < n_aperture; ++i)
		free_item(aperture_items[i]);
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

void selection(char *name) {
	/* Test setting item in another menu */
	set_top_row(shutter_menu, 3);
	/* This works, but plain refreshing doesn't work properly */
	/* But cheating and using menu_driver to go up/down does */
	menu_driver(shutter_menu, REQ_DOWN_ITEM);
	menu_driver(shutter_menu, REQ_UP_ITEM);
	wrefresh(shutter_win);

	/* Need to think about how to do the interaction
	 *
	 * Select ISO, but then just one or the other of aperture and shutter speed
	 * and on return know that adjusting other menu?A
	 *
	 * Perhaps a global variable/counter and display text saying "select first column"
	 * on enter increment counter, change to "select second column"
	 * then on next enter because of counter value, knows to run equations
	 * And cycle starts again. 
	 * Would do for a start
	 *
	 * Could even do entering numbers to select columns 1, 2, 3
	 *
	 * */
}

int exposure (int iso) {
	int ev100;
	ev100 = 15;
	return ev100 + (log (iso / 100) / log (2));
}

double shutter (int aperture, int exposure) {
	/* EV = log2 (N^2/t) */
	return pow(aperture, 2) / pow(2, exposure);
}

double aperture (int shutter, int exposure) {
	/* EV = log2 (N^2/t) */
	return shutter * sqrt(pow(2, exposure));
}
