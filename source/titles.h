#include "common.h"

typedef bool (*TitleFilterFn)(u64 tid);

void titlesClear(void);
bool titlesExists(u64 tid, u8 mediatype);
bool titlesCheckUpdate(bool async, UIState newState);
Result titlesLoadSmdh(smdh_s* smdh, u8 mediatype, u64 tid);
