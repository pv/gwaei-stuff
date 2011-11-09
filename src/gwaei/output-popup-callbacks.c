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
//! @file output-popup-callbacks.c
//!
//! @brief To be written
//!

#include <gwaei/gwaei.h>
#include <gwaei/search-private.h>

struct _GwResultPopupData {
  GwSearchWindow *window;
  GtkToggleButton *button;
  GtkWidget *popup;
  LwSearchItem *item;
  gchar* text;
};
typedef struct _GwResultPopupData GwResultPopupData;
#define GW_RESULTPOPUPDATA(obj) (GwResultPopupData*)obj



static GtkToggleButton* gw_popupbutton_new (int, GwResultPopupData*);
static GtkWidget* _resultpopup_new (GwResultPopupData*);
static void _resultpopup_show_cb (GtkWidget*, gpointer);
static void _searchwindow_new_tab_with_search_cb (GtkMenuItem*, gpointer);
static void _resultpopup_search_online_cb (GtkMenuItem*, gpointer);




GwResultPopupData* gw_resultpopupdata_new (GwSearchWindow *window, LwSearchItem *item, LwResultLine *resultline)
{
    //Sanity check
    g_assert (window != NULL && item != NULL && resultline != NULL);
    if (resultline->kanji_start == NULL && resultline->furigana_start == NULL) return NULL;

    //Declarations
    GwSearchWindowPrivate *priv;
    GwResultPopupData *temp;

    //Initializations
    temp = g_new (GwResultPopupData, 1);
    
    if (temp != NULL)
    {
      priv = window->priv;
      temp->window = window;
      temp->popup = NULL;
      temp->button = gw_popupbutton_new (priv->font_size, temp);
      temp->item = item;
      if (resultline->kanji_start != NULL)
        temp->text = g_strdup (resultline->kanji_start);
      else if (resultline->furigana_start != NULL)
        temp->text = g_strdup (resultline->furigana_start);
      else
        temp->text = NULL;
    }

    return temp;
}


void gw_resultpopupdata_free (GwResultPopupData *data)
{
    //Sanity check
    if (data == NULL) return;

    if (data->text != NULL) g_free (data->text); data->text = NULL;
    if (data->popup != NULL) gtk_widget_destroy (GTK_WIDGET (data->popup)); data->popup = NULL;
    g_free (data); data = NULL;
}

static GtkToggleButton* gw_popupbutton_new (int font_size, GwResultPopupData *rpdata)
{
    //Sanity check
    g_assert (rpdata != NULL);

    //Declarations
    GtkToggleButton *button;
    gchar *markup;
    GtkLabel *label;

    //Initializations
    button = GTK_TOGGLE_BUTTON (gtk_toggle_button_new ());
    g_signal_connect (G_OBJECT (button), "toggled", G_CALLBACK (_resultpopup_show_cb), rpdata);
    g_signal_connect_swapped (G_OBJECT (button), "destroy", G_CALLBACK (gw_resultpopupdata_free), rpdata);
    label = GTK_LABEL (gtk_label_new (NULL));
    markup = g_markup_printf_escaped ("<span size=\"%d\">▼</span>", font_size * PANGO_SCALE * 3 / 4);
    if (markup != NULL)
    {
      gtk_label_set_markup (label, markup);
      g_free (markup);
    }

    gtk_button_set_relief (GTK_BUTTON (button), GTK_RELIEF_NONE);
    gtk_container_add (GTK_CONTAINER (button), GTK_WIDGET (label));
    gtk_widget_show (GTK_WIDGET (label));

    return button;
}


void gw_searchwindow_insert_resultpopup_button (GwSearchWindow *window,     LwSearchItem *item, 
                                                LwResultLine   *resultline, GtkTextIter  *iter)
{
    //Declarations
    GwSearchData *sdata;
    GtkTextView *view;
    GtkTextBuffer *buffer;
    GtkTextChildAnchor *anchor;
    GwResultPopupData *rpdata;

    sdata = GW_SEARCHDATA (lw_searchitem_get_data (item));
    view = GTK_TEXT_VIEW (sdata->view);
    buffer = gtk_text_view_get_buffer (view);
    rpdata = gw_resultpopupdata_new (window, item, resultline);

    if (rpdata != NULL)
    {
      gtk_text_buffer_insert (buffer, iter, "   ", -1);
      anchor = gtk_text_buffer_create_child_anchor (buffer, iter);
      gtk_text_view_add_child_at_anchor (view, GTK_WIDGET (rpdata->button), anchor);
      gtk_widget_show (GTK_WIDGET (rpdata->button));
    }
}


