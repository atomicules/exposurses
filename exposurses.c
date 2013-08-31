/* This file based on menu_scroll.c from:
http://tldp.org/HOWTO/NCURSES-Programming-HOWTO/intro.html */
#include <curses.h>
#include <menu.h>
#include <math.h>

#define ARRAY_SIZE(a) (sizeof(a) / sizeof(a[0]))

char *iso_array[] = {
	"50",
	"100",
	"200",
	"400",
	"800",
	"1600",
	"3200",
	NULL
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
	"1",
	NULL
};

char *aperture_array[] = {
	"f/1.4",
	"f/2",
	"f/2.8",
	"f/4",
	"f/5.6",
	"f/8",
	"f/11",
	"f/16",
	NULL
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
int selection_counter;
double shutter(int exposure, double aperture);
double aperture(int exposure, int shutter);
int nearest_match(double x, int menu);
double fraction_to_double(char *fraction);
/* No one will ever need more than 9 bytes! */
char iso_sel[9] = "";
char shutter_sel[9] = "";
char aperture_sel[9] = "";
int menu_counter = 1;

int main() {
	int c;
	MENU **menu;
	WINDOW **win;
	int n_iso, i;
	int n_shutter, j;
	int n_aperture, k;
	int menu_sel_last;
	selection_counter = 0;

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
	for(i = 0; i < n_shutter; ++i) {
		shutter_items[i] = new_item(shutter_array[i], shutter_array[i]);
		set_item_userptr(shutter_items[i], selection);
	}
	aperture_items = (ITEM **)calloc(n_aperture, sizeof(ITEM *));
	for(i = 0; i < n_aperture; ++i) {
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

	/* set default menu */
	menu = &iso_menu;
	win = &iso_win;

	while((c = getch())) {
		switch(c) {
			case KEY_LEFT:
				if (menu_counter > 1)
					menu_counter -= 1;
				switch(menu_counter) {
					case 1:
						menu = &iso_menu;
						win = &iso_win;
						break;
					case 2:
						menu = &shutter_menu;
						win = &shutter_win;
						break;
					case 3:
						menu = &aperture_menu;
						win = &aperture_win;
						break;
				}
				break;
			case KEY_RIGHT:
				if (menu_counter < 3)
					menu_counter += 1;
				switch(menu_counter) {
					case 1:
						menu = &iso_menu;
						win = &iso_win;
						break;
					case 2:
						menu = &shutter_menu;
						win = &shutter_win;
						break;
					case 3:
						menu = &aperture_menu;
						win = &aperture_win;
						break;
				}
				break;
			case KEY_DOWN:
				menu_driver(*menu, REQ_DOWN_ITEM);
				break;
			case KEY_UP:
				menu_driver(*menu, REQ_UP_ITEM);
				break;
			case 10: { /* ENTER */
				ITEM *cur;
				void (*p)(char *);

				cur = current_item(*menu);
				p = item_userptr(cur);
				p((char *)item_name(cur));
				pos_menu_cursor(*menu);

				if (selection_counter == 0) {
					 menu_sel_last = menu_counter;
					 selection_counter += 1;
				}
				if (menu_counter != menu_sel_last)
					 selection_counter += 1;
				if (selection_counter == 2) { 
					/* calculate the other menu */
					/* how to get missing menu? */
					if (strcmp("", iso_sel) == 0) {
						/* Test searching for item in menu */
						set_menu_pattern(iso_menu, "200");
						/* Using menu_driver to go up/down to force refresh */
						menu_driver(iso_menu, REQ_DOWN_ITEM);
						menu_driver(iso_menu, REQ_UP_ITEM);
						wrefresh(iso_win);
					}
					if (strcmp("", shutter_sel) == 0) {
						/* Test finding nearest matching value in array? */
						char aperture_sel_[4] = "";
						strncpy(aperture_sel_, aperture_sel+2, 3);
						/* There is probably a nicer way to format the below */
						set_menu_pattern(
							shutter_menu,
							shutter_array[nearest_match(
								shutter(aperture_sel_, exposure(iso_sel)),
								2
							)]
						);
						set_top_row(shutter_menu, 3);
						menu_driver(shutter_menu, REQ_DOWN_ITEM);
						menu_driver(shutter_menu, REQ_UP_ITEM);
						wrefresh(shutter_win);
					}
					if (strcmp("", aperture_sel) == 0) {
						/* Tests setting to row number */
						set_top_row(aperture_menu, 3);
						menu_driver(aperture_menu, REQ_DOWN_ITEM);
						menu_driver(aperture_menu, REQ_UP_ITEM);
						wrefresh(aperture_win);
					}
					/* clear the selections for next time */
					selection_counter = 0;
					strcpy(iso_sel, "");
					strcpy(shutter_sel, "");
					strcpy(aperture_sel, "");
					break;
				}
				break;
			}
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

void print_in_middle(WINDOW *win, int starty, int startx, int width, char *string, chtype color) {
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
	switch(menu_counter) {
		case 1:
			strcpy(iso_sel, name);
			break;
		case 2:
			strcpy(shutter_sel, name);
			break;
		case 3:
			strcpy(aperture_sel, name);
			break;
	}
}

int exposure (int iso) {
	int ev100;
	ev100 = 15;
	return ev100 + (log (iso / 100) / log (2));
}

double shutter (int exposure, double aperture) {
	/* EV = log2 (N^2/t) */
	return pow(aperture, 2) / pow(2, exposure);
}

double aperture (int exposure, int shutter) {
	/* EV = log2 (N^2/t) */
	return sqrt(pow(2, exposure) * shutter);
}

int nearest_match (double x, int menu) {
	/* Need to search array for closest match */
	int n;
	int diff_idx = 0;
	char array_value_str[9];
	double array_value_db;
	double diff;

	/* Need a starting value for difference */
	switch(menu) {
		case 1:
			array_value_db = strtod(iso_array[0], NULL);
			break;
		case 2:
			array_value_db = fraction_to_double(shutter_array[0]);
			break;
		case 3:
			strncpy(array_value_str, aperture_array[0]+2, 3);
			array_value_db = strtod(array_value_str, NULL);
			break;
	}
	diff = abs(array_value_db - x);
	/* lots of repetition here but pointers to arrays seem to be a bad thing */
	switch(menu) {
		case 1:
			for ( n = 1; iso_array[n] != NULL; ++n ) {
				array_value_db = strtod(iso_array[n], NULL);
				if (abs(array_value_db - x) < diff) { 
					diff_idx = n;
					diff = abs(array_value_db - x);
				}
			}
			break;
		case 2:
			for ( n = 1; iso_array[n] != NULL; ++n ) {
				array_value_db = fraction_to_double(shutter_array[n]);
				if (abs(array_value_db - x) < diff) {
					diff_idx = n;
					diff = abs(array_value_db - x);
				}
			}
			break;
		case 3:
			for ( n = 1; iso_array[n] != NULL; ++n ) {
				strncpy(array_value_str, aperture_array[0]+2, 3);
				array_value_db = strtod(array_value_str, NULL);
				if (abs(array_value_db - x) < diff) { 
					diff_idx = n;
					diff = abs(array_value_db - x);
				}
			}
			break;
	}
	return diff_idx;
}

double fraction_to_double(char *fraction) {
	double fraction_as_db;
	char denominator[9];
	char *ptr = strchr(fraction, "/");

	if (ptr) {
		/*then split*/
		strncpy(denominator, fraction+2, 5);
		fraction_as_db = 1/strtod(denominator, NULL);
	}
	else {
		fraction_as_db = strtod(fraction, NULL);
	}
return fraction_as_db;
}
