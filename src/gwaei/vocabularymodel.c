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
//! @file vocabularymodel.c
//!
//! @brief To be written
//!

#include <stdlib.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <gtk/gtk.h>

#include <libwaei/libwaei.h>

#include <gwaei/vocabularymodel-private.h>
#include <gwaei/vocabularymodel.h>

G_DEFINE_TYPE (GwVocabularyModel, gw_vocabularymodel, GTK_TYPE_LIST_STORE)

typedef enum
{
  PROP_0,
  PROP_NAME,
} GwVocabularyModelProps;


GtkListStore*
gw_vocabularymodel_new (const gchar *NAME)
{
    //Declarations
    GwVocabularyModel *model;

    //Initializations
    model = GW_VOCABULARYMODEL (g_object_new (GW_TYPE_VOCABULARYMODEL,
                                              "name",        NAME,
                                              NULL));
    return GTK_LIST_STORE (model);

}


static void 
gw_vocabularymodel_init (GwVocabularyModel *model)
{
    GType types[] = { G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING };
    gtk_list_store_set_column_types (GTK_LIST_STORE (model), 3, types);

    model->priv = GW_VOCABULARYMODEL_GET_PRIVATE (model);
    memset(model->priv, 0, sizeof(GwVocabularyModelPrivate));
}


static void 
gw_vocabularymodel_finalize (GObject *object)
{
    GwVocabularyModel *model;
    GwVocabularyModelPrivate *priv;

    model = GW_VOCABULARYMODEL (object);
    priv = model->priv;

    if (priv->name != NULL) g_free (priv->name); 
    if (priv->vocabulary_list != NULL) lw_vocabularylist_free (priv->vocabulary_list); 

    G_OBJECT_CLASS (gw_vocabularymodel_parent_class)->finalize (object);
}


static void 
gw_vocabularymodel_set_property (GObject      *object,
                                 guint         property_id,
                                 const GValue *value,
                                 GParamSpec   *pspec)
{
    GwVocabularyModel *model;
    GwVocabularyModelPrivate *priv;

    model = GW_VOCABULARYMODEL (object);
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
gw_vocabularymodel_get_property (GObject      *object,
                                 guint         property_id,
                                 GValue       *value,
                                 GParamSpec   *pspec)
{
    GwVocabularyModel *model;
    GwVocabularyModelPrivate *priv;

    model = GW_VOCABULARYMODEL (object);
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
gw_vocabularymodel_class_init (GwVocabularyModelClass *klass)
{
    //Declarations
    GParamSpec *pspec;
    GObjectClass *object_class;

    //Initializations
    object_class = G_OBJECT_CLASS (klass);
    object_class->set_property = gw_vocabularymodel_set_property;
    object_class->get_property = gw_vocabularymodel_get_property;
    object_class->finalize = gw_vocabularymodel_finalize;

    g_type_class_add_private (object_class, sizeof (GwVocabularyModelPrivate));

    pspec = g_param_spec_string ("name",
                                 "Name of the vocabulary list",
                                 "Set vocabulary list's name.",
                                 "Vocabulary",
                                 G_PARAM_CONSTRUCT | G_PARAM_READWRITE
    );
    g_object_class_install_property (object_class, PROP_NAME, pspec);
}


void
gw_vocabularymodel_save (GwVocabularyModel *model)
{
    GwVocabularyModelPrivate *priv;

    priv = model->priv;
    priv->needs_sync = FALSE;
}


void 
gw_vocabularymodel_load (GwVocabularyModel *model)
{
    g_assert (model != NULL);

    if (gw_vocabularymodel_loaded (model)) return;

    //Declarations
    GwVocabularyModelPrivate *priv;
    GtkTreeIter treeiter;
    GList *listiter;
    LwVocabularyItem *item;

    //Initializations
    priv = model->priv;
    priv->needs_sync = FALSE;

    g_assert (priv->name != NULL);

    if (priv->vocabulary_list != NULL) lw_vocabularylist_free (priv->vocabulary_list);
    priv->vocabulary_list = lw_vocabularylist_new (priv->name);
    lw_vocabularylist_load (priv->vocabulary_list, NULL);

    for (listiter = priv->vocabulary_list->items; listiter != NULL; listiter = listiter->next)
    {
      item = LW_VOCABULARYITEM (listiter->data);
      gtk_list_store_append (GTK_LIST_STORE (model), &treeiter);
      gtk_list_store_set (GTK_LIST_STORE (model), &treeiter, 
          GW_VOCABULARYMODEL_COLUMN_KANJI, lw_vocabularyitem_get_kanji (item), 
          GW_VOCABULARYMODEL_COLUMN_FURIGANA, lw_vocabularyitem_get_furigana (item), 
          GW_VOCABULARYMODEL_COLUMN_DEFINITIONS, lw_vocabularyitem_get_definitions (item), 
      -1);
    }

    priv->loaded = TRUE;
}


void
gw_vocabularymodel_reset (GwVocabularyModel *model)
{
    GwVocabularyModelPrivate *priv;

    priv = model->priv;
    priv->needs_sync = FALSE;
}


LwVocabularyList*
gw_vocabularymodel_get_vocabularylist (GwVocabularyModel *model)
{
    GwVocabularyModelPrivate *priv;
    priv = model->priv;
    return priv->vocabulary_list;
}


void
gw_vocabularymodel_set_name (GwVocabularyModel *model, const gchar *name)
{
}


gboolean
gw_vocabularymodel_loaded (GwVocabularyModel *model)
{
    GwVocabularyModelPrivate *priv;
    priv = model->priv;
    return priv->loaded;
}
