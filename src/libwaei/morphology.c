/******************************************************************************
    AUTHOR:
    File written and Copyrighted by Pauli Virtanen. All Rights Reserved.

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
//! @file morphology.c
//!

#include <stdlib.h>
#include <stdio.h>

#include <glib.h>

#include <libwaei/libwaei.h>
#include "config.h"

static void _init_analysis();
static void _analyse(LwMorphology *result, const char *input);
static LwMorphologyItem *lw_morphology_item_new();
static void lw_morphology_item_free(LwMorphologyItem *item);

static gboolean _analysis_initialized = FALSE;
static GStaticMutex _morphology_mutex = G_STATIC_MUTEX_INIT;

//!
//! @brief Analyses input 
//! @param input The input to study
//! @return Returns an allocated LwMorphology object that should be freed with lw_morphology_new_free or NULL on error
//!
LwMorphology *lw_morphology_new (const gchar *input)
{
    LwMorphology *result;

    if (!_analysis_initialized) {
        g_static_mutex_lock(&_morphology_mutex);
        if (!_analysis_initialized) { // guard against race
            _init_analysis();
        }
        _analysis_initialized = TRUE;
        g_static_mutex_unlock(&_morphology_mutex);
    }

    result = (LwMorphology*) malloc(sizeof(LwMorphology));
    result->items = NULL;
    _analyse(result, input);
    return result;
}

//!
//! @brief Frees an allocated LwMorphology object. 
//! @param item The object to free
//!
void lw_morphology_free (LwMorphology *item)
{
    if (item->items)
        g_list_free_full(item->items, lw_morphology_item_free);
    free(item);
}

//!
//! @brief Allocates a new empty LwMorphologyItem object.
//!
static LwMorphologyItem *lw_morphology_item_new()
{
    LwMorphologyItem *item;
    item = (LwMorphology*) malloc(sizeof(LwMorphology));
    item->word = NULL;
    item->base_form = NULL;
    item->explanation = NULL;
    return item;
}

//!
//! @brief Frees an allocated LwMorphologyItem object.
//!
static void lw_morphology_item_free(LwMorphologyItem *item)
{
    if (item->word)
        g_free(item->word);
    if (item->base_form)
        g_free(item->base_form);
    if (item->explanation)
        g_free(item->explanation);
    free(item);
}


#if defined(HAVE_MECAB)

//
// Morphological analysis using the Mecab engine
//

#include <mecab.h>

static mecab_t *mecab = NULL;

// Keywords used by Mecab dictionaries (ID numbers vary between different
// dictionaries, and cannot be relied on, so just compare UTF-8 strings)
#define ID_VERB          "動詞"
#define ID_NOUN          "名詞"
#define ID_SUFFIX        "接尾辞"
#define ID_POSTPOSITION  "助詞"
#define ID_AUX_VERB      "助動詞"
#define ID_NON_INDEPENDENT   "非自立"

#define PLAIN_COPULA "だ"

//!
//! @brief Initializes the Mecab analysis engine.
//!
static void _init_analysis()
{
    char *argv[] = {"mecab", NULL};
    mecab = mecab_new(sizeof(argv)/sizeof(char*)-1, argv);
    if (mecab == NULL) {
        // Report that something went wrong
        g_warning("Failed to initialize Mecab engine: %s", mecab_strerror(NULL));
    }
}

//!
//! @brief Convert string from UTF-8 to Mecab's charset.
//!
static char *_encode_to_mecab(char *word, int nbytes)
{
    const mecab_dictionary_info_t *info = mecab_dictionary_info(mecab);
    gsize bytes_read, bytes_written;
    return g_convert(word, nbytes, info->charset, "UTF-8", &bytes_read, &bytes_written, NULL);
}

//!
//! @brief Convert string from Mecab's charset to UTF-8.
//!
static char *_decode_from_mecab(char *word, int nbytes)
{
    const mecab_dictionary_info_t *info = mecab_dictionary_info(mecab);
    gsize bytes_read, bytes_written;
    return g_convert(word, nbytes, "UTF-8", info->charset, &bytes_read, &bytes_written, NULL);
}


//!
//! @brief Morphological analysis of input using Mecab
//!
static void _analyse(LwMorphology *result, const char *input_raw)
{
    const mecab_node_t *node;
    char **fields = NULL, *surface = NULL;
    char *temp;
    char *output = NULL;
    char *input = NULL;
    LwMorphologyItem *item = NULL;

    if (!mecab) {
        return;
    }

    g_static_mutex_lock(&_morphology_mutex);

    input = _encode_to_mecab(input_raw, -1);
    if (!input)
        goto fail;
    node = mecab_sparse_tonode(mecab, input);

#define FLUSH_ITEM                                                                            \
        do {                                                                                  \
            if (item->explanation && !g_str_has_prefix(item->explanation, item->base_form)) { \
                temp = g_strdup_printf("%s(%s)", item->explanation, item->base_form);         \
                g_free(item->explanation);                                                    \
                item->explanation = temp;                                                     \
            }                                                                                 \
            result->items = g_list_prepend(result->items, item);                              \
            item = NULL;                                                                      \
        } while (0)

    for (; node; node = node->next) {
        char *base_form, *word_class;
        char *p;
        gboolean start_word = FALSE;

        if (node->stat != MECAB_NOR_NODE) {
            continue;
        }

        // Parse input
        p = _decode_from_mecab(node->feature, -1);
        if (!p)
            goto fail;
        fields = g_strsplit(p, ",", -1);
        g_free(p);

        if (g_strv_length(fields) < 7) {
            goto fail;
        }

        surface = _decode_from_mecab(node->surface, node->length);
        word_class = fields[0];
        base_form = fields[6];

        p = g_strrstr(base_form, ":");
        if (p) {
            base_form = p+1;
        }
        if (g_str_has_suffix(base_form, PLAIN_COPULA) &&
            !g_str_has_suffix(surface, PLAIN_COPULA)) {
            // Mecab may add the copula to adjectives -- strip it
            base_form[strlen(base_form) - 3] = '\0';
        }

        // Check whether to start a new word here
        start_word = TRUE;
        if (g_str_has_prefix(word_class, ID_SUFFIX) ||
                g_str_has_prefix(word_class, ID_AUX_VERB)) {
            // Skip suffixes & aux. verbs
            start_word = FALSE;
        }
        if (g_str_has_prefix(word_class, ID_POSTPOSITION) &&
                g_utf8_strlen(surface, -1) == 1) {
            // Skip single-letter postpositions (NO, TE, etc.)
            start_word = FALSE;
        }
        if (base_form[0] == '*' || base_form[0] == '\0') {
            // Start a word only if there is some base form listed
            start_word = FALSE;
        }

        // Process input
        if (start_word) {
            if (item) {
                FLUSH_ITEM;
            }
            item = lw_morphology_item_new();
            item->word = g_strdup(surface);
            item->base_form = g_strdup(base_form);
            item->explanation = NULL;
        }
        else {
            if (item) {
                temp = g_strconcat(item->word, surface, NULL);
                g_free(item->word);
                item->word = temp;
            }
        }

        // Construct explanation
        if (item) {
            if (item->explanation == NULL) {
                item->explanation = g_strdup(surface);
            }
            else {
                temp = g_strdup_printf ("%s-%s", item->explanation, surface);
                g_free(item->explanation);
                item->explanation = temp;
            }
        }

        g_strfreev(fields);
        g_free(surface);
        fields = NULL;
        surface = NULL;
    }

    if (item) {
        FLUSH_ITEM;
    }

    g_free(input);

    g_static_mutex_unlock(&_morphology_mutex);

    result->items = g_list_reverse(result->items);

    return;

fail:
    g_static_mutex_unlock(&_morphology_mutex);
    if (item)
        lw_morphology_item_free(item);
    if (fields)
        g_strfreev(fields);
    if (surface)
        g_free(surface);
    if (input)
        g_free(input);
    return;
}

#else

//
// Dummy implementation: no analysis
//

static void _init_analysis() {}
static void _analyse(LwMorphology *result, const char *input) {}

#endif
