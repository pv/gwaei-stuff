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
//! @file src/main-interface-ncurses.c
//!
//! @brief Abstraction layer for gtk objects
//!
//! Used as a go between for functions interacting with ncurses.
//!

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <libintl.h>
#include <regex.h>

#include <glib.h>
#include <curses.h>
#include <errno.h>

#include <gwaei/definitions.h>
#include <gwaei/regex.h>
#include <gwaei/dictionary-objects.h>
#include <gwaei/search-objects.h>
#include <gwaei/engine.h>
#include <gwaei/preferences.h>
#include <gwaei/utilities.h>

static gboolean ncurses_switch = FALSE;
static gboolean exact_switch = FALSE;
static gboolean quiet_switch = FALSE;
static gboolean list_switch = FALSE;
static gboolean version_switch = FALSE;

static char* dictionary_switch_data = NULL;
static char* install_switch_data = NULL;
static char* uninstall_switch_data = NULL;

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

//!
//! /brief Print the "no result" message where necessary.
//!
void gw_ncurses_no_result()
{
    wprintw(results,"%s\n\n", gettext("No results found!"));
}


//!
//! @brief This function prints the start banner
//!
//! This function prints the start banner in both
//! simple and ncurses interface.
//!
//! @param query The query string we are searching
//! @param dictionary The name (string) of the dictionary used
//!
static void print_search_start_banner(char *query, char *dictionary)
{
    wprintw(screen, " ");
    wprintw(screen, gettext("Searching for \""));
    wattron(screen, COLOR_PAIR(REDONBLACK));
    wprintw(screen,"%s", query);
    wattroff(screen, COLOR_PAIR(REDONBLACK));
    wprintw(screen, gettext("\" in the "));
    wattron(screen, COLOR_PAIR(REDONBLACK));
    wprintw(screen," %s", dictionary);
    wattroff(screen, COLOR_PAIR(REDONBLACK));
    wprintw(screen, gettext(" dictionary..."));

    wrefresh(screen);
    refresh();
}


//!
//! /brief Print the "less relevant" header where necessary.
//!
void gw_ncurses_append_less_relevant_header_to_output()
{
    wattron(results, COLOR_PAIR(REDONBLACK));
    wprintw(results,"\n*** ");
    wattroff(results, COLOR_PAIR(REDONBLACK));
    wprintw(results,"%s ", gettext("Other Results"));
    wattron(results, COLOR_PAIR(REDONBLACK));
    wprintw(results,"***************************\n\n\n");
    wattroff(results, COLOR_PAIR(REDONBLACK));
}


