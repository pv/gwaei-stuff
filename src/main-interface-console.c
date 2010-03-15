/******************************************************************************
    AUTHOR:
    File written and Copyrighted by Zachary Dovel. All Rights Reserved.

    LICENSE:
    This file is part of gWaei.

    gWaei is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    gWaei is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.
    
    You should have received a copy of the GNU General Public License
    along with gWaei.  If not, see <http://www.gnu.org/licenses/>.
*******************************************************************************/

//!
//! @file src/main-interface-console.c
//!
//! @brief Abstraction layer for the console
//!
//! Used as a go between for functions interacting with the console.
//!


#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <libintl.h>
#include <regex.h>

#include <glib.h>

#include <gwaei/definitions.h>
#include <gwaei/regex.h>
#include <gwaei/dictionary-objects.h>
#include <gwaei/search-objects.h>
#include <gwaei/engine.h>
#include <gwaei/preferences.h>

gboolean quiet_switch = FALSE;
gboolean exact_switch = FALSE;

/* NCURSES */

#include <curses.h>
#include <errno.h>

static WINDOW *mainWindows;
static WINDOW *results;
static WINDOW *search;
static WINDOW *screen;
int current_row = 0;
int maxY, maxX;
int cursesFlag = false;
int cursesSupportColorFlag = true;	//TODO: use this

#define GREENONBLACK	1
#define BLUEONBLACK		2
#define REDONBLACK		3

/**
 * Print the "less relevant" header where necessary.
 */
void gw_console_append_less_relevant_header_to_output(){
	if (cursesFlag) {
		wattron(results, COLOR_PAIR(REDONBLACK));
		wprintw(results,"\n*** ");
		wattroff(results, COLOR_PAIR(REDONBLACK));
		wprintw(results,"%s ", gettext("Other Results"));
		wattron(results, COLOR_PAIR(REDONBLACK));
		wprintw(results,"***************************\n\n\n");
		wattroff(results, COLOR_PAIR(REDONBLACK));
	}
	else
		printf("\n[0;31m***[0m[1m%s[0;31m***************************[0m\n\n\n", gettext("Other Results"));
}

/**
 * Print the "no result" message where necessary.
 */
void gw_console_no_result(){
	if (cursesFlag)
		wprintw(results,"%s\n\n", gettext("No results found!"));
	else
		printf("%s\n\n", gettext("No results found!"));
}


/**
 *
 */
void gw_console_uninstall_dictionary_by_name(char *name) {

    GwDictInfo* di;
    di = gw_dictlist_get_dictionary_by_name (name);
    if (di == NULL) return;

    gw_io_delete_dictionary_file(di);
    di->status = GW_DICT_STATUS_NOT_INSTALLED;
    di->load_position = -1;

    if (di->id == GW_DICT_ID_KANJI || di->id == GW_DICT_ID_RADICALS)
      gw_console_uninstall_dictionary_by_name ("Mix");
    else if (di->id == GW_DICT_ID_NAMES)
      gw_console_uninstall_dictionary_by_name ("Places");
}


gboolean gw_console_install_dictionary_by_name(char *name) {

    GwDictInfo* di;
    di = gw_dictlist_get_dictionary_by_alias(name);

    char *path = di->path;
    char *sync_path = di->sync_path;
    char *gz_path = di->gz_path;

    if (di->status != GW_DICT_STATUS_NOT_INSTALLED || di->id == GW_DICT_ID_PLACES) return FALSE;
    di->status = GW_DICT_STATUS_INSTALLING;
    
    char fallback_uri[100];
    gw_util_strncpy_fallback_from_key (fallback_uri, di->gckey, 100);
 
    char uri[100];
    gw_pref_get_string (uri, di->gckey, fallback_uri, 100);

    //Make sure the download folder exits
    char download_path[FILENAME_MAX] = "";
    gw_util_get_waei_directory(download_path);
    strcat(download_path, G_DIR_SEPARATOR_S);
    strcat(download_path, "download");

    gboolean ret = TRUE;

    if (ret)
      ret = ((g_mkdir_with_parents(download_path, 0755)) == 0);

    //Copy the file if it is a local file
    if (ret && g_file_test (uri, G_FILE_TEST_IS_REGULAR))
    {
      printf("  %s\n", gettext("Copying file..."));
      ret = gw_io_copy_dictionary_file (uri, gz_path, NULL);
    }
    else if (ret)
    {
      printf("* %s\n", gettext("Downloading file..."));
      ret = gw_io_download_dictionary_file (uri, gz_path, NULL, NULL);
    }

    if (ret)
    {
      printf("* %s\n", gettext("Gunzipping file..."));
      ret = gw_io_gunzip_dictionary_file(gz_path, NULL);
    }
   
    if (ret)
    {
      printf("* %s\n",gettext("Converting encoding..."));
      ret = gw_io_copy_with_encoding(sync_path, path, "EUC-JP","UTF-8");
    }

    if (ret)
      di->status = GW_DICT_STATUS_INSTALLED;
    else
      di->status = GW_DICT_STATUS_NOT_INSTALLED;

    //Special dictionary post processing
    if (ret)
    {
      printf("* %s\n", gettext("Postprocessing..."));
      gw_dictlist_preform_postprocessing_by_name(name);
    }

    return ret;
}


