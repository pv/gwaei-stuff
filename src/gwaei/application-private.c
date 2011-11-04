#include <gwaei/gwaei.h>
#include <gwaei/application-private.h>

static GtkTextTagTable* _application_texttagtable_new (void);

void gw_application_private_init (GwApplication *application)
{
      GwApplicationPrivate *priv;

      priv = GW_APPLICATION_GET_PRIVATE (application);

      priv->context = NULL;
      priv->arg_new_window_switch = FALSE;
      priv->arg_dictionary = NULL;
      priv->arg_query = NULL;
      priv->arg_version_switch = FALSE;

      priv->last_focused = NULL;

      priv->preferences = lw_preferences_new ();
      priv->dictinfolist = gw_dictinfolist_new (20, application);
      priv->dictinstlist = NULL;
      priv->block_new_searches = 0;

      priv->tagtable = _application_texttagtable_new ();
      lw_preferences_add_change_listener_by_schema (
        priv->preferences, LW_SCHEMA_HIGHLIGHT, LW_KEY_MATCH_FG, gw_application_sync_tag_cb, application);
      lw_preferences_add_change_listener_by_schema (
        priv->preferences, LW_SCHEMA_HIGHLIGHT, LW_KEY_MATCH_BG, gw_application_sync_tag_cb, application);
      lw_preferences_add_change_listener_by_schema (
        priv->preferences, LW_SCHEMA_HIGHLIGHT, LW_KEY_HEADER_FG, gw_application_sync_tag_cb, application);
      lw_preferences_add_change_listener_by_schema (
        priv->preferences, LW_SCHEMA_HIGHLIGHT, LW_KEY_HEADER_BG, gw_application_sync_tag_cb, application);
      lw_preferences_add_change_listener_by_schema (
        priv->preferences, LW_SCHEMA_HIGHLIGHT, LW_KEY_COMMENT_FG, gw_application_sync_tag_cb, application);

#ifdef OS_MINGW
      GtkSettings *settings;
      settings = gtk_settings_get_default ();
      g_object_set (settings, "gtk-theme-name", "MS-Windows", NULL);
      g_object_set (settings, "gtk-menu-images", FALSE, NULL);
      g_object_set (settings, "gtk-button-images", FALSE, NULL);
      g_object_set (settings, "gtk-cursor-blink", FALSE, NULL);
      g_object_set (settings, "gtk-alternative-button-order", TRUE, NULL);
      g_object_unref (settings);
#endif
}

void gw_application_private_finalize (GwApplication *application)
{
    GwApplicationPrivate *priv;

    priv = GW_APPLICATION_GET_PRIVATE (application);

    if (priv->dictinstlist != NULL) lw_dictinstlist_free (priv->dictinstlist);
    if (priv->dictinfolist != NULL) gw_dictinfolist_free (priv->dictinfolist);
    if (priv->context != NULL)      g_option_context_free (priv->context);
    if (priv->arg_query != NULL)    g_free(priv->arg_query);
    if (priv->preferences != NULL)  lw_preferences_free (priv->preferences);
}


//!
//! @brief Adds the tags to stylize the buffer text
//!
static GtkTextTagTable* _application_texttagtable_new ()
{
    GtkTextTagTable *temp;
    GtkTextTag *tag;

    temp = gtk_text_tag_table_new ();

    if (temp != NULL)
    {
      tag = gtk_text_tag_new ("italic");
      g_object_set (tag, "style", PANGO_STYLE_ITALIC, NULL);
      gtk_text_tag_table_add (temp, tag);

      tag = gtk_text_tag_new ("gray");
      g_object_set (tag, "foreground", "#888888", NULL);
      gtk_text_tag_table_add (temp, tag);

      tag = gtk_text_tag_new ("smaller");
      g_object_set (tag, "size", "smaller", NULL);
      gtk_text_tag_table_add (temp, tag);

      tag = gtk_text_tag_new ("small");
      g_object_set (tag, "font", "Serif 6", NULL);
      gtk_text_tag_table_add (temp, tag);

      tag = gtk_text_tag_new ("important");
      g_object_set (tag, "weight", PANGO_WEIGHT_BOLD, NULL);
      gtk_text_tag_table_add (temp, tag);

      tag = gtk_text_tag_new ("larger");
      g_object_set (tag, "font", "Sans 20", NULL);
      gtk_text_tag_table_add (temp, tag);

      tag = gtk_text_tag_new ("large");
      g_object_set (tag, "font", "Serif 40", NULL);
      gtk_text_tag_table_add (temp, tag);

      tag = gtk_text_tag_new ("center");
      g_object_set (tag, "justification", GTK_JUSTIFY_LEFT, NULL);
      gtk_text_tag_table_add (temp, tag);

      tag = gtk_text_tag_new ("comment");
      gtk_text_tag_table_add (temp, tag);

      tag = gtk_text_tag_new ("match");
      gtk_text_tag_table_add (temp, tag);

      tag = gtk_text_tag_new ("header");
      gtk_text_tag_table_add (temp, tag);
    }

    return temp;
}

