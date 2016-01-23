#include "common.h"

typedef bool (*TitleFilterFn)(u64 tid);

void titlesClear(void);
int titlesCount(void);
void titlesGetEntry(u64* outTid, u8* outMediatype, int index);
bool titlesExists(u64 tid, u8 mediatype);
bool titlesCheckUpdate(bool async, UIState newState);
bool titlesLoadSmdh(smdh_s* smdh, u8 mediatype, u64 tid);
