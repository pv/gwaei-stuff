#include <libwaei/libwaei.h>

const gchar* lw_vocabularyitem_get_kanji (LwVocabularyItem *item)
{
  return item->fields[LW_VOCABULARYITEM_FIELD_KANJI];
}

void lw_vocabularyitem_set_kanji (LwVocabularyItem *item, const gchar *text)
{
  if (item->fields[LW_VOCABULARYITEM_FIELD_KANJI] != NULL)
    g_free (item->fields[LW_VOCABULARYITEM_FIELD_KANJI]);
  item->fields[LW_VOCABULARYITEM_FIELD_KANJI] = g_strdup (text);
}

const gchar* lw_vocabularyitem_get_furigana (LwVocabularyItem *item)
{
  return item->fields[LW_VOCABULARYITEM_FIELD_FURIGANA];
}

void lw_vocabularyitem_set_furigana (LwVocabularyItem *item, const gchar *text)
{
  if (item->fields[LW_VOCABULARYITEM_FIELD_FURIGANA] != NULL)
    g_free (item->fields[LW_VOCABULARYITEM_FIELD_FURIGANA]);
  item->fields[LW_VOCABULARYITEM_FIELD_FURIGANA] = g_strdup (text);
}

const gchar* lw_vocabularyitem_get_definitions (LwVocabularyItem *item)
{
  return item->fields[LW_VOCABULARYITEM_FIELD_DEFINITIONS];
}

void lw_vocabularyitem_set_definitions (LwVocabularyItem *item, const gchar *text)
{
  if (item->fields[LW_VOCABULARYITEM_FIELD_DEFINITIONS] != NULL)
    g_free (item->fields[LW_VOCABULARYITEM_FIELD_DEFINITIONS]);
  item->fields[LW_VOCABULARYITEM_FIELD_DEFINITIONS] = g_strdup (text);
}


LwVocabularyItem*
lw_vocabularyitem_new ()
{
    LwVocabularyItem *item;

    item = g_new0 (LwVocabularyItem, 1);

    return item;
}


LwVocabularyItem*
lw_vocabularyitem_new_from_string (const gchar *text)
{
    LwVocabularyItem *item;

    item = g_new0 (LwVocabularyItem, 1);
    if (item != NULL)
    {
      gchar **atoms;
      gint i;

      atoms = g_strsplit (text, ";", 3);

      if (atoms != NULL)
      {
        for (i = 0; atoms[i] != NULL && i < TOTAL_LW_VOCABULARYITEM_FIELDS; i++)
        {
          item->fields[i] = g_strdup (g_strstrip(atoms[i]));
        }
        g_strfreev (atoms); atoms = NULL;
      }
    }
    return item;
}

void
lw_vocabularyitem_free (LwVocabularyItem *item)
{
  gint i;
  for (i = 0; i < TOTAL_LW_VOCABULARYITEM_FIELDS; i++)
  {
    if (item->fields[i] != NULL)
    {
      g_free (item->fields[i]);
      item->fields[i] = NULL;
    }
  }
  g_free (item);
}

