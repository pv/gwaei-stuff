#include <gwaei/gwaei.h>
#include <gwaei/application-private.h>


//!
//! @brief Resets the color tags according to the preferences
//!
void gw_application_sync_tag_cb (GSettings *settings, gchar *key, gpointer data)
{
    //Declarations
    char hex[20];
    GdkRGBA color;
    gchar **pair;
    GtkTextTag *tag;
    GwApplication *app;

    app = GW_APPLICATION (data);

    //Parse the color
    lw_preferences_get_string (hex, settings, key, 20);
    if (gdk_rgba_parse (&color, hex) == FALSE)
    {
      fprintf(stderr, "Failed to set tag to the tag table: %s\n", hex);
      lw_preferences_reset_value_by_schema (app->priv->preferences, LW_SCHEMA_HIGHLIGHT, key);
      return;
    }

    //Update the tag 
    pair = g_strsplit (key, "-", 2);
    if (pair != NULL && pair[0] != NULL && pair[1] != NULL)
    {
      tag = gtk_text_tag_table_lookup (app->priv->tagtable, pair[0]);
      g_object_set (G_OBJECT (tag), pair[1], hex, NULL);
      g_strfreev (pair);
    }
}

