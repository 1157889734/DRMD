

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>


#include "SzbhApi.h"
#include "XVehiclePlayerDefine.h"
#include "XConfigFile.h"
#include "XStationText.h"
#include "XScrollText.h"
#include "XMenuShow.h"



typedef struct {
	int mFontSize;
	unsigned int mFontHandle;
}XShowFont;
static int tXFontCount=0;
static XShowFont *pXShowFont=NULL;

XSurface *pXBackSurface=NULL;

static XSurface *pXTimeNumSurface[11];

#define dXStationNameMax		50
static XSurface *pXStationSurface[dXStationNameMax];

void XStationSurfaceReload(int tInputStationCount)
{
	static int tTempFirstInit=0;
	int i=0;

	if(tTempFirstInit==0)
	{
		tTempFirstInit=1;
		for(i=0; i<dXStationNameMax; i++)
			pXStationSurface[i]=NULL;
	}

	for(i=0; i<dXStationNameMax; i++)
	{
		if(pXStationSurface[i]!=NULL)
			Szbh_SurfaceDestroy(pXStationSurface[i]);
		pXStationSurface[i]=NULL;
	}

}

int XUnicodeToUtf8(ushort *pInputUnicode, int tInputUnicodeLen, unsigned char *pOutputData, int tInputDataLen, int *pOutputLen)
{
	if(pInputUnicode==NULL || tInputUnicodeLen<=0 || pOutputData==NULL || tInputDataLen<=0)
		return -1;

	int i=0, j=0;
	for(i=0; i<tInputUnicodeLen; i++)
	{	
		if(pInputUnicode[i]<=0x007f)
		{
			if(j<tInputDataLen)
			{
				pOutputData[j++]=(unsigned char)(pInputUnicode[i] & 0x7f);
			}else
			{
				break;	// out data len
			}
		}else if(pInputUnicode[i]>=0x0080 && pInputUnicode[i]<=0x07ff)
		{
			if(j<tInputDataLen-1)
			{
				pOutputData[j++]=((pInputUnicode[i] >> 6) & 0x1f) | 0xc0;
				pOutputData[j++]=(pInputUnicode[i] & 0x3f) | 0x80;
			}else
			{
				break;
			}
		}else if(pInputUnicode[i]>=0x0800 && pInputUnicode[i]<=0xffff)
		{
			if(j<tInputDataLen-2)
			{
				pOutputData[j++]=((pInputUnicode[i] >> 12) & 0x0f) | 0xe0;
				pOutputData[j++]=((pInputUnicode[i] >> 6) & 0x3f) | 0x80;
				pOutputData[j++]=(pInputUnicode[i] & 0x3f) | 0x80;
			}else
			{
				break;
			}
		}else if(pInputUnicode[i]>=0x010000 && pInputUnicode[i]<=0x1fffff)
		{
			if(j<tInputDataLen-3)
			{
				pOutputData[j++]=((pInputUnicode[i] >> 18) & 0x07) | 0xf0;
				pOutputData[j++]=((pInputUnicode[i] >> 12) & 0x3f) | 0x80;
				pOutputData[j++]=((pInputUnicode[i] >> 6) & 0x3f) | 0x80;
				pOutputData[j++]=(pInputUnicode[i] & 0x3f) | 0x80;
			}else
			{
				break;
			}
		}else if(pInputUnicode[i]>=0x200000 && pInputUnicode[i]<=0x03ffffff)
		{
			if(j<tInputDataLen-4)
			{
				pOutputData[j++]=((pInputUnicode[i] >> 24) & 0x03) | 0xf8;
				pOutputData[j++]=((pInputUnicode[i] >> 18) & 0x3f) | 0x80;
				pOutputData[j++]=((pInputUnicode[i] >> 12) & 0x3f) | 0x80;
				pOutputData[j++]=((pInputUnicode[i] >> 6) & 0x3f) | 0x80;
				pOutputData[j++]=(pInputUnicode[i] & 0x3f) | 0x80;
			}else
			{
				break;
			}
		}else if(pInputUnicode[i]>=0x04000000 && pInputUnicode[i]<=0x7fffffff)
		{
			if(j<tInputDataLen-4)
			{
				pOutputData[j++]=((pInputUnicode[i] >> 30) & 0x01) | 0xfc;
				pOutputData[j++]=((pInputUnicode[i] >> 24) & 0x3f) | 0x80;
				pOutputData[j++]=((pInputUnicode[i] >> 18) & 0x3f) | 0x80;
				pOutputData[j++]=((pInputUnicode[i] >> 12) & 0x3f) | 0x80;
				pOutputData[j++]=((pInputUnicode[i] >> 6) & 0x3f) | 0x80;
				pOutputData[j++]=(pInputUnicode[i] & 0x3f) | 0x80;
			}else
			{
				break;
			}			
		}
	}

	if(pOutputLen!=NULL)
		*pOutputLen=j;

	return 0;
}


static int CheckFontSize(int tInputFontSize)
{
	if(tXFontCount<=0 || pXShowFont==NULL)
		return -1;

	int i=0;
	for(i=0; i<tXFontCount; i++)
	{
		if(pXShowFont[i].mFontSize==tInputFontSize)
			return i;
	}

	return -2;
}

int XShowFontInit(const char *pInputFontName)
{
	if(pInputFontName==NULL || access(pInputFontName, F_OK))
		return -1;

	int tTempSectionNum=XConfigGetSectionNum();
	if(tTempSectionNum>0)
	{
		if(pXShowFont!=NULL)
		{
			int j=0;
			for(j=0; j<tXFontCount; j++)
				Szbh_DestroyFont(pXShowFont[j].mFontHandle);
			free(pXShowFont);
			pXShowFont=NULL;
			tXFontCount=0;
		}
	
		int i=0;
		for(i=0; i<tTempSectionNum; i++)
		{
			XConfigSection *pTempSection=XConfigGetSectionWithIdx(i);
			if(pTempSection->mType==1)
			{
				if(CheckFontSize(pTempSection->mFontSize)<0)
				{
					unsigned int tTempFontHandle=0;
					tXFontCount++;
					if(pXShowFont==NULL)
						pXShowFont=(XShowFont *)malloc(sizeof(XShowFont));
					else
						pXShowFont=(XShowFont *)realloc(pXShowFont, sizeof(XShowFont)*tXFontCount);
					if(pXShowFont==NULL)
					{
						printf("Damon ==> Error show font malloc failed !\n");
						return -2;
					}
					pXShowFont[tXFontCount-1].mFontSize=pTempSection->mFontSize;
					if(Szbh_CreateFont(pInputFontName, pTempSection->mFontSize, &tTempFontHandle)==0)
						pXShowFont[tXFontCount-1].mFontHandle=tTempFontHandle;
					else
						pXShowFont[tXFontCount-1].mFontHandle=0;
				}
			}
		}
	}
	printf("Damon ==> show font init count : %d \n", tXFontCount);
	
	return 0;
}

