

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#include "XSectionFile.h"
#include "XStationText.h"

#define dXSectionStationName	"station_name"

static int tXPrevTemplate=0;
static XStationName *pXStationList=NULL;
static int tXStationCount=0;

extern void XStationSurfaceReload(int tInputStationCount);


static void XStationListFree(void)
{
	while(pXStationList)
	{
		XStationName *pTempFree=pXStationList;
		pXStationList=pXStationList->pNext;

		free(pTempFree);
	}

	pXStationList=NULL;
	tXStationCount=0;
}

static void XStationTextAdd(int tInputStationId, char *pInputName, int tInputNameLen)
{
	XStationName *pTempNew=malloc(sizeof(XStationName));

	if(pTempNew==NULL)
		return ;

	pTempNew->mId=0;
	memset(pTempNew->pName, 0, sizeof(pTempNew->pName));
	pTempNew->pNext=NULL;

	pTempNew->mId=tInputStationId;
	strncpy(pTempNew->pName, pInputName, tInputNameLen);

	if(pXStationList==NULL)
		pXStationList=pTempNew;
	else
	{
		XStationName *pTempNext=pXStationList;
		while(pTempNext->pNext!=NULL)
		{
			pTempNext=pTempNext->pNext;
		}

		pTempNext->pNext=pTempNew;
	}

	tXStationCount++;
}


char *XStationTextGetString(int tInputIdx)
{
	if(pXStationList==NULL)
		return NULL;

	XStationName *pTempNext=pXStationList;
	while(pTempNext)
	{
		if(pTempNext->mId==tInputIdx)
			break;
		
		pTempNext=pTempNext->pNext;
	}

	if(pTempNext)
		return pTempNext->pName;

	return NULL;
}

int XStationGetCount(void)
{
	return tXStationCount;
}

int XStationTextInit(int tInputCurTempLate)
{
	if(tInputCurTempLate==tXPrevTemplate)
		return 0;

	if(pXStationList!=NULL)
		XStationListFree();

	tXPrevTemplate=tInputCurTempLate;

	char pTempName[64]={0};
	char pTempString[128]={0};

	sprintf(pTempName, "/usrdata/disp/Template%d/station.conf", tInputCurTempLate);

	memset(pTempString, 0, sizeof(pTempString));
	if(XSectionGetValue(pTempName, dXSectionStationName, "count", pTempString, sizeof(pTempString)-1))
		return -1;

	int tTempStationCount=atoi(pTempString);
	printf("Damon ==> station count=%d \n", tTempStationCount);
	if(tTempStationCount<=0 || tTempStationCount>100)
		return -2;

	int i=0;
	char pTempKey[8]={0};
	for(i=0; i<tTempStationCount; i++)
	{
		sprintf(pTempKey, "%03d", i+1);
		if(XSectionGetValue(pTempName, dXSectionStationName, pTempKey, pTempString, sizeof(pTempString)-1)==0)
		{
			char *pTempStartPt=strchr(pTempString, '\"');
			if(pTempStartPt!=NULL)
			{
				char *pTempEndPt=strchr(pTempStartPt+1, '\"');
				if(pTempEndPt!=NULL)
				{
					XStationTextAdd(i+1, pTempStartPt+1, pTempEndPt-pTempStartPt-1);
				}
			}
		}
	}

	// current \ next \ end station
	for(i=100; i<=102; i++)
	{
		sprintf(pTempKey, "%03d", i);
		if(XSectionGetValue(pTempName, dXSectionStationName, pTempKey, pTempString, sizeof(pTempString)-1)==0)
		{
			char *pTempStartPt=strchr(pTempString, '\"');
			if(pTempStartPt!=NULL)
			{
				char *pTempEndPt=strchr(pTempStartPt+1, '\"');
				if(pTempEndPt!=NULL)
				{
					XStationTextAdd(i, pTempStartPt+1, pTempEndPt-pTempStartPt-1);
				}
			}
		}
	}
	

	if(pXStationList)
	{
		XStationName *pTempTest=pXStationList;
		while(pTempTest)
		{
			printf("Damon ==> station:[%d][%s]\n", pTempTest->mId, pTempTest->pName);
			pTempTest=pTempTest->pNext;
		}
	}

	XStationSurfaceReload(tXStationCount);

	return 0;
}

