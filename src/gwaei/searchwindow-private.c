#include <libintl.h>
#include <gwaei/gwaei.h>
#include <gwaei/search-private.h>

void gw_searchwindow_private_init (GwSearchWindow *window)
{
    //Declarations
    GwSearchWindowPrivate *priv;
    int i;

    //Initializations
    priv = window->priv;

    priv->entry = NULL;
    priv->notebook = NULL;
    priv->toolbar = NULL;
    priv->statusbar = NULL;
    priv->combobox = NULL;
    priv->accelgroup = NULL;

    priv->dictinfo = NULL;
    priv->tablist = NULL;
    priv->font_size = 0;

    priv->feedback_item = NULL;
    priv->feedback_line = 0;
    priv->feedback_status = LW_SEARCHSTATUS_IDLE;

    for (i = 0; i < TOTAL_GW_SEARCHWINDOW_TIMEOUTIDS; i++)
      priv->timeoutid[i] = 0;

    priv->previous_tip = 0;

    //Mouse initialize
    priv->mouse_item = NULL;
    priv->mouse_button_press_x = 0;
    priv->mouse_button_press_y = 0;
    priv->mouse_button_press_root_x = 0;
    priv->mouse_button_press_root_y = 0;
    priv->mouse_button_character = 0;
    priv->mouse_hovered_word = NULL; 

    //Init keep searching
    priv->keep_searching_delay = 0;
    priv->keep_searching_query = NULL;
    priv->keep_searching_enabled = FALSE;

    priv->text_selected = FALSE;

    priv->new_tab = FALSE; 

    priv->history = lw_history_new (20);
    priv->spellcheck = NULL;
}


void gw_searchwindow_private_finalize (GwSearchWindow *window)
{
    //Declarations
    GwSearchWindowPrivate *priv;

    priv = window->priv;

    gw_searchwindow_cancel_all_searches (window);

    if (priv->spellcheck != NULL) gw_spellcheck_free (priv->spellcheck);
    if (priv->history != NULL) lw_history_free (priv->history);
    if (priv->tablist != NULL) g_list_free (priv->tablist);

    //Mouse finalize
    if (priv->mouse_hovered_word != NULL)
    {
      g_free (priv->mouse_hovered_word);
      priv->mouse_hovered_word = NULL;
    }

    //Keep searching finalize
    if (priv->keep_searching_query != NULL)
    {
      g_free (priv->keep_searching_query);
      priv->keep_searching_query = NULL;
    } 

    g_object_unref (priv->accelgroup);
}