int XShowTime(char *pInputTime)
{
	if(pInputTime==NULL)
		return -1;
	XConfigSection *pTempSection=XConfigGetSection("time");
	if(pTempSection!=NULL  && pTempSection->mEnable)
	{
		if(pTempSection->mType==1)
		{
			int tTempFontIdx=CheckFontSize(pTempSection->mFontSize);
			if(tTempFontIdx<0)
				return -2;
		
			XSurface *pTempTimeSurface=Szbh_LoadString(pXShowFont[tTempFontIdx].mFontHandle,
							pInputTime, pTempSection->mFontColor, pTempSection->mFontBackColor);
			if(pTempTimeSurface==NULL)
				return -3;

		#if 1
			{
				X2DRect tTempBackRect;
				tTempBackRect.x=pTempSection->mXPos;
				tTempBackRect.y=pTempSection->mYPos;
				tTempBackRect.width=pTempTimeSurface->mWidth;
				tTempBackRect.height=pTempTimeSurface->mHeight;

				if(pTempSection->mWidth>0)
				{
					if(tTempBackRect.width<pTempSection->mWidth)
						tTempBackRect.x += (pTempSection->mWidth-tTempBackRect.width)/2;
					else 
						tTempBackRect.width=pTempSection->mWidth;
				}
				if(pTempSection->mHeight>0)
				{
					if(tTempBackRect.height<pTempSection->mHeight)
						tTempBackRect.y += (pTempSection->mHeight-tTempBackRect.height)/2;
					else
						tTempBackRect.height=pTempSection->mHeight;
				}

				Szbh_LayerShowSurface2(dXLayerId_Bottom, pXBackSurface, (pXBackSurface!=NULL)?(&tTempBackRect):NULL,
					pTempTimeSurface, NULL, &tTempBackRect);
			}
		#else
			Szbh_LayerShowSurface(dXLayerId_Top, pTempTimeSurface, pTempSection->mXPos, pTempSection->mYPos);
		#endif

			Szbh_SurfaceDestroy(pTempTimeSurface);
			pTempTimeSurface=NULL;
		}
	}

	return 0;
}

