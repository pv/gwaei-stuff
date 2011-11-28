#include <libwaei/libwaei.h>


gchar**
lw_vocabularylist_get_lists ()
{
    GDir *dir;
    gchar **atoms;
    gchar *buffer;
    const gchar *name;
    guint chars;
    gchar *uri;

    chars = 0;
    buffer = NULL;
    atoms = NULL;

    if ((uri = lw_util_build_filename (LW_PATH_VOCABULARY, NULL)) != NULL)
    {
      if ((dir = g_dir_open (uri, 0, NULL)) != NULL)
      {
        //Get the size needed for the buffer
        while ((name = g_dir_read_name (dir)) != NULL)
        {
          chars += strlen(name) + 1;
        }

        if ((buffer = g_new0 (gchar, chars)) != NULL)
        {
          g_dir_rewind (dir);

          //Set the buffer
          while ((name = g_dir_read_name (dir)) != NULL)
          {
            strcat(buffer, name);
            strcat(buffer, ";");
          }
          buffer[strlen(buffer) - 1] = '\0';

          //Split it
          atoms = g_strsplit (buffer, ";", -1);

          g_free (buffer); buffer = NULL;
        }
        g_dir_close (dir); dir = NULL;
      }
      g_free (uri); uri = NULL;
    }

    return atoms;
}

LwVocabularyList*
lw_vocabularylist_new (const gchar *NAME)
{
    g_assert (NAME != NULL && strlen (NAME) > 0);

    LwVocabularyList *list;

    list = g_new0 (LwVocabularyList, 1);
    if (list != NULL)
    {
      list->name = g_strdup (NAME);
    }
    return list;
}


void
lw_vocabularylist_free (LwVocabularyList *list)
{
    if (list->name != NULL) g_free (list->name);
    g_list_foreach (list->items, lw_vocabularyitem_free, NULL);
    g_list_free (list->items); list->items = NULL;
}


void
lw_vocabularylist_load (LwVocabularyList *list, LwIoProgressCallback cb)
{
    LwVocabularyItem *item;
    gchar *uri;
    FILE *stream;
    const gint MAX = 200;
    gchar buffer[MAX + 1];

    uri = lw_util_build_filename (LW_PATH_VOCABULARY, list->name);
    stream = fopen (uri, "r");
    buffer[MAX] = '\0';

    while (stream != NULL && feof(stream) == 0)
    {
      if (fgets (buffer, MAX, stream) != NULL)
      {
        item = lw_vocabularyitem_new_from_string (buffer);
        if (item != NULL)
        {
          list->items = g_list_append (list->items, item);
        }
      }
    }
}

void
lw_vocabularylist_save (LwVocabularyList *list, LwIoProgressCallback cb)
{
    //Declarations
    LwVocabularyItem *item;
    GList *iter;
    gint i;
    gchar *uri;
    FILE *stream;

    uri = lw_util_build_filename (LW_PATH_VOCABULARY, list->name);
    stream = fopen (uri, "w");

    for (iter = list->items; iter != NULL; iter = iter->next)
    {
      item = LW_VOCABULARYITEM (iter->data);
      if (item != NULL)
      {
        for (i = 0; i < TOTAL_LW_VOCABULARYITEM_FIELDS - 1 && feof (stream) == 0; i++)
        {
          if (item->fields[i] != NULL)
          {
            fputs(item->fields[i], stream);
            fputc(';', stream);
          }
        }
        if (item->fields[i] != NULL) fputs(item->fields[i], stream);
        fputc('\n', stream);
      }
    }

    fclose(stream);
    g_free (uri);
}

void
lw_vocabularylist_set_name (LwVocabularyList *list, const gchar *name)
{
    if (list->name != NULL)
      g_free (list->name);
    list->name = g_strdup (name);
}

const gchar*
lw_vocabularylist_get_name (LwVocabularyList *list)
{
    return list->name;
}

GList *
lw_vocabularylist_get_items (LwVocabularyList *list)
{
    return list->items;
}

void
lw_vocabularylist_set_changed (LwVocabularyList *list, gboolean changed)
{
    list->changed = changed;
}

gboolean
lw_vocabularylist_changed (LwVocabularyList *list)
{
  return list->changed;
}