static void print_about_program ()
{
    printf ("[1;31m%s Version %s[0m with ", PACKAGE, VERSION);
    if (strcmp (INTERFACE, "NONE") == 0)
      printf ("no");
    else if (strcmp (INTERFACE, "gwaei"))
      printf("the Gnome");
    printf (" front end compiled in.\n\n");
    printf ("[0mCheck for the latest updates at <http://gwaei.sourceforge.net/>\n");
    printf ("Code Copyright (C) 2009 Zachary Dovel[0m\n\n");

    printf ("License:\n");
    printf ("Copyright (C) 2008 Free Software Foundation, Inc.\nLicense GPLv3+: GNU GPL version 3 or later <http://gnu.org/licenses/gpl.html>\nThis is free software: you are free to change and redistribute it.\nThere is NO WARRANTY, to the extent permitted by law.\n\n");
}

//!
//! @brief This function print the start banner
//!
//! This function print the start banner in both
//! simple and ncurses interface.
//!
//! @param query The query string we are searching
//! @param dictionary The name (string) of the dictionary used
//!
static void print_search_start_banner(char *query, char *dictionary)
{
	if (cursesFlag)
	{
		wprintw(screen, " ");
		wprintw(screen, gettext("Searching for \""));
		wattron(screen, COLOR_PAIR(REDONBLACK));
		wprintw(screen,"%s", query);
		wattroff(screen, COLOR_PAIR(REDONBLACK));
		wprintw(screen, gettext("\" in the "));
		wattron(screen, COLOR_PAIR(REDONBLACK));
		wprintw(screen,"%s", dictionary);
		wattroff(screen, COLOR_PAIR(REDONBLACK));
		wprintw(screen, gettext(" dictionary..."));

		wrefresh(screen);
		refresh();
	}
	else
	{
		printf("");
		printf("%s", gettext("Searching for \""));
		printf("[1;31m%s[0m", query);
		printf("%s", gettext("\" in the "));
		printf("[1;31m%s[0m", dictionary);
		printf("%s", gettext(" dictionary..."));
		printf("[0m\n\n");
	}

    #ifdef G_OS_UNIX
    sleep(1);
    #endif
}

static void print_installable_dictionaries()
{
    if (quiet_switch == FALSE)
    {
      printf("[1;31m");
      printf("%s ", gettext("Installable:"));
      printf("[0m");
    }

    int i = 0; 

    GwDictInfo* di;
    GList *list = gw_dictlist_get_list();
    while (list != NULL)
    {
      di = list->data;
      if (di->gckey[0] != '\0' && di->status == GW_DICT_STATUS_NOT_INSTALLED)
      {
        if (i != 0) printf(" ");
        printf("%s", di->name);
        i++;
      }
      list = list->next;
    }

    if (i == 0 && quiet_switch == FALSE)
      printf(gettext("All dictionaries are already installed"));

    printf ("\n");
}

/*
 *
 */
//[1;31m	RED
//[0m		BLACK
static void printRed(char* string) {
	printf ("[1;31m%s[0m", string);
}

/*
 *
 * -u (alone)
 */
static void print_uninstallable_dictionaries() {

	if (quiet_switch == FALSE)
    	printRed(gettext("Uninstallable:"));

    int i = 0; 

    GwDictInfo* di;
    GList *list = gw_dictlist_get_list();
    while (list != NULL) {
      di = list->data;
      if (di->status == GW_DICT_STATUS_INSTALLED) {
        printf(" %s", di->name);
        i++;
      }
      list = list->next;
    }

    if (i == 0 && quiet_switch == FALSE) {
    	printf(" ");
    	printf(gettext("There are no dictionaries installed"));
    }

    printf ("\n");
}

/*
 *
 * -i (alone)
 */
static void print_available_dictionaries() {

    int i = 0;
    GwDictInfo* di;
    GList *list;

	if (quiet_switch == FALSE)
    	printRed(gettext("Available:"));

	list = gw_dictlist_get_list();
    while (list != NULL) {
      di = list->data;
      if (di->status == GW_DICT_STATUS_INSTALLED) {
        printf(" %s", di->name);
        i++;
      }
      list = list->next;
    }

    if (i == 0)
      printf(gettext(" There are no dictionaries available"));

    printf ("\n");
}