int XShowTime2(int tInputHour, int tInputMin, int tInputSec)
{
	static int tTempShowFirst=0;

	if(tInputHour>24)
		tInputHour=0;
	if(tInputMin>60)
		tInputMin=0;
	if(tInputSec>60)
		tInputSec=0;

	XConfigSection *pTempSection=XConfigGetSection("time");
	if(pTempSection!=NULL  && pTempSection->mEnable)
	{
		if(pTempSection->mType==1)
		{
			int tTempFontIdx=CheckFontSize(pTempSection->mFontSize);
			if(tTempFontIdx<0)
				return -2;

			if(tTempShowFirst==0)
			{
				char pTempString[2]={0};
				int i=0;
				for(i=0; i<10; i++)
				{
					pTempString[0]='0'+i;
					pXTimeNumSurface[i]=Szbh_LoadString(pXShowFont[tTempFontIdx].mFontHandle,
									pTempString, pTempSection->mFontColor, pTempSection->mFontBackColor);
					if(pXTimeNumSurface[i]==NULL)
					{
						printf("Damon ==> load string failed !!!\n");
					}
				}

				pTempString[0]=':';
				pXTimeNumSurface[10]=Szbh_LoadString(pXShowFont[tTempFontIdx].mFontHandle,
								pTempString, pTempSection->mFontColor, pTempSection->mFontBackColor);
				
				tTempShowFirst=1;
			}


			{
				int tTempTotalWidth=0;
				int tTempMaxHeight=0;
				XSurface *pTempTimeSurface[6]={NULL, NULL, NULL, NULL, NULL, NULL};
				
				int tTempVal=0;
				X2DRect tTempBackRect;
				tTempBackRect.x=pTempSection->mXPos;
				tTempBackRect.y=pTempSection->mYPos;

				// ========== hour =========
				tTempVal=tInputHour/10;
			#if 0
				tTempBackRect.width=pXTimeNumSurface[tTempVal]->mWidth;
				tTempBackRect.height=pXTimeNumSurface[tTempVal]->mHeight;
				Szbh_LayerShowSurface2(dXLayerId_Bottom, pXBackSurface, (pXBackSurface!=NULL)?(&tTempBackRect):NULL,
					pXTimeNumSurface[tTempVal], NULL, &tTempBackRect);
			#else
				pTempTimeSurface[0]=pXTimeNumSurface[tTempVal];
				tTempTotalWidth += pXTimeNumSurface[tTempVal]->mWidth;
				if(tTempMaxHeight<pXTimeNumSurface[tTempVal]->mHeight)
					tTempMaxHeight=pXTimeNumSurface[tTempVal]->mHeight;
			#endif

				tTempBackRect.x += tTempBackRect.width;
				tTempVal=tInputHour%10;
			#if 0
				tTempBackRect.width=pXTimeNumSurface[tTempVal]->mWidth;
				tTempBackRect.height=pXTimeNumSurface[tTempVal]->mHeight;
				Szbh_LayerShowSurface2(dXLayerId_Bottom, pXBackSurface, (pXBackSurface!=NULL)?(&tTempBackRect):NULL,
					pXTimeNumSurface[tTempVal], NULL, &tTempBackRect);
			#else
				pTempTimeSurface[1]=pXTimeNumSurface[tTempVal];
				tTempTotalWidth += pXTimeNumSurface[tTempVal]->mWidth;
				if(tTempMaxHeight<pXTimeNumSurface[tTempVal]->mHeight)
					tTempMaxHeight=pXTimeNumSurface[tTempVal]->mHeight;				
			#endif


			#if 0
				tTempBackRect.x += tTempBackRect.width;
				tTempBackRect.width=pXTimeNumSurface[10]->mWidth;
				tTempBackRect.height=pXTimeNumSurface[10]->mHeight;
				Szbh_LayerShowSurface2(dXLayerId_Bottom, pXBackSurface, (pXBackSurface!=NULL)?(&tTempBackRect):NULL,
					pXTimeNumSurface[10], NULL, &tTempBackRect);
			#else
				tTempTotalWidth += pXTimeNumSurface[10]->mWidth;
				if(tTempMaxHeight<pXTimeNumSurface[10]->mHeight)
					tTempMaxHeight=pXTimeNumSurface[10]->mHeight;
			#endif


				// ========== min =========
				tTempBackRect.x += tTempBackRect.width;
				tTempVal=tInputMin/10;
			#if 0
				tTempBackRect.width=pXTimeNumSurface[tTempVal]->mWidth;
				tTempBackRect.height=pXTimeNumSurface[tTempVal]->mHeight;
				Szbh_LayerShowSurface2(dXLayerId_Bottom, pXBackSurface, (pXBackSurface!=NULL)?(&tTempBackRect):NULL,
					pXTimeNumSurface[tTempVal], NULL, &tTempBackRect);
			#else
				pTempTimeSurface[2]=pXTimeNumSurface[tTempVal];
				tTempTotalWidth += pXTimeNumSurface[tTempVal]->mWidth;
				if(tTempMaxHeight<pXTimeNumSurface[tTempVal]->mHeight)
					tTempMaxHeight=pXTimeNumSurface[tTempVal]->mHeight;
			#endif
				
				tTempBackRect.x += tTempBackRect.width;
				tTempVal=tInputMin%10;
			#if 0
				tTempBackRect.width=pXTimeNumSurface[tTempVal]->mWidth;
				tTempBackRect.height=pXTimeNumSurface[tTempVal]->mHeight;
				Szbh_LayerShowSurface2(dXLayerId_Bottom, pXBackSurface, (pXBackSurface!=NULL)?(&tTempBackRect):NULL,
					pXTimeNumSurface[tTempVal], NULL, &tTempBackRect);
			#else
				pTempTimeSurface[3]=pXTimeNumSurface[tTempVal];
				tTempTotalWidth += pXTimeNumSurface[tTempVal]->mWidth;
				if(tTempMaxHeight<pXTimeNumSurface[tTempVal]->mHeight)
					tTempMaxHeight=pXTimeNumSurface[tTempVal]->mHeight;
			#endif


			#if 0
				tTempBackRect.x += tTempBackRect.width;
				tTempBackRect.width=pXTimeNumSurface[10]->mWidth;
				tTempBackRect.height=pXTimeNumSurface[10]->mHeight;
				Szbh_LayerShowSurface2(dXLayerId_Bottom, pXBackSurface, (pXBackSurface!=NULL)?(&tTempBackRect):NULL,
					pXTimeNumSurface[10], NULL, &tTempBackRect);
			#else
				tTempTotalWidth += pXTimeNumSurface[10]->mWidth;
				if(tTempMaxHeight<pXTimeNumSurface[10]->mHeight)
					tTempMaxHeight=pXTimeNumSurface[tTempVal]->mHeight;
			#endif


				// ========== sec =========
				tTempBackRect.x += tTempBackRect.width;
				tTempVal=tInputSec/10;
			#if 0
				tTempBackRect.width=pXTimeNumSurface[tTempVal]->mWidth;
				tTempBackRect.height=pXTimeNumSurface[tTempVal]->mHeight;
				Szbh_LayerShowSurface2(dXLayerId_Bottom, pXBackSurface, (pXBackSurface!=NULL)?(&tTempBackRect):NULL,
					pXTimeNumSurface[tTempVal], NULL, &tTempBackRect);
			#else
				pTempTimeSurface[4]=pXTimeNumSurface[tTempVal];
				tTempTotalWidth += pXTimeNumSurface[tTempVal]->mWidth;
				if(tTempMaxHeight<pXTimeNumSurface[tTempVal]->mHeight)
					tTempMaxHeight=pXTimeNumSurface[tTempVal]->mHeight;
			#endif
				
				tTempBackRect.x += tTempBackRect.width;
				tTempVal=tInputSec%10;
			#if 0
				tTempBackRect.width=pXTimeNumSurface[tTempVal]->mWidth;
				tTempBackRect.height=pXTimeNumSurface[tTempVal]->mHeight;
				Szbh_LayerShowSurface2(dXLayerId_Bottom, pXBackSurface, (pXBackSurface!=NULL)?(&tTempBackRect):NULL,
					pXTimeNumSurface[tTempVal], NULL, &tTempBackRect);
			#else
				pTempTimeSurface[5]=pXTimeNumSurface[tTempVal];
				tTempTotalWidth += pXTimeNumSurface[tTempVal]->mWidth;
				if(tTempMaxHeight<pXTimeNumSurface[tTempVal]->mHeight)
					tTempMaxHeight=pXTimeNumSurface[tTempVal]->mHeight;
			#endif

				XSurface *pTempShowSurface=Szbh_SurfaceCreate(tTempTotalWidth, tTempMaxHeight);
				if(pTempShowSurface!=NULL)
				{
					int i=0;
					memset(&tTempBackRect, 0, sizeof(X2DRect));
					for(i=0; i<6; i++)
					{
						tTempBackRect.width=pTempTimeSurface[i]->mWidth;
						tTempBackRect.height=pTempTimeSurface[i]->mHeight;
						Szbh_SurfaceQuickCopy(pTempTimeSurface[i], NULL, pTempShowSurface, &tTempBackRect);
						tTempBackRect.x += tTempBackRect.width;

						if(i==1 || i==3)
						{
							tTempBackRect.width=pXTimeNumSurface[10]->mWidth;
							tTempBackRect.height=pXTimeNumSurface[10]->mHeight;
							Szbh_SurfaceQuickCopy(pXTimeNumSurface[10], NULL, pTempShowSurface, &tTempBackRect);
							tTempBackRect.x += tTempBackRect.width;
						}
					}

					tTempBackRect.x=pTempSection->mXPos;
					tTempBackRect.y=pTempSection->mYPos;
					tTempBackRect.width=pTempShowSurface->mWidth;
					tTempBackRect.height=pTempShowSurface->mHeight;
					if(pTempSection->mWidth>0)
					{
						if(tTempBackRect.width<pTempSection->mWidth)
							tTempBackRect.x += (pTempSection->mWidth-tTempBackRect.width)/2;
						else
							tTempBackRect.width=pTempSection->mWidth;
					}
					if(pTempSection->mHeight>0)
					{
						if(tTempBackRect.height<pTempSection->mHeight)
							tTempBackRect.y += (pTempSection->mHeight-tTempBackRect.height)/2;
						else
							tTempBackRect.height=pTempSection->mHeight;
					}

					Szbh_LayerShowSurface2(dXLayerId_Bottom, pXBackSurface, (pXBackSurface!=NULL)?(&tTempBackRect):NULL,
						pTempShowSurface, NULL, &tTempBackRect);

					Szbh_SurfaceDestroy(pTempShowSurface);
				}
			}

						
		}
	}


	return 0;
}

