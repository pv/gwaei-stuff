#include <gwaei/gwaei.h>
#include <gwaei/addvocabularywindow-private.h>


G_MODULE_EXPORT void 
gw_addvocabularywindow_add_cb (GtkWidget *widget, gpointer data)
{
    GwAddVocabularyWindow *window;
    GwAddVocabularyWindowClass *klass;
    GtkListStore *wordstore;
    const gchar *kanji, *furigana, *definitions;
    GtkTreeIter iter;

    window = GW_ADDVOCABULARYWINDOW (gtk_widget_get_ancestor (GTK_WIDGET (data), GW_TYPE_ADDVOCABULARYWINDOW));
    if (window == NULL) return;
    klass = GW_ADDVOCABULARYWINDOW_CLASS (G_OBJECT_GET_CLASS (window));

    kanji = gw_addvocabularywindow_get_kanji (window);
    furigana = gw_addvocabularywindow_get_furigana (window);
    definitions = gw_addvocabularywindow_get_definitions (window);
    wordstore = gw_addvocabularywindow_get_wordstore (window);

    gw_vocabularywordstore_load (GW_VOCABULARYWORDSTORE (wordstore));
    gw_vocabularywordstore_new_word (GW_VOCABULARYWORDSTORE (wordstore), &iter, NULL, kanji, furigana, definitions);
    gw_vocabularywordstore_save (GW_VOCABULARYWORDSTORE (wordstore));

    if (klass->last_selected_list_name != NULL)
      g_free (klass->last_selected_list_name);
    klass->last_selected_list_name = g_strdup (gw_addvocabularywindow_get_list (window));

    gtk_widget_destroy (GTK_WIDGET (window));
}


G_MODULE_EXPORT void 
gw_addvocabularywindow_cancel_cb (GtkWidget *widget, gpointer data)
{
    GwAddVocabularyWindow *window;

    window = GW_ADDVOCABULARYWINDOW (gtk_widget_get_ancestor (GTK_WIDGET (data), GW_TYPE_ADDVOCABULARYWINDOW));
    if (window == NULL) return;

    gtk_widget_destroy (GTK_WIDGET (window));
}


G_MODULE_EXPORT void
gw_addvocabularywindow_kanji_changed_cb (GtkWidget *widget, gpointer data)
{
    GwAddVocabularyWindow *window;
    GwAddVocabularyWindowPrivate *priv;

    window = GW_ADDVOCABULARYWINDOW (gtk_widget_get_ancestor (GTK_WIDGET (data), GW_TYPE_ADDVOCABULARYWINDOW));
    if (window == NULL) return;
    priv = window->priv;

    if (priv->kanji_text != NULL) g_free (priv->kanji_text);
    priv->kanji_text = g_strdup (gtk_entry_get_text (priv->kanji_entry));
    g_strstrip (priv->kanji_text);

    gw_addvocabularywindow_validate (window);
}


G_MODULE_EXPORT void
gw_addvocabularywindow_furigana_changed_cb (GtkWidget *widget, gpointer data)
{
    GwAddVocabularyWindow *window;
    GwAddVocabularyWindowPrivate *priv;

    window = GW_ADDVOCABULARYWINDOW (gtk_widget_get_ancestor (GTK_WIDGET (data), GW_TYPE_ADDVOCABULARYWINDOW));
    if (window == NULL) return;
    priv = window->priv;

    if (priv->furigana_text != NULL) g_free (priv->furigana_text);
    priv->furigana_text = g_strdup (gtk_entry_get_text (priv->furigana_entry));
    g_strstrip (priv->furigana_text);

    gw_addvocabularywindow_validate (window);
}

G_MODULE_EXPORT void
gw_addvocabularywindow_definitions_changed_cb (GtkWidget *widget, gpointer data)
{
    GwAddVocabularyWindow *window;
    GwAddVocabularyWindowPrivate *priv;
    GtkTextBuffer *buffer;
    GtkTextIter start, end;

    window = GW_ADDVOCABULARYWINDOW (gtk_widget_get_ancestor (GTK_WIDGET (data), GW_TYPE_ADDVOCABULARYWINDOW));
    if (window == NULL) return;
    priv = window->priv;

    buffer = gtk_text_view_get_buffer (priv->definitions_textview);
    gtk_text_buffer_get_start_iter (buffer, &start);
    gtk_text_buffer_get_end_iter (buffer, &end);

    if (priv->definitions_text != NULL) g_free (priv->definitions_text);
    priv->definitions_text =  gtk_text_buffer_get_text (buffer, &start, &end, FALSE);

    gw_addvocabularywindow_validate (window);
}


G_MODULE_EXPORT void
gw_addvocabularywindow_list_changed_cb (GtkWidget *widget, gpointer data)
{
    GwAddVocabularyWindow *window;

    window = GW_ADDVOCABULARYWINDOW (gtk_widget_get_ancestor (GTK_WIDGET (data), GW_TYPE_ADDVOCABULARYWINDOW));
    if (window == NULL) return;

    gw_addvocabularywindow_validate (window);
}

