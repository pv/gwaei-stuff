#ifndef GW_REGEX_INCLUDED
#define GW_REGEX_INCLUDED

#define GW_RE_COMPILE_FLAGS        (G_REGEX_CASELESS | G_REGEX_OPTIMIZE)
#define GW_RE_LOCATE_FLAGS   (0)
#define GW_RE_EXIST_FLAGS   (0)

#include <glib.h>
#include <gwaei/utilities.h>

typedef enum
{
  GW_RELEVANCE_HIGH,
  GW_RELEVANCE_MEDIUM,
  GW_RELEVANCE_LOW,
  GW_RELEVANCE_LOCATE,
  GW_RELEVANCE_TOTAL
} GwRelevance;


void gw_regex_initialize (void);
void gw_regex_free (void);

GRegex* gw_regex_kanji_new (const char*, GwEngine, GwRelevance);
GRegex* gw_regex_furi_new (const char*, GwEngine, GwRelevance);
GRegex* gw_regex_romaji_new (const char*, GwEngine, GwRelevance);
GRegex* gw_regex_mix_new (const char*, GwEngine, GwRelevance);
GRegex* gw_regex_new (const char*, GwEngine, GwRelevance);


typedef enum {
  GW_RE_STROKES,
  GW_RE_GRADE,
  GW_RE_FREQUENCY,
  GW_RE_JLPT,
/*
  GW_RE_WORD_I_ADJ_PASTFORM,
  GW_RE_WORD_I_ADJ_NEGATIVE,
  GW_RE_WORD_I_ADJ_TE_FORM,
  GW_RE_WORD_I_ADJ_CAUSATIVE,
  GW_RE_WORD_I_ADJ_CONDITIONAL,
  GW_RE_WORD_NA_ADJ_PASTFORM,
  GW_RE_WORD_NA_ADJ_NEGATIVE,
  GW_RE_WORD_NA_ADJ_TE_FORM,
  GW_RE_WORD_NA_ADJ_CAUSATIVE,
  GW_RE_WORD_NA_ADJ_CONDITIONAL,
*/
  GW_RE_TOTAL
} GwRegexDataIndex;

extern GRegex *gw_re[GW_RE_TOTAL + 1];

#endif