int XShowWeek(int tInputWeek)
{
	if(tInputWeek<0 || tInputWeek>=7)
		return -1;
	char pTempString[64]={0};
	XConfigSection *pTempSection=XConfigGetSection("week");
	if(pTempSection!=NULL && pTempSection->mEnable)
	{
		if(pTempSection->mType==1)
		{
			int tTempFontIdx=CheckFontSize(pTempSection->mFontSize);
			if(tTempFontIdx<0)
				return -2;

			if(tInputWeek==1)
				strncpy(pTempString, "星 期 一",sizeof(pTempString)-1);
			else if(tInputWeek==2)
				strncpy(pTempString, "星 期 二",sizeof(pTempString)-1);
			else if(tInputWeek==3)
				strncpy(pTempString, "星 期 三",sizeof(pTempString)-1);
			else if(tInputWeek==4)
				strncpy(pTempString, "星 期 四",sizeof(pTempString)-1);
			else if(tInputWeek==5)
				strncpy(pTempString, "星 期 五",sizeof(pTempString)-1);
			else if(tInputWeek==6)
				strncpy(pTempString, "星 期 六",sizeof(pTempString)-1);
			else if(tInputWeek==0)
				strncpy(pTempString, "星 期 日",sizeof(pTempString)-1);
			
		
			XSurface *pTempTimeSurface=Szbh_LoadString(pXShowFont[tTempFontIdx].mFontHandle,
							pTempString, pTempSection->mFontColor, pTempSection->mFontBackColor);
			if(pTempTimeSurface==NULL)
				return -3;			 	

		#if 1
			{
				X2DRect tTempBackRect;
				tTempBackRect.x=pTempSection->mXPos;
				tTempBackRect.y=pTempSection->mYPos;
				tTempBackRect.width=pTempTimeSurface->mWidth;
				tTempBackRect.height=pTempTimeSurface->mHeight;

				if(pTempSection->mWidth>0)
				{
					if(tTempBackRect.width<pTempSection->mWidth)
						tTempBackRect.x += (pTempSection->mWidth-tTempBackRect.width)/2;
					else 
						tTempBackRect.width=pTempSection->mWidth;
				}
				if(pTempSection->mHeight>0)
				{
					if(tTempBackRect.height<pTempSection->mHeight)
						tTempBackRect.y += (pTempSection->mHeight-tTempBackRect.height)/2;
					else
						tTempBackRect.height=pTempSection->mHeight;
				}

				Szbh_LayerShowSurface2(dXLayerId_Bottom, pXBackSurface, (pXBackSurface!=NULL)?(&tTempBackRect):NULL,
					pTempTimeSurface, NULL, &tTempBackRect);
			}
		#else
			Szbh_LayerShowSurface(dXLayerId_Top, pTempTimeSurface, pTempSection->mXPos, pTempSection->mYPos);
		#endif

			Szbh_SurfaceDestroy(pTempTimeSurface);
			pTempTimeSurface=NULL;
		}
	}

	return 0;
}

int XShowSectionString(char *pInputSectionName, char *pInputText)
{
	if(pInputSectionName==NULL || pInputText==NULL)
		return -1;

	XConfigSection *pTempSection=XConfigGetSection(pInputSectionName);
	if(pTempSection!=NULL && pTempSection->mEnable)
	{
		if(pTempSection->mType==1)
		{
			int tTempFontIdx=CheckFontSize(pTempSection->mFontSize);
			if(tTempFontIdx<0)
				return -2;
		
			XSurface *pTempTimeSurface=Szbh_LoadString(pXShowFont[tTempFontIdx].mFontHandle,
							pInputText, pTempSection->mFontColor, pTempSection->mFontBackColor);
			if(pTempTimeSurface==NULL)
				return -3;

		#if 1
			{
				X2DRect tTempBackRect;
				tTempBackRect.x=pTempSection->mXPos;
				tTempBackRect.y=pTempSection->mYPos;
				tTempBackRect.width=pTempTimeSurface->mWidth;
				tTempBackRect.height=pTempTimeSurface->mHeight;

				if(pTempSection->mWidth>0)
				{
					if(tTempBackRect.width<pTempSection->mWidth)
						tTempBackRect.x += (pTempSection->mWidth-tTempBackRect.width)/2;
					else 
						tTempBackRect.width=pTempSection->mWidth;
				}
				if(pTempSection->mHeight>0)
				{
					if(tTempBackRect.height<pTempSection->mHeight)
						tTempBackRect.y += (pTempSection->mHeight-tTempBackRect.height)/2;
					else
						tTempBackRect.height=pTempSection->mHeight;
				}


				Szbh_LayerShowSurface2(dXLayerId_Bottom, pXBackSurface, (pXBackSurface!=NULL)?(&tTempBackRect):NULL,
					pTempTimeSurface, NULL, &tTempBackRect);
			}
		#else
			Szbh_LayerShowSurface(dXLayerId_Top, pTempTimeSurface, pTempSection->mXPos, pTempSection->mYPos);
		#endif

			Szbh_SurfaceDestroy(pTempTimeSurface);
			pTempTimeSurface=NULL;
		}
	}

	return 0;

}