static void print_help_message() {

    printf("%s\n\n", gettext("waei [options]... pattern"));
    printf("%s\n", gettext("This is a Japanese-English dictionary program that allows regex style searches.\nThe dictionaries include: English, Places, Names, Radicals and Kanji. Periods\ncan be used in place of unknown kanji."));

    printf("\n%s\n", gettext("OPTIONS:"));
    printf("%s\n", gettext("  -m                         "
                   "Open up the multisearch window (beta)"));

    printf("%s\n", gettext("  -e, --exact                "
                   "Do not display less relevant results"));

    printf("%s\n", gettext("  -d, --dictionary name      "
                   "Search using a chosen dictionary"));

    printf("%s\n", gettext("  -l, --list                 "
                   "Show available dictionaries for searches"));

    printf("%s\n", gettext("  -i, --install              "
                   "List installable dictionaries"));

    printf("%s\n", gettext("  -i, --install dictionary   "
                   "Install dictionary"));

    printf("%s\n", gettext("  -u, --uninstall            "
                   "List uninstallable dictionaries"));

    printf("%s\n", gettext("  -u, --uninstall dictionary "
                   "Uninstall dictionary"));

    printf("%s\n", gettext("  -s, --sync                 "
                   "Sync installed dictionaries using rsync"));

    printf("%s\n", gettext("  -v, --version              "
                   "Check the waei version info"));

    printf("%s\n", gettext("  -q, --quiet                "
                   "Display less information"));

    printf("%s\n", gettext("  -h, --help                 "
                   "Display this help"));

    printf("\n%s\n", gettext("EXAMPLES:"));

    printf("%s\n", gettext("  waei English               "
                   "Search for the english word English"));

    printf("%s\n", gettext("  waei \"cats&dogs\"           "
                   "Search for results containing cats and dogs"));

    printf("%s\n", gettext("  waei \"cats|dogs\"           "
                   "Search for results containing cats or dogs"));

    printf("%s\n", gettext("  waei cats dogs             "
                   "Search for results containing \"cats dogs\""));

    printf("%s日本語%s日本語\n", gettext("  waei "), gettext("                "
                    "Search for the Japanese word "));

    printf("%s日本%s日本%s\n", gettext("  waei -e "), gettext("               "
                    "Search for "), gettext(" ignoring similar results"));

    printf("%s日.語%s\n", gettext("  waei "), gettext("                 "
                   "When you don't know the middle character"));

    printf("%s伍%s\n", gettext("  waei -d Kanji "), gettext("           "
                   "Find a kanji character in the kanji database"));

    printf("%s田中%s\n", gettext("  waei -d Names "), gettext("         "
                   "Look up a name in the name database"));

    printf("%s秋葉原%s\n", gettext("  waei -d Places "), gettext("      "
                   "Look up a place in the place database"));
}

/*
 *
 */
static gboolean is_switch (char *arg, char *short_switch, char *long_switch) {
  return (strcmp(arg, short_switch) == 0 || strcmp(arg, long_switch) == 0);
}




/*** NCURSES ***/


//!
//! @brief Color Initialization
//!
//! The pairs of colors that will be used in the printing
//! functions are initialized here.
//!
//! @param hasColors True if the shell support colors
//!
static void ncurses_color_init(bool hasColors)
{
	int check;	//Check flag
	if (hasColors)
	{
		check = start_color();
		if (check == ERR){
			cursesSupportColorFlag = false;
			return;
		}
		init_pair(GREENONBLACK, COLOR_GREEN, COLOR_BLACK);
		init_pair(BLUEONBLACK, COLOR_BLUE, COLOR_BLACK);
		init_pair(REDONBLACK, COLOR_RED, COLOR_BLACK);
	}
	else
		cursesSupportColorFlag = false;

	return;
}


//!
//! @brief Create a box around a WINDOW and write a title
//!
//! Create a box around a WINDOW and write a title
//!
//! @param thisWin The window we want a box around
//! @param intro A string that we want to be title of the box
//!
static void ncurses_add_intro_and_box (WINDOW* thisWin, char* intro)
{
	//Draw  a box around the edges of a window
	box(thisWin, ACS_VLINE, ACS_HLINE);

	wattron(thisWin,A_BOLD);
	mvwprintw(thisWin,0,2,intro);
	wattroff(thisWin,A_BOLD);

	wrefresh(thisWin);
	wrefresh(stdscr);

	return;
}


