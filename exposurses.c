/* This file based on menu_scroll.c from:
http://tldp.org/HOWTO/NCURSES-Programming-HOWTO/intro.html */
#include <curses.h>
#include <menu.h>
#include <math.h>
#include <stdlib.h>

/* Learning notes - This is a macro that is expanded (text substitution) before compiling */
#define ARRAY_SIZE(a) (sizeof(a) / sizeof(a[0]))

char *exposure_array[] = {
	"-6",		
	"-5",		
	"-4",		
	"-3",		
	"-2",		
	"-1",		
	"0",		
	"1",		
	"2",		
	"3",		
	"4",		
	"5",		
	"6",		
	"7",		
	"8",		
	"9",		
	"10",		
	"11",		
	"12",		
	"13",		
	"14",		
	"15",		
	"16",		
	NULL
};

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

ITEM **exposure_items;
ITEM **iso_items;
ITEM **shutter_items;
ITEM **aperture_items;
MENU *exposure_menu;
MENU *iso_menu;
MENU *shutter_menu;
MENU *aperture_menu;
WINDOW *exposure_win;
WINDOW *iso_win;
WINDOW *shutter_win;
WINDOW *aperture_win;

void selection(char *name);
void print_in_middle(WINDOW *win, int starty, int startx, int width, char *string, chtype color);
int exposure(int iso);
double shutter(int exposure, double aperture);
double aperture(int exposure, double shutter);
int nearest_match(double x, int menu);
double fraction_to_double(char *fraction);
/* No one will ever need more than 9 bytes! */
char exposure_sel[9] = "";
char iso_sel[9] = "";
char shutter_sel[9] = "";
char aperture_sel[9] = "";
int selection_counter = 1;
int menu_counter = 1;