/******************************
*tInputFlag : 1. cur station
			  2. next station
			  3. cur station name
			  4. next station name
			  5. end station
			  6. end station name
*******************************/
static int XShowStationText(int tInputFlag, int tInputCurStation, int tInputNextStation, int tInputEndStation, XConfigSection *pInputConfig)
{
	if(tInputFlag<=0 || tInputFlag>6)
		return -1;
	if(pInputConfig==NULL || pInputConfig->mEnable!=1 || pInputConfig->mType!=1)
		return -2;

	int tTempFontIdx=CheckFontSize(pInputConfig->mFontSize);
	if(tTempFontIdx<0)
		return -3;

	char *pTempString=NULL;
	int tTempStringIdx=0;
//printf("Damon ==> show station tInputFlag=%d \n", tInputFlag);
	switch(tInputFlag)
	{
		case 1:
			tTempStringIdx=100;
			break;
		case 2:
			tTempStringIdx=101;
			break;
		case 3:
			tTempStringIdx=tInputCurStation;
			break;
		case 4:
			tTempStringIdx=tInputNextStation;
			break;
		case 5:
			tTempStringIdx=102;
			break;
		case 6:
			tTempStringIdx=tInputEndStation;
			break;
		default:
			return -4;
	}

	pTempString=XStationTextGetString(tTempStringIdx);
	if(pTempString==NULL)
		return -5;
printf("Damon ==> station name : %s \n", pTempString);
	XSurface *pTempTimeSurface=NULL;/*Szbh_LoadString(pXShowFont[tTempFontIdx].mFontHandle,
						pTempString, pInputConfig->mFontColor, pInputConfig->mFontBackColor);*/

	if(tTempStringIdx>=100 && tTempStringIdx<=102)
	{
		int tTempSurIdx=(dXStationNameMax-3)+(tTempStringIdx-101);
		if(pXStationSurface[tTempSurIdx]==NULL)
		{
			pXStationSurface[tTempSurIdx]=	Szbh_LoadString(pXShowFont[tTempFontIdx].mFontHandle,
									pTempString, pInputConfig->mFontColor, pInputConfig->mFontBackColor);
		}
		pTempTimeSurface=pXStationSurface[tTempSurIdx];
	}else if(tTempStringIdx>0 && (tTempStringIdx<dXStationNameMax-3))
	{
		int tTempSurIdx=tTempStringIdx-1;
		if(pXStationSurface[tTempSurIdx]==NULL)
		{
			pXStationSurface[tTempSurIdx]=	Szbh_LoadString(pXShowFont[tTempFontIdx].mFontHandle,
									pTempString, pInputConfig->mFontColor, pInputConfig->mFontBackColor);
		}
		pTempTimeSurface=pXStationSurface[tTempSurIdx];
	}
	
	if(pTempTimeSurface==NULL)
		return -6;



	X2DRect tTempSrcRect, tTempDestRect;

	XSurface *pTempBakSurface=Szbh_SurfaceCreate(pInputConfig->mWidth, pInputConfig->mHeight);
	if(pTempBakSurface==NULL)
		return -7;

	{
		tTempSrcRect.x=pInputConfig->mXPos;
		tTempSrcRect.y=pInputConfig->mYPos;
		tTempSrcRect.width=pInputConfig->mWidth;
		tTempSrcRect.height=pInputConfig->mHeight;

		tTempDestRect.x=0;
		tTempDestRect.y=0;
		tTempDestRect.width=pInputConfig->mWidth;
		tTempDestRect.height=pInputConfig->mHeight;

		Szbh_SurfaceQuickCopy(pXBackSurface, &tTempSrcRect, pTempBakSurface, &tTempDestRect);
	}

	if(pTempTimeSurface)
	{
		tTempSrcRect.x=0;
		tTempSrcRect.y=0;
		tTempSrcRect.width=pTempTimeSurface->mWidth;
		tTempSrcRect.height=pTempTimeSurface->mHeight;

		tTempDestRect.x=0;
		tTempDestRect.y=0;
		tTempDestRect.width=pInputConfig->mWidth;
		tTempDestRect.height=pInputConfig->mHeight;

		if(tTempSrcRect.width<pInputConfig->mWidth)
		{
			tTempDestRect.x += (pInputConfig->mWidth-tTempSrcRect.width)/2;
			tTempDestRect.width=tTempSrcRect.width;
		}else
		{
			tTempSrcRect.width=pInputConfig->mWidth;
		}

		if(tTempSrcRect.height<pInputConfig->mHeight)
		{
			tTempDestRect.y += (pInputConfig->mHeight-tTempSrcRect.height)/2;
			tTempDestRect.height=tTempSrcRect.height;
		}

//	printf("Damon ==> show station : [%d %d %d %d] [%d %d %d %d] \n", tTempSrcRect.x, tTempSrcRect.y, tTempSrcRect.width, tTempSrcRect.height,
//		tTempDestRect.x, tTempDestRect.y, tTempDestRect.width, tTempDestRect.height);
		Szbh_SurfaceBlitWithAlpha(pTempTimeSurface, &tTempSrcRect, pTempBakSurface, &tTempDestRect, EBlitAlpha_Both);
		Szbh_LayerShowSurface(dXLayerId_Bottom, pTempBakSurface, pInputConfig->mXPos, pInputConfig->mYPos);
	}

	Szbh_SurfaceDestroy(pTempBakSurface);
	pTempBakSurface=NULL;
//	Szbh_SurfaceDestroy(pTempTimeSurface);
//	pTempTimeSurface=NULL;

	return 0;
}


