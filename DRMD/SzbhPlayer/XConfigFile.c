

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#include "SzbhDefine.h"
#include "XStationText.h"
#include "XMenuShow.h"
#include "XConfigFile.h"

XSectionList *pSectionList=NULL;

static u32 StringToHex(char *pInputData)
{
	if(pInputData==NULL)
		return 0;

	int tTempDataLen=strlen(pInputData);
	if(tTempDataLen<3)
		return 0;

	if(pInputData[0]!='0' || pInputData[1]!='x')
		return 0;

	u32 tTempRet=0;
	sscanf(pInputData, "%x", &tTempRet);
	
	return tTempRet;
}

static void ParseKeyValue(char *pInputString, int tInputLen, XConfigSection *pInputSection)
{
	if(pInputString==NULL || tInputLen<=0 || pInputSection==NULL)
		return ;

	char pTempKey[64]={0};
	char pTempVal[64]={0};

	char *pTempSplitPt=strchr(pInputString, '=');
	if(pTempSplitPt==NULL)
		return ;

	int tTempSplitLen=pTempSplitPt-pInputString;
	int i=0;
	int k=0;
	for(i=0; i<tTempSplitLen; i++)
	{
		if(pInputString[i]<=0x20)
		{
			if(k==0)
				continue;
			else
				break;
		}

		pTempKey[k++]=pInputString[i];
	}

	k=0;
	for(i=tTempSplitLen+1; i<tInputLen; i++)
	{
		if(pInputString[i]<=0x20)
		{
			if(k==0)
				continue;
			else
				break;
		}

		pTempVal[k++]=pInputString[i];
	}

//	printf("Damon ==> key va: %s - %s \n", pTempKey, pTempVal);

	if(strcmp(pTempKey, "enable")==0)
	{
		pInputSection->mEnable=atoi(pTempVal);		
	}else if(strcmp(pTempKey, "type")==0)
	{
		pInputSection->mType=atoi(pTempVal);
	}else if(strcmp(pTempKey, "move")==0)
	{
		pInputSection->mMove=atoi(pTempVal);		
	}else if(strcmp(pTempKey, "xpos")==0)
	{
		pInputSection->mXPos=atoi(pTempVal);
	}else if(strcmp(pTempKey, "ypos")==0)
	{
		pInputSection->mYPos=atoi(pTempVal);		
	}else if(strcmp(pTempKey, "width")==0)
	{
		pInputSection->mWidth=atoi(pTempVal);		
	}else if(strcmp(pTempKey, "height")==0)
	{
		pInputSection->mHeight=atoi(pTempVal);		
	}else if(strcmp(pTempKey, "pic_type")==0)
	{
		if(strcasecmp(pTempVal, "bmp")==0)
			pInputSection->mPicType=1;
		else if(strcasecmp(pTempVal, "png")==0)
			pInputSection->mPicType=2;
		else if(strcasecmp(pTempVal, "jpg")==0)
			pInputSection->mPicType=3;
		else if(strcasecmp(pTempVal, "gif")==0)
			pInputSection->mPicType=4;
	}else if(strcmp(pTempKey, "pic_name")==0)
	{
		strncpy(pInputSection->pPicName, pTempVal, dXPicNameLen);
	}else if(strcmp(pTempKey, "font_size")==0)
	{
		pInputSection->mFontSize=atoi(pTempVal);
	}else if(strcmp(pTempKey, "font_color")==0)
	{
		pInputSection->mFontColor=StringToHex(pTempVal);
	}else if(strcmp(pTempKey, "font_backcolor")==0)
	{
		pInputSection->mFontBackColor=StringToHex(pTempVal);		
	}

}

void XSectionListDeleteAll(void)
{
	if(pSectionList==NULL || pSectionList->mCount<=0)
		return ;

	XSectionNode *pTempNode=pSectionList->pHeader;
	XSectionNode *pTempFree=NULL;
	while(pTempNode)
	{
		pTempFree=pTempNode;
		pTempNode=pTempNode->pNext;

		if(pTempFree->pData)
			free(pTempFree->pData);
		free(pTempFree);
	}

	pSectionList->pHeader=NULL;
	pSectionList->pTail=NULL;
	pSectionList->mCount=0;
}

static void XSectionListAdd(XConfigSection *pInputNewSection)
{
	if(pInputNewSection==NULL || pSectionList==NULL)
		return ;

/*	printf("Damon ==> %d %d %d %d %d - %d  %s - %d 0x%x 0x%x \n", pInputNewSection->mType, 
		pInputNewSection->mXPos, pInputNewSection->mYPos, pInputNewSection->mWidth, pInputNewSection->mHeight,
		pInputNewSection->mPicType, pInputNewSection->pPicName, 
		pInputNewSection->mFontSize, pInputNewSection->mFontColor, pInputNewSection->mFontBackColor);*/

	XSectionNode *pTempNewNode=(XSectionNode *)malloc(sizeof(XSectionNode));
	if(pTempNewNode==NULL)
	{
		printf("Damon ==> [%s][%d] : Error not enough memory \n", __FUNCTION__, __LINE__);
		free(pInputNewSection);
		return ;
	}
	pTempNewNode->pData=pInputNewSection;
	pTempNewNode->pNext=NULL;
	
	if(pSectionList->pHeader==NULL)
	{
		pSectionList->pHeader=pTempNewNode;
		pSectionList->pTail=pTempNewNode;
	}else
	{
		pSectionList->pTail->pNext=pTempNewNode;
		pSectionList->pTail=pTempNewNode;
	}

	pSectionList->mCount++;
}

