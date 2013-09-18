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
	"OVER",
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
	"UNDER",
	NULL
};

char *aperture_array[] = {
	"UNDER",
	"f/1.4",
	"f/2",
	"f/2.8",
	"f/4",
	"f/5.6",
	"f/8",
	"f/11",
	"f/16",
	"OVER",
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
void remove_menu(ITEM **items, MENU *men, int n);
void print_in_middle(WINDOW *win, int starty, int startx, int width, char *string, chtype color);
MENU *add_menu(ITEM **items);
WINDOW *add_window(int xpos, char *title);
ITEM **add_item(char **array, int n);
int exposure(int iso);
double shutter(int exposure, double aperture);
double aperture(int exposure, double shutter);
int nearest_match(double x, int menu, int n_array);
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
	/* Can't really avoid finding array sizes here */
	n_exposure = ARRAY_SIZE(exposure_array);
	n_iso = ARRAY_SIZE(iso_array);
	n_shutter = ARRAY_SIZE(shutter_array);
	n_aperture = ARRAY_SIZE(aperture_array);
	exposure_items = add_item(exposure_array, n_exposure);
	iso_items = add_item(iso_array, n_iso);
	aperture_items = add_item(aperture_array, n_aperture);
	shutter_items = add_item(shutter_array, n_shutter);
	exposure_menu = add_menu(exposure_items);
	iso_menu = add_menu(iso_items);
	shutter_menu = add_menu(shutter_items);
	aperture_menu = add_menu(aperture_items);
	exposure_win = add_window(4, "EV");
	iso_win = add_window(45, "ISO");
	shutter_win = add_window(86, "Shutter");
	aperture_win = add_window(127, "Aperture");
	/* Don't know how to avoid the repition below */
	set_menu_win(exposure_menu, exposure_win);
	set_menu_win(iso_menu, iso_win);
	set_menu_win(shutter_menu, shutter_win);
	set_menu_win(aperture_menu, aperture_win);
	set_menu_sub(exposure_menu, derwin(exposure_win, 6, 38, 3, 1));
	set_menu_sub(iso_menu, derwin(iso_win, 6, 38, 3, 1));
	set_menu_sub(shutter_menu, derwin(shutter_win, 6, 38, 3, 1));
	set_menu_sub(aperture_menu, derwin(aperture_win, 6, 38, 3, 1));
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
	mvprintw(LINES - 1, 0, "Arrow keys to navigate, Enter to select, Q to exit");
	attroff(COLOR_PAIR(2));
	refresh();

	/* Set default menu */
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
				/* Learning notes - Don't understand this bit. Is this a function pointer? */
				p((char *)item_name(cur));
				/* Need to igore over/under if selected, probably easier than try to prevent selection */
				if (!((strcmp("OVER", shutter_sel) == 0)
						|| (strcmp("UNDER", shutter_sel) == 0) 
						|| (strcmp("OVER", aperture_sel) == 0) 
						|| (strcmp("UNDER", aperture_sel) == 0))) {
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
										3,
										n_shutter
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
										4,
										n_aperture
									)]
								);
								menu_driver(aperture_menu, REQ_DOWN_ITEM);
								menu_driver(aperture_menu, REQ_UP_ITEM);
								wrefresh(aperture_win);
							}
							/* Clear the selections for next time */
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
				/* If over/under need to clear selection so know which is blank when a proper selection is made */
				if ((strcmp("OVER", shutter_sel) == 0) || (strcmp("UNDER", shutter_sel) == 0)) {
					strcpy(shutter_sel, "");
				}
				if ((strcmp("OVER", aperture_sel) == 0) || (strcmp("UNDER", aperture_sel) == 0)) {
					strcpy(aperture_sel, "");
				}
			}
			break;
		}
		wrefresh(*win);
	}	
	/* Unpost and free all the memory taken up */
	remove_menu(exposure_items, exposure_menu, n_exposure);
	remove_menu(iso_items, iso_menu, n_iso);
	remove_menu(shutter_items, shutter_menu, n_shutter);
	remove_menu(aperture_items, aperture_menu, n_aperture);
	endwin();
}

