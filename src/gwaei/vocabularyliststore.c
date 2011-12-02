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
//! @file vocabularyliststore.c
//!
//! @brief To be written
//!

#include <stdlib.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <glib/gstdio.h>
#include <gtk/gtk.h>

#include <libwaei/libwaei.h>

#include <gwaei/vocabularyliststore-private.h>
#include <gwaei/vocabularyliststore.h>

G_DEFINE_TYPE (GwVocabularyListStore, gw_vocabularyliststore, GTK_TYPE_LIST_STORE)

typedef enum
{
  PROP_0,
} GwVocabularyListStoreProps;


GtkListStore*
gw_vocabularyliststore_new ()
{
    //Declarations
    GwVocabularyListStore *model;

    //Initializations
    model = GW_VOCABULARYLISTSTORE (g_object_new (GW_TYPE_VOCABULARYLISTSTORE, NULL));

    return GTK_LIST_STORE (model);
}


static void 
gw_vocabularyliststore_init (GwVocabularyListStore *store)
{
    GType types[] = { G_TYPE_STRING, G_TYPE_BOOLEAN, G_TYPE_OBJECT };
    gtk_list_store_set_column_types (GTK_LIST_STORE (store), 3, types);

    store->priv = GW_VOCABULARYLISTSTORE_GET_PRIVATE (store);
    memset(store->priv, 0, sizeof(GwVocabularyListStorePrivate));

    gw_vocabularyliststore_revert_all (store);
}


static void 
gw_vocabularyliststore_finalize (GObject *object)
{
    GwVocabularyListStore *store;

    store = GW_VOCABULARYLISTSTORE (object);

    gtk_list_store_clear (GTK_LIST_STORE (store));

    G_OBJECT_CLASS (gw_vocabularyliststore_parent_class)->finalize (object);
}


static void
gw_vocabularyliststore_class_init (GwVocabularyListStoreClass *klass)
{
    //Declarations
    GObjectClass *object_class;

    //Initializations
    object_class = G_OBJECT_CLASS (klass);
    object_class->finalize = gw_vocabularyliststore_finalize;

    g_type_class_add_private (object_class, sizeof (GwVocabularyListStorePrivate));
}


GtkListStore*
gw_vocabularyliststore_get_wordstore_by_iter (GwVocabularyListStore *store, GtkTreeIter *iter)
{
    GtkListStore *wordstore;
    GtkTreeModel *model;
    model = GTK_TREE_MODEL (store);
    gtk_tree_model_get (model, iter, GW_VOCABULARYLISTSTORE_COLUMN_OBJECT, &wordstore, -1);
    gw_vocabularywordstore_load (GW_VOCABULARYWORDSTORE (wordstore));
    g_object_unref (wordstore);
    return wordstore;
}


GtkListStore*
gw_vocabularyliststore_get_wordstore_by_index (GwVocabularyListStore *store, gint index)
{
    //Declarations
    GtkTreeModel *model;
    GtkListStore *wordstore;
    GtkTreeIter iter;
    gboolean valid;

    //Initializations
    model = GTK_TREE_MODEL (store);
    valid = gtk_tree_model_get_iter_first (model, &iter);
    wordstore = NULL;

    while (valid && index > 0)
    {
      valid = gtk_tree_model_iter_next (model, &iter);
      index--;
    }

    if (valid)
    {
      gw_vocabularyliststore_get_wordstore_by_iter (store, &iter);
    }

    return wordstore;
}


//!
//! @brief Deletes files that have to coresponding vocabulary list
//!
void
gw_vocabularyliststore_clean_files (GwVocabularyListStore *store)
{
    //Definitions
    GDir *dir;
    gchar *uri;
    gchar *filename;
    const gchar *name;

    if ((uri = lw_util_build_filename (LW_PATH_VOCABULARY, NULL)) != NULL)
    {
      if ((dir = g_dir_open (uri, 0, NULL)) != NULL)
      {
        while ((name = g_dir_read_name (dir)) != NULL)
        {
          if (!gw_vocabularyliststore_list_exists (store, name))
          {
            if ((filename = g_build_filename (uri, name, NULL)) != NULL)
            {
              g_remove (filename);
              g_free (filename); filename = NULL;
            }
          }
        }
        g_dir_close (dir); dir = NULL;
      }
      g_free (uri); uri = NULL;
    }
}


gboolean
gw_vocabularyliststore_list_exists (GwVocabularyListStore *store, const gchar *NAME)
{
    //Declarations
    GtkTreeModel *model;
    GtkTreeIter iter;
    gboolean valid;
    gchar *name;
    gboolean exists;

    model = GTK_TREE_MODEL (store);
    valid = gtk_tree_model_get_iter_first (model, &iter);
    exists = FALSE;

    while (valid && !exists)
    {
      gtk_tree_model_get (model, &iter, GW_VOCABULARYLISTSTORE_COLUMN_NAME, &name, -1);
      if (name != NULL)
      {
        if (strcmp(NAME, name) == 0) exists = TRUE;
      }
      valid = gtk_tree_model_iter_next (model, &iter);
    }

    return exists;
}


