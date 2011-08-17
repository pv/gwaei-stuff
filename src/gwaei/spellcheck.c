#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include <gtk/gtk.h>

#include <gwaei/gwaei.h>


GwSpellcheck* gw_spellcheck_new (GtkEntry *entry)
{
    GwSpellcheck *temp;

    temp = (GwSpellcheck*) malloc(sizeof(GwSpellcheck));

    if (temp != NULL)
    {
      temp->entry = entry;
      temp->corrections = NULL;
      temp->mutex = g_mutex_new ();
      temp->needs_spellcheck = FALSE;
      temp->query_text = NULL;
      temp->sensitive = TRUE;
      temp->running_check = FALSE;
      temp->timeout = 0;
      temp->signalid[GW_SPELLCHECK_SIGNALID_DRAW] = g_signal_connect_after (
          G_OBJECT (entry), "draw", G_CALLBACK (gw_spellcheck_draw_underline_cb), temp);
      temp->signalid[GW_SPELLCHECK_SIGNALID_CHANGED] = g_signal_connect (
          G_OBJECT (entry), "changed", G_CALLBACK (gw_spellcheck_queue_cb), temp);
      temp->signalid[GW_SPELLCHECK_SIGNALID_POPULATE_POPUP] = g_signal_connect (
          G_OBJECT (entry), "populate-popup", G_CALLBACK (gw_spellcheck_populate_cb), temp);
      temp->signalid[GW_SPELLCHECK_SIGNALID_UPDATE_TIMEOUT] = g_timeout_add_full (
          G_PRIORITY_LOW, 100, (GSourceFunc) gw_spellcheck_update_timeout, (gpointer) temp, NULL);
    }

    return temp;
}

void gw_spellcheck_free (GwSpellcheck *spellcheck)
{
    g_mutex_lock (spellcheck->mutex);
    g_signal_handler_disconnect (G_OBJECT (spellcheck->entry), spellcheck->signalid[GW_SPELLCHECK_SIGNALID_DRAW]);
    g_signal_handler_disconnect (G_OBJECT (spellcheck->entry), spellcheck->signalid[GW_SPELLCHECK_SIGNALID_CHANGED]);
    g_signal_handler_disconnect (G_OBJECT (spellcheck->entry), spellcheck->signalid[GW_SPELLCHECK_SIGNALID_POPULATE_POPUP]);
    //g_signal_disconnect (G_OBJECT (spellcheck->entry), spellcheck->signalid[GW_SPELLCHECK_SIGNALID_UPDATE_TIMEOUT]);
    g_mutex_unlock (spellcheck->mutex);
    g_mutex_free (spellcheck->mutex);
    free (spellcheck);
}


void gw_spellcheck_free_menuitem_data_cb (GtkWidget *widget, gpointer data)
{
    //Declarations
    _SpellingReplacementData *srd;

    //Initializations
    srd = data;

    //Cleanup
    g_free (srd->replacement_text);
    free (srd);
}


static void _update_button_sensitivities ()
{
/*
    //Declarations
    GtkBuilder *builder;
    GtkWidget *checkbox;
    GtkWidget *toolbutton;
    GtkWidget *entry;
    gboolean exists;

    //Initializations
    builder = gw_common_get_builder ();
    entry = GTK_WIDGET (gw_common_get_widget_by_target (LW_TARGET_ENTRY));
    checkbox = GTK_WIDGET (gtk_builder_get_object (builder, "query_spellcheck"));
    toolbutton = GTK_WIDGET (gtk_builder_get_object (builder, "spellcheck_toolbutton"));
    exists = g_file_test (ENCHANT, G_FILE_TEST_IS_REGULAR);

    if (exists && !_sensitive)
    {
      _sensitive = exists;
      gtk_widget_set_sensitive (GTK_WIDGET (checkbox), TRUE);
      gtk_widget_set_sensitive (GTK_WIDGET (toolbutton), TRUE);
      g_free (_query_text);
      _query_text = g_strdup ("FORCE UPDATE");
    }
    else if (!exists && _sensitive)
    {
      _sensitive = exists;
      gtk_widget_set_sensitive (GTK_WIDGET (checkbox), FALSE);
      gtk_widget_set_sensitive (GTK_WIDGET (toolbutton), FALSE);
      g_free (_query_text);
      _query_text = g_strdup ("FORCE UPDATE");
    }
    */
}


int gw_spellcheck_get_y_offset (GwSpellcheck *spellcheck)
{
    //Declarations
    PangoRectangle rect;
    PangoLayout *layout;

    int allocation_offset;
    int layout_offset;
    int rect_offset;

    //Initializations
    layout = gtk_entry_get_layout (spellcheck->entry);
    pango_layout_get_pixel_extents (layout, &rect, NULL);
    rect_offset = rect.height;
    allocation_offset = gtk_widget_get_allocated_height (GTK_WIDGET (spellcheck->entry));
    gtk_entry_get_layout_offsets (spellcheck->entry, NULL, &layout_offset);

    return (((allocation_offset - rect_offset) / 2) - layout_offset);
}


int gw_spellcheck_get_x_offset (GwSpellcheck *spellcheck)
{
    //Declarations
    PangoRectangle rect;
    PangoLayout *layout;

    int allocation_offset;
    int layout_offset;
    int rect_offset;

    //Initializations
    layout = gtk_entry_get_layout (spellcheck->entry);
    pango_layout_get_pixel_extents (layout, &rect, NULL);
    rect_offset = rect.width;
    allocation_offset = gtk_widget_get_allocated_height (GTK_WIDGET (spellcheck->entry));
    gtk_entry_get_layout_offsets (spellcheck->entry, &layout_offset, NULL);

    return (layout_offset);
}