int XShowStation(int tInputCurTemplate, u8 tInputTrigger, u8 tInputCurStation, u8 tInputNextStation, u8 tInputEndStation)
{
	if((tInputTrigger & 0x02) || (tInputTrigger & 0x01))
	{
		char pTempString[128]={0};
		int tTempPicId=0;
		if(tInputTrigger & 0x02)
			tTempPicId=1001;
		else
			tTempPicId=1002;

		// ==== show current or next station =====
		XConfigSection *pTempSection=XConfigGetSection("cur_next_station");
		XSurface *pTempPicSurface=NULL;
		if(pTempSection!=NULL && pTempSection->mEnable)
		{
			if(pTempSection->mType==1)
			{
				XShowStationText(tTempPicId-1000, tInputCurStation, tInputNextStation, tInputEndStation, pTempSection);
			}else
			{
				memset(pTempString, 0, sizeof(pTempString));
				if(pTempSection->mPicType==1)
				{
					snprintf(pTempString, sizeof(pTempString)-1, "%s/Template%d/images/stationname/%d.bmp", dXResourcePath, tInputCurTemplate, tTempPicId);
					pTempPicSurface=Szbh_LoadPic(pTempString, dXPicType_Bmp);
				}else if(pTempSection->mPicType==2)
				{
					snprintf(pTempString, sizeof(pTempString)-1, "%s/Template%d/images/stationname/%d.png", dXResourcePath, tInputCurTemplate, tTempPicId);
					pTempPicSurface=Szbh_LoadPic(pTempString, dXPicType_Png);
				}else if(pTempSection->mPicType==3)
				{
					snprintf(pTempString, sizeof(pTempString)-1, "%s/Template%d/images/stationname/%d.jpg", dXResourcePath, tInputCurTemplate, tTempPicId);
					pTempPicSurface=Szbh_LoadPic(pTempString, dXPicType_Jpeg);
				}

				if(pTempPicSurface!=NULL)
				{
				#if 1
					{
						X2DRect tTempBackRect;
						tTempBackRect.x=pTempSection->mXPos;
						tTempBackRect.y=pTempSection->mYPos;
						tTempBackRect.width=pTempPicSurface->mWidth;
						tTempBackRect.height=pTempPicSurface->mHeight;
				
						Szbh_LayerShowSurface2(dXLayerId_Bottom, pXBackSurface, (pXBackSurface!=NULL)?(&tTempBackRect):NULL,
							pTempPicSurface, NULL, &tTempBackRect);
					}
				#else
					Szbh_LayerShowSurface(dXLayerId_Top, pTempPicSurface, pTempSection->mXPos, pTempSection->mYPos);
				#endif
					Szbh_SurfaceDestroy(pTempPicSurface);
					pTempPicSurface=NULL;
				}
			}
		}


		// show station name 
		if(tInputTrigger & 0x02)
			tTempPicId=tInputCurStation;
		else
			tTempPicId=tInputNextStation;
		pTempPicSurface=NULL;
		pTempSection=XConfigGetSection("cur_next_station_name");
		if(pTempSection && pTempSection->mEnable)
		{			
			if(pTempSection->mType==1)
			{
				int tTempFlag=0;
				if(tInputTrigger & 0x02)
					tTempFlag=3;
				else
					tTempFlag=4;
				XShowStationText(tTempFlag, tInputCurStation, tInputNextStation, tInputEndStation, pTempSection);
			}else
			{
				memset(pTempString, 0, sizeof(pTempString));
				if(pTempSection->mPicType==1)
				{
					snprintf(pTempString, sizeof(pTempString)-1, "%s/Template%d/images/stationname/%03d.bmp", dXResourcePath, tInputCurTemplate, tTempPicId);
					pTempPicSurface=Szbh_LoadPic(pTempString, dXPicType_Bmp);
				}else if(pTempSection->mPicType==2)
				{
					snprintf(pTempString, sizeof(pTempString)-1, "%s/Template%d/images/stationname/%03d.png", dXResourcePath, tInputCurTemplate, tTempPicId);
					pTempPicSurface=Szbh_LoadPic(pTempString, dXPicType_Png);
				}else if(pTempSection->mPicType==3)
				{
					snprintf(pTempString, sizeof(pTempString)-1, "%s/Template%d/images/stationname/%03d.jpg", dXResourcePath, tInputCurTemplate, tTempPicId);
					pTempPicSurface=Szbh_LoadPic(pTempString, dXPicType_Jpeg);
				}

				if(pTempPicSurface!=NULL)
				{
				#if 1
					{
						X2DRect tTempBackRect;
						tTempBackRect.x=pTempSection->mXPos;
						tTempBackRect.y=pTempSection->mYPos;
						tTempBackRect.width=pTempPicSurface->mWidth;
						tTempBackRect.height=pTempPicSurface->mHeight;

						Szbh_LayerShowSurface2(dXLayerId_Bottom, pXBackSurface, (pXBackSurface!=NULL)?(&tTempBackRect):NULL,
							pTempPicSurface, NULL, &tTempBackRect);
					}
				#else
					Szbh_LayerShowSurface(dXLayerId_Top, pTempPicSurface, pTempSection->mXPos, pTempSection->mYPos);
				#endif
					Szbh_SurfaceDestroy(pTempPicSurface);
					pTempPicSurface=NULL;
				}
			}
		}
		

		// ==== show end station ====
		tTempPicId=1003;
		pTempPicSurface=NULL;
		pTempSection=XConfigGetSection("end_station");
//	printf("Damon ==> show end station : %p \n", pTempSection);
		if(pTempSection && pTempSection->mEnable)
		{
	//	printf("Damon ==> end station type : %d \n", pTempSection->mType);
			if(pTempSection->mType==1)
				XShowStationText(5, tInputCurStation, tInputNextStation, tInputEndStation, pTempSection);
			else
			{
				memset(pTempString, 0, sizeof(pTempString));
				if(pTempSection->mPicType==1)
				{
					snprintf(pTempString, sizeof(pTempString)-1, "%s/Template%d/images/stationname/%d.bmp", dXResourcePath, tInputCurTemplate, tTempPicId);
					pTempPicSurface=Szbh_LoadPic(pTempString, dXPicType_Bmp);
				}else if(pTempSection->mPicType==2)
				{
					snprintf(pTempString, sizeof(pTempString)-1, "%s/Template%d/images/stationname/%d.png", dXResourcePath, tInputCurTemplate, tTempPicId);
					pTempPicSurface=Szbh_LoadPic(pTempString, dXPicType_Png);
				}else if(pTempSection->mPicType==3)
				{
					snprintf(pTempString, sizeof(pTempString)-1, "%s/Template%d/images/stationname/%d.jpg", dXResourcePath, tInputCurTemplate, tTempPicId);
					pTempPicSurface=Szbh_LoadPic(pTempString, dXPicType_Jpeg);
				}

				if(pTempPicSurface!=NULL)
				{
				#if 1
					{
						X2DRect tTempBackRect;
						tTempBackRect.x=pTempSection->mXPos;
						tTempBackRect.y=pTempSection->mYPos;
						tTempBackRect.width=pTempPicSurface->mWidth;
						tTempBackRect.height=pTempPicSurface->mHeight;
				
						Szbh_LayerShowSurface2(dXLayerId_Bottom, pXBackSurface, (pXBackSurface!=NULL)?(&tTempBackRect):NULL,
							pTempPicSurface, NULL, &tTempBackRect);
					}
				#else
					Szbh_LayerShowSurface(dXLayerId_Top, pTempPicSurface, pTempSection->mXPos, pTempSection->mYPos);
				#endif
					Szbh_SurfaceDestroy(pTempPicSurface);
					pTempPicSurface=NULL;
				}
			}
		}


		// ==== show end station name ===		
		tTempPicId=tInputEndStation;
		pTempPicSurface=NULL;
		pTempSection=XConfigGetSection("end_station_name");
		if(pTempSection && pTempSection->mEnable)
		{
			if(pTempSection->mType==1)
				XShowStationText(6, tInputCurStation, tInputNextStation, tInputEndStation, pTempSection);
			else
			{
				memset(pTempString, 0, sizeof(pTempString));
				if(pTempSection->mPicType==1)
				{
					snprintf(pTempString, sizeof(pTempString)-1, "%s/Template%d/images/stationname/%03d.bmp", dXResourcePath, tInputCurTemplate, tTempPicId);
					pTempPicSurface=Szbh_LoadPic(pTempString, dXPicType_Bmp);
				}else if(pTempSection->mPicType==2)
				{
					snprintf(pTempString, sizeof(pTempString)-1, "%s/Template%d/images/stationname/%03d.png", dXResourcePath, tInputCurTemplate, tTempPicId);
					pTempPicSurface=Szbh_LoadPic(pTempString, dXPicType_Png);
				}else if(pTempSection->mPicType==3)
				{
					snprintf(pTempString, sizeof(pTempString)-1, "%s/Template%d/images/stationname/%03d.jpg", dXResourcePath, tInputCurTemplate, tTempPicId);
					pTempPicSurface=Szbh_LoadPic(pTempString, dXPicType_Jpeg);
				}

				if(pTempPicSurface!=NULL)
				{
				#if 1
					{
						X2DRect tTempBackRect;
						tTempBackRect.x=pTempSection->mXPos;
						tTempBackRect.y=pTempSection->mYPos;
						tTempBackRect.width=pTempPicSurface->mWidth;
						tTempBackRect.height=pTempPicSurface->mHeight;
				
						Szbh_LayerShowSurface2(dXLayerId_Bottom, pXBackSurface, (pXBackSurface!=NULL)?(&tTempBackRect):NULL,
							pTempPicSurface, NULL, &tTempBackRect);
					}
				#else
					Szbh_LayerShowSurface(dXLayerId_Top, pTempPicSurface, pTempSection->mXPos, pTempSection->mYPos);
				#endif
					Szbh_SurfaceDestroy(pTempPicSurface);
					pTempPicSurface=NULL;
				}
			}
		}
	}else
	{
		return 1;
	}

	return 0;
}

