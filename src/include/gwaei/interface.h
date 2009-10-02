void initialize_text_views (void);

void initialize_window_attributes (char* window_id);
void save_window_attributes (char* window_id);

void rebuild_history_menuitem_popup (void);
int  rebuild_combobox_dictionary_list (void);

void update_toolbar_buttons (void);
void update_history_popup (void);
void gwaei_ui_update_total_results_label (SearchItem*);
gboolean gwaei_load_gtk_builder_xml (const char );

char* gwaei_ui_get_text_slice_from_buffer (int, int, int);

//Settings
int gwaei_ui_update_progressbar (void*, double, double, double, double);

const char* gwaei_ui_get_active_dictionary(void);
char* gwaei_ui_get_text_from_text_buffer(const int);

void initialize_gui_interface(int*, char***);
