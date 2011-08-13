
//!
//! @brief Print the "less relevant" header where necessary.
//! @param item A LwSearchItem to gleam information from
//!
void nw_append_less_relevant_header_cb (LwSearchItem *item)
{
    wattron(results, COLOR_PAIR(NW_NCCOLORS_REDONBLACK));
    wprintw(results,"\n*** ");
    wattroff(results, COLOR_PAIR(NW_NCCOLORS_REDONBLACK));
    wprintw(results,"%s ", gettext("Other Results"));
    wattron(results, COLOR_PAIR(NW_NCCOLORS_REDONBLACK));
    wprintw(results,"***************************\n\n\n");
    wattroff(results, COLOR_PAIR(NW_NCCOLORS_REDONBLACK));
}


//!
//! @brief  Print the "more relevant" header where necessary
//! @param item A LwSearchItem to gleam information from
//!
void nw_append_more_relevant_header_cb (LwSearchItem *item)
{
}


//!
//! @brief Not yet written
//!
void nw_append_edict_results_cb (LwSearchItem *item)
{
		//Definitions
		int cont = 0;
		LwResultLine *resultline = item->resultline;

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
//! @brief Not yet written

void nw_append_kanjidict_result_cb (LwSearchItem *item)
{
		char line_started = FALSE;
	    LwResultLine *resultline = item->resultline;

		//Kanji
		wattron(results, COLOR_PAIR(NW_NCCOLORS_GREENONBLACK));
		wprintw(results,"%s\n", resultline->kanji);
		wattroff(results, COLOR_PAIR(NW_NCCOLORS_REDONBLACK));

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
			wprintw(results,"%s%s\n", gettext("Readings:"), resultline->readings[0]);
    if (resultline->readings[1])
      wprintw(results,"%s%s\n", gettext("Name:"), resultline->readings[1]);
    if (resultline->readings[2])
      wprintw(results,"%s%s\n", gettext("Radical Name:"), resultline->readings[2]);

		if (resultline->meanings)
			wprintw(results,"%s%s\n", gettext("Meanings:"), resultline->meanings);
		wprintw(results,"\n");
}


//!
//! @brief Not yet written
//!
void nw_append_examplesdict_result_cb (LwSearchItem *item)
{
		LwResultLine *resultline = item->resultline;
		int i = 0;
		while (resultline->number[i] != NULL && resultline->def_start[i] != NULL)
		{
		  if (resultline->number[i][0] == 'A' || resultline->number[i][0] == 'B')
		  {
			  wattron(results, COLOR_PAIR(NW_NCCOLORS_BLUEONBLACK));
			  wprintw(results,"%s:\t", resultline->number[i]);
			  wattroff(results, COLOR_PAIR(NW_NCCOLORS_BLUEONBLACK));
			  wprintw(results,"%s\n", resultline->def_start[i]);
		  }
		  else
			  wprintw(results,"\t%s\n", resultline->def_start[i]);
		  i++;
		}
}


//!
//! @brief Not yet written
//!
void nw_append_unknowndict_result_cb (LwSearchItem *item)
{
  	wprintw(results,"%s\n", item->resultline->string);
}


//!
//! @brief Sets up the interface before each search begins
//! @param item A LwSearchItem pointer to get information from
//!
void nw_prepare_search (LwSearchItem *item)
{
}


//!
//! @brief The details to be taken care of after a search is finished
//! @param item A LwSearchItem pointer to get information from
//!
void nw_cleanup_search (LwSearchItem *item)
{
    //Finish up
    if (item->total_results == 0 &&
        item->target != LW_TARGET_KANJI &&
        item->status == LW_SEARCH_SEARCHING)
    {
      nw_no_result (item);
    }
}


