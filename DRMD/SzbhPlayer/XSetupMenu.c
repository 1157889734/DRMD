

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <net/if.h>
#include <arpa/inet.h>
#include <sys/ioctl.h>

#include "SzbhApi.h"
#include "SzbhDefine.h"

#include "XConfigFile.h"
#include "XSectionFile.h"
#include "XVehiclePlayerDefine.h"
#include "XPlayer.h"
#include "XSetupMenu.h"

#define dXSetupMenuShowX	0
#define dXSetupMenuShowY	0
#define dXSetupMenuWidth		1000
#define dXSetupMenuHeight	480

#define dXTitleHeight	100
#define dXLineHeight	47

#define dXLeftMenuWidth	300
#define dXSubMenuWidth	(dXSetupMenuWidth-dXLeftMenuWidth)

#define dXLeftMenuSelectColor	0xff1e90ff

typedef struct tagXMenuParam{
	int mItemCount;
	int mItemIndex;
	unsigned int mItemSelectColor;
	unsigned int mItemDefaultColor;
	int mItemWidth;
	int mItemHeight;
	XSurface *pBackSurface;
	int mShowXPos;
	int mShowYPos;
}XMenuParam;


extern int tXBacklightHz;

static XMenuParam *pXLeftMenu=NULL, *pXDispSubMenu=NULL, *pXIpSubMenu=NULL, *pXVolSubMenu=NULL;

static XSurface *pXSetupBackMenu=NULL, *pXSubMenuSurface=NULL;

static char pXMenuOption[4][16]={
	{0xE5, 0x8F, 0x82, 0xE6, 0x95, 0xB0, 0xE8, 0xAE, 0xBE, 0xE7, 0xBD, 0xAE, 0x00},	// param set
	{0xE7, 0xBD, 0x91, 0xE7, 0xBB, 0x9C, 0xE8, 0xAE, 0xBE, 0xE7, 0xBD, 0xAE, 0x00},  // net set
	{0xE9, 0x9F, 0xB3, 0xE9, 0x87, 0x8F, 0xE8, 0xB0, 0x83, 0xE8, 0x8A, 0x82, 0x00},  // volume set
	{0xE4, 0xB8, 0x89, 0xE8, 0x89, 0xB2, 0xE6, 0xA3, 0x80, 0xE6, 0xB5, 0x8B, 0x00}	// three color check
};

static char pXDispSubMenuTitle[2][16]={
	{0xE4, 0xBA, 0xAE, 0xE5, 0xBA, 0xA6, 0x00},	// light
	{0xE5, 0xAF, 0xB9, 0xE6, 0xAF, 0x94, 0xE5, 0xBA, 0xA6, 0x00} 	// contrast
};

static char pXIpSubMenuTitle[2][16]={
	{0x49, 0x50, 0xE5, 0x9C, 0xB0, 0xE5, 0x9D, 0x80, 0x00},	// ip addr
	{0xE5, 0xAD, 0x90, 0xE7, 0xBD, 0x91, 0xE6, 0x8E, 0xA9, 0xE7, 0xA0, 0x81, 0x00}	//sub mask
};

static char pXVolSubMenuTitle[1][16]={
	{0xE9, 0x9F, 0xB3, 0xE9, 0x87, 0x8F, 0x00}	// volume set
};

static int tXSubMenuSelect=0;

static int tXMenuEnterFlag=0;

static u32 tXFontHandle=0;

static XSurface *pXLeftFontSurface[4]={NULL, NULL, NULL, NULL};

extern int tXCheckColorFlag;
extern void XSetColorMode(int tInputOpenOrClose, int tInputTick, int tInputColorIdx);


static int tXSetValue=0;

static int tXIpSelectIndex=0;
static unsigned char pXIpAddr[4]={0};
static unsigned char pXNetMaskAddr[4]={0};


static pthread_t tXSetupMenuExitHandle=0;
static unsigned int tXKeyDownPrevTick=0;

extern int XGetTick(unsigned int *pOutputMs);
extern void XSetScreenSaveOrCloseTimeout(int tInputScreenSaveOrClose, int tInputTimeout);


//#define dXTest

static void *XSetupMenuTimeoutExitProc(void *pInputArg)
{
	unsigned int tTempCurTick=0;

	while(tXMenuEnterFlag)
	{
		if(tXKeyDownPrevTick!=0)
		{
			if(XGetTick(&tTempCurTick)==0)
			{
				if(tTempCurTick-tXKeyDownPrevTick>1000*10)
				{
					// time out exit setup menu
					printf("\nDamon ==> time out close setup menu !\n\n");
					X2DRect tTempRect={dXSetupMenuShowX, dXSetupMenuShowY, dXSetupMenuWidth, dXSetupMenuHeight};
					Szbh_LayerClear(dXLayerId_Top, tTempRect, 0);
					Szbh_LayerRender(dXLayerId_Top);

					tXMenuEnterFlag=0;
					break;
				}
			}
		}
	
		usleep(1000*500);
	}

	tXSetupMenuExitHandle=0;

	return NULL;
}

/*static int XGetIntToCharLen(int tInputVal)
{
	char pTempString[32]={0};

	snprintf(pTempString, sizeof(pTempString)-1, "%d", tInputVal);

	return strlen(pTempString);
}*/

