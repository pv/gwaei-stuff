#ifndef GW_GTK_SETTINGS_INTERFACE_INSTALL_LINE_INCLUDED
#define GW_GTK_SETTINGS_INTERFACE_INSTALL_LINE_INCLUDED

//!
//! @brief Primitive for storing gui/information for a dictionary 'install line'
//! in the interface
//!
struct _GwUiDictInstallLine {
    GtkWidget *status_icon;          //!< pointer to the static icon widget
    GtkWidget *status_message;       //!< pointer to the status message widget
    GtkWidget *status_progressbar;   //!< pointer to the progressbar widget
    GtkWidget *status_hbox;          //!< pointer to the status hbox widget
    GtkWidget *message_hbox;         //!< pointer to the message hbox widget

    GtkWidget *action_button;        //!< pointer to the action button widget
    GtkWidget *action_button_hbox;   //!< poiner to the action button hbox

    GtkWidget *source_uri_entry;     //!< pointer to the source uri entry widget
    GtkWidget *source_browse_button; //!< pointer to the source browse button
    GtkWidget *source_reset_button;  //!< pointer to the source reset button
    GtkWidget *source_hbox;          //!< pointer to the source hbox

    GtkWidget *advanced_expander;    //!< pointer to the advanced expander widget
    GtkWidget *advanced_hbox;        //!< pointer to the advanced hbox widget

    GwDictInfo *di;                  //!< pointer to the dictinfo object
};
typedef struct _GwUiDictInstallLine GwUiDictInstallLine;


//Methods
GwUiDictInstallLine *gw_ui_new_dict_install_line (GwDictInfo*);
void gw_ui_dict_install_set_action_button (GwUiDictInstallLine*, const gchar*, gboolean);
void gw_ui_dict_install_set_message (GwUiDictInstallLine*, const gchar*, const char*);
void gw_ui_progressbar_set_fraction_by_install_line (GwUiDictInstallLine*, const gdouble);
void gw_ui_add_dict_install_line_to_table (GtkTable*, GwUiDictInstallLine*);


#endif