//!
//! @brief Screen Initialization
//!
//! The main screen is initialized and configured.
//!
static int ncurses_screen_init()
{

	int check, checkSecond;	//Check flag

	mainWindows = initscr();
	if (mainWindows == NULL)
	{
		printf("The main screen cannot be initialized. Exiting...\n");
		return(ERR);
	}

	check = cbreak();
	checkSecond = nodelay(mainWindows, TRUE);
	if ((check == ERR) || (checkSecond == ERR))
	{
		printf("The main screen cannot be configured. Exiting...\n");
		return(ERR);
	}

	ncurses_color_init(has_colors());
	curs_set(0);						/*< No cursor */

	wrefresh(mainWindows);

	getmaxyx(mainWindows, maxY, maxX);

	//alt rett, larg rett, quanto in basso, quanto a destra
	search = newwin(3, maxX, (maxY - 3), 0);
	screen = newwin((maxY - 3), maxX, 0, 0);
	if ((search == NULL) || (screen == NULL))
	{
		printf("The secondary screens cannot be initialized. Exiting...\n");
		return(ERR);
	}

	ncurses_add_intro_and_box(search,"Search:");
	ncurses_add_intro_and_box(screen,"Results:");

	results = newpad(500, (maxX - 2));
	if (results == NULL)
	{
		printf("The pad cannot be initialized.\n");
		if (errno == ENOMEM)
			strerror(errno);
		return(ERR);
	}

	cursesFlag = TRUE;	//All good. We are using the ncurses now.

	return (OK);
}


//!
//! @brief Manage the scrolling and the search input
//!
//! This function controls how the scrolling work and the
//! data insertion.
//!
//!	TODO: Manage the scrolling DOWN
//! TODO: Check result
//!
//! @param query Pointer to the query string
//!
static void ncurses_input_and_scrolling(char *query)
{
	// mousemask(ALL_MOUSE_EVENTS, NULL);	<-- IT WORKS WITHOUT THIS
	// wgetnstr(search, query, 250);		<-- If we use this we cannot scroll

	int scrollingControl = 0;	/*< Needed to check the scrolling up */
	int stringControl = 0;		/*< Segfault control */
	wchar_t singleChar;
	keypad(search, TRUE);		/*<  */

	while(true)
	{
		if (stringControl == (MAX_QUERY - 1))
		{
			query[stringControl] = '\0';
			return;
		}
		noecho();
		cbreak(); //characters will be returned one at a time instead of waiting for a newline character
		singleChar = wgetch(search);
		switch(singleChar)
		{
			case KEY_PPAGE:
			case KEY_UP:
				scrollingControl--;
				if (scrollingControl < 0)
					scrollingControl = 0;
				prefresh(results,scrollingControl,0,2,2,(maxY - 5), (maxX - 2));
				break;
			case KEY_NPAGE:
			case KEY_DOWN:
				scrollingControl++;
				prefresh(results,scrollingControl,0,2,2,(maxY - 5), (maxX - 2));
				break;
			case KEY_DC:
			case KEY_LEFT:
			case KEY_RIGHT:
				break;
			case KEY_BACKSPACE:
				if (stringControl > 0)
				{ //SEGFAULT CHECK
					stringControl--;
					query[stringControl] = '\0';
					//wdelch(search); <--- Doesn't work well. So... let's ack!
					wclear(search);
					ncurses_add_intro_and_box(search,"Search:");
					wmove(search, 1, 2);
					wprintw(search,query);
				}
				break;
			default:
				echo();
				waddch(search, singleChar);
				query[stringControl] = singleChar;
				if (singleChar == '\n' || singleChar == '\0')
					return;
				stringControl++;
		}
	}
}