static int XSubMenuEnter(int tInputLeftMenuIndex, bool tInputShowOrNot)
{
	if(pXLeftMenu==NULL || tInputLeftMenuIndex>=pXLeftMenu->mItemCount)
		return -1;

printf("Damon ==> left menu idx = %d \n", tInputLeftMenuIndex);

	if(tInputLeftMenuIndex==0)	// display param
	{
		if(pXDispSubMenu==NULL)
		{
			pXDispSubMenu=(XMenuParam *)malloc(sizeof(XMenuParam));
			if(pXDispSubMenu==NULL)
			{
				printf("Damon ==> line:[%d] not enought memory !\n", __LINE__);
				return -2;
			}

			pXDispSubMenu->mItemCount=sizeof(pXDispSubMenuTitle)/sizeof(pXDispSubMenuTitle[0]);
			pXDispSubMenu->mItemIndex=-1;
			pXDispSubMenu->mItemDefaultColor=0xffaaaaaa;
			pXDispSubMenu->mItemSelectColor=dXLeftMenuSelectColor;
			pXDispSubMenu->mItemWidth=dXLeftMenuWidth;
			pXDispSubMenu->mItemHeight=dXLineHeight;
			pXDispSubMenu->mShowXPos=dXLeftMenuWidth;
			pXDispSubMenu->mShowYPos=dXTitleHeight;
			pXDispSubMenu->pBackSurface=pXSubMenuSurface;
		}

		pXDispSubMenu->mItemIndex=-1;
		Szbh_SurfaceFill(pXDispSubMenu->pBackSurface, NULL, pXDispSubMenu->mItemDefaultColor);
		
		int i=0;
		X2DRect tTempRect;
		XSurface *pTempFontSurface=NULL;
		tTempRect.x=0;
		tTempRect.width=pXDispSubMenu->mItemWidth;
		tTempRect.height=dXLineHeight;
		for(i=0; i<pXDispSubMenu->mItemCount; i++)
		{
			X2DRect tTempShowRect;
			tTempRect.y=dXLineHeight*i;
			memcpy(&tTempShowRect, &tTempRect, sizeof(X2DRect));

			if((pTempFontSurface=Szbh_LoadString(tXFontHandle, pXDispSubMenuTitle[i], dXColor_White, 0))!=NULL)
			{
		//	printf("Damon ==> show string : %s \n", pXDispSubMenuTitle[i]);
				if(tTempRect.width>pTempFontSurface->mWidth)
				{
					tTempShowRect.x=tTempRect.x+(tTempRect.width-pTempFontSurface->mWidth)/2;
					tTempShowRect.width=pTempFontSurface->mWidth;
				}
				if(tTempRect.height>pTempFontSurface->mHeight)
				{
					tTempShowRect.y=tTempRect.y+(tTempRect.height-pTempFontSurface->mHeight)/2;
					tTempShowRect.height=pTempFontSurface->mHeight;
				}

				Szbh_SurfaceBlitWithAlpha(pTempFontSurface, NULL, pXDispSubMenu->pBackSurface, &tTempShowRect, EBlitAlpha_Both);
				Szbh_SurfaceDestroy(pTempFontSurface);
				pTempFontSurface=NULL;

				char pTempString[16]={0};
				if(i==0)	// brightness
				{
				#ifdef dXTest
					if(XSectionGetValue(dXSetupConfigFile, dXSetupDispSectionName, dXSetupBacklightKeyName, pTempString, sizeof(pTempString)-1)!=0)
						strcpy(pTempString, "0");
				#else
					if(XSectionGetValue(dXSetupConfigFile, dXSetupDispSectionName, dXSetupBrightnessKeyName, pTempString, sizeof(pTempString)-1)!=0)
						strcpy(pTempString, "0");
				#endif
				}else if(i==1)	// contrast
				{
					if(XSectionGetValue(dXSetupConfigFile, dXSetupDispSectionName, dXSetupContrastKeyName, pTempString, sizeof(pTempString)-1)!=0)
						strcpy(pTempString, "0");
				}
				
				pTempFontSurface=Szbh_LoadString(tXFontHandle, pTempString, dXColor_White, 0);
				if(pTempFontSurface!=NULL)
				{
					memcpy(&tTempShowRect, &tTempRect, sizeof(X2DRect));
					tTempShowRect.x += pXDispSubMenu->mItemWidth+50;
					tTempShowRect.width=pTempFontSurface->mWidth;
					if(tTempRect.height>pTempFontSurface->mHeight)
					{
						tTempShowRect.y=tTempRect.y+(tTempRect.height-pTempFontSurface->mHeight)/2;
						tTempShowRect.height=pTempFontSurface->mHeight;
					}
					
					Szbh_SurfaceBlitWithAlpha(pTempFontSurface, NULL, pXDispSubMenu->pBackSurface, &tTempShowRect, EBlitAlpha_Both);
					Szbh_SurfaceDestroy(pTempFontSurface);
					pTempFontSurface=NULL;
				}
			}
		}

		tTempRect.x=pXDispSubMenu->mShowXPos;
		tTempRect.y=pXDispSubMenu->mShowYPos;
		tTempRect.width=pXDispSubMenu->pBackSurface->mWidth;
		tTempRect.height=pXDispSubMenu->pBackSurface->mHeight;
		if(Szbh_SurfaceQuickCopy(pXDispSubMenu->pBackSurface, NULL, pXSetupBackMenu, &tTempRect)!=0)
		{
			printf("Damon ==> blit sub menu surface failed !\n");
			return -3;
		}
	}else if(tInputLeftMenuIndex==1)	// ip config
	{
		if(pXIpSubMenu==NULL)
		{
			pXIpSubMenu=(XMenuParam *)malloc(sizeof(XMenuParam));
			if(pXIpSubMenu==NULL)
			{
				printf("Damon ==> line:[%d] not enought memory !\n", __LINE__);
				return -2;
			}

			pXIpSubMenu->mItemCount=sizeof(pXIpSubMenuTitle)/sizeof(pXIpSubMenuTitle[0]);
			pXIpSubMenu->mItemIndex=-1;
			pXIpSubMenu->mItemDefaultColor=0xffaaaaaa;
			pXIpSubMenu->mItemSelectColor=dXLeftMenuSelectColor;
			pXIpSubMenu->mItemWidth=dXLeftMenuWidth;
			pXIpSubMenu->mItemHeight=dXLineHeight;
			pXIpSubMenu->mShowXPos=dXLeftMenuWidth;
			pXIpSubMenu->mShowYPos=dXTitleHeight;
			pXIpSubMenu->pBackSurface=pXSubMenuSurface;
		}

		pXIpSubMenu->mItemIndex=-1;
		Szbh_SurfaceFill(pXIpSubMenu->pBackSurface, NULL, pXIpSubMenu->mItemDefaultColor);

		int i=0;
		X2DRect tTempRect;
		XSurface *pTempFontSurface=NULL;
		tTempRect.x=0;
		tTempRect.width=pXIpSubMenu->mItemWidth;
		tTempRect.height=dXLineHeight;
		for(i=0; i<pXIpSubMenu->mItemCount; i++)
		{
			X2DRect tTempShowRect;
			tTempRect.y=dXLineHeight*i;
			memcpy(&tTempShowRect, &tTempRect, sizeof(X2DRect));

			if((pTempFontSurface=Szbh_LoadString(tXFontHandle, pXIpSubMenuTitle[i], dXColor_White, 0))!=NULL)
			{
			printf("Damon ==> show string : %s \n", pXIpSubMenuTitle[i]);
				if(tTempRect.width>pTempFontSurface->mWidth)
				{
					tTempShowRect.x=tTempRect.x+(tTempRect.width-pTempFontSurface->mWidth)/2;
					tTempShowRect.width=pTempFontSurface->mWidth;
				}
				if(tTempRect.height>pTempFontSurface->mHeight)
				{
					tTempShowRect.y=tTempRect.y+(tTempRect.height-pTempFontSurface->mHeight)/2;
					tTempShowRect.height=pTempFontSurface->mHeight;
				}

				Szbh_SurfaceBlitWithAlpha(pTempFontSurface, NULL, pXIpSubMenu->pBackSurface, &tTempShowRect, EBlitAlpha_Both);
				Szbh_SurfaceDestroy(pTempFontSurface);
				pTempFontSurface=NULL;

				char pTempString[20]={0};
				if(i==0) 	// ip addr
				{
				/*	if(XSectionGetValue(dXSetupConfigFile, dXSetupIpSectionName, dXSetupIpKeyName, pTempString, sizeof(pTempString)-1)!=0)
					{
						if(XSetupGetLocalIp("eth0", pTempString)!=0)
							strcpy(pTempString, "0.0.0.0");
					}*/
					snprintf(pTempString, sizeof(pTempString)-1, "%d.%d.%d.%d", pXIpAddr[0], pXIpAddr[1], pXIpAddr[2], pXIpAddr[3]);
				}else if(i==1)	// net mask
				{
				/*	if(XSectionGetValue(dXSetupConfigFile, dXSetupIpSectionName, dXSetupMaskKeyName, pTempString, sizeof(pTempString)-1)!=0)
					{
						if(XSetupGetLocalIp("eth0", pTempString)!=0)
							strcpy(pTempString, "0.0.0.0");
					}*/
					snprintf(pTempString, sizeof(pTempString)-1, "%d.%d.%d.%d", pXNetMaskAddr[0], pXNetMaskAddr[1], pXNetMaskAddr[2], pXNetMaskAddr[3]);
				}
				
				pTempFontSurface=Szbh_LoadString(tXFontHandle, pTempString, dXColor_White, 0);
				if(pTempFontSurface!=NULL)
				{
					memcpy(&tTempShowRect, &tTempRect, sizeof(X2DRect));
					tTempShowRect.x += pXIpSubMenu->mItemWidth+50;
					tTempShowRect.width=pTempFontSurface->mWidth;
					if(tTempRect.height>pTempFontSurface->mHeight)
					{
						tTempShowRect.y=tTempRect.y+(tTempRect.height-pTempFontSurface->mHeight)/2;
						tTempShowRect.height=pTempFontSurface->mHeight;
					}
					
					Szbh_SurfaceBlitWithAlpha(pTempFontSurface, NULL, pXIpSubMenu->pBackSurface, &tTempShowRect, EBlitAlpha_Both);
					Szbh_SurfaceDestroy(pTempFontSurface);
					pTempFontSurface=NULL;
				}
			}
		}

		tTempRect.x=pXIpSubMenu->mShowXPos;
		tTempRect.y=pXIpSubMenu->mShowYPos;
		tTempRect.width=pXIpSubMenu->pBackSurface->mWidth;
		tTempRect.height=pXIpSubMenu->pBackSurface->mHeight;
		if(Szbh_SurfaceQuickCopy(pXIpSubMenu->pBackSurface, NULL, pXSetupBackMenu, &tTempRect)!=0)
		{
			printf("Damon ==> blit sub menu surface failed !\n");
			return -3;
		}
	}else if(tInputLeftMenuIndex==2)	// volume set
	{
		if(pXVolSubMenu==NULL)
		{
			pXVolSubMenu=(XMenuParam *)malloc(sizeof(XMenuParam));
			if(pXVolSubMenu==NULL)
			{
				printf("Damon ==> line:[%d] not enought memory !\n", __LINE__);
				return -2;
			}

			pXVolSubMenu->mItemCount=sizeof(pXVolSubMenuTitle)/sizeof(pXVolSubMenuTitle[0]);
			pXVolSubMenu->mItemIndex=-1;
			pXVolSubMenu->mItemDefaultColor=0xffaaaaaa;
			pXVolSubMenu->mItemSelectColor=dXLeftMenuSelectColor;
			pXVolSubMenu->mItemWidth=dXLeftMenuWidth;
			pXVolSubMenu->mItemHeight=dXLineHeight;
			pXVolSubMenu->mShowXPos=dXLeftMenuWidth;
			pXVolSubMenu->mShowYPos=dXTitleHeight;
			pXVolSubMenu->pBackSurface=pXSubMenuSurface;
		}

		pXVolSubMenu->mItemIndex=-1;
		Szbh_SurfaceFill(pXVolSubMenu->pBackSurface, NULL, pXVolSubMenu->mItemDefaultColor);

		int i=0;
		X2DRect tTempRect;
		XSurface *pTempFontSurface=NULL;
		tTempRect.x=0;
		tTempRect.width=pXVolSubMenu->mItemWidth;
		tTempRect.height=dXLineHeight;
		for(i=0; i<pXVolSubMenu->mItemCount; i++)
		{
			X2DRect tTempShowRect;
			tTempRect.y=dXLineHeight*i;
			memcpy(&tTempShowRect, &tTempRect, sizeof(X2DRect));

			if((pTempFontSurface=Szbh_LoadString(tXFontHandle, pXVolSubMenuTitle[i], dXColor_White, 0))!=NULL)
			{
			printf("Damon ==> show string : %s \n", pXVolSubMenuTitle[i]);
				if(tTempRect.width>pTempFontSurface->mWidth)
				{
					tTempShowRect.x=tTempRect.x+(tTempRect.width-pTempFontSurface->mWidth)/2;
					tTempShowRect.width=pTempFontSurface->mWidth;
				}
				if(tTempRect.height>pTempFontSurface->mHeight)
				{
					tTempShowRect.y=tTempRect.y+(tTempRect.height-pTempFontSurface->mHeight)/2;
					tTempShowRect.height=pTempFontSurface->mHeight;
				}

				Szbh_SurfaceBlitWithAlpha(pTempFontSurface, NULL, pXVolSubMenu->pBackSurface, &tTempShowRect, EBlitAlpha_Both);
				Szbh_SurfaceDestroy(pTempFontSurface);
				pTempFontSurface=NULL;

				char pTempString[16]={0};
				if(i==0)	// brightness
				{
					if(XSectionGetValue(dXSetupConfigFile, dXSetupDispSectionName, dXSetupVolumeKeyName, pTempString, sizeof(pTempString)-1)!=0)
						strcpy(pTempString, "0");
				}				
				pTempFontSurface=Szbh_LoadString(tXFontHandle, pTempString, dXColor_White, 0);
				if(pTempFontSurface!=NULL)
				{
					memcpy(&tTempShowRect, &tTempRect, sizeof(X2DRect));
					tTempShowRect.x += pXVolSubMenu->mItemWidth+50;
					tTempShowRect.width=pTempFontSurface->mWidth;
					if(tTempRect.height>pTempFontSurface->mHeight)
					{
						tTempShowRect.y=tTempRect.y+(tTempRect.height-pTempFontSurface->mHeight)/2;
						tTempShowRect.height=pTempFontSurface->mHeight;
					}
					
					Szbh_SurfaceBlitWithAlpha(pTempFontSurface, NULL, pXVolSubMenu->pBackSurface, &tTempShowRect, EBlitAlpha_Both);
					Szbh_SurfaceDestroy(pTempFontSurface);
					pTempFontSurface=NULL;
				}
			}
		}

		tTempRect.x=pXVolSubMenu->mShowXPos;
		tTempRect.y=pXVolSubMenu->mShowYPos;
		tTempRect.width=pXVolSubMenu->pBackSurface->mWidth;
		tTempRect.height=pXVolSubMenu->pBackSurface->mHeight;
		if(Szbh_SurfaceQuickCopy(pXVolSubMenu->pBackSurface, NULL, pXSetupBackMenu, &tTempRect)!=0)
		{
			printf("Damon ==> blit sub menu surface failed !\n");
			return -3;
		}
	
	}else if(tInputLeftMenuIndex==3)		// three color check
	{
		if(pXSubMenuSurface!=NULL)
		{
			X2DRect tTempRect;
			Szbh_SurfaceFill(pXSubMenuSurface, NULL, 0xffaaaaaa);

			tTempRect.x=dXLeftMenuWidth;
			tTempRect.y=dXTitleHeight;
			tTempRect.width=dXSetupMenuWidth-dXLeftMenuWidth;
			tTempRect.height=dXSetupMenuHeight-dXTitleHeight;
			if(Szbh_SurfaceQuickCopy(pXSubMenuSurface, NULL, pXSetupBackMenu, &tTempRect)!=0)
			{
				printf("Damon ==> blit sub menu surface failed !\n");
				return -3;
			}
		}		
	}

	if(tInputShowOrNot)
	{
		if(Szbh_LayerShowSurface(dXLayerId_Top, pXSetupBackMenu, dXSetupMenuShowX, dXSetupMenuShowY)!=0)
		{
			printf("Damon ==> line:[%d] show surface failed !\n", __LINE__);
			return -4;
		}
	}

printf("Damon ==> sub menu exit !\n");
	return 0;
}

