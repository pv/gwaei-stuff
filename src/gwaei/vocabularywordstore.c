/******************************************************************************
    AUTHOR:
    File written and Copyrighted by Zachary Dovel. All Rights Reserved.

    LICENSE:
    This file is part of gWaei.

    gWaei is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    gWaei is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.
    
    You should have received a copy of the GNU General Public License
    along with gWaei.  If not, see <http://www.gnu.org/licenses/>.
*******************************************************************************/

//!
//! @file vocabularywordstore.c
//!
//! @brief To be written
//!

#include <stdlib.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <gtk/gtk.h>

#include <libwaei/libwaei.h>

#include <gwaei/vocabularywordstore-private.h>
#include <gwaei/vocabularywordstore.h>

G_DEFINE_TYPE (GwVocabularyWordStore, gw_vocabularywordstore, GTK_TYPE_LIST_STORE)

typedef enum {
  PROP_0,
  PROP_NAME,
} GwVocabularyWordStoreProp;


GtkListStore*
gw_vocabularywordstore_new (const gchar *NAME)
{
    //Declarations
    GwVocabularyWordStore *model;

    //Initializations
    model = GW_VOCABULARYWORDSTORE (g_object_new (GW_TYPE_VOCABULARYWORDSTORE,
                                                  "name",        NAME,
                                                  NULL));
    return GTK_LIST_STORE (model);

}


static void 
gw_vocabularywordstore_init (GwVocabularyWordStore *model)
{
    GType types[] = { G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_INT };
    gtk_list_store_set_column_types (GTK_LIST_STORE (model), TOTAL_GW_VOCABULARYWORDSTORE_COLUMNS, types);

    model->priv = GW_VOCABULARYWORDSTORE_GET_PRIVATE (model);
    memset(model->priv, 0, sizeof(GwVocabularyWordStorePrivate));
}


static void 
gw_vocabularywordstore_finalize (GObject *object)
{
    GwVocabularyWordStore *model;
    GwVocabularyWordStorePrivate *priv;

    model = GW_VOCABULARYWORDSTORE (object);
    priv = model->priv;

    if (priv->name != NULL) g_free (priv->name); 
    if (priv->filename != NULL) g_free (priv->filename);
    if (priv->vocabulary_list != NULL) lw_vocabularylist_free (priv->vocabulary_list); 

    G_OBJECT_CLASS (gw_vocabularywordstore_parent_class)->finalize (object);
}


static void 
gw_vocabularywordstore_set_property (GObject      *object,
                                 guint         property_id,
                                 const GValue *value,
                                 GParamSpec   *pspec)
{
    GwVocabularyWordStore *model;
    GwVocabularyWordStorePrivate *priv;

    model = GW_VOCABULARYWORDSTORE (object);
    priv = model->priv;

    switch (property_id)
    {
      case PROP_NAME:
        if (priv->name != NULL) g_free (priv->name);
        priv->name = g_strdup ((gchar*) (g_value_get_string (value)));
        break;
      default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
        break;
    }
}


static void 
gw_vocabularywordstore_get_property (GObject      *object,
                                 guint         property_id,
                                 GValue       *value,
                                 GParamSpec   *pspec)
{
    GwVocabularyWordStore *model;
    GwVocabularyWordStorePrivate *priv;

    model = GW_VOCABULARYWORDSTORE (object);
    priv = model->priv;

    switch (property_id)
    {
      case PROP_NAME:
        g_value_set_string (value, priv->name);
        break;
      default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
        break;
    }
}


static void
gw_vocabularywordstore_class_init (GwVocabularyWordStoreClass *klass)
{
    //Declarations
    GParamSpec *pspec;
    GObjectClass *object_class;

    //Initializations
    object_class = G_OBJECT_CLASS (klass);
    object_class->set_property = gw_vocabularywordstore_set_property;
    object_class->get_property = gw_vocabularywordstore_get_property;
    object_class->finalize = gw_vocabularywordstore_finalize;

    g_type_class_add_private (object_class, sizeof (GwVocabularyWordStorePrivate));

    pspec = g_param_spec_string ("name",
                                 "Name of the vocabulary list",
                                 "Set vocabulary list's name.",
                                 "Vocabulary",
                                 G_PARAM_CONSTRUCT | G_PARAM_READWRITE
    );
    g_object_class_install_property (object_class, PROP_NAME, pspec);

    klass->signalid[GW_VOCABULARYWORDSTORE_CLASS_SIGNALID_CHANGED] = g_signal_new (
        "changed",
        G_OBJECT_CLASS_TYPE (object_class),
        G_SIGNAL_RUN_FIRST,
        G_STRUCT_OFFSET (GwVocabularyWordStoreClass, changed),
        NULL, NULL,
        g_cclosure_marshal_VOID__VOID,
        G_TYPE_NONE, 0
    );
}