//!
//! @brief Color Initialization
//!
//! The pairs of colors that will be used in the printing
//! functions are initialized here.
//!
//! @param hasColors True if the shell support colors
//!
static void ncurses_color_init (bool hasColors)
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
static int ncurses_screen_init ()
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

	ncurses_add_intro_and_box(search, gettext("Search: "));
	ncurses_add_intro_and_box(screen, gettext("Results: "));

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
					ncurses_add_intro_and_box(search,gettext("Search: "));
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
//! Zachary's recommendations:
//! * Pressing "Enter" with no text should bring up a help list of commands
//! * /exit  and /quit should exit the program
//! * /dictionary dictionaryname should switch the dictonary
//! * /exact on should turn no exact searches
//! * quiet mode doesn't really apply to ncurses so you should ignore it totally
//!
//! @param argc The number of arguments
//! @param argv An array of strings
//!
void initialize_ncurses_interface (int argc, char *argv[])
{
    GwDictInfo *di = NULL;
    GError *error = NULL;
    GOptionContext *context;
    context = g_option_context_new (gettext("- Japanese-English dictionary program that allows regex searches"));
    GOptionEntry entries[] = 
    {
      { "ncurses", 'n', 0, G_OPTION_ARG_NONE, &ncurses_switch, gettext("Open up the multisearch window (beta)"), NULL },
      { "exact", 'e', 0, G_OPTION_ARG_NONE, &exact_switch, gettext("Do not display less relevant results"), NULL },
      { "quiet", 'q', 0, G_OPTION_ARG_NONE, &quiet_switch, gettext("Display less information"), NULL },
      { "dictionary", 'd', 0, G_OPTION_ARG_STRING, &dictionary_switch_data, gettext("Search using a chosen dictionary"), NULL },
      { NULL }
    };
    g_option_context_add_main_entries (context, entries, PACKAGE);
    if (!g_option_context_parse (context, &argc, &argv, &error))
    {
      g_print (gettext("Option parsing failed: %s\n"), error->message);
      exit (EXIT_FAILURE);
    }
    if (error != NULL)
    {
      printf("%s\n", error->message);
      g_error_free (error);
      error = NULL;
      exit (EXIT_FAILURE);
    }

    //We weren't interupted by any switches! Now to the search....

    //Set dictionary
    if (dictionary_switch_data == NULL)
      di = gw_dictlist_get_dictinfo_by_alias ("English");
    else
      di = gw_dictlist_get_dictinfo_by_alias (dictionary_switch_data);
    if (di == NULL || di->status != GW_DICT_STATUS_INSTALLED)
    {
      printf (gettext("Requested dictionary not found!\n"));
      exit (EXIT_FAILURE);
    }

    //Set query text
    static char* query_text_data;
    if (argc > 1 && query_text_data == NULL)
    {
      query_text_data = gw_util_strdup_args_to_query (argc, argv);
      if (query_text_data == NULL)
      {
        printf ("Memory error creating initial query string!\n");
        exit (EXIT_FAILURE);
      }
    }
    char query[MAX_QUERY];
    if (query_text_data != NULL)
    {
      strncpy (query, query_text_data, MAX_QUERY);
      g_free (query_text_data);
      query_text_data = NULL;
    }

    //nCurses initializations
    if (ncurses_screen_init() == ERR) return;

    //Enter the main loop of ncurses
    int cont = 0;
    gboolean loop = TRUE;
    while(loop)
    {
      wmove(search, 1, 2);
      ncurses_input_and_scrolling(query);
      wmove(search, 1, 2);

      ncurses_add_intro_and_box(screen, gettext("Results: "));
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

      if (quiet_switch == FALSE)
        print_search_start_banner (query, di->name);

      if (query[0] == '\0')
        continue;

      GwSearchItem *item;
      item = gw_searchitem_new (query, di, GW_TARGET_CONSOLE);
      if (item == NULL)
      {
        wprintw(screen, "There was an error creating your search.");
        wrefresh(screen);
        cbreak(); wgetch(search); endwin();
        query[0] = '\0';
        continue;
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

      //Print the number of results
      if (quiet_switch == FALSE)
      {
        char *message_total = ngettext("Found %d result", "Found %d results", item->total_results);
        char *message_relevant = ngettext("(%d Relevant)", "(%d Relevant)", item->total_relevant_results);

        wclear(screen);
        ncurses_add_intro_and_box (screen, gettext("Results: "));
        wprintw(screen, message_total, item->total_results);

        if (item->total_relevant_results != item->total_results)
          wprintw(screen, message_relevant, item->total_relevant_results);

        wrefresh(screen);
        refresh();
      }

      scrollok(results,TRUE);
      prefresh(results,0,0,2,2,(maxY - 5), (maxX - 2));

      wclear(search);
      ncurses_add_intro_and_box(search,gettext("Search: "));

      if (exact_switch == TRUE)
        wprintw(search, " EXACT ");
      if (quiet_switch == TRUE)
        wprintw(search, " QUIET ");

      gw_searchitem_free(item);
      item = NULL;

    }

    endwin();

    printf("Bye...\n");

    exit(EXIT_SUCCESS);
}


//!
//! /brief Not yet written
//!
void gw_ncurses_append_edict_results (GwSearchItem *item, gboolean unused)
{
		//Definitions
		int cont = 0;
		GwResultLine *resultline = item->resultline;

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
		while (cont < resultline->def_total)
		{
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


//!
//! /brief Not yet written
//!
void gw_ncurses_append_kanjidict_results (GwSearchItem *item, gboolean unused)
{
		char line_started = FALSE;
	    GwResultLine *resultline = item->resultline;

		//Kanji
		wattron(results, COLOR_PAIR(GREENONBLACK));
		wprintw(results,"%s\n", resultline->kanji);
		wattroff(results, COLOR_PAIR(REDONBLACK));

		if (resultline->radicals)
			wprintw(results,"%s%s\n", gettext("Radicals:"), resultline->radicals);

		if (resultline->strokes)
		{
		  line_started = TRUE;
		  wprintw(results,"%s%s", gettext("Stroke:"), resultline->strokes);
		}

		if (resultline->frequency)
		{
		  if (line_started)
			  wprintw(results," ");
		  line_started = TRUE;
		  wprintw(results,"%s%s", gettext("Freq:"), resultline->frequency);
		}

		if (resultline->grade)
		{
		  if (line_started)
			  wprintw(results," ");
		  line_started = TRUE;
		  wprintw(results,"%s%s", gettext("Grade:"), resultline->grade);
		}

		if (resultline->jlpt)
		{
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


//!
//! /brief Not yet written
//!
void gw_ncurses_append_examplesdict_results (GwSearchItem *item, gboolean unused)
{
		GwResultLine *resultline = item->resultline;
		int i = 0;
		while (resultline->number[i] != NULL && resultline->def_start[i] != NULL)
		{
		  if (resultline->number[i][0] == 'A' || resultline->number[i][0] == 'B')
		  {
			  wattron(results, COLOR_PAIR(BLUEONBLACK));
			  wprintw(results,"%s:\t", resultline->number[i]);
			  wattroff(results, COLOR_PAIR(BLUEONBLACK));
			  wprintw(results,"%s\n", resultline->def_start[i]);
		  }
		  else
			  wprintw(results,"\t%s\n", resultline->def_start[i]);
		  i++;
		}
}


//!
//! /brief Not yet written
//!
void gw_ncurses_append_unknowndict_results (GwSearchItem *item, gboolean unused)
{
  	wprintw(results,"%s\n", item->resultline->string);
}