static int XSubMenuChange(int tInputMenuIdx)
{
	if(tXMenuEnterFlag<=0 || pXLeftMenu==NULL)
		return -1;

	XMenuParam *pTempMenu=NULL;
	char (*pTempMenuTitle)[16]=NULL;

	if(pXLeftMenu->mItemIndex==0)	// param set
	{
		pTempMenu=pXDispSubMenu;
		pTempMenuTitle=pXDispSubMenuTitle;
	
	}else if(pXLeftMenu->mItemIndex==1)	// net set
	{
		pTempMenu=pXIpSubMenu;
		pTempMenuTitle=pXIpSubMenuTitle;
	
	}else if(pXLeftMenu->mItemIndex==2)	// volume set
	{
		pTempMenu=pXVolSubMenu;
		pTempMenuTitle=pXVolSubMenuTitle;
	}

	if(pTempMenu==NULL)
		return -2;

	{
		int i=0;
		X2DRect tTempRect;
		XSurface *pTempFontSurface=NULL;
		tTempRect.x=0;
		tTempRect.width=pTempMenu->mItemWidth;
		tTempRect.height=dXLineHeight;
		for(i=0; i<pTempMenu->mItemCount; i++)
		{
			if(i!=pTempMenu->mItemIndex && i!=tInputMenuIdx)
				continue;
		
			X2DRect tTempShowRect;
			tTempRect.y=dXLineHeight*i;
			memcpy(&tTempShowRect, &tTempRect, sizeof(X2DRect));


			if(i==pTempMenu->mItemIndex)
			{
				Szbh_SurfaceFill(pTempMenu->pBackSurface, &tTempRect, pTempMenu->mItemDefaultColor);
			}else if(i==tInputMenuIdx)
			{
				Szbh_SurfaceFill(pTempMenu->pBackSurface, &tTempRect, pTempMenu->mItemSelectColor);
			}

			if((pTempFontSurface=Szbh_LoadString(tXFontHandle, pTempMenuTitle[i], dXColor_White, 0))!=NULL)
			{
			printf("Damon ==> show string : %s \n", pTempMenuTitle[i]);
				if(tTempRect.width>pTempFontSurface->mWidth)
				{
					tTempShowRect.x=tTempRect.x+(tTempRect.width-pTempFontSurface->mWidth)/2;
					tTempShowRect.width=pTempFontSurface->mWidth;
				}
				if(tTempRect.height>pTempFontSurface->mHeight)
				{
					tTempShowRect.y=tTempRect.y+(tTempRect.height-pTempFontSurface->mHeight)/2;
					tTempShowRect.height=pTempFontSurface->mHeight;
				}

				Szbh_SurfaceBlitWithAlpha(pTempFontSurface, NULL, pTempMenu->pBackSurface, &tTempShowRect, EBlitAlpha_Both);
				Szbh_SurfaceDestroy(pTempFontSurface);
				pTempFontSurface=NULL;
			}
		}

		tTempRect.x=pTempMenu->mShowXPos;
		tTempRect.y=pTempMenu->mShowYPos;
		tTempRect.width=pTempMenu->pBackSurface->mWidth;
		tTempRect.height=pTempMenu->pBackSurface->mHeight;
		if(Szbh_SurfaceQuickCopy(pTempMenu->pBackSurface, NULL, pXSetupBackMenu, &tTempRect)!=0)
		{
			printf("Damon ==> blit sub menu surface failed !\n");
			return -3;
		}

		pTempMenu->mItemIndex=tInputMenuIdx;
	}

	if(Szbh_LayerShowSurface(dXLayerId_Top, pXSetupBackMenu, dXSetupMenuShowX, dXSetupMenuShowY)!=0)
	{
		printf("Damon ==> line:[%d] show surface failed !\n", __LINE__);
		return -4;
	}

	return 0;
}

static int XSubMenuChangeBakColor(unsigned int tInputColor)
{
	if(tXMenuEnterFlag<=0 || pXLeftMenu==NULL)
		return -1;

	XMenuParam *pTempMenu=NULL;
	char (*pTempMenuTitle)[16]=NULL;

	if(pXLeftMenu->mItemIndex==0)	// param set
	{
		pTempMenu=pXDispSubMenu;
		pTempMenuTitle=pXDispSubMenuTitle;
	
	}else if(pXLeftMenu->mItemIndex==1)	// net set
	{
		pTempMenu=pXIpSubMenu;
		pTempMenuTitle=pXIpSubMenuTitle;
	
	}else if(pXLeftMenu->mItemIndex==2)	// volume set
	{
		pTempMenu=pXVolSubMenu;
		pTempMenuTitle=pXVolSubMenuTitle;
	}

	if(pTempMenu==NULL)
		return -2;

	{
		X2DRect tTempRect;
		XSurface *pTempFontSurface=NULL;
		tTempRect.x=0;
		tTempRect.width=pTempMenu->mItemWidth;
		tTempRect.height=dXLineHeight;
		{
		
			X2DRect tTempShowRect;
			tTempRect.y=dXLineHeight*pTempMenu->mItemIndex;
			memcpy(&tTempShowRect, &tTempRect, sizeof(X2DRect));

			Szbh_SurfaceFill(pTempMenu->pBackSurface, &tTempRect, tInputColor);

			if((pTempFontSurface=Szbh_LoadString(tXFontHandle, pTempMenuTitle[pTempMenu->mItemIndex], dXColor_White, 0))!=NULL)
			{
				if(tTempRect.width>pTempFontSurface->mWidth)
				{
					tTempShowRect.x=tTempRect.x+(tTempRect.width-pTempFontSurface->mWidth)/2;
					tTempShowRect.width=pTempFontSurface->mWidth;
				}
				if(tTempRect.height>pTempFontSurface->mHeight)
				{
					tTempShowRect.y=tTempRect.y+(tTempRect.height-pTempFontSurface->mHeight)/2;
					tTempShowRect.height=pTempFontSurface->mHeight;
				}

				Szbh_SurfaceBlitWithAlpha(pTempFontSurface, NULL, pTempMenu->pBackSurface, &tTempShowRect, EBlitAlpha_Both);
				Szbh_SurfaceDestroy(pTempFontSurface);
				pTempFontSurface=NULL;
			}
		}

		tTempRect.x=pTempMenu->mShowXPos;
		tTempRect.y=pTempMenu->mShowYPos;
		tTempRect.width=pTempMenu->pBackSurface->mWidth;
		tTempRect.height=pTempMenu->pBackSurface->mHeight;
		if(Szbh_SurfaceQuickCopy(pTempMenu->pBackSurface, NULL, pXSetupBackMenu, &tTempRect)!=0)
		{
			printf("Damon ==> blit sub menu surface failed !\n");
			return -3;
		}
	}

	if(Szbh_LayerShowSurface(dXLayerId_Top, pXSetupBackMenu, dXSetupMenuShowX, dXSetupMenuShowY)!=0)
	{
		printf("Damon ==> line:[%d] show surface failed !\n", __LINE__);
		return -4;
	}

	return 0;
}