void
gw_vocabularywordstore_save (GwVocabularyWordStore *store)
{
    GwVocabularyWordStorePrivate *priv;
    LwVocabularyItem *item;
    GtkTreeModel *model;
    GtkTreeIter iter;
    PangoWeight weight;
    gchar *text;
    gboolean valid;

    priv = store->priv;
    model = GTK_TREE_MODEL (store);
    weight = PANGO_WEIGHT_NORMAL;

    if (!gw_vocabularywordstore_has_changes (store)) return;

    gw_vocabularywordstore_load (store);

    if (priv->vocabulary_list != NULL) lw_vocabularylist_free (priv->vocabulary_list);

    if ((priv->vocabulary_list = lw_vocabularylist_new (priv->name)) != NULL)
    {
      valid = gtk_tree_model_get_iter_first (model, &iter);
      while (valid)
      {
        if ((item = lw_vocabularyitem_new ()) != NULL)
        {
          gtk_tree_model_get (model, &iter,  GW_VOCABULARYWORDSTORE_COLUMN_KANJI, &text, -1);
          if (text != NULL)
          {
            lw_vocabularyitem_set_kanji (item, text);
            g_free (text);
          }

          gtk_tree_model_get (model, &iter,  GW_VOCABULARYWORDSTORE_COLUMN_FURIGANA, &text, -1);
          if (text != NULL)
          {
            lw_vocabularyitem_set_furigana (item, text);
            g_free (text);
          }

          gtk_tree_model_get (model, &iter,  GW_VOCABULARYWORDSTORE_COLUMN_DEFINITIONS, &text, -1);
          if (text != NULL)
          {
            lw_vocabularyitem_set_definitions (item, text);
            g_free (text);
          }

          priv->vocabulary_list->items = g_list_append (priv->vocabulary_list->items, item);
          gtk_list_store_set (GTK_LIST_STORE (model), &iter, GW_VOCABULARYWORDSTORE_COLUMN_CHANGED, weight, -1);
        }
        valid = gtk_tree_model_iter_next (model, &iter);
      }

      lw_vocabularylist_save (priv->vocabulary_list, NULL);
      lw_vocabularylist_free (priv->vocabulary_list); priv->vocabulary_list = NULL;
      gw_vocabularywordstore_set_has_changes (store, FALSE);
    }
}


void 
gw_vocabularywordstore_load (GwVocabularyWordStore *store)
{
    //Sanity checks
    g_assert (store != NULL);
    if (gw_vocabularywordstore_loaded (store)) return;
    g_assert (store->priv->name != NULL);

    //Declarations
    GwVocabularyWordStorePrivate *priv;
    GtkTreeModel *model;
    GtkTreeIter treeiter;
    GList *listiter;
    LwVocabularyItem *item;

    //Initializations
    priv = store->priv;
    model = GTK_TREE_MODEL (store);

    if (priv->vocabulary_list != NULL) lw_vocabularylist_free (priv->vocabulary_list);
    priv->vocabulary_list = lw_vocabularylist_new (priv->name);
    lw_vocabularylist_load (priv->vocabulary_list, NULL);

    for (listiter = priv->vocabulary_list->items; listiter != NULL; listiter = listiter->next)
    {
      item = LW_VOCABULARYITEM (listiter->data);
      gtk_list_store_append (GTK_LIST_STORE (model), &treeiter);
      gtk_list_store_set (GTK_LIST_STORE (model), &treeiter, 
          GW_VOCABULARYWORDSTORE_COLUMN_KANJI, lw_vocabularyitem_get_kanji (item), 
          GW_VOCABULARYWORDSTORE_COLUMN_FURIGANA, lw_vocabularyitem_get_furigana (item), 
          GW_VOCABULARYWORDSTORE_COLUMN_DEFINITIONS, lw_vocabularyitem_get_definitions (item), 
          GW_VOCABULARYWORDSTORE_COLUMN_CHANGED, PANGO_WEIGHT_NORMAL,
      -1);
    }

    priv->loaded = TRUE;
}


void
gw_vocabularywordstore_reset (GwVocabularyWordStore *store)
{
    GwVocabularyWordStorePrivate *priv;

    priv = store->priv;
    priv->loaded = FALSE;

    if (priv->vocabulary_list != NULL) 
    {
      lw_vocabularylist_free (priv->vocabulary_list);
      priv->vocabulary_list = NULL;
    }
    gtk_list_store_clear (GTK_LIST_STORE (store));
    priv->loaded = FALSE;
    gw_vocabularywordstore_set_has_changes (store, FALSE);
}