int main() {
	int c;
	MENU **menu;
	WINDOW **win;
	int i;
	int n_exposure;
	int n_iso;
	int n_shutter;
	int n_aperture;

	/* Initialize curses */
	initscr();
	start_color();
	cbreak();
	noecho();
	keypad(stdscr, TRUE);
	init_pair(1, COLOR_RED, COLOR_BLACK);
	init_pair(2, COLOR_CYAN, COLOR_BLACK);

	/* Create items */
	/* Lot's of repitition here. Surely can be wrapped in a function */
	n_exposure = ARRAY_SIZE(exposure_array);
	n_iso = ARRAY_SIZE(iso_array);
	n_shutter = ARRAY_SIZE(shutter_array);
	n_aperture = ARRAY_SIZE(aperture_array);
	exposure_items = (ITEM **)calloc(n_exposure, sizeof(ITEM *));
	for(i = 0; i < n_exposure; ++i) {
		exposure_items[i] = new_item(exposure_array[i], exposure_array[i]);
		set_item_userptr(exposure_items[i], selection);
	}
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
	exposure_menu = new_menu((ITEM **)exposure_items);
	iso_menu = new_menu((ITEM **)iso_items);
	shutter_menu = new_menu((ITEM **)shutter_items);
	aperture_menu = new_menu((ITEM **)aperture_items);

	/* Create the window to be associated with the menu */
	exposure_win = newwin(10, 40, 4, 4);
	keypad(exposure_win, TRUE);
	iso_win = newwin(10, 40, 4, 45);
	keypad(iso_win, TRUE);
	shutter_win = newwin(10, 40, 4, 86);
	keypad(shutter_win, TRUE);
	aperture_win = newwin(10, 40, 4, 127);
	keypad(aperture_win, TRUE);

	/* Set main window and sub window */
	set_menu_win(exposure_menu, exposure_win);
	set_menu_sub(exposure_menu, derwin(exposure_win, 6, 38, 3, 1));
	set_menu_win(iso_menu, iso_win);
	set_menu_sub(iso_menu, derwin(iso_win, 6, 38, 3, 1));
	set_menu_win(shutter_menu, shutter_win);
	set_menu_sub(shutter_menu, derwin(shutter_win, 6, 38, 3, 1));
	set_menu_win(aperture_menu, aperture_win);
	set_menu_sub(aperture_menu, derwin(aperture_win, 6, 38, 3, 1));
	set_menu_format(exposure_menu, 5, 1);
	set_menu_format(iso_menu, 5, 1);
	set_menu_format(shutter_menu, 5, 1);
	set_menu_format(aperture_menu, 5, 1);

	/* Set menu mark to the string " * " */
	set_menu_mark(exposure_menu, " * ");
	set_menu_mark(iso_menu, " * ");
	set_menu_mark(shutter_menu, " * ");
	set_menu_mark(aperture_menu, " * ");

	/* Print a border around the main window and print a title */
	box(exposure_win, 0, 0);
	box(iso_win, 0, 0);
	box(shutter_win, 0, 0);
	box(aperture_win, 0, 0);
	print_in_middle(exposure_win, 1, 0, 40, "EV", COLOR_PAIR(1));
	print_in_middle(iso_win, 1, 0, 40, "ISO", COLOR_PAIR(1));
	print_in_middle(shutter_win, 1, 0, 40, "Shutter", COLOR_PAIR(1));
	print_in_middle(aperture_win, 1, 0, 40, "Aperture", COLOR_PAIR(1));
	mvwaddch(exposure_win, 2, 0, ACS_LTEE);
	mvwaddch(iso_win, 2, 0, ACS_LTEE);
	mvwaddch(shutter_win, 2, 0, ACS_LTEE);
	mvwaddch(aperture_win, 2, 0, ACS_LTEE);
	mvwhline(exposure_win, 2, 1, ACS_HLINE, 38);
	mvwhline(iso_win, 2, 1, ACS_HLINE, 38);
	mvwhline(shutter_win, 2, 1, ACS_HLINE, 38);
	mvwhline(aperture_win, 2, 1, ACS_HLINE, 38);
	mvwaddch(exposure_win, 2, 39, ACS_RTEE);
	mvwaddch(iso_win, 2, 39, ACS_RTEE);
	mvwaddch(shutter_win, 2, 39, ACS_RTEE);
	mvwaddch(aperture_win, 2, 39, ACS_RTEE);

	/* Post the menu */
	post_menu(exposure_menu);
	post_menu(iso_menu);
	post_menu(shutter_menu);
	post_menu(aperture_menu);
	wrefresh(exposure_win);
	wrefresh(iso_win);
	wrefresh(shutter_win);
	wrefresh(aperture_win);

	attron(COLOR_PAIR(2));
	mvprintw(LINES - 2, 0, "Select EV");
	/*mvprintw(LINES - 2, 0, "Select ISO and then one of Shutter/Aperture to calculate other of Shutter/Aperture");*/
	mvprintw(LINES - 1, 0, "Arrow keys to navigate, Enter to select, Q to exit");
	attroff(COLOR_PAIR(2));
	refresh();

	/* set default menu */
	menu = &exposure_menu;
	win = &exposure_win;

	while((c = getch()) != 81) { /* 81 is Q */
		switch(c) {
			case KEY_LEFT:
				if (selection_counter > 2) {
					menu_counter = 3;
					menu = &shutter_menu;
					win = &shutter_win;
				}
				break;
			case KEY_RIGHT:
				if (selection_counter > 2) {
					menu_counter = 4;
					menu = &aperture_menu;
					win = &aperture_win;
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
				/* Learning notes - Don't understand this bit */
				p((char *)item_name(cur));
				
				switch (selection_counter) {
					case 1: { /* Exposure selected */
						selection_counter += 1;
						menu_counter += 1;
						move(LINES - 2, 0);
						clrtoeol();
						mvprintw(LINES - 2, 0, "Select ISO");
						refresh();
						menu = &iso_menu;
						win = &iso_win;
					}
					break;
					case 2: { /* ISO Selected */
						selection_counter += 1;
						menu_counter += 1;
						move(LINES - 2, 0);
						clrtoeol();
						mvprintw(LINES - 2, 0, "Select Shutter or Aperture");
						refresh();
						menu = &shutter_menu;
						win = &shutter_win;
					}
					break;
					case 3: { /* Shutter or Aperture selected */
						if (strcmp("", shutter_sel) == 0) {
							char aperture_sel_[4] = "";
							strncpy(aperture_sel_, aperture_sel+2, 3);
							/* Using menu_driver to go up/down to force refresh and correct highlighting */
							menu_driver(shutter_menu, REQ_SCR_UPAGE);
							menu_driver(shutter_menu, REQ_SCR_DPAGE);
							/* There is probably a nicer way to format the below */
							set_menu_pattern(
								shutter_menu,
								shutter_array[nearest_match(
									shutter(exposure(atoi(iso_sel)), strtod(aperture_sel_, NULL)),
									2
								)]
							);
							menu_driver(shutter_menu, REQ_DOWN_ITEM);
							menu_driver(shutter_menu, REQ_UP_ITEM);
							wrefresh(shutter_win);
						}
						if (strcmp("", aperture_sel) == 0) {
							menu_driver(aperture_menu, REQ_SCR_UPAGE);
							menu_driver(aperture_menu, REQ_SCR_DPAGE);
							set_menu_pattern(
								aperture_menu,
								aperture_array[nearest_match(
									aperture(exposure(atoi(iso_sel)), fraction_to_double(shutter_sel)),
									3
								)]
							);
							menu_driver(aperture_menu, REQ_DOWN_ITEM);
							menu_driver(aperture_menu, REQ_UP_ITEM);
							wrefresh(aperture_win);
						}
						/* clear the selections for next time */
						strcpy(iso_sel, "");
						strcpy(shutter_sel, "");
						strcpy(aperture_sel, "");
						/* And set defaults back to start */
						selection_counter = 1;
						menu_counter = 1;
						menu = &exposure_menu;
						win = &exposure_win;
						move(LINES - 2, 0);
						clrtoeol();
						mvprintw(LINES - 2, 0, "Select EV");
						refresh();
					}
					break;
				}
			}
			break;
		}
		wrefresh(*win);
	}	
	/* Unpost and free all the memory taken up */
	unpost_menu(exposure_menu);
	unpost_menu(iso_menu);
	unpost_menu(shutter_menu);
	unpost_menu(aperture_menu);
	free_menu(exposure_menu);
	free_menu(iso_menu);
	free_menu(shutter_menu);
	free_menu(aperture_menu);
	for(i = 0; i < n_exposure; ++i)
		free_item(iso_items[i]);
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
			strcpy(exposure_sel, name);
			break;
		case 2:
			strcpy(iso_sel, name);
			break;
		case 3:
			strcpy(shutter_sel, name);
			break;
		case 4:
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

double aperture (int exposure, double shutter) {
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
			strncpy(array_value_str, aperture_array[0]+2, 4);
			array_value_db = strtod(array_value_str, NULL);
			break;
	}
	diff = fabs(array_value_db - x);
	/* lots of repetition here but pointers to arrays seem to be a bad thing */
	switch(menu) {
		case 1:
			for ( n = 1; iso_array[n] != NULL; ++n ) {
				array_value_db = strtod(iso_array[n], NULL);
				if (fabs(array_value_db - x) < diff) { 
					diff_idx = n;
					diff = fabs(array_value_db - x);
				}
			}
			break;
		case 2:
			for ( n = 1; shutter_array[n] != NULL; ++n ) {
				array_value_db = fraction_to_double(shutter_array[n]);
				if (fabs(array_value_db - x) < diff) {
					diff_idx = n;
					diff = fabs(array_value_db - x);
				}
			}
			break;
		case 3:
			for ( n = 1; aperture_array[n] != NULL; ++n ) {
				strncpy(array_value_str, aperture_array[n]+2, 4);
				array_value_db = strtod(array_value_str, NULL);
				if (fabs(array_value_db - x) < diff) { 
					diff_idx = n;
					diff = fabs(array_value_db - x);
				}
			}
			break;
	}
	return diff_idx;
}

double fraction_to_double(char *fraction) {
	double fraction_as_db;
	char denominator[9];
	char *ptr = strstr(fraction, "/");

	if (ptr) {
		/*then split*/
		strncpy(denominator, fraction+2, 5);
		fraction_as_db = 1 / strtod(denominator, NULL);
	}
	else {
		fraction_as_db = strtod(fraction, NULL);
	}
	return fraction_as_db;
}

/* Debug lines
 * sprintf(temp, "%f", x);
 * mvprintw(LINES - 4, 0, temp);
 * refresh();*/
