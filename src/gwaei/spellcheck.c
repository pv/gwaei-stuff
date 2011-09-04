#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include <gtk/gtk.h>

#include <gwaei/gwaei.h>

void  _spellcheck_attach_signals (GwSpellcheck*);
void  _spellcheck_remove_signals (GwSpellcheck*);

GwSpellcheck* gw_spellcheck_new (GtkEntry *entry)
{
    GwSpellcheck *temp;

    temp = (GwSpellcheck*) malloc(sizeof(GwSpellcheck));

    if (temp != NULL)
    {
      gw_spellcheck_init (temp, entry);
    }

    return temp;
}

void gw_spellcheck_free (GwSpellcheck *spellcheck)
{
    gw_spellcheck_deinit (spellcheck);
    free (spellcheck);
}


void gw_spellcheck_init (GwSpellcheck *spellcheck, GtkEntry *entry)
{
    spellcheck->entry = entry;
    spellcheck->corrections = NULL;
    spellcheck->mutex = g_mutex_new ();
    spellcheck->needs_spellcheck = FALSE;
    spellcheck->query_text = NULL;
    spellcheck->sensitive = TRUE;
    spellcheck->running_check = FALSE;
    spellcheck->timeout = 0;

    _spellcheck_attach_signals (spellcheck);
}


void gw_spellcheck_deinit (GwSpellcheck *spellcheck)
{
    g_mutex_lock (spellcheck->mutex);

    g_free (spellcheck->query_text);

    _spellcheck_remove_signals (spellcheck);

    g_mutex_unlock (spellcheck->mutex);
    g_mutex_free (spellcheck->mutex);
}


void  _spellcheck_attach_signals (GwSpellcheck *spellcheck)
{
    //Declarations
    int i;

    for (i = 0; i < TOTAL_GW_SPELLCHECK_SIGNALIDS; i++)
      spellcheck->signalid[i] = 0;

    spellcheck->signalid[GW_SPELLCHECK_SIGNALID_DRAW] = g_signal_connect_after (
        G_OBJECT (spellcheck->entry), "draw", G_CALLBACK (gw_spellcheck_draw_underline_cb), spellcheck);
    spellcheck->signalid[GW_SPELLCHECK_SIGNALID_CHANGED] = g_signal_connect (
        G_OBJECT (spellcheck->entry), "changed", G_CALLBACK (gw_spellcheck_queue_cb), spellcheck);
    spellcheck->signalid[GW_SPELLCHECK_SIGNALID_POPULATE_POPUP] = g_signal_connect (
        G_OBJECT (spellcheck->entry), "populate-popup", G_CALLBACK (gw_spellcheck_populate_cb), spellcheck);

    for (i = 0; i < TOTAL_GW_SPELLCHECK_TIMEOUTIDS; i++)
      spellcheck->timeoutid[i] = 0;

    spellcheck->timeoutid[GW_SPELLCHECK_TIMEOUTID_UPDATE] = g_timeout_add_full (
        G_PRIORITY_LOW, 100, (GSourceFunc) gw_spellcheck_update_timeout, spellcheck, NULL);
}


void _spellcheck_remove_signals (GwSpellcheck *spellcheck)
{
    //Declarations
    GSource *source;
    int i;

    g_signal_handler_disconnect (G_OBJECT (spellcheck->entry), spellcheck->signalid[GW_SPELLCHECK_SIGNALID_DRAW]);
    g_signal_handler_disconnect (G_OBJECT (spellcheck->entry), spellcheck->signalid[GW_SPELLCHECK_SIGNALID_CHANGED]);
    g_signal_handler_disconnect (G_OBJECT (spellcheck->entry), spellcheck->signalid[GW_SPELLCHECK_SIGNALID_POPULATE_POPUP]);

    for (i = 0; i < TOTAL_GW_SPELLCHECK_TIMEOUTIDS; i++)
    {
      if (g_main_current_source () != NULL &&
          !g_source_is_destroyed (g_main_current_source ()) &&
          spellcheck->timeoutid[i] > 0
         )
      {
        source = g_main_context_find_source_by_id (NULL, spellcheck->timeoutid[i]);
        if (source != NULL)
        {
          g_source_destroy (source);
        }
      }
      spellcheck->timeoutid[i] = 0;
    }
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