LwVocabularyList*
gw_vocabularywordstore_get_vocabularylist (GwVocabularyWordStore *store)
{
    GwVocabularyWordStorePrivate *priv;
    priv = store->priv;
    return priv->vocabulary_list;
}


void
gw_vocabularywordstore_set_name (GwVocabularyWordStore *store, const gchar *NAME)
{
    GwVocabularyWordStorePrivate *priv;

    priv = store->priv;

    if (priv->name != NULL) g_free (priv->name);
    priv->name = g_strdup (NAME);

    if (priv->filename != NULL)
    {
      g_free (priv->filename);
      priv->filename = NULL;
    }
}


gboolean
gw_vocabularywordstore_loaded (GwVocabularyWordStore *store)
{
    return store->priv->loaded;
}


const gchar*
gw_vocabularywordstore_get_name (GwVocabularyWordStore *store)
{
    return store->priv->name;
}


gchar*
gw_vocabularywordstore_get_filename (GwVocabularyWordStore *store)
{
    //Declarations
    GwVocabularyWordStorePrivate *priv;
    const gchar *name;

    //Initializations
    priv = store->priv;

    if (priv->filename == NULL)
    {
      name = gw_vocabularywordstore_get_name (store);
      priv->filename = lw_util_build_filename (LW_PATH_VOCABULARY, name);
    }

    return priv->filename;
}


gboolean 
gw_vocabularywordstore_file_exists (GwVocabularyWordStore *store)
{
    //Declarations
    gchar *filename;
    gboolean exists;

    if ((filename = gw_vocabularywordstore_get_filename (store)) != NULL)
      exists = g_file_test (filename, G_FILE_TEST_IS_REGULAR);
    else
      exists = FALSE;

    return exists;
}


void
gw_vocabularywordstore_set_has_changes (GwVocabularyWordStore *store, gboolean has_changes)
{
  GwVocabularyWordStoreClass *klass;

  klass = GW_VOCABULARYWORDSTORE_CLASS (G_OBJECT_GET_CLASS (store));
  store->priv->has_changes = has_changes;
  g_signal_emit (G_OBJECT (store), klass->signalid[GW_VOCABULARYWORDSTORE_CLASS_SIGNALID_CHANGED], 0);
}


gboolean
gw_vocabularywordstore_has_changes (GwVocabularyWordStore *store)
{
  return store->priv->has_changes;
}


gchar*
gw_vocabularywordstore_iter_to_string (GwVocabularyWordStore *store, GtkTreeIter *iter)
{
    GtkTreeModel *model;
    gchar *text;
    gchar *kanji, *furigana, *definitions;

    model = GTK_TREE_MODEL (store);

    gtk_tree_model_get (model, iter,
        GW_VOCABULARYWORDSTORE_COLUMN_KANJI,       &kanji,
        GW_VOCABULARYWORDSTORE_COLUMN_FURIGANA,    &furigana,
        GW_VOCABULARYWORDSTORE_COLUMN_DEFINITIONS, &definitions,
    -1);

    text = g_strjoin (";", kanji, furigana, definitions, NULL);

    if (kanji != NULL) g_free (kanji); 
    if (furigana != NULL) g_free (furigana); 
    if (definitions != NULL) g_free (definitions);

    return text;
}


gchar*
gw_vocabularywordstore_path_list_to_string (GwVocabularyWordStore *store, GList* list)
{
    //Declarations
    GList *link;
    GtkTreeIter iter;
    GtkTreePath *path;
    GtkTreeModel *model;
    gint i;
    gchar** atoms;
    gchar *text;

    //Initializations
    atoms = g_new0 (gchar*, g_list_length (list) + 1);
    i = 0;
    model = GTK_TREE_MODEL (store);

    for (link = list; link != NULL; link = link->next)
    {
      path = (GtkTreePath*) link->data;
      if (gtk_tree_model_get_iter (model, &iter, path))
      {
        atoms[i] = gw_vocabularywordstore_iter_to_string (store, &iter);
        i++;
      }
    }

    text = g_strjoinv ("\n", atoms);
    g_strfreev (atoms); atoms = NULL;

    return text;
}