static void _resultpopup_hide_cb (GtkWidget *widget, gpointer data)
{
    if (data == NULL) return;

    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (data), FALSE);
}


//!
//! @brief Populates the main contextual menu with search options
//!
//! @param view The GtkTexView that was right clicked
//! @param menu The Popup menu to populate
//! @param data  Currently unused gpointer containing user data
//!
static GtkWidget* _resultpopup_new (GwResultPopupData *rpdata)
{
    if (rpdata == NULL) return NULL;

    //Declarations
    GwSearchWindow *window;
    GwApplication *application;
    LwDictInfoList *dictinfolist;
    LwPreferences *preferences;
    LwSearchItem *item;
    GwSearchData *sdata;
    LwDictInfo *di;
    GtkWidget *popup;
    char *menu_text;
    GtkWidget *menuitem;
    int i;

    //Initializations
    window = rpdata->window;
    application = gw_window_get_application (GW_WINDOW (window));
    dictinfolist = LW_DICTINFOLIST (gw_application_get_dictinfolist (application));
    preferences = gw_application_get_preferences (application);
    popup = gtk_menu_new ();
    g_signal_connect_after (G_OBJECT (popup), "hide", G_CALLBACK (_resultpopup_hide_cb), rpdata->button);
    char *website_url_menuitems[] = {
      "Wikipedia", "http://www.wikipedia.org/wiki/%s", "wikipedia.png",
      "Goo.ne.jp", "http://dictionary.goo.ne.jp/srch/all/%s/m0u/", "goo.png",
      "Google.com", "http://www.google.com/search?q=%s", "google.png",
      NULL, NULL, NULL
    };

    //Add internal dictionary links
    i = 0;
    while ((di = lw_dictinfolist_get_dictinfo_by_load_position (dictinfolist, i)) != NULL)
    {
      if (di != NULL && (item = lw_searchitem_new (rpdata->text, di, preferences, NULL)) != NULL)
      {
        sdata = gw_searchdata_new (gw_searchwindow_get_current_textview (window), window);
        lw_searchitem_set_data (item, sdata, LW_SEARCHITEM_DATA_FREE_FUNC (gw_searchdata_free));
        menu_text = g_strdup_printf ("%s", di->longname);
        if (menu_text != NULL)
        {
          menuitem = GTK_WIDGET (gtk_image_menu_item_new_with_label (menu_text));
          g_signal_connect (G_OBJECT (menuitem), "activate", G_CALLBACK (_searchwindow_new_tab_with_search_cb), item);
          g_signal_connect_swapped (G_OBJECT (menuitem), "destroy", G_CALLBACK (lw_searchitem_free), item);
          gtk_menu_shell_append (GTK_MENU_SHELL (popup), GTK_WIDGET (menuitem));
          gtk_widget_show (GTK_WIDGET (menuitem));
//          gtk_widget_show (GTK_WIDGET (menuimage));
          g_free (menu_text);
          menu_text = NULL;
        }
      }
      i++;
    }

    //Separator
    menuitem = gtk_separator_menu_item_new ();
    gtk_menu_shell_append (GTK_MENU_SHELL (popup), GTK_WIDGET (menuitem));
    gtk_widget_show (GTK_WIDGET (menuitem));

    //Add weblinks
    i = 0;
    di =  lw_dictinfolist_get_dictinfo_by_load_position (dictinfolist, 0);
    while (website_url_menuitems[i] != NULL)
    {
      if (di != NULL && (item = lw_searchitem_new (rpdata->text, di, preferences, NULL)) != NULL)
      {
        //Create handy variables
        sdata = gw_searchdata_new (gw_searchwindow_get_current_textview (window), window);
        lw_searchitem_set_data (item, sdata, LW_SEARCHITEM_DATA_FREE_FUNC (gw_searchdata_free));
        char *name = website_url_menuitems[i];
        char *url = g_strdup_printf(website_url_menuitems[i + 1], rpdata->text);
        if (url != NULL)
        {
          g_free (item->queryline->string);
          item->queryline->string = g_strdup (url);
          g_free (url);
          url = NULL;
        }
        char *icon_path = website_url_menuitems[i + 2];

        //Start creating
        menu_text = g_strdup_printf ("%s", name);
        if (menu_text != NULL)
        {
          menuitem = GTK_WIDGET (gtk_image_menu_item_new_with_label (menu_text));
          char *path = g_build_filename (DATADIR2, PACKAGE, icon_path, NULL);
          if (path != NULL)
          {
//            menuimage = gtk_image_new_from_file (path);
//            gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (menuitem), GTK_WIDGET (menuimage));
            g_free (path);
            path = NULL;
          }
          g_signal_connect (G_OBJECT (menuitem), "activate", G_CALLBACK (_resultpopup_search_online_cb), item);
          g_signal_connect_swapped (G_OBJECT (menuitem), "destroy", G_CALLBACK (lw_searchitem_free), item);
          gtk_menu_shell_append (GTK_MENU_SHELL (popup), GTK_WIDGET (menuitem));
          gtk_widget_show (GTK_WIDGET (menuitem));
//          gtk_widget_show (GTK_WIDGET (menuimage));
          g_free (menu_text);
          menu_text = NULL;
        }
      }
      i += 3;
    }

    return popup;
}