//!
//! @brief NCURSES Main function
//!
//! TODO: Show the chosen dictionary and search option
//! TODO: Accept a !quit/!q like vim?
//! TODO: Check result
//!
//! @param dictionary The dictionary we want to use
//!
void initialize_ncurses_interface (GwDictInfo *dictionary)
{
	GError *error = NULL;	//UNUSED
	GwSearchItem *item;
	char query[MAX_QUERY];
	int cont, loop, test;
	char *fgetsTest;
	char *thisArg = NULL;

	test = ncurses_screen_init();
	if (test == ERR)
		return;

	loop = TRUE;		//When False close the program

	while(loop)
	{

		wmove(search, 1, 2);
		ncurses_input_and_scrolling(query);
	    wmove(search, 1, 2);

		ncurses_add_intro_and_box(screen,"Results:");
		wclear(results);

		//If there is nothing, exit
		if ((query[0] == '\n') || (query[0] == '\0'))
		{
			loop = FALSE;
			break;
		}

		for (cont = 0; cont < sizeof(query); ++cont)
		{
			if (query[cont] == '\n')
			{
				query[cont] = '\0';
				break;
			}
		}

		// Search for keyword...

		thisArg = NULL;
		thisArg = strtok (query," ");
		test = false;
		while (thisArg != NULL)
		{
			if (test)
			{
				test = false;
				dictionary = gw_dictlist_get_dictionary_by_alias(thisArg);
				if (dictionary == NULL || dictionary->status != GW_DICT_STATUS_INSTALLED)
				{
					wattron(results, COLOR_PAIR(REDONBLACK));
					wprintw(results, gettext("Requested dictionary not found!\n"));
					wprintw(results, gettext("Press any key to exit.\n"));
					wattroff(results, COLOR_PAIR(REDONBLACK));
					prefresh(results,0,0,2,2,(maxY - 5), (maxX - 2));
					wgetch(search);
					endwin();
					return;
				}
			}
			else if (is_switch (thisArg, "-e", "--exact"))
				exact_switch = TRUE;
			else if (is_switch (thisArg, "-q", "--quiet"))
				quiet_switch = TRUE;
			else if (is_switch (thisArg, "-d", "--dictionary"))
			{
				  test = true;
			}
			else
				break; //Search only the first word inserted

			thisArg = strtok (NULL, " ,");
		}


		//TODO: Extract all the spaces

		if(thisArg == NULL)
		{
			/* No word to search, only option */
			wattron(results, COLOR_PAIR(REDONBLACK));
			wprintw(results, gettext("No word inserted. Only option."));
			wattroff(results, COLOR_PAIR(REDONBLACK));
			prefresh(results,0,0,2,2,(maxY - 5), (maxX - 2));
		}
		else
		{

			if (quiet_switch == FALSE)
				print_search_start_banner(thisArg, dictionary->name);

			item = gw_searchitem_new(thisArg, dictionary, GW_TARGET_CONSOLE);
			if (item == NULL)
			{
				wprintw(screen, "Insufficient storage space is available. Please close something and retry.");
				wrefresh(screen);
				cbreak(); wgetch(search); endwin();
				exit (EXIT_FAILURE);
			}

			item->show_less_relevant_results = !exact_switch;

								/*
									WINDOW* subWin;
									subWin = subwin(results, (maxY - 10), (maxX - 10), 2, 2);
									scrollok(subWin,1);
									touchwin(results);
									refresh();
									werase(subWin);
									results = subWin; //Fin qui funziona

									WINDOW *subbb = subpad(pad, (maxY - 10), (maxX - 10), 2, 2);
									scrollok(subbb,1);
									touchwin(pad);
									refresh();
								*/

			gw_search_get_results (item); //TODO: Print here?? <---
			if (item == NULL)
				wprintw(results, "Something went wrong. Please repeat the search");
			else
			{

				//Print the number of results
				if (quiet_switch == FALSE)
				{
					wclear(screen);
					ncurses_add_intro_and_box(screen,"Results:");
					wprintw(screen, " %s%d%s", gettext("Found "), item->total_results, gettext(" Results"));

					if (item->total_relevant_results != item->total_results)
						wprintw(screen, "%s%d%s", gettext(" ("), item->total_relevant_results, gettext(" Relevant)"));

					wrefresh(screen);
					refresh();
				}

				scrollok(results,TRUE);
				prefresh(results,0,0,2,2,(maxY - 5), (maxX - 2));

				wclear(search);
				ncurses_add_intro_and_box(search,"Search:");

				if (exact_switch == TRUE)
					wprintw(search, " EXACT ");
				if (quiet_switch == TRUE)
					wprintw(search, " QUIET ");

				gw_searchitem_free(item);
			}
		}
	}

	endwin();

	printf("Bye...\n");

	return;
}

