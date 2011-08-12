#ifndef LW_ENGINEDATA_INCLUDED
#define LW_ENGINEDATA_INCLUDED

#define LW_ENGINEDATA(object) (LwEngineData*) object

struct _LwEngineData {
    LwEngine *engine;
    LwSearchItem *item;
};
typedef struct _LwEngineData LwEngineData;

LwEngineData* lw_enginedata_new (LwEngine*, LwSearchItem*);
void lw_enginedata_free (LwEngineData*);

#endif
