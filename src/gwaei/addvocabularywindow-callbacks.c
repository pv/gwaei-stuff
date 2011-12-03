#include <gwaei/gwaei.h>
#include <gwaei/addvocabularywindow-private.h>


G_MODULE_EXPORT void 
gw_addvocabularywindow_add_cb (GtkWidget *widget, gpointer data)
{
    GwAddVocabularyWindow *window;
    GtkListStore *wordstore;
    const gchar *kanji, *furigana, *definitions;
    GtkTreeIter iter;

    window = GW_ADDVOCABULARYWINDOW (gtk_widget_get_ancestor (GTK_WIDGET (data), GW_TYPE_ADDVOCABULARYWINDOW));
    if (window == NULL) return;

    kanji = gw_addvocabularywindow_get_kanji (window);
    furigana = gw_addvocabularywindow_get_furigana (window);
    definitions = gw_addvocabularywindow_get_definitions (window);
    wordstore = gw_addvocabularywindow_get_wordstore (window);

    gw_vocabularywordstore_new_word (GW_VOCABULARYWORDSTORE (wordstore), &iter, NULL, kanji, furigana, definitions);
    gw_vocabularywordstore_save (GW_VOCABULARYWORDSTORE (wordstore));

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