//!
//! @brief CONSOLE Main function
//!
//! This function will NOT use GError because it have
//! nobody to report to.
//!
//! @param argc Standard argc from main
//! @param argv Standard argv from main
//!
void initialize_console_interface(int argc, char **argv)
{

	GError *error = NULL;

	int thisArg, leftover, total_args;
	char query[MAX_QUERY];
	char *args[argc];

	GwDictInfo *di;
	di = gw_dictlist_get_dictionary_by_alias("English");
	//Make sure the selected dictionary exists
	if (di == NULL || di->status != GW_DICT_STATUS_INSTALLED) {
		printf(gettext("Requested dictionary not found!\n"));
		return;
	}

	//Filter out modification arguments
	thisArg = 1;
	total_args = 0;
	while (thisArg < argc) {

		if (is_switch (argv[thisArg], "-e", "--exact"))
		  exact_switch = TRUE;

		else if (is_switch (argv[thisArg], "-q", "--quiet"))
		  quiet_switch = TRUE;

		else if (is_switch (argv[thisArg], "-d", "--dictionary")) {
		  thisArg++;
		  di = gw_dictlist_get_dictionary_by_alias(argv[thisArg]);
		  if (di == NULL || di->status != GW_DICT_STATUS_INSTALLED) {
			printf(gettext("Requested dictionary not found!\n"));
			return;
		  }
		}

		else {
		  args[total_args] = argv[thisArg];
		  total_args++;
		}

		thisArg++;
	}

	//User requests help
	if (total_args == 0 || (total_args == 1 && is_switch (argv[1], "-h", "--help"))){
		print_help_message();
		return;
	}

	//User wants to see the available dictionaries
	if (total_args == 1){

		//TODO: STUB
		if (is_switch (args[0], "-n", "--ncurses")){
			initialize_ncurses_interface(di);
			return;
		}

		if (is_switch (args[0], "-l", "--list")){
			print_available_dictionaries ();
			return;
		}

		//User wants to see the version of waei
		else if (is_switch (args[0], "-v", "--version")){
			print_about_program ();
			return;
		}

		//Show installable dictionaries
		else if (is_switch (args[0], "-i", "--install")){
			print_installable_dictionaries();
			return;
		}

		//Show uninstallable dictionaries
		else if (is_switch (args[0], "-u", "--uninstall")){
			print_uninstallable_dictionaries();
			return;
		}

		//User wants to sync dictionaries
		else if (is_switch (args[0], "-s", "--sync")) {

			printf("");
			printf(gettext("Syncing possible installed dictionaries..."));
			printf("[0m\n");

			GwDictInfo* di;
			GList *list = gw_dictlist_get_list();

			while (list != NULL && error == NULL) {
				di = list->data;
				gw_dictlist_sync_dictionary (di, &error);
				list = list->next;
			}

			if (error == NULL) {
				printf("");
				printf(gettext("Finished"));
				printf("[0m\n");
			}
			else {
				printf("");
				printf("%s", error->message);
				printf("[0m\n");
				g_error_free (error);
				error = NULL;
			}

			return;
		}
	}


	if (total_args == 2){

		//User wants to install dictionary
		if (is_switch (args[0], "-i", "--install")) {

			printf(gettext("%sTrying to install %s%s%s...%s"), "", "[1;31m", args[1], "[0m", "\n[0m");

			di = gw_dictlist_get_dictionary_by_alias(args[1]);

			if (di != NULL && di->status != GW_DICT_STATUS_NOT_INSTALLED && di->gckey[0] != '\0') {
				//printf("");
				printf("%s\n", gettext("Already Installed"));
				//printf("[0m");
			}
			else if (di == NULL || di->gckey[0] == '\0') {
				//printf("");
				printf("%s\n", gettext("That dictionary is not installable with this mechanism"));
				//printf("[0m");
				exit (1);
			}
			else if (gw_console_install_dictionary_by_name (args[1])) {
				printf("");
				printf(gettext("Finished"));
				printf("[0m\n");
			}
			else {
				printf("");
				printf(gettext("Failed"));
				printf("[0m\n");
			}

			return;
		}

		//User wants to uninstall dictionary
		if (is_switch (args[0], "-u", "--uninstall")) {

			printf(gettext("%sTrying to uninstall %s%s%s...%s"), "", "[1;31m", args[1], "[0m", "\n[0m");

			if (gw_dictlist_check_if_loaded_by_name(argv[2])) {
				gw_console_uninstall_dictionary_by_name(argv[2]);
				printf("");
				printf(gettext("Finished"));
				printf("[0m\n");
			}
			else
				printf("%s\n", gettext("Is not installed"));

			return;
		}

	}

	//Collect the query terms
	thisArg = 0;
	query[0] = '\0';
	leftover = MAX_QUERY;

	while (leftover > 0 && thisArg < total_args) {
		strncat(query, args[thisArg], leftover);
		leftover -= strlen (args[thisArg]);
		thisArg++;
		if (thisArg != total_args) {
			strncat (query, " ", leftover);
			leftover -= 1;
		}
	}

	//Print the search intro
	if (quiet_switch == FALSE)
		print_search_start_banner(query, di->name);


	GwSearchItem *item;
	item = gw_searchitem_new(query, di, GW_TARGET_CONSOLE);
	if (item == NULL){
		printf(gettext("Results seem to have incorrect formatting. Did you "
						"close all of your\nparenthesis?  You may want to tr"
						"y quotes too.\n"));
		printf("Or\n");
		printf("Out of memory.\n Exiting.\n");
		exit (EXIT_FAILURE);
	}

	item->show_less_relevant_results = !exact_switch;
	gw_search_get_results (item);
	if (item == NULL){
		printf("Sorry. Something went wrong. Please try again.");
	}
	else
	{
		//Print the number of results
		if (quiet_switch == FALSE) {
			printf("");
			printf("\n%s%d%s", gettext("Found "), item->total_results, gettext(" Results"));

			if (item->total_relevant_results != item->total_results)
				printf("%s%d%s", gettext(" ("), item->total_relevant_results, gettext(" Relevant)"));

			printf("[0m\n");
		}

		gw_searchitem_free(item);
	}
	return;
}