int XConfigLoadTemplate(int tInputTemplateIdx)
{
	char pTempString[64]={0};
	sprintf(pTempString, "%s/Template%d/template.conf", dXResourcePath, tInputTemplateIdx);

	FILE *pTempFile=fopen(pTempString, "r");
	if(pTempFile==NULL)
	{
		printf("Damon ==> [%s][%d] : Error open file failed \n", __FUNCTION__, __LINE__);
		return -1;
	}

	int i=0;
	char *pTempLine=NULL;
	XConfigSection *pTempSection=NULL;
	size_t tTempLen;
	size_t tTempReadLen;
	int tTempFindSection=0;
	int tTempBreakLine=0;

	XSectionListDeleteAll();

	while((tTempReadLen=getline(&pTempLine, &tTempLen, pTempFile))!=-1)
	{
	//	printf("Damon ==> %d %d : %s \n", tTempReadLen, tTempLen, pTempLine);
		tTempBreakLine=0;
		for(i=0; i<tTempReadLen; i++)
		{
			if(pTempLine[i]<=0x20)
				continue;
			if(pTempLine[i]=='#')
				tTempBreakLine=1;
			break;
		}

		if(tTempBreakLine==0 && i<tTempReadLen)
		{
			char *pTempStartPt=strchr(pTempLine+i, '[');
			char *pTempEndPt=strchr(pTempLine+i, ']');
			if(pTempStartPt!=NULL && pTempEndPt!=NULL)
			{
				if(pTempSection!=NULL)
				{
				//	printf("Damon ==> section name : %s \n\n", pTempSection->pSectionName);
					XSectionListAdd(pTempSection);
				}
			
				pTempSection=(XConfigSection *)malloc(sizeof(XConfigSection));
				if(pTempSection!=NULL)
					memset(pTempSection, 0, sizeof(XConfigSection));

				tTempFindSection=1;

				int k=0;
				i++;
				for(; i<pTempEndPt-pTempStartPt; i++)
				{
					if(pTempLine[i]<=0x20)
					{
						if(k==0)
							continue;
						else
							break;
					}
					pTempSection->pSectionName[k++]=pTempLine[i];
				}
				
				if(k==0 && pTempSection!=NULL)
				{
					free(pTempSection);
					pTempSection=NULL;
				}
			}else if(tTempFindSection==0)
			{
				continue;
			}else
			{
				ParseKeyValue(pTempLine+i, tTempReadLen-i, pTempSection);
			}
		}
	}
	
	if(pTempSection!=NULL)
	{
	//	printf("Damon ==> section name : %s \n\n", pTempSection->pSectionName);
		XSectionListAdd(pTempSection);
	}

	if(pTempLine)
		free(pTempLine);

	fclose(pTempFile);

	if(pSectionList->mCount<=0)
	{
		printf("Damon ==> not fount section \n");
		return -2;
	}

	printf("Damon ==> section count : %d \n", pSectionList->mCount);
	snprintf(pTempString, sizeof(pTempString)-1, "%s/font.ttf", dXResourcePath);
	XShowFontInit(pTempString);
	
/*	XSectionNode *pTempNode=pSectionList->pHeader;
	while(pTempNode)
	{
		XConfigSection *pTempSection=pTempNode->pData;
		printf("Damon ==> %s : %d %d %d %d %d - %d  %s - %d 0x%x 0x%x \n", pTempSection->pSectionName, pTempSection->mType, 
			pTempSection->mXPos, pTempSection->mYPos, pTempSection->mWidth, pTempSection->mHeight,
			pTempSection->mPicType, pTempSection->pPicName, 
			pTempSection->mFontSize, pTempSection->mFontColor, pTempSection->mFontBackColor);

		pTempNode=pTempNode->pNext;
	}*/


	XStationTextInit(tInputTemplateIdx);

	return 0;
}

XConfigSection *XConfigGetSection(char *pInputSectionName)
{
	if(pInputSectionName==NULL || pSectionList==NULL || pSectionList->mCount==0)
		return NULL;

	XSectionNode *pTempRet=pSectionList->pHeader;
	while(pTempRet)
	{
		if(pTempRet->pData!=NULL)
		{
			if(strcmp(pTempRet->pData->pSectionName, pInputSectionName)==0)
				break;
		}
		
		pTempRet=pTempRet->pNext;
	}

	if(pTempRet==NULL)
		return NULL;

	return pTempRet->pData;
}

XConfigSection *XConfigGetSectionWithIdx(int tInputIdx)
{
	if(pSectionList==NULL || pSectionList->mCount==0 || tInputIdx>=pSectionList->mCount)
		return NULL;

	int i=0;
	XSectionNode *pTempRet=pSectionList->pHeader;
	while(pTempRet)
	{
		if(i==tInputIdx)
			break;
		i++;
		pTempRet=pTempRet->pNext;
	}

	if(pTempRet==NULL)
		return NULL;

	return pTempRet->pData;
}


int XConfigGetSectionNum(void)
{
	if(pSectionList==NULL || pSectionList->mCount<=0)
		return 0;

	return pSectionList->mCount;
}

int XConfigFileInit(void)
{
	if(pSectionList==NULL)
		pSectionList=(XSectionList *)malloc(sizeof(XSectionList));

	if(pSectionList)
	{
		pSectionList->pHeader=NULL;
		pSectionList->pTail=NULL;
		pSectionList->mCount=0;
	}

	return 0;
}