static void _resultpopup_show_cb (GtkWidget *widget, gpointer data)
{
    //Sanity check
    g_assert (data != NULL); 

    if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (widget)) == FALSE) return;

    //Declarations
    GwResultPopupData *rpdata;
    GtkMenu *popup;

    //Initializations
    rpdata = GW_RESULTPOPUPDATA (data);
    popup = GTK_MENU (_resultpopup_new (rpdata));
    if (rpdata->popup != NULL) gtk_widget_destroy (rpdata->popup); rpdata->popup = GTK_WIDGET (popup);

    gtk_menu_attach_to_widget (GTK_MENU (popup), widget, NULL);
    gtk_menu_popup (popup, NULL, NULL, NULL, NULL, 0, gtk_get_current_event_time ());
}


//!
//! @brief Searches for the word in a webbrower in an external dictionary
//! @param widget Unused GtkWidget pointer
//! @param data Unused gpointer
//!
static void _resultpopup_search_online_cb (GtkMenuItem *widget, gpointer data)
{
    //Sanity check
    g_assert (data != NULL);

    //Declarations
    GwApplication *application;
    LwSearchItem *item;
    GError *error;
    GwSearchWindow *window;
    GtkTextView *view;
    GwSearchData *sdata;

    //Initializations
    item = LW_SEARCHITEM (data);
    if (item != NULL)
    {
      sdata = GW_SEARCHDATA (lw_searchitem_get_data (item));
      window = GW_SEARCHWINDOW (sdata->window);
      application = GW_APPLICATION (gw_window_get_application (GW_WINDOW (window)));
      view = gw_searchwindow_get_current_textview (window);
      sdata = gw_searchdata_new (view, window);
      lw_searchitem_set_data (item, sdata, LW_SEARCHITEM_DATA_FREE_FUNC (gw_searchdata_free));
      error = NULL;
      printf("%s\n", item->queryline->string);

      gtk_show_uri (NULL, item->queryline->string, gtk_get_current_event_time (), &error);
      gw_application_handle_error (application, GTK_WINDOW (window), TRUE, &error);
    }
}


//!
//! @brief Sets up an initites a new search in a new tab
//!
//! @param widget Currently unused widget pointer
//! @param data A gpointer to a LwSearchItem that hold the search information
//!
static void _searchwindow_new_tab_with_search_cb (GtkMenuItem *widget, gpointer data)
{
    //Sanity check
    g_assert (data != NULL);

    //Declarations
    GwApplication *application;
    GwSearchWindow *window;
    GwSearchWindowPrivate *priv;
    LwPreferences *preferences;
    LwSearchItem *item;
    LwSearchItem *item_new;
    GtkTextView *view;
    GwSearchData *sdata;
    int index;

    //Initializations
    item = LW_SEARCHITEM (data);
    sdata = GW_SEARCHDATA (lw_searchitem_get_data (item));
    window = GW_SEARCHWINDOW (sdata->window);
    application = gw_window_get_application (GW_WINDOW (window));
    priv = window->priv;
    preferences = gw_application_get_preferences (application);
    item_new = lw_searchitem_new (item->queryline->string, item->dictionary, preferences, NULL);

    if (!gw_application_can_start_search (application)) return;

    if (item_new != NULL)
    {
      view = gw_searchwindow_get_current_textview (window);
      sdata = gw_searchdata_new (view, window);
      lw_searchitem_set_data (item_new, sdata, LW_SEARCHITEM_DATA_FREE_FUNC (gw_searchdata_free));

      index = gw_searchwindow_new_tab (window);
      gtk_notebook_set_current_page (priv->notebook, index);
      gw_searchwindow_start_search (window, item_new);
    }
}