void gw_console_append_edict_results (GwSearchItem *item, gboolean unused) {

	if (item != NULL){

		//Definitions
		int cont = 0;
		GwResultLine *resultline = item->resultline;

		if (cursesFlag) {

			wattron(results, COLOR_PAIR(1));

			//Kanji
			wprintw(results,"%s", resultline->kanji_start);
			//Furigana
			if (resultline->furigana_start)
			  wprintw(results," [%s]", resultline->furigana_start);

			wattroff(results, COLOR_PAIR(1));

			//Other info
			if (resultline->classification_start)
			  wprintw(results," %s", resultline->classification_start);
			//Important Flag
			if (resultline->important)
			  wprintw(results," %s", "P");


			wprintw(results,"\n");
			while (cont < resultline->def_total) {
				wattron(results, COLOR_PAIR(2));
				wprintw(results,"\t%s ", resultline->number[cont]);
				wattroff(results, COLOR_PAIR(2));
				wprintw(results,"%s\n", resultline->def_start[cont]);
				cont++;
			}
			wprintw(results,"\n");

			wrefresh(screen);
			wrefresh(results);
			refresh();
		}
		else {

			//Kanji
			printf("[32m%s", resultline->kanji_start);
			//Furigana
			if (resultline->furigana_start)
			  printf(" [%s]", resultline->furigana_start);
			//Other info
			if (resultline->classification_start)
			  printf("[0m %s", resultline->classification_start);
			//Important Flag
			if (resultline->important)
			  printf("[0m %s", "P");

			printf("\n");
			while (cont < resultline->def_total) {
			  printf("[0m      [35m%s [0m%s\n", resultline->number[cont], resultline->def_start[cont]);
			  cont++;
			}
			printf("\n");
		}
	}
	else {
		if (cursesFlag) {
			wattron(results, COLOR_PAIR(REDONBLACK));
			wprintw(results,"INTERNAL ERROR (gw_console_append_normal_results): NULL POINTER!");
			wattroff(results, COLOR_PAIR(REDONBLACK));
		}
		else
			printf("INTERNAL ERROR (gw_console_append_normal_results): NULL POINTER!");
	}

    return;
}


void gw_console_append_kanjidict_results (GwSearchItem *item, gboolean unused)
{
	if (item != NULL){

		char line_started = FALSE;
	    GwResultLine *resultline = item->resultline;

		if (cursesFlag) {
			//Kanji
			wattron(results, COLOR_PAIR(GREENONBLACK));
			wprintw(results,"%s\n", resultline->kanji);
			wattroff(results, COLOR_PAIR(REDONBLACK));

			if (resultline->radicals)
				wprintw(results,"%s%s\n", gettext("Radicals:"), resultline->radicals);

			if (resultline->strokes) {
			  line_started = TRUE;
			  wprintw(results,"%s%s", gettext("Stroke:"), resultline->strokes);
			}

			if (resultline->frequency) {
			  if (line_started)
				  wprintw(results," ");
			  line_started = TRUE;
			  wprintw(results,"%s%s", gettext("Freq:"), resultline->frequency);
			}

			if (resultline->grade) {
			  if (line_started)
				  wprintw(results," ");
			  line_started = TRUE;
			  wprintw(results,"%s%s", gettext("Grade:"), resultline->grade);
			}

			if (resultline->jlpt) {
			  if (line_started)
				  wprintw(results," ");
			  line_started = TRUE;
			  wprintw(results,"%s%s", gettext("JLPT:"), resultline->jlpt);
			}

			if (line_started)
				wprintw(results,"\n");

			if (resultline->readings[0])
				wprintw(results,"%s%s", gettext("Readings:"), resultline->readings[0]);
			if (resultline->readings[1])
				wprintw(results,"%s", resultline->readings[1]);

			wprintw(results,"\n");
			if (resultline->meanings)
				wprintw(results,"%s%s\n", gettext("Meanings:"), resultline->meanings);
			wprintw(results,"\n");
		}
		else {
			//Kanji
			printf("[32;1m%s[0m\n", resultline->kanji);

			if (resultline->radicals)
				printf("%s%s\n", gettext("Radicals:"), resultline->radicals);

			if (resultline->strokes) {
			  line_started = TRUE;
			  printf("%s%s", gettext("Stroke:"), resultline->strokes);
			}

			if (resultline->frequency) {
			  if (line_started) printf(" ");
			  line_started = TRUE;
			  printf("%s%s", gettext("Freq:"), resultline->frequency);
			}

			if (resultline->grade) {
			  if (line_started)
				  printf(" ");
			  line_started = TRUE;
			  printf("%s%s", gettext("Grade:"), resultline->grade);
			}

			if (resultline->jlpt) {
			  if (line_started)
				  printf(" ");
			  line_started = TRUE;
			  printf("%s%s", gettext("JLPT:"), resultline->jlpt);
			}

			if (line_started)
				printf("\n");

			if (resultline->readings[0])
				printf("%s%s", gettext("Readings:"), resultline->readings[0]);
			if (resultline->readings[1])
				printf("%s", resultline->readings[1]);

			printf("\n");
			if (resultline->meanings)
				printf("%s%s\n", gettext("Meanings:"), resultline->meanings);
			printf("\n");
		}
	}
	else {
		if (cursesFlag) {
			wattron(results, COLOR_PAIR(REDONBLACK));
			wprintw(results,"INTERNAL ERROR (gw_console_append_kanji_results): NULL POINTER!");
			wattroff(results, COLOR_PAIR(REDONBLACK));
		}
		else
			printf("INTERNAL ERROR (gw_console_append_kanji_results): NULL POINTER!");
	}

	return;
}