static int XSetValMenuSelectOrNot(bool tInputSelect, unsigned int tInputFontBakColor, int tInputIpIndex)
{
	if(tXMenuEnterFlag<=0 || pXLeftMenu==NULL)
		return -1;

	char pTempShowString[32]={0};
	XMenuParam *pTempSubMenu=NULL;

	if(pXLeftMenu->mItemIndex==0)	// param set 
	{
		pTempSubMenu=pXDispSubMenu;
		if(pTempSubMenu->mItemIndex==0)		// brightness
		{
		#ifdef dXTest		
			if(XSectionGetValue(dXSetupConfigFile, dXSetupDispSectionName, dXSetupBacklightKeyName, pTempShowString, sizeof(pTempShowString)-1)!=0)
				strcpy(pTempShowString, "0");
		#else
			if(XSectionGetValue(dXSetupConfigFile, dXSetupDispSectionName, dXSetupBrightnessKeyName, pTempShowString, sizeof(pTempShowString)-1)!=0)
				strcpy(pTempShowString, "0");
		#endif
		}else if(pTempSubMenu->mItemIndex==1)	// contrast
		{
			if(XSectionGetValue(dXSetupConfigFile, dXSetupDispSectionName, dXSetupContrastKeyName, pTempShowString, sizeof(pTempShowString)-1)!=0)
				strcpy(pTempShowString, "0");
		}else
		{
			return -2;
		}
	}else if(pXLeftMenu->mItemIndex==1)	// net set
	{
		pTempSubMenu=pXIpSubMenu;
		if(pTempSubMenu->mItemIndex==0)		// ip addr
		{
		/*	if(XSectionGetValue(dXSetupConfigFile, dXSetupIpSectionName, dXSetupIpKeyName, pTempShowString, sizeof(pTempShowString)-1)!=0)
			{
				if(XSetupGetLocalIp("eth0", pTempShowString)!=0)
					strcpy(pTempShowString, "0.0.0.0");
			}*/
			snprintf(pTempShowString, sizeof(pTempShowString)-1, "%d.%d.%d.%d", pXIpAddr[0], pXIpAddr[1], pXIpAddr[2], pXIpAddr[3]);
		}else if(pTempSubMenu->mItemIndex==1)	// net mask
		{
		/*	if(XSectionGetValue(dXSetupConfigFile, dXSetupIpSectionName, dXSetupMaskKeyName, pTempShowString, sizeof(pTempShowString)-1)!=0)
			{
				if(XSetupGetLocalIp("eth0", pTempShowString)!=0)
					strcpy(pTempShowString, "0.0.0.0");
			}*/
			snprintf(pTempShowString, sizeof(pTempShowString)-1, "%d.%d.%d.%d", pXNetMaskAddr[0], pXNetMaskAddr[1], pXNetMaskAddr[2], pXNetMaskAddr[3]);
		}else
		{
			return -2;
		}
	}else if(pXLeftMenu->mItemIndex==2)	// volume set
	{
		pTempSubMenu=pXVolSubMenu;
		if(pTempSubMenu->mItemIndex==0)
		{
			if(XSectionGetValue(dXSetupConfigFile, dXSetupDispSectionName, dXSetupVolumeKeyName, pTempShowString, sizeof(pTempShowString)-1)!=0)
			{
				strcpy(pTempShowString, "100");
			}
		}else
		{
			return -2;
		}
	}

	if(pTempSubMenu==NULL)
		return -3;

	X2DRect tTempRect;
	XSurface *pTempFontSurface=NULL;
	tTempRect.x= pTempSubMenu->mItemWidth;
	tTempRect.width=pTempSubMenu->pBackSurface->mWidth-pTempSubMenu->mItemWidth;
	tTempRect.height=dXLineHeight;
	tTempRect.y=dXLineHeight*pTempSubMenu->mItemIndex;

	if(tInputSelect)
		Szbh_SurfaceFill(pTempSubMenu->pBackSurface, &tTempRect, pTempSubMenu->mItemSelectColor);
	else 
		Szbh_SurfaceFill(pTempSubMenu->pBackSurface, &tTempRect, pTempSubMenu->mItemDefaultColor);		

	pTempFontSurface=Szbh_LoadString(tXFontHandle, pTempShowString, dXColor_White, 0);
	if(pTempFontSurface!=NULL)
	{
		tTempRect.x = pTempSubMenu->mItemWidth+50;
		tTempRect.width=pTempFontSurface->mWidth;
		if(tTempRect.height>pTempFontSurface->mHeight)
		{
			tTempRect.y=tTempRect.y+(tTempRect.height-pTempFontSurface->mHeight)/2;
			tTempRect.height=pTempFontSurface->mHeight;
		}

		if(tInputFontBakColor>0)
		{
			if(pXLeftMenu->mItemIndex==1)	// net set
			{
				unsigned char *pTempAddr=NULL;
				if(pTempSubMenu->mItemIndex==0)
					pTempAddr=pXIpAddr;
				else if(pTempSubMenu->mItemIndex==1)
					pTempAddr=pXNetMaskAddr;

			#if 0
				int i=0;
				int tTempSumLen=0;
				int tTempShowLen=0;
				for(i=0; i<tInputIpIndex+1; i++)
				{
					tTempShowLen=XGetIntToCharLen(pTempAddr[i]);
					tTempSumLen += tTempShowLen;
					if(i>0)
						tTempSumLen += 1;
				}

				X2DRect tTempShowRect;
				memcpy(&tTempShowRect, &tTempRect, sizeof(X2DRect));
				int tTempStringLen=strlen(pTempShowString);
				if(tTempStringLen>0)
				{
					tTempShowRect.x += (((tTempSumLen-tTempShowLen)*pTempFontSurface->mWidth)/tTempStringLen);
					tTempShowRect.width=(tTempShowLen*pTempFontSurface->mWidth)/tTempStringLen;

					Szbh_SurfaceFill(pTempSubMenu->pBackSurface, &tTempShowRect, tInputFontBakColor);
				}
			#else
				XSurface *pTempFontSurface=NULL;
				if(pTempAddr)
				{
					char pTempIpString[20]={0};
					X2DRect tTempShowRect;

					memcpy(&tTempShowRect, &tTempRect, sizeof(X2DRect));
					if(tInputIpIndex==0)
					{
						snprintf(pTempIpString, sizeof(pTempIpString)-1, "%d", pTempAddr[0]);
						pTempFontSurface=Szbh_LoadString(tXFontHandle, pTempIpString, dXColor_White, 0);
						if(pTempFontSurface!=NULL)
						{
							tTempShowRect.width=pTempFontSurface->mWidth;
						
							Szbh_SurfaceDestroy(pTempFontSurface);
							pTempFontSurface=NULL;
						}
					}else if(tInputIpIndex==1)
					{
						snprintf(pTempIpString, sizeof(pTempIpString)-1, "%d.", pTempAddr[0]);
						pTempFontSurface=Szbh_LoadString(tXFontHandle, pTempIpString, dXColor_White, 0);
						if(pTempFontSurface!=NULL)
						{
							tTempShowRect.x += pTempFontSurface->mWidth;
						
							Szbh_SurfaceDestroy(pTempFontSurface);
							pTempFontSurface=NULL;
						}

						snprintf(pTempIpString, sizeof(pTempIpString)-1, "%d", pTempAddr[1]);
						pTempFontSurface=Szbh_LoadString(tXFontHandle, pTempIpString, dXColor_White, 0);
						if(pTempFontSurface!=NULL)
						{
							tTempShowRect.width= pTempFontSurface->mWidth;
						
							Szbh_SurfaceDestroy(pTempFontSurface);
							pTempFontSurface=NULL;
						}
					}else if(tInputIpIndex==2)
					{
						snprintf(pTempIpString, sizeof(pTempIpString)-1, "%d.%d.", pTempAddr[0], pTempAddr[1]);
						pTempFontSurface=Szbh_LoadString(tXFontHandle, pTempIpString, dXColor_White, 0);
						if(pTempFontSurface!=NULL)
						{
							tTempShowRect.x += pTempFontSurface->mWidth;
						
							Szbh_SurfaceDestroy(pTempFontSurface);
							pTempFontSurface=NULL;
						}

						snprintf(pTempIpString, sizeof(pTempIpString)-1, "%d", pTempAddr[2]);
						pTempFontSurface=Szbh_LoadString(tXFontHandle, pTempIpString, dXColor_White, 0);
						if(pTempFontSurface!=NULL)
						{
							tTempShowRect.width= pTempFontSurface->mWidth;
						
							Szbh_SurfaceDestroy(pTempFontSurface);
							pTempFontSurface=NULL;
						}
					}else if(tInputIpIndex==3)
					{
						snprintf(pTempIpString, sizeof(pTempIpString)-1, "%d.%d.%d.", pTempAddr[0], pTempAddr[1], pTempAddr[2]);
						pTempFontSurface=Szbh_LoadString(tXFontHandle, pTempIpString, dXColor_White, 0);
						if(pTempFontSurface!=NULL)
						{
							tTempShowRect.x += pTempFontSurface->mWidth;
						
							Szbh_SurfaceDestroy(pTempFontSurface);
							pTempFontSurface=NULL;
						}

						snprintf(pTempIpString, sizeof(pTempIpString)-1, "%d", pTempAddr[3]);
						pTempFontSurface=Szbh_LoadString(tXFontHandle, pTempIpString, dXColor_White, 0);
						if(pTempFontSurface!=NULL)
						{
							tTempShowRect.width= pTempFontSurface->mWidth;
						
							Szbh_SurfaceDestroy(pTempFontSurface);
							pTempFontSurface=NULL;
						}
					}

					Szbh_SurfaceFill(pTempSubMenu->pBackSurface, &tTempShowRect, tInputFontBakColor);					
				}
			#endif
			}else
			{
				Szbh_SurfaceFill(pTempSubMenu->pBackSurface, &tTempRect, tInputFontBakColor);
			}
		}
		
		Szbh_SurfaceBlitWithAlpha(pTempFontSurface, NULL, pTempSubMenu->pBackSurface, &tTempRect, EBlitAlpha_Both);
		Szbh_SurfaceDestroy(pTempFontSurface);
		pTempFontSurface=NULL;

		tTempRect.x=pTempSubMenu->mShowXPos;
		tTempRect.y=pTempSubMenu->mShowYPos;
		tTempRect.width=pTempSubMenu->pBackSurface->mWidth;
		tTempRect.height=pTempSubMenu->pBackSurface->mHeight;
		if(Szbh_SurfaceQuickCopy(pTempSubMenu->pBackSurface, NULL, pXSetupBackMenu, &tTempRect)!=0)
		{
			printf("Damon ==> blit sub menu surface failed !\n");
			return -4;
		}

		if(Szbh_LayerShowSurface(dXLayerId_Top, pXSetupBackMenu, dXSetupMenuShowX, dXSetupMenuShowY)!=0)
		{
			printf("Damon ==> line:[%d] show surface failed !\n", __LINE__);
			return -5;
		}
	}else
	{
		printf("Damon ==> load string failed !\n");
		return -4;
	}
	
	return 0;
}


static int XSetDispParamValue(int tInputVal, unsigned int tInputFontBakColor)
{
	if(tXMenuEnterFlag<=0 || pXLeftMenu==NULL || tInputVal<0 || tInputVal>100)
		return -1;

	char pTempShowString[32]={0};
	XMenuParam *pTempSubMenu=NULL;

	if(pXLeftMenu->mItemIndex==0)	// param set 
	{
		pTempSubMenu=pXDispSubMenu;
	}else if(pXLeftMenu->mItemIndex==2)	// volume set
	{
		pTempSubMenu=pXVolSubMenu;
	}

	snprintf(pTempShowString, sizeof(pTempShowString)-1, "%d", tInputVal);

	if(pTempSubMenu==NULL)
		return -3;

	X2DRect tTempRect;
	XSurface *pTempFontSurface=NULL;
	tTempRect.x= pTempSubMenu->mItemWidth;
	tTempRect.width=pTempSubMenu->pBackSurface->mWidth-pTempSubMenu->mItemWidth;
	tTempRect.height=dXLineHeight;
	tTempRect.y=dXLineHeight*pTempSubMenu->mItemIndex;

	Szbh_SurfaceFill(pTempSubMenu->pBackSurface, &tTempRect, pTempSubMenu->mItemSelectColor);

	pTempFontSurface=Szbh_LoadString(tXFontHandle, pTempShowString, dXColor_White, 0);
	if(pTempFontSurface!=NULL)
	{
		tTempRect.x = pTempSubMenu->mItemWidth+50;
		tTempRect.width=pTempFontSurface->mWidth;
		if(tTempRect.height>pTempFontSurface->mHeight)
		{
			tTempRect.y=tTempRect.y+(tTempRect.height-pTempFontSurface->mHeight)/2;
			tTempRect.height=pTempFontSurface->mHeight;
		}

		if(tInputFontBakColor>0)
			Szbh_SurfaceFill(pTempSubMenu->pBackSurface, &tTempRect, tInputFontBakColor);
		
		Szbh_SurfaceBlitWithAlpha(pTempFontSurface, NULL, pTempSubMenu->pBackSurface, &tTempRect, EBlitAlpha_Both);
		Szbh_SurfaceDestroy(pTempFontSurface);
		pTempFontSurface=NULL;

		tTempRect.x=pTempSubMenu->mShowXPos;
		tTempRect.y=pTempSubMenu->mShowYPos;
		tTempRect.width=pTempSubMenu->pBackSurface->mWidth;
		tTempRect.height=pTempSubMenu->pBackSurface->mHeight;
		if(Szbh_SurfaceQuickCopy(pTempSubMenu->pBackSurface, NULL, pXSetupBackMenu, &tTempRect)!=0)
		{
			printf("Damon ==> blit sub menu surface failed !\n");
			return -4;
		}

		if(Szbh_LayerShowSurface(dXLayerId_Top, pXSetupBackMenu, dXSetupMenuShowX, dXSetupMenuShowY)!=0)
		{
			printf("Damon ==> line:[%d] show surface failed !\n", __LINE__);
			return -5;
		}
	}else
	{
		printf("Damon ==> load string failed !\n");
		return -4;
	}


	// set display param
	if(pXLeftMenu->mItemIndex==0)
	{
		if(pTempSubMenu->mItemIndex==0)
		//	Szbh_DisplaySetBacklight(1000, tInputVal);
			Szbh_DisplaySetBrightness(tInputVal);
		else if(pTempSubMenu->mItemIndex==1)
			Szbh_DisplaySetContrast(tInputVal);
	}else if(pXLeftMenu->mItemIndex==2)
	{
		if(pTempSubMenu->mItemIndex==0)
			XPlayerSetVolume(tInputVal);
	//		Szbh_TsPlayerSetVolume(tInputVal);
	}
	
	return 0;	
}


static int XSetNetAddrParamValue(int tInputVal, unsigned int tInputFontBakColor)
{
	if(tXMenuEnterFlag<=0 || pXLeftMenu==NULL || pXLeftMenu->mItemIndex!=1)
		return -1;

	if(tXIpSelectIndex<0 || tXIpSelectIndex>3)
		return -2;

	unsigned char *pTempAddr=NULL;
	
	if(pXIpSubMenu->mItemIndex==0)
		pTempAddr=pXIpAddr;
	else if(pXIpSubMenu->mItemIndex==1)
		pTempAddr=pXNetMaskAddr;
	else
		return -3;
	
	pTempAddr[tXIpSelectIndex]=tInputVal;

	return XSetValMenuSelectOrNot(true, 0xffff0000, tXIpSelectIndex);
}

