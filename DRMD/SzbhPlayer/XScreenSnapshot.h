

#ifndef __X_SCREEN_SNAPSHOT_H__
#define __X_SCREEN_SNAPSHOT_H__

#include "SzbhDefine.h"

int XScreenSnapshotEncodeBmp(ELayerId tInputLayerId, X2DRect tInputRect, char *pInputFileName);
XSurface *XScreenSnapshot(ELayerId tInputLayerId, X2DRect tInputRect);
int XScreenSnapshotOutFile(EPicType tInputPicType, char *pInputFileName);

#endif

