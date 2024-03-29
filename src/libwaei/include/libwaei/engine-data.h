#ifndef LW_ENGINEDATA_INCLUDED
#define LW_ENGINEDATA_INCLUDED

#define LW_ENGINEDATA(object) (LwEngineData*) object

struct _LwEngineData {
    LwSearchItem *item;
    gboolean exact;
};
typedef struct _LwEngineData LwEngineData;

LwEngineData* lw_enginedata_new (LwSearchItem*, gboolean);
void lw_enginedata_free (LwEngineData*);

#endif