static int XLeftMenuInit(void)
{
	if(pXLeftMenu==NULL)
	{
		pXLeftMenu=malloc(sizeof(XMenuParam));
		if(pXLeftMenu==NULL)
		{
			printf("Damon ==> not enought memory !\n");
			return -1;
		}

		memset(pXLeftMenu, 0, sizeof(XMenuParam));
		pXLeftMenu->pBackSurface=NULL;
	}

	if(pXLeftMenu->pBackSurface==NULL)
	{
		pXLeftMenu->pBackSurface=Szbh_SurfaceCreate(dXLeftMenuWidth, dXSetupMenuHeight-dXTitleHeight);
		if(pXLeftMenu->pBackSurface==NULL)
		{
			printf("Damon ==> line : [%d] create surface failed !\n", __LINE__);
			return -2;
		}

		printf("Damon ==> left back menu addr = %p - %d %d \n", pXLeftMenu->pBackSurface, 
			pXLeftMenu->pBackSurface->mWidth, pXLeftMenu->pBackSurface->mHeight);

		Szbh_SurfaceFill(pXLeftMenu->pBackSurface, NULL, 0xff555555);
	}

	pXLeftMenu->mItemCount=sizeof(pXMenuOption)/sizeof(pXMenuOption[0]);
	pXLeftMenu->mItemIndex=0;
	pXLeftMenu->mItemDefaultColor=0xff555555;
	pXLeftMenu->mItemSelectColor=dXLeftMenuSelectColor;
	pXLeftMenu->mItemWidth=dXLeftMenuWidth;
	pXLeftMenu->mItemHeight=dXLineHeight;
	pXLeftMenu->mShowXPos=0;
	pXLeftMenu->mShowYPos=dXTitleHeight;

	return 0;
}


static int XLeftMenuChange(int tInputMenuIdx)
{
	if(pXLeftMenu==NULL || pXLeftMenu->pBackSurface==NULL)
		return -1;

	if(tInputMenuIdx==pXLeftMenu->mItemIndex || tInputMenuIdx>=pXLeftMenu->mItemCount)
		return -1;

printf("Damon ==> index : %d %d \n", pXLeftMenu->mItemIndex, tInputMenuIdx);
	if(tInputMenuIdx==1)	// net set
	{
		// init ip addr and net  mask
		char pTempString[32]={0};
		if(XSectionGetValue(dXSetupConfigFile, dXSetupIpSectionName, dXSetupIpKeyName, pTempString, sizeof(pTempString))!=0)
		{
			if(XSetupGetLocalIp("eth0", pTempString)!=0)
			{
				strcpy(pTempString, "0.0.0.0");
			}
		}

		int pTempAddr[4]={0};
		sscanf(pTempString, "%d.%d.%d.%d", (pTempAddr+0), (pTempAddr+1), (pTempAddr+2), (pTempAddr+3));
		pXIpAddr[0]=(unsigned char)pTempAddr[0];
		pXIpAddr[1]=(unsigned char)pTempAddr[1];
		pXIpAddr[2]=(unsigned char)pTempAddr[2];
		pXIpAddr[3]=(unsigned char)pTempAddr[3];
		printf("Damon ==> init ip : %d.%d.%d.%d \n", pXIpAddr[0], pXIpAddr[1], pXIpAddr[2], pXIpAddr[3]);

		memset(pTempString, 0, sizeof(pTempString));
		if(XSectionGetValue(dXSetupConfigFile, dXSetupIpSectionName, dXSetupMaskKeyName, pTempString, sizeof(pTempString))!=0)
		{
			if(XSetupGetLocalNetMask("eth0", pTempString)!=0)
			{
				strcpy(pTempString, "0.0.0.0");
			}
		}

		sscanf(pTempString, "%d.%d.%d.%d", (pTempAddr+0), (pTempAddr+1), (pTempAddr+2), (pTempAddr+3));
		pXNetMaskAddr[0]=(unsigned char)pTempAddr[0];
		pXNetMaskAddr[1]=(unsigned char)pTempAddr[1];
		pXNetMaskAddr[2]=(unsigned char)pTempAddr[2];
		pXNetMaskAddr[3]=(unsigned char)pTempAddr[3];
		printf("Damon ==> init net mask : %d.%d.%d.%d \n", pXNetMaskAddr[0], pXNetMaskAddr[1], pXNetMaskAddr[2], pXNetMaskAddr[3]);
	}

	int i=0;
	X2DRect tTempSelectRect;
	for(i=0; i<pXLeftMenu->mItemCount; i++)
	{
		if(i!=pXLeftMenu->mItemIndex && i!=tInputMenuIdx)
			continue;
	
		if(pXLeftFontSurface[i]!=NULL)
		{
			tTempSelectRect.x=0;
			tTempSelectRect.y=i*dXLineHeight;
			tTempSelectRect.width=dXLeftMenuWidth;
			tTempSelectRect.height=dXLineHeight;
			if(i==pXLeftMenu->mItemIndex)
			{
				Szbh_SurfaceFill(pXLeftMenu->pBackSurface, &tTempSelectRect, pXLeftMenu->mItemDefaultColor);
			}else if(i==tInputMenuIdx)
			{
				Szbh_SurfaceFill(pXLeftMenu->pBackSurface, &tTempSelectRect, pXLeftMenu->mItemSelectColor);
			}

			if(tTempSelectRect.width>pXLeftFontSurface[i]->mWidth)
			{
				tTempSelectRect.x=tTempSelectRect.x+(tTempSelectRect.width-pXLeftFontSurface[i]->mWidth)/2;
				tTempSelectRect.width=pXLeftFontSurface[i]->mWidth;
			}
			if(tTempSelectRect.height>pXLeftFontSurface[i]->mHeight)
			{
				tTempSelectRect.y=tTempSelectRect.y+(tTempSelectRect.height-pXLeftFontSurface[i]->mHeight)/2;
				tTempSelectRect.height=pXLeftFontSurface[i]->mHeight;
			}

			Szbh_SurfaceBlitWithAlpha(pXLeftFontSurface[i], NULL, pXLeftMenu->pBackSurface, &tTempSelectRect, EBlitAlpha_Both);
		}
	}

	X2DRect tTempDstRect;
	tTempDstRect.x=pXLeftMenu->mShowXPos;
	tTempDstRect.y=pXLeftMenu->mShowYPos;
	tTempDstRect.width=dXLeftMenuWidth;
	tTempDstRect.height=(dXSetupMenuHeight-dXTitleHeight);
	if(Szbh_SurfaceQuickCopy(pXLeftMenu->pBackSurface, NULL, pXSetupBackMenu, &tTempDstRect)!=0)
	{
		printf("Damon ==> line:[%d] blit left menu surface failed !\n", __LINE__);
		return -2;
	}

	if(Szbh_LayerShowSurface(dXLayerId_Top, pXSetupBackMenu, dXSetupMenuShowX, dXSetupMenuShowY)!=0)
	{
		printf("Damon ==> show surface failed !\n");
		return -5;
	}

	
	return 0;
}

static int XLeftMenuChangeBakColor(int tInputMenuIdx, unsigned int tInputColor)
{
	if(tXMenuEnterFlag==0 || pXLeftMenu==NULL)
		return -1;

	if(tInputMenuIdx<0 || tInputMenuIdx>=pXLeftMenu->mItemCount)
		return -1;

	if(tInputMenuIdx!=pXLeftMenu->mItemIndex)
	{
		printf("Damon ==> input menu index valid !\n");
		return -1;
	}


	X2DRect tTempSelectRect;
	if(pXLeftFontSurface[tInputMenuIdx]!=NULL)
	{
		tTempSelectRect.x=0;
		tTempSelectRect.y=tInputMenuIdx*dXLineHeight;
		tTempSelectRect.width=dXLeftMenuWidth;
		tTempSelectRect.height=dXLineHeight;
		Szbh_SurfaceFill(pXLeftMenu->pBackSurface, &tTempSelectRect, tInputColor);

		if(tTempSelectRect.width>pXLeftFontSurface[tInputMenuIdx]->mWidth)
		{
			tTempSelectRect.x=tTempSelectRect.x+(tTempSelectRect.width-pXLeftFontSurface[tInputMenuIdx]->mWidth)/2;
			tTempSelectRect.width=pXLeftFontSurface[tInputMenuIdx]->mWidth;
		}
		if(tTempSelectRect.height>pXLeftFontSurface[tInputMenuIdx]->mHeight)
		{
			tTempSelectRect.y=tTempSelectRect.y+(tTempSelectRect.height-pXLeftFontSurface[tInputMenuIdx]->mHeight)/2;
			tTempSelectRect.height=pXLeftFontSurface[tInputMenuIdx]->mHeight;
		}

		Szbh_SurfaceBlitWithAlpha(pXLeftFontSurface[tInputMenuIdx], NULL, pXLeftMenu->pBackSurface, &tTempSelectRect, EBlitAlpha_Both);
	}


	X2DRect tTempDstRect;
	tTempDstRect.x=pXLeftMenu->mShowXPos;
	tTempDstRect.y=pXLeftMenu->mShowYPos;
	tTempDstRect.width=dXLeftMenuWidth;
	tTempDstRect.height=(dXSetupMenuHeight-dXTitleHeight);
	if(Szbh_SurfaceQuickCopy(pXLeftMenu->pBackSurface, NULL, pXSetupBackMenu, &tTempDstRect)!=0)
	{
		printf("Damon ==> line:[%d] blit left menu surface failed !\n", __LINE__);
		return -2;
	}

	if(Szbh_LayerShowSurface(dXLayerId_Top, pXSetupBackMenu, dXSetupMenuShowX, dXSetupMenuShowY)!=0)
	{
		printf("Damon ==> show surface failed !\n");
		return -3;
	}

	return 0;	
}