void
gw_vocabularyliststore_save_all (GwVocabularyListStore *store)
{
    //Declarations
    GtkTreeModel *model;
    GtkListStore *wordstore;
    GtkTreeIter iter;
    gboolean valid;

    gw_vocabularyliststore_clean_files (store);
    valid = gtk_tree_model_get_iter_first (GTK_TREE_MODEL (store), &iter);
    model = GTK_TREE_MODEL (store);

    while (valid)
    {
      gtk_tree_model_get (model, &iter, GW_VOCABULARYLISTSTORE_COLUMN_OBJECT, &wordstore, -1);
      if (wordstore != NULL)
      {
        gw_vocabularywordstore_save (GW_VOCABULARYWORDSTORE (wordstore));
        gw_vocabularywordstore_set_has_changes (GW_VOCABULARYWORDSTORE (wordstore), FALSE);
        g_object_unref (wordstore); wordstore = NULL;
      }
      valid = gtk_tree_model_iter_next (model, &iter);
    }
}


void
gw_vocabularyliststore_revert_all (GwVocabularyListStore* store)
{
    //Declarations
    GtkTreeIter iter;
    gchar **lists;
    int i;
    GtkListStore *liststore;
    GtkListStore *wordstore;

    //Initializations
    liststore = GTK_LIST_STORE (store);

    gtk_list_store_clear (liststore);

    if ((lists = lw_vocabularylist_get_lists ()) != NULL)
    {
      for (i = 0; lists[i] != NULL; i++)
      {
        wordstore = gw_vocabularywordstore_new (lists[i]);
        gtk_list_store_append (liststore, &iter);
        gtk_list_store_set (liststore, &iter, 
          GW_VOCABULARYLISTSTORE_COLUMN_NAME, lists[i],
          GW_VOCABULARYLISTSTORE_COLUMN_CHANGED, FALSE,
          GW_VOCABULARYLISTSTORE_COLUMN_OBJECT, wordstore,
          -1);
      }
      g_strfreev (lists); lists = NULL;
    }
}


gchar*
gw_vocabularyliststore_build_unique_name (GwVocabularyListStore *store)
{
    GwVocabularyListStorePrivate *priv;
    gchar *name;

    priv = store->priv;
    name = g_strdup_printf (gettext("New List %d"), ++priv->list_new_index);

    while (gw_vocabularyliststore_list_exists (store, name))
    {
      g_free (name);
      name = g_strdup_printf (gettext("New List %d"), ++priv->list_new_index);
    }

    return name;
}


void
gw_vocabularyliststore_new_list (GwVocabularyListStore *store, GtkTreeIter *iter)
{
    //Declarations
    gchar *name;
    GtkListStore *wordstore;

    name = gw_vocabularyliststore_build_unique_name (store);
    wordstore = gw_vocabularywordstore_new (name);
    
    gtk_list_store_append (GTK_LIST_STORE (store), iter);
    gtk_list_store_set (GTK_LIST_STORE (store), iter, 
        GW_VOCABULARYLISTSTORE_COLUMN_NAME, name, 
        GW_VOCABULARYLISTSTORE_COLUMN_CHANGED, FALSE, 
        GW_VOCABULARYLISTSTORE_COLUMN_OBJECT, wordstore,
    -1);

    gw_vocabularywordstore_set_has_changes (GW_VOCABULARYWORDSTORE (wordstore), TRUE);

    g_free (name);
}


void
gw_vocabularyliststore_remove_path_list (GwVocabularyListStore *store, GList *list)
{
    //Declarations
    GList *link;
    GtkTreeIter iter;
    GtkTreePath *path;

    //Convert the tree paths to row references
    for (link = g_list_last (list); link != NULL; link = link->prev)
    {
      path = (GtkTreePath*) link->data;
      gtk_tree_model_get_iter (GTK_TREE_MODEL (store), &iter, path);
      gtk_list_store_remove (GTK_LIST_STORE (store), &iter);
    }
}


gboolean
gw_vocabularyliststore_has_changes (GwVocabularyListStore *store)
{
    //Declarations
    gboolean has_changes;
    GtkTreeIter iter;
    gboolean valid;
    GwVocabularyWordStore *wordstore;
    GtkTreeModel *model;

    //Initializations
    has_changes = FALSE;
    model = GTK_TREE_MODEL (store);
    valid = gtk_tree_model_get_iter_first (model, &iter);

    while (valid && !has_changes)
    {
      gtk_tree_model_get (model, &iter, GW_VOCABULARYLISTSTORE_COLUMN_OBJECT, &wordstore, -1);
      if (wordstore != NULL)
      {
        if (gw_vocabularywordstore_has_changes (GW_VOCABULARYWORDSTORE (wordstore)))
          has_changes = TRUE;
        g_object_unref (wordstore); wordstore = NULL;
      }
      valid = gtk_tree_model_iter_next (model, &iter);
    }

    return has_changes; 
}