void
gw_vocabularywordstore_remove_path_list (GwVocabularyWordStore *store, GList *list)
{
    //Declarations
    GtkTreeModel *model;
    GtkTreePath *path;
    GtkTreeIter iter;
    GList *link;

    //Initializations
    model = GTK_TREE_MODEL (store);

    for (link = g_list_last (list); link != NULL; link = link->prev)
    {
      path = (GtkTreePath*) link->data;
      gtk_tree_model_get_iter (model, &iter, path);
      gtk_list_store_remove (GTK_LIST_STORE (model), &iter);
    }

    if (list != NULL)
    {
      gw_vocabularywordstore_set_has_changes (GW_VOCABULARYWORDSTORE (model), TRUE);
    }
}


void
gw_vocabularywordstore_append_text (GwVocabularyWordStore *store, GtkTreeIter *iter, gboolean before, const gchar *text)
{
    //Declarations
    gchar **rows;
    gchar **atoms;
    gint i, j;
    GtkTreeIter new_iter;
    gboolean modified;

    rows = g_strsplit (text, "\n", -1);
    if (rows != NULL)
    {
      for (i = 0; rows[i] != NULL; i++)
      {
        atoms = g_strsplit (rows[i], ";", 3);
        if (atoms != NULL)
        {
          if (before)
            gtk_list_store_insert_before (GTK_LIST_STORE (store), &new_iter, iter);
          else
            gtk_list_store_insert_after (GTK_LIST_STORE (store), &new_iter, iter);
          for (j = 0; atoms[j] != NULL; j++)
            gtk_list_store_set (GTK_LIST_STORE (store), &new_iter, j, atoms[j], -1);
          gtk_list_store_set (GTK_LIST_STORE (store), &new_iter, GW_VOCABULARYWORDSTORE_COLUMN_CHANGED, PANGO_WEIGHT_SEMIBOLD, -1);
          g_strfreev (atoms); atoms = NULL;
          modified = TRUE;
        }
      }
      g_strfreev (rows); rows = NULL;
    }

    if (modified)
    {
      gw_vocabularywordstore_set_has_changes (store, TRUE);
    }
}


void
gw_vocabularywordstore_new_word (GwVocabularyWordStore *store, 
                                 GtkTreeIter           *iter,
                                 GtkTreeIter           *sibling,
                                 const gchar           *KANJI, 
                                 const gchar           *FURIGANA, 
                                 const gchar           *DEFINITIONS)
{
    //Declarations
    GtkListStore *wordstore;
    const gchar *kanji, *furigana, *definitions;
    PangoWeight weight;

    //Initializations
    wordstore = GTK_LIST_STORE (store);
    if ((kanji = KANJI) == NULL)             kanji = gettext("(Click to set Kanji)");
    if ((furigana = FURIGANA) == NULL)       furigana = gettext("(Click to set Furigana)");
    if ((definitions = DEFINITIONS) == NULL) definitions = gettext("(Click to set Definitions)");
    weight = PANGO_WEIGHT_SEMIBOLD;

    gtk_list_store_insert_before (wordstore, iter, sibling);
    gtk_list_store_set (wordstore, iter, 
        GW_VOCABULARYWORDSTORE_COLUMN_KANJI,       kanji, 
        GW_VOCABULARYWORDSTORE_COLUMN_FURIGANA,    furigana,
        GW_VOCABULARYWORDSTORE_COLUMN_DEFINITIONS, definitions,
        GW_VOCABULARYWORDSTORE_COLUMN_CHANGED,     weight,
    -1);
    gw_vocabularywordstore_set_has_changes (store, TRUE);
}


void
gw_vocabularywordstore_set_string (GwVocabularyWordStore *store, 
                                   GtkTreeIter           *iter, 
                                   gint                   column, 
                                   const gchar           *NEW_TEXT)
{
    //Sanity check
    g_assert (column == GW_VOCABULARYWORDSTORE_COLUMN_KANJI ||
              column == GW_VOCABULARYWORDSTORE_COLUMN_FURIGANA ||
              column == GW_VOCABULARYWORDSTORE_COLUMN_DEFINITIONS);

    //Declarations
    GtkTreeModel *wordmodel;
    GtkListStore *wordstore;
    gchar *text;

    //Initializations
    wordmodel = GTK_TREE_MODEL (store);
    wordstore = GTK_LIST_STORE (store);

    gtk_tree_model_get (wordmodel, iter, column, &text, -1);
    if (text != NULL)
    {
      if (strcmp(text, NEW_TEXT) != 0)
      {
        gtk_list_store_set (wordstore, iter, 
          column, NEW_TEXT,
          GW_VOCABULARYWORDSTORE_COLUMN_CHANGED, PANGO_WEIGHT_SEMIBOLD,
          -1);
        gw_vocabularywordstore_set_has_changes (store, TRUE);
      }
      g_free (text); text = NULL;
    }
}