void gw_console_append_radicalsdict_results (GwSearchItem *item, gboolean unused) {
	if (item != NULL){
		if (cursesFlag) {
			wattron(results, COLOR_PAIR(GREENONBLACK));
			wprintw(results,"%s:", item->resultline->kanji);
			wattron(results, COLOR_PAIR(GREENONBLACK));
			wprintw(results," %s\n\n", item->resultline->radicals);
		}
		else
			printf("[32;1m%s:[0m %s\n\n", item->resultline->kanji, item->resultline->radicals);
	}
	else {
		if (cursesFlag) {
			wattron(results, COLOR_PAIR(REDONBLACK));
			wprintw(results,"INTERNAL ERROR (gw_console_append_radical_results): NULL POINTER!");
			wattroff(results, COLOR_PAIR(REDONBLACK));
		}
		else
			printf("INTERNAL ERROR (gw_console_append_radical_results): NULL POINTER!");
	}
	return;
}


void gw_console_append_examplesdict_results (GwSearchItem *item, gboolean unused) {

	if (item != NULL){

		GwResultLine *resultline = item->resultline;
		int i = 0;

		if (cursesFlag) {
			while (resultline->number[i] != NULL && resultline->def_start[i] != NULL) {
			  if (resultline->number[i][0] == 'A' || resultline->number[i][0] == 'B') {
				  wattron(results, COLOR_PAIR(BLUEONBLACK));
				  wprintw(results,"%s:\t", resultline->number[i]);
				  wattroff(results, COLOR_PAIR(BLUEONBLACK));
				  wprintw(results,"%s\n", resultline->def_start[i]);
			  }
			  else
				  wprintw(results,"\t%s\n", resultline->def_start[i]);
			  i++;
			}
			wprintw(results,"\n");
		}
		else {
			while (resultline->number[i] != NULL && resultline->def_start[i] != NULL) {
			  if (resultline->number[i][0] == 'A' || resultline->number[i][0] == 'B')
				printf("[32;1m%s:[0m\t%s\n", resultline->number[i], resultline->def_start[i]);
			  else
				printf("\t%s\n", resultline->def_start[i]);
			  i++;
			}
			printf("\n");
		}
	}
	else {
		if (cursesFlag) {
			wattron(results, COLOR_PAIR(REDONBLACK));
			wprintw(results,"INTERNAL ERROR (gw_console_append_examples_results): NULL POINTER!");
			wattroff(results, COLOR_PAIR(REDONBLACK));
		}
		else
			printf("INTERNAL ERROR (gw_console_append_examples_results): NULL POINTER!");
	}
	return;
}


void gw_console_append_unknowndict_results (GwSearchItem *item, gboolean unused) {
	if (item != NULL){
		if (cursesFlag)
			wprintw(results,"%s\n", item->resultline->string);
		else
			printf("%s\n", item->resultline->string);
	}
	else {
		if (cursesFlag) {
			wattron(results, COLOR_PAIR(REDONBLACK));
			wprintw(results,"INTERNAL ERROR (gw_console_append_unknown_results): NULL POINTER!");
			wattroff(results, COLOR_PAIR(REDONBLACK));
		}
		else
			printf("INTERNAL ERROR (gw_console_append_unknown_results): NULL POINTER!");
	}
	return;
}
