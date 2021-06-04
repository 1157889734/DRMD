

#ifndef __X_STATION_TEXT_H__
#define __X_STATION_TEXT_H__

typedef struct __StationName{
	int mId;
	char pName[128];

	struct __StationName *pNext;
}XStationName;


int XStationTextInit(int tInputCurTempLate);
int XStationGetCount(void);
char *XStationTextGetString(int tInputIdx);


#endif

