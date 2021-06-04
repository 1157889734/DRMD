

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <pthread.h>

#include "XVehiclePlayerDefine.h"


#include "SzbhApi.h"
#include "XScrollText.h"


typedef struct{
	int mRun;
	int mPause;
	pthread_t mThreadHandle;
	int mThreadExit;
	unsigned int mFontHandle;
	X2DRect mShowRect;
	XSurface *pFontSurface;
	XSurface *pBackSurface;
	unsigned int mFontBackColor;
}XScrollText;

static XScrollText tXScrollText;

extern int XGetScreenSaveFlag();

int XScrollTextIsRun(void)
{
	if(tXScrollText.mRun && tXScrollText.mPause==0)
		return 1;

	return 0;
}

int XScrollTextStop(void)
{
	if(tXScrollText.mRun)
	{
		tXScrollText.mRun=0;

		if(pthread_join(tXScrollText.mThreadHandle, NULL)!=0)
			sleep(1);
		
		do{
			usleep(1000*100);
		}while(tXScrollText.mThreadExit==0);

	#ifdef dXCustomer_IptvDrmd
		Szbh_LayerClear(dXLayerId_Top, tXScrollText.mShowRect, 0);
		Szbh_LayerRender(dXLayerId_Top);
	#else
		Szbh_LayerClear(dXLayerId_Bottom, tXScrollText.mShowRect, 0);
		Szbh_LayerRender(dXLayerId_Bottom);
	#endif

		if(tXScrollText.pFontSurface!=NULL)
			Szbh_SurfaceDestroy(tXScrollText.pFontSurface);
		tXScrollText.pFontSurface=NULL;
		if(tXScrollText.pBackSurface!=NULL)
			Szbh_SurfaceDestroy(tXScrollText.pBackSurface);
		tXScrollText.pBackSurface=NULL;
	}

	return 0;
}

void XScrollTextPause(void)
{
	if(tXScrollText.mRun)
	{
		tXScrollText.mPause=1;

	#ifdef dXCustomer_IptvDrmd
		usleep(1000*100);
		Szbh_LayerClear(dXLayerId_Top, tXScrollText.mShowRect, 0);
		Szbh_LayerRender(dXLayerId_Top);
	#endif
	}
}

void XScrollTextResume(void)
{
	if(XGetScreenSaveFlag()>=1)
		return ;

	if(tXScrollText.mRun)
		tXScrollText.mPause=0;
}