static int XSetupMenuEnter(void)
{
	if(pXSetupBackMenu==NULL)
	{
		pXSetupBackMenu=Szbh_SurfaceCreate(dXSetupMenuWidth, dXSetupMenuHeight);
		if(pXSetupBackMenu==NULL)
		{
			printf("Damon ==> create surface failed !\n");
			return -1;
		}

		Szbh_SurfaceFill(pXSetupBackMenu, NULL, dXColor_Black);

	}

	if(pXLeftMenu==NULL || pXLeftMenu->pBackSurface==NULL)
	{
		if(XLeftMenuInit()!=0)
		{
			printf("Damon ==> left menu init failed !\n");
			return -1;
		}
	}else
	{
		if(pXLeftMenu->pBackSurface!=NULL)
			Szbh_SurfaceFill(pXLeftMenu->pBackSurface, NULL, 0xff555555);
	}

	if(pXSubMenuSurface==NULL)
	{
		pXSubMenuSurface=Szbh_SurfaceCreate(dXSubMenuWidth, dXSetupMenuHeight-dXTitleHeight);
		if(pXSubMenuSurface==NULL)
		{
			printf("Damon ==> create sub menu surface failed !\n");
			return -1;
		}

		Szbh_SurfaceFill(pXSubMenuSurface, NULL, 0xffaaaaaa);
	}

	if(tXFontHandle==0)
	{
		if(Szbh_CreateFont(dXResourcePath"/font.ttf", 40, &tXFontHandle)!=0)
		{
			printf("Damon ==> create font failed !\n");
			return -2;
		}

	//	char pTempTitle[16]={0xE8, 0x8F, 0x9C, 0xE5, 0x8D, 0x95, 0x00};	// menu
		char *pTempTitle="菜单 (修改参数后按'OK'键保存)";
		XSurface *pTempMenuString=Szbh_LoadString(tXFontHandle, pTempTitle,dXColor_White, 0);
		if(pTempMenuString!=NULL)
		{
			X2DRect tTempDstRect;
			tTempDstRect.x=50;
			tTempDstRect.y=30;
			tTempDstRect.width=pTempMenuString->mWidth;
			tTempDstRect.height=pTempMenuString->mHeight;
			
			Szbh_SurfaceBlitWithAlpha(pTempMenuString, NULL, pXSetupBackMenu, &tTempDstRect, EBlitAlpha_Both);

			Szbh_SurfaceDestroy(pTempMenuString);
			pTempMenuString=NULL;
		}		
	}

	pXLeftMenu->mItemIndex=0;


	// init left menu font
	int i=0;
	for(i=0; i<pXLeftMenu->mItemCount; i++)
	{
		if(pXLeftFontSurface[i]==NULL)
		{
			pXLeftFontSurface[i]=Szbh_LoadString(tXFontHandle, pXMenuOption[i], dXColor_White, 0);
			if(pXLeftFontSurface[i]==NULL)
			{
				printf("Damon ==> load string failed : %s \n", pXMenuOption[i]);
				return -3;
			}
		}
	}


	// blit left menu font
	X2DRect tTempSelectRect;
	for(i=0; i<pXLeftMenu->mItemCount; i++)
	{
		if(pXLeftFontSurface[i]!=NULL)
		{
			tTempSelectRect.x=0;
			tTempSelectRect.y=i*dXLineHeight;
			tTempSelectRect.width=dXLeftMenuWidth;
			tTempSelectRect.height=dXLineHeight;
			if(i==pXLeftMenu->mItemIndex)
			{
				Szbh_SurfaceFill(pXLeftMenu->pBackSurface, &tTempSelectRect, dXLeftMenuSelectColor);
			}

			if(tTempSelectRect.width>pXLeftFontSurface[i]->mWidth)
			{
				tTempSelectRect.x=tTempSelectRect.x+(tTempSelectRect.width-pXLeftFontSurface[i]->mWidth)/2;
				tTempSelectRect.width=pXLeftFontSurface[i]->mWidth;
			}
			if(tTempSelectRect.height>pXLeftFontSurface[i]->mHeight)
			{
				tTempSelectRect.y=tTempSelectRect.y+(tTempSelectRect.height-pXLeftFontSurface[i]->mHeight)/2;
				tTempSelectRect.height=pXLeftFontSurface[i]->mHeight;
			}

			Szbh_SurfaceBlitWithAlpha(pXLeftFontSurface[i], NULL, pXLeftMenu->pBackSurface, &tTempSelectRect, EBlitAlpha_Both);
		}
	}


	X2DRect tTempDstRect;
	tTempDstRect.x=pXLeftMenu->mShowXPos;
	tTempDstRect.y=pXLeftMenu->mShowYPos;
	tTempDstRect.width=dXLeftMenuWidth;
	tTempDstRect.height=(dXSetupMenuHeight-dXTitleHeight);
	if(Szbh_SurfaceQuickCopy(pXLeftMenu->pBackSurface, NULL, pXSetupBackMenu, &tTempDstRect)!=0)
	{
		printf("Damon ==> blit left menu surface failed !\n");
		return -3;
	}

	if(XSubMenuEnter(pXLeftMenu->mItemIndex, false)!=0)
	{
		tTempDstRect.x=dXLeftMenuWidth;
		tTempDstRect.y=dXTitleHeight;
		tTempDstRect.width=dXSubMenuWidth;
		tTempDstRect.height=(dXSetupMenuHeight-dXTitleHeight);
		if(Szbh_SurfaceQuickCopy(pXSubMenuSurface, NULL, pXSetupBackMenu, &tTempDstRect)!=0)
		{
			printf("Damon ==> blit sub menu surface failed !\n");
			return -4;
		}
	}

	if(Szbh_LayerShowSurface(dXLayerId_Top, pXSetupBackMenu, dXSetupMenuShowX, dXSetupMenuShowY)!=0)
	{
		printf("Damon ==> show surface failed !\n");
		return -5;
	}

	tXSubMenuSelect=0;

	if(tXSetupMenuExitHandle==0)
	{
		unsigned int tTempCurTick=0;
		if(XGetTick(&tTempCurTick)==0)
			tXKeyDownPrevTick=tTempCurTick;
	
		if(pthread_create(&tXSetupMenuExitHandle, NULL, XSetupMenuTimeoutExitProc, NULL)!=0)
		{
			printf("Damon ==> Error create setup menu exit thread failed !\n");
			tXSetupMenuExitHandle=0;
		}
	}
	
	return 0;
}

static int XSetupMenuClose(void)
{
	if(tXMenuEnterFlag)
	{
		X2DRect tTempRect={dXSetupMenuShowX, dXSetupMenuShowY, dXSetupMenuWidth, dXSetupMenuHeight};
		Szbh_LayerClear(dXLayerId_Top, tTempRect, 0);
	}

	tXMenuEnterFlag=0;
	if(tXSetupMenuExitHandle)
	{
		pthread_join(tXSetupMenuExitHandle, NULL);
		tXSetupMenuExitHandle=0;
	}

	return 0;
}


int XSetupMenuHandle(unsigned int tInputIrCode)
{
	char tTempVal=-1;

	
	unsigned int tTempCurTick=0;
	if(XGetTick(&tTempCurTick)==0)
	{
		tXKeyDownPrevTick=tTempCurTick;
	}

	switch(tInputIrCode)
	{
		case dXIrCode_Menu:
			{
				if(tXMenuEnterFlag==0)
				{
					if(tXCheckColorFlag>0)
					{
						tXCheckColorFlag=0;
						XSetColorMode(0, 0, 0);
						
						X2DRect tTempRect={0, 0, dXScreenWidth, dXScreenHeight};
						Szbh_LayerClear(dXLayerId_Top, tTempRect, 0);
					}
				
					if(XSetupMenuEnter()==0)
						tXMenuEnterFlag=1;
				}else
				{
					XSetupMenuClose();
					tXMenuEnterFlag=0;
				}
			}
			break;
		case dXIrCode_Up:
			{
				if(tXMenuEnterFlag)
				{
					if(tXSubMenuSelect==0)	// left menu
					{
						if(pXLeftMenu->mItemIndex>0)
						{
							XLeftMenuChange(pXLeftMenu->mItemIndex-1);
						
							pXLeftMenu->mItemIndex--;
							XSubMenuEnter(pXLeftMenu->mItemIndex, true);
						}
					}else if(tXSubMenuSelect==1)	// sub menu
					{
						if(pXLeftMenu->mItemIndex==0) 	// param set
						{
							if(pXDispSubMenu->mItemIndex>0)
								XSubMenuChange(pXDispSubMenu->mItemIndex-1);
						}else if(pXLeftMenu->mItemIndex==1)	// net set
						{
							if(pXIpSubMenu->mItemIndex>0)
								XSubMenuChange(pXIpSubMenu->mItemIndex-1);
						}else if(pXLeftMenu->mItemIndex==2)	// volume set
						{
							if(pXVolSubMenu->mItemIndex>0)
								XSubMenuChange(pXVolSubMenu->mItemIndex-1);
						}
					}
				}
			}
			break;
		case dXIrCode_Down:
			{
				if(tXMenuEnterFlag)
				{
					if(tXSubMenuSelect==0)
					{	// left  menu select  
						if(pXLeftMenu->mItemIndex<pXLeftMenu->mItemCount-1)
						{
							XLeftMenuChange(pXLeftMenu->mItemIndex+1);
						
							pXLeftMenu->mItemIndex++;
							XSubMenuEnter(pXLeftMenu->mItemIndex, true);
						}
					}else if(tXSubMenuSelect==1)	// sub menu
					{
						if(pXLeftMenu->mItemIndex==0) 	// param set
						{
							if(pXDispSubMenu->mItemIndex<pXDispSubMenu->mItemCount-1)
								XSubMenuChange(pXDispSubMenu->mItemIndex+1);
						}else if(pXLeftMenu->mItemIndex==1)	// net set
						{
							if(pXIpSubMenu->mItemIndex<pXIpSubMenu->mItemCount-1)
								XSubMenuChange(pXIpSubMenu->mItemIndex+1);
						}else if(pXLeftMenu->mItemIndex==2)	// volume set
						{
							if(pXVolSubMenu->mItemIndex<pXVolSubMenu->mItemCount-1)
								XSubMenuChange(pXVolSubMenu->mItemIndex+1);
						}
					}
				}
			}
			break;
		case dXIrCode_Left:
			{
				if(tXMenuEnterFlag)
				{
					XMenuParam *pTempMenu=NULL;
					if(pXLeftMenu->mItemIndex==0)
						pTempMenu=pXDispSubMenu;
					else if(pXLeftMenu->mItemIndex==1)
						pTempMenu=pXIpSubMenu;
					else if(pXLeftMenu->mItemIndex==2)
						pTempMenu=pXVolSubMenu;

					if(pTempMenu)
					{
						if(tXSubMenuSelect==1)
						{
							if(XSubMenuChangeBakColor(0xffaaaaaa)==0)
							{
								pTempMenu->mItemIndex=-1;
							
								XLeftMenuChangeBakColor(pXLeftMenu->mItemIndex, dXLeftMenuSelectColor);
								tXSubMenuSelect=0;
							}
						}else if(tXSubMenuSelect==2)
						{
							if(XSetValMenuSelectOrNot(false, 0, 0)==0)
							{
								XSubMenuChangeBakColor(dXLeftMenuSelectColor);
								tXSubMenuSelect=1;
							}
						}else if(tXSubMenuSelect==3)
						{
							if(pXLeftMenu->mItemIndex==1 && tXIpSelectIndex>0)
							{
								tXIpSelectIndex--;
								
								XSetValMenuSelectOrNot(true, 0xffff0000, tXIpSelectIndex);
								tXSetValue=-1;
							}
						}
					}
				}
			}
			break;
		case dXIrCode_Right:
			{
				if(tXMenuEnterFlag)
				{printf("Damon ==> sub menu select : %d %d \n", pXLeftMenu->mItemIndex, tXSubMenuSelect);
					if(tXSubMenuSelect==0)
					{
						if((pXLeftMenu->mItemIndex>=0) && (pXLeftMenu->mItemIndex<pXLeftMenu->mItemCount-1))
						{
							if(XLeftMenuChangeBakColor(pXLeftMenu->mItemIndex, 0xffaaaaaa)==0)
							{
								XSubMenuChange(0);
							
								tXSubMenuSelect=1;
							}
						}
					}else if(tXSubMenuSelect==1)
					{
						if(XSubMenuChangeBakColor(0xffaaaaaa)==0)
						{
							XSetValMenuSelectOrNot(true, 0, 0);
							tXSubMenuSelect=2;
						}
					}else if(tXSubMenuSelect==3)
					{
						if(pXLeftMenu->mItemIndex==1 && tXIpSelectIndex<3)
						{
							tXIpSelectIndex++;
							
							XSetValMenuSelectOrNot(true, 0xffff0000, tXIpSelectIndex);
							tXSetValue=-1;
						}
					}
				}
			}
			break;
		case dXIrCode_Enter:
			{
				if(tXMenuEnterFlag)
				{
					if(tXSubMenuSelect==0)	// left menu
					{
						if(pXLeftMenu->mItemIndex==3)	// three color check
						{
							XSetupMenuClose();
							tXMenuEnterFlag=0;

							XSetColorMode(1, 2, 0);
							tXCheckColorFlag=1;
						}
					}else if(tXSubMenuSelect==2)
					{
						tXIpSelectIndex=0;
						XSetValMenuSelectOrNot(true, 0xffff0000, 0);
						tXSubMenuSelect=3;

						tXSetValue=-1;
					}else if(tXSubMenuSelect==3)
					{
						if(tXSetValue>=0)	// save value
						{
							char pTempString[32]={0};
							snprintf(pTempString, sizeof(pTempString)-1, "%d", tXSetValue);
							if(pXLeftMenu->mItemIndex==0)
							{
								if(pXDispSubMenu->mItemIndex==0)	// brightness
								{
								#ifdef dXTest
									XSectionSaveValue(dXSetupConfigFile, dXSetupDispSectionName, dXSetupBacklightKeyName, pTempString);
								#else
									XSectionSaveValue(dXSetupConfigFile, dXSetupDispSectionName, dXSetupBrightnessKeyName, pTempString);
								#endif
								}
								else if(pXDispSubMenu->mItemIndex==1)	// contrast
									XSectionSaveValue(dXSetupConfigFile, dXSetupDispSectionName, dXSetupContrastKeyName, pTempString);
							}else if(pXLeftMenu->mItemIndex==1)
							{
								unsigned char *pTempAddr=NULL;
								if(pXIpSubMenu->mItemIndex==0)
									pTempAddr=pXIpAddr;
								else if(pXIpSubMenu->mItemIndex==1)
									pTempAddr=pXNetMaskAddr;
								if(pTempAddr!=NULL)
								{
									memset(pTempString, 0, sizeof(pTempString));
									snprintf(pTempString, sizeof(pTempString)-1, "%d.%d.%d.%d", pTempAddr[0], pTempAddr[1], pTempAddr[2], pTempAddr[3]);

									if(pXIpSubMenu->mItemIndex==0)
										XSectionSaveValue(dXSetupConfigFile, dXSetupIpSectionName, dXSetupIpKeyName, pTempString);
									else if(pXIpSubMenu->mItemIndex==1)
										XSectionSaveValue(dXSetupConfigFile, dXSetupIpSectionName, dXSetupMaskKeyName, pTempString);
								}
							}else if(pXLeftMenu->mItemIndex==2)
							{
								if(pXVolSubMenu->mItemIndex==0) // volume
								{
								//	if(Szbh_TsPlayerSetVolume(tXSetValue)==0)
									if(XPlayerSetVolume(tXSetValue)==0)
										XSectionSaveValue(dXSetupConfigFile, dXSetupDispSectionName, dXSetupVolumeKeyName, pTempString);
								}
							}
						}
					
						if(XSetValMenuSelectOrNot(true, 0, 0)==0)
						{						
							tXSubMenuSelect=2;
						}
					}
				}
			}
			break;
		case dXIrCode_Exit:
			{
			#ifdef dXTest
				static int tTempFlag=0;
				if(tTempFlag==0)
					XPlayerLocalVideoPauseOrResume(true);
				else
					XPlayerLocalVideoPauseOrResume(false);
				tTempFlag=(tTempFlag+1)%2;
			#else
				if(tXMenuEnterFlag)
				{
					XSetupMenuClose();
					tXMenuEnterFlag=0;
				}else if(tXCheckColorFlag>0)
				{
					XSetColorMode(0, 0, 0);
					tXCheckColorFlag=0;
					X2DRect tTempRect={0, 0, dXScreenWidth, dXScreenHeight};
					Szbh_LayerClear(dXLayerId_Top, tTempRect, 0);
				}
			#endif
			}
			break;
		case dXIrCode_Num0:
			tTempVal=0;
			break;
		case dXIrCode_Num1:
			tTempVal=1;
			break;
		case dXIrCode_Num2:
			tTempVal=2;
			break;
		case dXIrCode_Num3:
			tTempVal=3;
			break;
		case dXIrCode_Num4:
			tTempVal=4;
			break;
		case dXIrCode_Num5:
			tTempVal=5;
			break;
		case dXIrCode_Num6:
			tTempVal=6;
			break;
		case dXIrCode_Num7:
			tTempVal=7;
			break;
		case dXIrCode_Num8:
			tTempVal=8;
			break;
		case dXIrCode_Num9:
			tTempVal=9;
			break;
	}

	if(tTempVal>=0 && tTempVal<=9)
	{
		if(tXMenuEnterFlag)
		{
			if(tXSubMenuSelect==3)
			{
			/*	if(!(tXSetValue<0 && tInputIrCode==dXIrCode_Num0))*/
				{
					if(tXSetValue<0)
						tXSetValue=0;

					if(pXLeftMenu->mItemIndex==1)
					{
						if(tXSetValue<100)
						{
							tXSetValue *= 10;
							tXSetValue += tTempVal;
							if(tXSetValue>255)
								tXSetValue=255;
						}else /*if(tTempVal!=0)*/
						{
							tXSetValue=tTempVal;
						}
					}else
					{
						if(tXSetValue<100)
						{
							tXSetValue *= 10;
							tXSetValue += tTempVal;
							if(tXSetValue>100)
								tXSetValue=100;
						}else /*if(tTempVal!=0)*/
						{
							tXSetValue=tTempVal;
						}
					}

					printf("Damon ==> set value : %d \n", tXSetValue);
					if(pXLeftMenu->mItemIndex==0 || pXLeftMenu->mItemIndex==2)
					{
						XSetDispParamValue(tXSetValue, 0xffff0000);
					}else if(pXLeftMenu->mItemIndex==1)
					{
						XSetNetAddrParamValue(tXSetValue, 0xffff0000);
					}
				}
			}
		}
	}

	return 0;
}