int XShowEmergencyCode(int tInputEmergCode)
{
	char pTempString[64]={0};

	snprintf(pTempString, sizeof(pTempString)-1, "%s/Emerg/%02d.png", dXResourcePath, tInputEmergCode);
	if(access(pTempString, F_OK))
		return -1;

	XSurface *pTempSurface=Szbh_LoadPic(pTempString, dXPicType_Png);
	if(pTempSurface!=NULL)
	{
	//	Szbh_LayerShowSurface(dXLayerId_Top, pTempSurface, 0, 0);
	#ifdef dXCustomer_IptvDrmd
		X2DRect tTempDstRect={0, 0, dXScreenWidth, 540};
	#else
		X2DRect tTempDstRect={0, 0, dXScreenWidth, dXScreenHeight};
	#endif
		Szbh_LayerShowSurfaceWithRect(dXLayerId_Top, pTempSurface, NULL, &tTempDstRect);

		Szbh_SurfaceDestroy(pTempSurface);
		pTempSurface=NULL;
	}

	return 0;
}

int XShowEmergencyMsg(unsigned short *pInputUnicode, int tInputDataLen)
{
	char pTempString[64]={0};

	snprintf(pTempString, sizeof(pTempString)-1, "%s/Emerg/no.png", dXResourcePath);
	if(access(pTempString, F_OK))
		return -1;

	XConfigSection *pTempSection=XConfigGetSection("scroll_text");
	if(pTempSection==NULL)
		return -2;
	int tTempFontIdx=CheckFontSize(pTempSection->mFontSize);
	if(tTempFontIdx<0)
		return -3;


	XSurface *pTempSurface=Szbh_LoadPic(pTempString, dXPicType_Png);
	if(pTempSurface!=NULL)
	{
		// resize surface
	#ifdef dXCustomer_IptvDrmd
		if(pTempSurface->mWidth!=dXScreenWidth || pTempSurface->mWidth!=540)
		{
			XSurface *pTempCopySur=Szbh_SurfaceCreate(dXScreenWidth, 540);
			if(pTempCopySur!=NULL)
			{
				X2DRect tTempRect={0, 0, dXScreenWidth, 540};
				Szbh_SurfaceQuickResize(pTempSurface, NULL, pTempCopySur, &tTempRect);
		
				Szbh_SurfaceDestroy(pTempSurface);
				pTempSurface=pTempCopySur;
			}
		}
	#else
		if(pTempSurface->mWidth!=dXScreenWidth || pTempSurface->mWidth!=dXScreenHeight)
		{
			XSurface *pTempCopySur=Szbh_SurfaceCreate(dXScreenWidth, dXScreenHeight);
			if(pTempCopySur!=NULL)
			{
				X2DRect tTempRect={0, 0, dXScreenWidth, dXScreenHeight};
				Szbh_SurfaceQuickResize(pTempSurface, NULL, pTempCopySur, &tTempRect);

				Szbh_SurfaceDestroy(pTempSurface);
				pTempSurface=pTempCopySur;
			}
		}
	#endif
		
	
		int i=0;
		int tTempBreak=0;
		int tTempStartX=140, tTempStartY=100;
		unsigned char pTempUtf8[8]={0};
		int tTempDataLen=0;
		for(i=0; i<tInputDataLen; i++)
		{
			XUnicodeToUtf8(pInputUnicode+i, 1, pTempUtf8, sizeof(pTempUtf8), &tTempDataLen);
			pTempUtf8[tTempDataLen]=0;
			if(pTempUtf8[0]==0)
				continue;
			XSurface *pTempFontSur=Szbh_LoadString(pXShowFont[tTempFontIdx].mFontHandle, (char *)pTempUtf8, 0xff000000, 0);
			if(pTempFontSur!=NULL)
			{
				if(tTempStartY+pTempFontSur->mHeight>pTempSurface->mHeight-100)
				{
					tTempBreak=1;
				}else
				{
					X2DRect tTempDstRect;
					if(tTempStartX+pTempFontSur->mWidth>pTempSurface->mWidth-100)
					{
						tTempStartX=100;
						tTempStartY += pTempFontSur->mHeight+20;
						if(tTempStartY+pTempFontSur->mHeight>pTempSurface->mHeight-100)
						{
							tTempBreak=1;
						}
					}

					if(tTempBreak==0)
					{
						tTempDstRect.x=tTempStartX;
						tTempDstRect.y=tTempStartY;
						tTempDstRect.width=pTempFontSur->mWidth;
						tTempDstRect.height=pTempFontSur->mHeight;
						Szbh_SurfaceBlitWithAlpha(pTempFontSur, NULL, pTempSurface, &tTempDstRect, EBlitAlpha_Both);
					}

					tTempStartX += pTempFontSur->mWidth;
				}

				Szbh_SurfaceDestroy(pTempFontSur);
				pTempFontSur=NULL;
			}
			
			if(tTempBreak)
				break;
		}
	
		Szbh_LayerShowSurface(dXLayerId_Top, pTempSurface, 0, 0);

		Szbh_SurfaceDestroy(pTempSurface);
		pTempSurface=NULL;
	}

	return 0;
}


