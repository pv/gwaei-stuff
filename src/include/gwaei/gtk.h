#define GW_GTK_INCLUDED

GtkBuilder *builder;
GtkWidget *search_entry, *kanji_tv;
GtkWidget* get_widget_from_target(const int);

struct GwUiDictInstallLine {
    GtkWidget *status_icon_check;
    GtkWidget *status_icon_error;
    GtkWidget *status_message;
    GtkWidget *status_progressbar;
    GtkWidget *status_hbox;

    GtkWidget *action_cancel_button;
    GtkWidget *action_install_button;
    GtkWidget *action_remove_button;
    GtkWidget *action_button_hbox;

    GtkWidget *source_uri_entry;
    GtkWidget *source_browse_button;
    GtkWidget *source_reset_button;
    GtkWidget *source_hbox;

    GtkWidget *advanced_expander;
    GtkWidget *advanced_hbox;
};
typedef struct GwUiDictInstallLine GwUiDictInstallLine;