int XSetupGetLocalIp(char *pInputEthInf, char *pOutputIp)
{
	if(pInputEthInf==NULL || pOutputIp==NULL)
		return -1;

	int tTempFd=0;
	struct sockaddr_in tTempSin;
	struct ifreq tTempIfr;

	tTempFd=socket(AF_INET, SOCK_DGRAM, 0);
	if(tTempFd<0)
	{
		printf("Damon ==> create socket failed \n");		
		close(tTempFd);
		return -2;
	}

	strncpy(tTempIfr.ifr_name, pInputEthInf, IFNAMSIZ);
	tTempIfr.ifr_name[IFNAMSIZ-1]=0;
	if(ioctl(tTempFd, SIOCGIFADDR, &tTempIfr)<0)
	{
		printf("Damon ==> ioctl error \n");
		close(tTempFd);
		return -1;
	}

	memcpy(&tTempSin, &tTempIfr.ifr_addr, sizeof(tTempSin));
	snprintf(pOutputIp, 16, "%s", inet_ntoa(tTempSin.sin_addr));

	close(tTempFd);

	return 0;
}

int XSetupGetLocalNetMask(char *pInputEthInf, char *pOutputNetMask)
{
	if(pInputEthInf==NULL || pOutputNetMask==NULL)
		return -1;

	int tTempFd=0;
	struct sockaddr_in tTempSin;
	struct ifreq tTempIfr;

	tTempFd=socket(AF_INET, SOCK_DGRAM, 0);
	if(tTempFd<0)
	{
		printf("Damon ==> create socket failed \n");		
		close(tTempFd);
		return -2;
	}

	strncpy(tTempIfr.ifr_name, pInputEthInf, IFNAMSIZ);
	tTempIfr.ifr_name[IFNAMSIZ-1]=0;
	if(ioctl(tTempFd, SIOCGIFNETMASK, &tTempIfr)<0)
	{
		printf("Damon ==> ioctl error \n");
		close(tTempFd);
		return -1;
	}

	memcpy(&tTempSin, &tTempIfr.ifr_netmask, sizeof(tTempSin));
	snprintf(pOutputNetMask, 16, "%s", inet_ntoa(tTempSin.sin_addr));

	close(tTempFd);

	return 0;
}


int XSetupGetLocalMacAddr(char *pInputEthInf, char *pOutputMac)
{
	if(pInputEthInf==NULL || pOutputMac==NULL)
		return -1;

	int tTempFd=0;
	struct ifreq tTempIfr;

	tTempFd=socket(AF_INET, SOCK_DGRAM, 0);
	if(tTempFd<0)
	{
		printf("Damon ==> create socket failed \n");		
		close(tTempFd);
		return -2;
	}

	strncpy(tTempIfr.ifr_name, pInputEthInf, IFNAMSIZ);
	tTempIfr.ifr_name[IFNAMSIZ-1]=0;
	if(ioctl(tTempFd, SIOCGIFHWADDR, &tTempIfr)<0)
	{
		printf("Damon ==> ioctl error \n");
		close(tTempFd);
		return -1;
	}

	sprintf(pOutputMac, "%02x:%02x:%02x:%02x:%02x:%02x", 
		(unsigned char )tTempIfr.ifr_hwaddr.sa_data[0], 
		(unsigned char )tTempIfr.ifr_hwaddr.sa_data[1],
		(unsigned char )tTempIfr.ifr_hwaddr.sa_data[2],
		(unsigned char )tTempIfr.ifr_hwaddr.sa_data[3],
		(unsigned char )tTempIfr.ifr_hwaddr.sa_data[4],
		(unsigned char )tTempIfr.ifr_hwaddr.sa_data[5]);

	close(tTempFd);

	return 0;
}