void *XScrollTextProc(void *pInputArg)
{
	tXScrollText.mRun=1;
	tXScrollText.mPause=0;
	tXScrollText.mThreadExit=0;

	ELayerId tTempLayerId=dXLayerId_Bottom;
#ifdef dXCustomer_IptvDrmd
	tTempLayerId=dXLayerId_Top;
#endif

//	int tTempTotalWidth=tXScrollText.pBackSurface->mWidth+tXScrollText.pFontSurface->mWidth*2;
	int tTempStartX=tXScrollText.mShowRect.x+tXScrollText.mShowRect.width;

	int tTempPosX=tTempStartX;
	while(tXScrollText.mRun)
	{
		if(tXScrollText.mPause==0)
		{
			if(0==Szbh_SurfaceFill(tXScrollText.pBackSurface, NULL, tXScrollText.mFontBackColor))
			{
				int tTempWidth;
				tTempPosX -= 2;
	//		printf("\nDamon ==> scroll text pos : %d %d \n", tTempPosX, tTempStartX);
				if(tTempPosX>=tXScrollText.mShowRect.x)
				{
					tTempWidth=tTempStartX-tTempPosX;
					if(tTempWidth>tXScrollText.pFontSurface->mWidth)
						tTempWidth=tXScrollText.pFontSurface->mWidth;
					if(tTempWidth<=0)
						continue;

					X2DRect tTempSrcRect, tTempDstRect;
					tTempSrcRect.x=0;
					tTempSrcRect.y=0;
					tTempSrcRect.width=tTempWidth;
					tTempSrcRect.height=tXScrollText.pFontSurface->mHeight;

					tTempDstRect.x=tTempPosX-tXScrollText.mShowRect.x;
					tTempDstRect.width=tTempWidth;
					if(tXScrollText.mShowRect.height>tXScrollText.pFontSurface->mHeight)
					{
						tTempDstRect.y=(tXScrollText.mShowRect.height-tXScrollText.pFontSurface->mHeight)/2;
						tTempDstRect.height=tXScrollText.pFontSurface->mHeight;
					}else
					{
						tTempDstRect.y=0;
						tTempDstRect.height=tXScrollText.mShowRect.height;
					}

			/*	printf("Damon ==> %d %d %d %d - %d %d %d %d \n", tTempSrcRect.x, tTempSrcRect.y, tTempSrcRect.width, tTempSrcRect.height,
						tTempDstRect.x, tTempDstRect.y, tTempDstRect.width, tTempDstRect.height);*/
				//	if(Szbh_SurfaceQuickCopy(tXScrollText.pFontSurface, &tTempSrcRect, tXScrollText.pBackSurface, &tTempDstRect)==0)
					if(Szbh_SurfaceBlitWithAlpha(tXScrollText.pFontSurface, &tTempSrcRect, tXScrollText.pBackSurface, &tTempDstRect, EBlitAlpha_Both)==0)
					{
						Szbh_LayerShowSurface(tTempLayerId, tXScrollText.pBackSurface, tXScrollText.mShowRect.x, tXScrollText.mShowRect.y);
					}
				}else if(tTempPosX<tXScrollText.mShowRect.x-tXScrollText.pFontSurface->mWidth)
				{
					Szbh_LayerClear(tTempLayerId, tXScrollText.mShowRect, tXScrollText.mFontBackColor);
					sleep(2);
					tTempPosX=tTempStartX;
				}else // over left rect x
				{
					tTempWidth=tXScrollText.pFontSurface->mWidth-(tXScrollText.mShowRect.x-tTempPosX);
					if(tTempWidth>tXScrollText.mShowRect.width)
						tTempWidth=tXScrollText.mShowRect.width;
					if(tTempWidth<=0)
						continue;
					
					X2DRect tTempSrcRect, tTempDstRect;
					tTempSrcRect.x=tXScrollText.mShowRect.x-tTempPosX;
					tTempSrcRect.y=0;
					tTempSrcRect.width=tTempWidth;
					tTempSrcRect.height=tXScrollText.pFontSurface->mHeight;

					tTempDstRect.x=0;	
					tTempDstRect.width=tTempWidth;
					if(tXScrollText.mShowRect.height>tXScrollText.pFontSurface->mHeight)
					{
						tTempDstRect.y=(tXScrollText.mShowRect.height-tXScrollText.pFontSurface->mHeight)/2;
						tTempDstRect.height=tXScrollText.pFontSurface->mHeight;
					}else
					{
						tTempDstRect.y=0;
						tTempDstRect.height=tXScrollText.mShowRect.height;
					}
					
				//	if(Szbh_SurfaceQuickResize(tXScrollText.pFontSurface, &tTempSrcRect, tXScrollText.pBackSurface, &tTempDstRect)==0)
					if(Szbh_SurfaceBlitWithAlpha(tXScrollText.pFontSurface, &tTempSrcRect, tXScrollText.pBackSurface, &tTempDstRect, EBlitAlpha_Both)==0)
					{
						Szbh_LayerShowSurface(tTempLayerId, tXScrollText.pBackSurface, tXScrollText.mShowRect.x, tXScrollText.mShowRect.y);
					}
				}
				
				Szbh_LayerRender(tTempLayerId);
			}
		}

		usleep(1000*10);
	}

	tXScrollText.mThreadExit=1;

	return NULL;
}

int XScrollTextStart(unsigned int tInputFontHandle, char *pInputString, unsigned int tInputFontColor, unsigned int tInputBackColor, X2DRect *pInputShowRect)
{
	if(tInputFontHandle<=0 || pInputString==NULL || pInputShowRect==NULL)
	{
		printf("Damon ==> start scroll text error input param !\n");
		return -1;
	}


	if(XGetScreenSaveFlag()>=1)
		return 0;

	XScrollTextStop();

	unsigned int tTempFontHandle=tInputFontHandle;

	if((tXScrollText.pFontSurface=Szbh_LoadString(tTempFontHandle, pInputString, tInputFontColor, 0))==NULL)
	{
		printf("Damon ==> load string failed ! \n");
		Szbh_DestroyFont(tTempFontHandle);
		return -4;
	}

	tXScrollText.mShowRect.x=pInputShowRect->x;
	tXScrollText.mShowRect.y=pInputShowRect->y;
	tXScrollText.mShowRect.width=pInputShowRect->width;	
	tXScrollText.mShowRect.height=pInputShowRect->height;

	if((tXScrollText.pBackSurface=Szbh_SurfaceCreate(pInputShowRect->width, pInputShowRect->height))==NULL)
	{
		printf("Damon ==> create back surface failed ! \n");
		Szbh_SurfaceDestroy(tXScrollText.pFontSurface);
		return -5;
	}

	if(pthread_create(&tXScrollText.mThreadHandle, NULL, XScrollTextProc, NULL)!=0)
	{
		printf("Damon ==> thread_create failed \n");
		Szbh_SurfaceDestroy(tXScrollText.pFontSurface);
		return -6;
	}

	tXScrollText.mFontHandle=tTempFontHandle;
	tXScrollText.mFontBackColor=tInputBackColor;

	return 0;
}


