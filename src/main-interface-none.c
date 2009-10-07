#include <string.h>
#include <regex.h>
#include <stdlib.h>
#include <stdio.h>
#include <libintl.h>

#include <glib.h>

#include <gwaei/definitions.h>
#include <gwaei/regex.h>
#include <gwaei/dictionaries.h>
#include <gwaei/history.h>

void initialize_global_widget_pointers ()
{
}


void initialize_window_attributes(char* window_id)
{
}


void save_window_attributes_and_hide(char* window_id)
{
}


void gw_ui_show_window (char *id)
{
}


void update_toolbar_buttons()
{
}


void gw_ui_reinitialize_results_label (GwSearchItem *item)
{
}


void gw_ui_update_total_results_label (GwSearchItem* item)
{
}

void gw_ui_finalize_total_results_label (GwSearchItem* item)
{
}


//
// Menu dictionary combobox section
//

int rebuild_combobox_dictionary_list() 
{
}


//
// Menu popups section
//
void gw_ui_update_history_menu_popup()
{
}


//Populate the menu item lists for the back and forward buttons
void rebuild_history_button_popup(char* id, GList* list) {
}

void gw_ui_rebuild_back_history_popup() {
}

void gw_ui_rebuild_forward_history_popup() {
}


void gw_ui_update_history_popups()
{
}


void gw_ui_set_font(char *family, int size)
{
}

void gw_ui_set_toolbar_style(char *request) 
{
}


void gw_ui_set_toolbar_show(gboolean request)
{
}


void gw_ui_set_less_relevant_show(gboolean show)
{
}


void gw_ui_set_romanji_kana_conv(int request)
{
}


void gw_ui_set_hiragana_katakana_conv(gboolean request)
{
}


void gw_ui_set_katakana_hiragana_conv(gboolean request)
{
}


void gw_ui_set_spellcheck(gboolean request)
{
}


void gw_ui_set_color_to_swatch(const char *widget_id, uint r, uint g, uint b)
{
}


void gw_ui_append_to_buffer(const int TARGET, char *text, char *tag1,
                               char *tag2, int *start_line, int *end_line)
{
}


void gw_ui_clear_buffer_by_target (const int TARGET)
{
}


void gw_ui_search_entry_insert(char* text)
{
}


void gw_ui_grab_focus_by_target (const int TARGET)
{
}


void gw_ui_clear_search_entry()
{
}


void gw_ui_strcpy_from_widget(char* output, int MAX, int TARGET)
{
}


void gw_ui_text_select_all_by_target (int TARGET)
{
}

void gw_ui_text_select_none_by_target (int TARGET)
{
}

guint gw_ui_get_current_widget_focus (char *window_id)
{
}

void gw_ui_copy_text(guint TARGET)
{
}

void gw_ui_cut_text(guint TARGET)
{
}

void gw_ui_paste_text(guint TARGET)
{
}


gboolean gw_ui_load_gtk_builder_xml(const char *name) {
    return FALSE;
}


void initialize_history_popups()
{
}



/////////
//  Tag handling
/////////////////////////////////////////////////


gboolean gw_ui_set_color_to_tagtable (char    *id,     int      TARGET,
                                         gboolean set_fg, gboolean set_bg )
{
    return TRUE;
}



void  gw_ui_set_tag_to_tagtable (char *id,   int      TARGET,
                                    char *atr,  gpointer val    )
{
}


char* gw_ui_get_text_slice_from_buffer (int TARGET, int sl, int el)
{
}


void gw_ui_apply_tag_to_text (int TARGET, char tag[],
                                 int sl, int so, int el, int eo)
{
}


gunichar gw_get_hovered_character(int *x, int *y)
{
} 


void gw_ui_set_cursor(const int CURSOR)
{
}


void gw_open_kanji_results()
{
}


void gw_close_kanji_results()
{
}



char* locate_offset( char *string, char *line_start, regex_t *re_locate,
                     gint *start,  gint    *end                          )
{
}



void gw_ui_add_results_tagging ( gint sl, gint el, GwSearchItem* item )
{
}


void gw_ui_display_no_results_found_page()
{
}


void gw_ui_cancel_search_status_by_target(const int TARGET)
{
}


const char* gw_ui_get_active_dictionary ()
{
}


void gw_ui_set_active_dictionary_by_name (char* name)
{
}


void gw_ui_next_dictionary()
{
}


void gw_ui_cycle_dictionaries(gboolean cycle_forward)
{
}

void gw_ui_cycle_dictionaries_forward ()
{
}

void gw_ui_cycle_dictionaries_backward ()
{
}



char* gw_ui_get_text_from_text_buffer(const int TARGET)
{
}


void gw_reload_tagtable_tags()
{
}


void gw_initialize_tags()
{
}


void initialize_gui_interface(int *argc, char ***argv)
{
}


void gw_ui_update_settings_interface()
{
}

void gw_ui_set_dictionary_source(const char* id, const char* value)
{
}


//Sets the status of an individual feature
void gw_ui_set_feature_line_status(char* name, char* status)
{
}


//Sets the install status of an individual dictionary
void gw_ui_set_install_line_status(char *name, char *status, char *message)
{
}


//The layout of this function is specifically for a libcurl callback
int gw_ui_update_progressbar (void   *id,
                                 double  dltotal,
                                 double  dlnow,
                                 double  ultotal,
                                 double  ulnow   )
{
}


void gw_ui_set_progressbar (char *name, double percent, char *message)
{
}

void gw_ui_update_search_progressbar (long current, long total)
{ 
}