int XSetupMenuInit(void)
{
#if 0
	if(pXSetupBackMenu==NULL)
	{
		pXSetupBackMenu=Szbh_SurfaceCreate(dXSetupMenuWidth, dXSetupMenuHeight);
		if(pXSetupBackMenu==NULL)
		{
			printf("Damon ==> [%s] create surface failed !\n", __FILE__);
			return -1;
		}
		Szbh_SurfaceFill(pXSetupBackMenu, NULL, dXColor_Black);

	}

	if(pXLeftMenu==NULL || pXLeftMenu->pBackSurface==NULL)
	{
		if(XLeftMenuInit()!=0)
		{
			printf("Damon ==> [%s] left menu init failed !\n", __FILE__);
			return -2;
		}
	}else
	{
		if(pXLeftMenu->pBackSurface!=NULL)
			Szbh_SurfaceFill(pXLeftMenu->pBackSurface, NULL, 0xff555555);
	}

	if(pXSubMenuSurface==NULL)
	{
		pXSubMenuSurface=Szbh_SurfaceCreate(dXSubMenuWidth, dXSetupMenuHeight-dXTitleHeight);
		if(pXSubMenuSurface==NULL)
		{
			printf("Damon ==> [%s] create sub menu surface failed !\n", __FILE__);
			return -3;
		}
		Szbh_SurfaceFill(pXSubMenuSurface, NULL, 0xffaaaaaa);
	}


	if(pXDispSubMenu==NULL)
	{
		pXDispSubMenu=(XMenuParam *)malloc(sizeof(XMenuParam));
		if(pXDispSubMenu==NULL)
		{
			printf("Damon ==> line:[%d] not enought memory !\n", __LINE__);
			return -4;
		}

		pXDispSubMenu->mItemCount=sizeof(pXDispSubMenuTitle)/sizeof(pXDispSubMenuTitle[0]);
		pXDispSubMenu->mItemIndex=-1;
		pXDispSubMenu->mItemDefaultColor=0xffaaaaaa;
		pXDispSubMenu->mItemSelectColor=dXLeftMenuSelectColor;
		pXDispSubMenu->mItemWidth=dXLeftMenuWidth;
		pXDispSubMenu->mItemHeight=dXLineHeight;
		pXDispSubMenu->mShowXPos=dXLeftMenuWidth;
		pXDispSubMenu->mShowYPos=dXTitleHeight;
		pXDispSubMenu->pBackSurface=pXSubMenuSurface;
	}

	if(pXIpSubMenu==NULL)
	{
		pXIpSubMenu=(XMenuParam *)malloc(sizeof(XMenuParam));
		if(pXIpSubMenu==NULL)
		{
			printf("Damon ==> line:[%d] not enought memory !\n", __LINE__);
			return -5;
		}

		pXIpSubMenu->mItemCount=sizeof(pXIpSubMenuTitle)/sizeof(pXIpSubMenuTitle[0]);
		pXIpSubMenu->mItemIndex=-1;
		pXIpSubMenu->mItemDefaultColor=0xffaaaaaa;
		pXIpSubMenu->mItemSelectColor=dXLeftMenuSelectColor;
		pXIpSubMenu->mItemWidth=dXLeftMenuWidth;
		pXIpSubMenu->mItemHeight=dXLineHeight;
		pXIpSubMenu->mShowXPos=dXLeftMenuWidth;
		pXIpSubMenu->mShowYPos=dXTitleHeight;
		pXIpSubMenu->pBackSurface=pXSubMenuSurface;
	}	

	if(pXVolSubMenu==NULL)
	{
		pXVolSubMenu=(XMenuParam *)malloc(sizeof(XMenuParam));
		if(pXVolSubMenu==NULL)
		{
			printf("Damon ==> line:[%d] not enought memory !\n", __LINE__);
			return -6;
		}
	
		pXVolSubMenu->mItemCount=sizeof(pXVolSubMenuTitle)/sizeof(pXVolSubMenuTitle[0]);
		pXVolSubMenu->mItemIndex=-1;
		pXVolSubMenu->mItemDefaultColor=0xffaaaaaa;
		pXVolSubMenu->mItemSelectColor=dXLeftMenuSelectColor;
		pXVolSubMenu->mItemWidth=dXLeftMenuWidth;
		pXVolSubMenu->mItemHeight=dXLineHeight;
		pXVolSubMenu->mShowXPos=dXLeftMenuWidth;
		pXVolSubMenu->mShowYPos=dXTitleHeight;
		pXVolSubMenu->pBackSurface=pXSubMenuSurface;
	}
#endif

	return 0;
}

int XSetupConfigInit(unsigned char *pOutputVol, int *pOutputDeviceId)
{
	char pTempCmd[64]={0};
	char pTempString[32]={0};
	// ip 
	if(XSectionGetValue(dXSetupConfigFile, dXSetupIpSectionName, dXSetupIpKeyName, pTempString, sizeof(pTempString))!=0)
	{
		if(XSetupGetLocalIp("eth0", pTempString)==0)
		{
			XSectionSaveValue(dXSetupConfigFile, dXSetupIpSectionName, dXSetupIpKeyName, pTempString);
		}
		printf("Damon ==> : ipaddr[%s]\n", pTempString);
	}else
	{
		snprintf(pTempCmd, sizeof(pTempCmd)-1, "ifconfig eth0 %s", pTempString);

		int tTempDeviceId=0;
		char *pTempPt=strrchr(pTempString, '.');
		if(pTempPt!=NULL)
		{
			pTempPt++;
			while(pTempPt && *pTempPt)
			{
				tTempDeviceId=tTempDeviceId*10+(*pTempPt-'0');
				pTempPt++;
			}
		}

		if(pOutputDeviceId!=NULL)
			*pOutputDeviceId=tTempDeviceId;
		
		printf("Damon => cmd:%s\n", pTempCmd);
		system(pTempCmd);
	}

	// mask
	memset(pTempCmd, 0, sizeof(pTempCmd));
	memset(pTempString, 0, sizeof(pTempString));
	if(XSectionGetValue(dXSetupConfigFile, dXSetupIpSectionName, dXSetupMaskKeyName, pTempString, sizeof(pTempString))!=0)
	{
		if(XSetupGetLocalNetMask("eth0", pTempString)==0)
		{
			XSectionSaveValue(dXSetupConfigFile, dXSetupIpSectionName, dXSetupMaskKeyName, pTempString);
		}
		else
		{
		
			log_save(dXLogLevel_Error, "XSetupGetLocalNetMask failed  [%s][%d]\n", __FUNCTION__, __LINE__);
			exit(-1);
		}
		printf("Damon ==> : netmask[%s]\n", pTempString);
	}else
	{
		snprintf(pTempCmd, sizeof(pTempCmd)-1, "ifconfig eth0 netmask %s", pTempString);
		printf("Damon => cmd:%s\n", pTempCmd);
		system(pTempCmd);
	}

	// mac addr 
	memset(pTempCmd, 0, sizeof(pTempCmd));
	memset(pTempString, 0, sizeof(pTempString));
	if(XSectionGetValue(dXSetupConfigFile, dXSetupIpSectionName, dXSetupMacKeyName, pTempString, sizeof(pTempString))!=0)
	{
		if(XSetupGetLocalMacAddr("eth0", pTempString)==0)
		{
			XSectionSaveValue(dXSetupConfigFile, dXSetupIpSectionName, dXSetupMacKeyName, pTempString);
		}
		else
		{
		
			log_save(dXLogLevel_Error, "XSetupGetLocalMacAddr failed [%s][%d]\n", __FUNCTION__, __LINE__);
			exit(-1);
		}
		printf("Damon ==> : macaddr[%s]\n", pTempString);
	}else
	{
		snprintf(pTempCmd, sizeof(pTempCmd)-1, "ifconfig eth0 hw ether ");
		int i=0, j=0;;
		int tTempLen=strlen(pTempCmd);
		for(i=0; i<strlen(pTempString); i++)
		{
			if((i+1)%3==0)
				continue;
			pTempCmd[tTempLen+j]=pTempString[i];
			j++;
		}
		printf("Damon => cmd:%s\n", pTempCmd);
		system(pTempCmd);
	}

	sleep(2);

	//brightness
	memset(pTempCmd, 0, sizeof(pTempCmd));
	memset(pTempString, 0, sizeof(pTempString));
	if(XSectionGetValue(dXSetupConfigFile, dXSetupDispSectionName, dXSetupBrightnessKeyName, pTempString, sizeof(pTempString))!=0)
	{
		int tTempBrightness=0;
		if(Szbh_DisplayGetBrightness(&tTempBrightness)==0)
		{
			snprintf(pTempString, sizeof(pTempString)-1, "%d", tTempBrightness);
			XSectionSaveValue(dXSetupConfigFile, dXSetupDispSectionName, dXSetupBrightnessKeyName, pTempString);
		}
		printf("Damon ==> : brightness[%s]\n", pTempString);
	}else
	{
		int tTempBrightness=atoi(pTempString);
		if(tTempBrightness>=0 && tTempBrightness<=100)
			Szbh_DisplaySetBrightness(tTempBrightness);
	}
	

	//contrast
	memset(pTempCmd, 0, sizeof(pTempCmd));
	memset(pTempString, 0, sizeof(pTempString));
	if(XSectionGetValue(dXSetupConfigFile, dXSetupDispSectionName, dXSetupContrastKeyName, pTempString, sizeof(pTempString))!=0)
	{
		int tTempContrast=0;
		if(Szbh_DisplayGetContrast(&tTempContrast)==0)
		{
			snprintf(pTempString, sizeof(pTempString)-1, "%d", tTempContrast);
			XSectionSaveValue(dXSetupConfigFile, dXSetupDispSectionName, dXSetupContrastKeyName, pTempString);
		}
		printf("Damon ==> : contrast[%s]\n", pTempString);
	}else
	{
		int tTempContrast=atoi(pTempString);
		if(tTempContrast>=0 && tTempContrast<=100)
			Szbh_DisplaySetContrast(tTempContrast);
	}

	//backlight
	memset(pTempCmd, 0, sizeof(pTempCmd));
	memset(pTempString, 0, sizeof(pTempString));
	if(XSectionGetValue(dXSetupConfigFile, dXSetupDispSectionName, dXSetupBacklightKeyName, pTempString, sizeof(pTempString))!=0)
	{
		int tTempBacklight=50;
		{
			snprintf(pTempString, sizeof(pTempString)-1, "%d", tTempBacklight);
			XSectionSaveValue(dXSetupConfigFile, dXSetupDispSectionName, dXSetupBacklightKeyName, pTempString);
			Szbh_DisplaySetBacklight(tXBacklightHz, tTempBacklight);
		}
		printf("Damon ==> : backlight[%s]\n", pTempString);
	}else
	{
		int tTempBacklight=atoi(pTempString);
		if(tTempBacklight>=0 && tTempBacklight<=100)
			Szbh_DisplaySetBacklight(tXBacklightHz, tTempBacklight);
	}

	// volume
	memset(pTempCmd, 0, sizeof(pTempCmd));
	memset(pTempString, 0, sizeof(pTempString));
	int tTempVolume=100;
	if(XSectionGetValue(dXSetupConfigFile, dXSetupDispSectionName, dXSetupVolumeKeyName, pTempString, sizeof(pTempString))!=0)
	{
		{
			snprintf(pTempString, sizeof(pTempString)-1, "%d", tTempVolume);
			XSectionSaveValue(dXSetupConfigFile, dXSetupDispSectionName, dXSetupVolumeKeyName, pTempString);
		}
		printf("Damon ==> : volume[%s]\n", pTempString);
	}else
	{
		tTempVolume=atoi(pTempString);
		if(pOutputVol!=NULL)
		{
			if(tTempVolume>=0 && tTempVolume<=100)
				*pOutputVol=tTempVolume;
			else
				*pOutputVol=101;
		}
	}


	// screen save timeout
	memset(pTempString, 0, sizeof(pTempString));
	if(XSectionGetValue(dXSetupConfigFile, dXSetupDispSectionName, dXSetupScreenSaveTimeoutKeyName, pTempString, sizeof(pTempString))==0)
	{
		int tTempScreenSaveTimeout=atoi(pTempString);
		XSetScreenSaveOrCloseTimeout(1, tTempScreenSaveTimeout);
		printf("Damon ==> : screen save [%s]\n", pTempString);
	}

	// screen close timeout
	memset(pTempString, 0, sizeof(pTempString));
	if(XSectionGetValue(dXSetupConfigFile, dXSetupDispSectionName, dXSetupScreenCloseTimeoutKeyName, pTempString, sizeof(pTempString))==0)
	{
		int tTempScreenCloseTimeout=atoi(pTempString);
		XSetScreenSaveOrCloseTimeout(0, tTempScreenCloseTimeout);
		printf("Damon ==> : screen close [%s]\n", pTempString);
	}

	// add route
	system("route add -net 225.1.1.40 netmask 255.255.255.255 eth0");
	system("route add -net 225.1.1.41 netmask 255.255.255.255 eth0");
	system("route add -net 225.1.1.42 netmask 255.255.255.255 eth0");
	usleep(1000*300);
	
	return 0;
}