int XShowScrollText(char *pInputString)
{	
	int tTempRet=-1;
	XConfigSection *pTempSection=XConfigGetSection("scroll_text");
	if(pTempSection!=NULL)
	{
		if(pTempSection->mType==1 && pTempSection->mEnable==1)
		{
			int tTempFontIdx=CheckFontSize(pTempSection->mFontSize);
			if(tTempFontIdx<0)
				return -2;

			X2DRect tTempRect;
			tTempRect.x=pTempSection->mXPos;
			tTempRect.y=pTempSection->mYPos;
			tTempRect.width=pTempSection->mWidth;
			tTempRect.height=pTempSection->mHeight;
printf("Damon ==> scroll text : %d %d %d %d 0x%x \n", tTempRect.x, tTempRect.y, tTempRect.width, tTempRect.height, pTempSection->mFontColor);
			tTempRet=XScrollTextStart(pXShowFont[tTempFontIdx].mFontHandle, pInputString, pTempSection->mFontColor, pTempSection->mFontBackColor, &tTempRect);
		}
	}

	return tTempRet;
}

int XShowMainMenu(int tInputTemplateId)
{
	int i=0;
	int tTempCount=XConfigGetSectionNum();
	if(tTempCount<=0)
		return -1;

	if(pXBackSurface!=NULL)
	{
		Szbh_SurfaceDestroy(pXBackSurface);
		pXBackSurface=NULL;
	}

	X2DRect tTempRect={0, 0, dXFramebufWidth, dXFramebufHeight};
	Szbh_LayerClear(dXLayerId_Bottom, tTempRect, 0);
	Szbh_LayerClear(dXLayerId_Top, tTempRect, 0);
	XConfigSection *pTempSection=NULL;
	for(i=0; i<tTempCount; i++)
	{
		pTempSection=XConfigGetSectionWithIdx(i);
		if(pTempSection!=NULL)
		{
			if(pTempSection->mEnable==1 && pTempSection->mMove==0)	// show static menu
			{
				if(pTempSection->mType==2)
				{
					int tTempPicType=-1;
					char pTempString[64]={0};
					sprintf(pTempString, "%s/Template%d/images/%s", dXResourcePath, tInputTemplateId, pTempSection->pPicName);
					switch(pTempSection->mPicType)
					{
						case 1:
							tTempPicType=dXPicType_Bmp;
							break;
						case 2:
							tTempPicType=dXPicType_Png;
							break;
						case 3:
							tTempPicType=dXPicType_Jpeg;
							break;
						default:
							break;
					}

					if(tTempPicType==-1)
						continue;
					
					XSurface *pTempSurface=Szbh_LoadPic(pTempString, tTempPicType);
					if(pTempSurface!=NULL)
					{
						Szbh_LayerShowSurface(dXLayerId_Bottom, pTempSurface, pTempSection->mXPos, pTempSection->mYPos);

						if(strcmp(pTempSection->pSectionName, "background")==0)
							pXBackSurface=pTempSurface;
						else
						{
							Szbh_SurfaceDestroy(pTempSurface);
							pTempSurface=NULL;
						}
					}
				}else if(pTempSection->mType==3)
				{
					X2DRect tTempRect;
					tTempRect.x=pTempSection->mXPos;
					tTempRect.y=pTempSection->mYPos;
					tTempRect.width=pTempSection->mWidth;
					tTempRect.height=pTempSection->mHeight;
					if(tTempRect.width>0 && tTempRect.height>0)
					{
						Szbh_LayerClear(dXLayerId_Bottom, tTempRect, 0);
					}
				}
			}
		}
	}

	return 0;
}


