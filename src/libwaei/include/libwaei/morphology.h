#ifndef LW_MORPHOLOGY_INCLUDED
#define LW_MORPHOLOGY_INCLUDED 

//!
//! @brief Morphological analysis of input
//!
struct _LwMorphologyItem {
    gchar *word;           //!< Original word
    gchar *base_form;      //!< Deduced (most likely) dictionary form of the word. NULL if no result.
    gchar *explanation;    //!< Free-form explanation of the morphological analysis. NULL if none.
};
typedef struct _LwMorphologyItem LwMorphologyItem;
struct _LwMorphology {
    GList *items;            //!< Morphology items found in the input
};
typedef struct _LwMorphology LwMorphology;

LwMorphology* lw_morphology_new (const gchar *);
void lw_morphology_free (LwMorphology*);

#endif