/* Below pointer configuration figured out from:
 * https://github.com/Yurickh/Psycho-Tetris/blob/master/src/menu.c#L9 */
ITEM **add_item(char **array, int n) {
	int i;
	ITEM **local_items;

	local_items = (ITEM **)calloc(n, sizeof(ITEM *));
	for(i = 0; i<n; ++i) {
		local_items[i] = new_item(array[i], array[i]);
		set_item_userptr(local_items[i], selection);
	}
	return local_items;
}

MENU *add_menu(ITEM **items) {
	MENU *local_menu;

	local_menu = new_menu((ITEM **)items);
	set_menu_format(local_menu, 5, 1);
	set_menu_mark(local_menu, " * ");
	return local_menu;
}

WINDOW *add_window(int xpos, char *title) {
	WINDOW *local_win;

	local_win = newwin(10, 40, 4, xpos);
	keypad(local_win, TRUE);
	box(local_win, 0, 0);
	print_in_middle(local_win, 1, 0, 40, title, COLOR_PAIR(1));
	mvwaddch(local_win, 2, 0, ACS_LTEE);
	mvwhline(local_win, 2, 1, ACS_HLINE, 38);
	mvwaddch(local_win, 2, 39, ACS_RTEE);
	return local_win;
}

void remove_menu(ITEM **items, MENU *men, int n) {
	int i;

	unpost_menu(men);
	free_menu(men);
	for(i = 0; i < n; ++i)
		free_item(items[i]);
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

int nearest_match (double x, int menu, int n_array) {
	/* Need to search array for closest match */
	int n;
	int diff_idx = 1;
	char array_value_str[9];
	double array_value_db;
	double diff;

	/* Need a starting value for difference */
	switch(menu) {
		case 3:
			array_value_db = fraction_to_double(shutter_array[1]);
			break;
		case 4:
			strncpy(array_value_str, aperture_array[1]+2, 4);
			array_value_db = strtod(array_value_str, NULL);
			break;
	}
	diff = fabs(array_value_db - x);
	/* Lots of repetition here but pointers to arrays seem to be a bad thing */
	switch(menu) {
		case 3:
			for ( n = 2; n < n_array-2; ++n ) {
				array_value_db = fraction_to_double(shutter_array[n]);
				if (fabs(array_value_db - x) < diff) {
					diff_idx = n;
					diff = fabs(array_value_db - x);
				}
			}
			/* Check if at extremities and then if under/over exposed */
			if (diff_idx == 1) {
				if (diff >= fraction_to_double(shutter_array[1])/2) { 
					/* diff is greater than diff of next one down minus max/min */
					diff_idx = 0;
				}
			}
			if (diff_idx == n_array-3) {
				if (diff >= fraction_to_double(shutter_array[n_array-3])*2) {
					diff_idx = n_array-2;	
				}
			}
			break;
		case 4:
			for ( n = 2; n < n_array-2; ++n ) {
				strncpy(array_value_str, aperture_array[n]+2, 4);
				array_value_db = strtod(array_value_str, NULL);
				if (fabs(array_value_db - x) < diff) { 
					diff_idx = n;
					diff = fabs(array_value_db - x);
				}
			}
			/* Apertures similarly. Although progression is fiddlier.*/
			if (diff_idx == 1) {
				strncpy(array_value_str, aperture_array[1]+2, 4);
				array_value_db = strtod(array_value_str, NULL);
				if (diff >= array_value_db/sqrt(2.0)) {
					/* diff is greater than diff of next one down minus max/min */
					diff_idx = 0;
				}
			}
			if (diff_idx == n_array-3) {
				strncpy(array_value_str, aperture_array[n_array-3]+2, 4);
				array_value_db = strtod(array_value_str, NULL);
				if (diff >= array_value_db*sqrt(2.0)) {
					diff_idx = n_array-2;	
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
