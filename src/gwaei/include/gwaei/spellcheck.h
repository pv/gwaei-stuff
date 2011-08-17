#ifndef GW_SPELLCHECK_INCLUDED
#define GW_SPELLCHECK_INCLUDED


typedef enum {
  GW_SPELLCHECK_SIGNALID_DRAW,
  GW_SPELLCHECK_SIGNALID_CHANGED,
  GW_SPELLCHECK_SIGNALID_POPULATE_POPUP,
  GW_SPELLCHECK_SIGNALID_UPDATE_TIMEOUT,
  TOTAL_GW_SPELLCHECK_SIGNALIDS
} GwSpellcheckSignalId;

struct _GwSpellcheck {
  GtkEntry *entry;
  GList *corrections;
  GMutex *mutex;
  gboolean needs_spellcheck;
  char* query_text;
  gboolean sensitive;
  gboolean running_check;
  int timeout;
  guint signalid[TOTAL_GW_SPELLCHECK_SIGNALIDS];
};
typedef struct _GwSpellcheck GwSpellcheck;

#define GW_SPELLCHECK(object) (GwSpellcheck*)object

GwSpellcheck* gw_spellcheck_new (GtkEntry*);
void gw_spellcheck_free (GwSpellcheck*);

int gw_spellcheck_get_x_offset (GwSpellcheck *spellcheck);
int gw_spellcheck_get_y_offset (GwSpellcheck *spellcheck);

struct _SpellingReplacementData {
    GtkEntry *entry;
    int start_offset;
    int end_offset;
    char* replacement_text;
};
typedef struct _SpellingReplacementData _SpellingReplacementData;

struct _GwSpellcheckStreamWithData {
    GwSpellcheck *spellcheck;
    int stream;
    gpointer data;
    int length;
    GPid pid;
};
typedef struct _GwSpellcheckStreamWithData GwSpellcheckStreamWithData;

GwSpellcheckStreamWithData* gw_spellcheck_streamwithdata_new (GwSpellcheck*, int, const char*, int, GPid);
void gw_spellcheck_streamwithdata_free (GwSpellcheckStreamWithData*);

#include <gwaei/spellcheck-callbacks.h>

#endif
