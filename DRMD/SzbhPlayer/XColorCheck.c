

#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "SzbhApi.h"
#include "XVehiclePlayerDefine.h"

#include "XColorCheck.h"


XColorTest tXColorTest;

int tXCheckColorFlag=0;
static unsigned int pXCheckColor[7]={
	0xffff0000,
	0xff00ff00,
	0xff0000ff,
	0xff000000,
	0xff555555,
	0xffffffff,
	0x00000000
};
static int tXCheckColorIndex=0;


void XSetColorMode(int tInputOpenOrClose, int tInputTick, int tInputColorIdx)
{
	tXColorTest.mOpenOrClose=tInputOpenOrClose;
	tXCheckColorFlag=tInputOpenOrClose;
	tXColorTest.mTick=tInputTick;
	if(tXColorTest.mTick<1)
		tXColorTest.mTick=1;
	tXColorTest.mColor=tInputColorIdx;

	if(tInputOpenOrClose==0)
	{
		X2DRect tTempRect={0, 0, dXScreenWidth, dXScreenHeight};
		Szbh_LayerClear(dXLayerId_Top, tTempRect, 0);
		
		Szbh_LayerRender(dXLayerId_Top);		
	}
}

void XGetColorMode(int *pOutputOpenOrClose, int *pOutputTick, int *pOutputColorIdx)
{
	if(tXCheckColorFlag!=0)
		*pOutputOpenOrClose=1;
	else
		*pOutputOpenOrClose=0;
	
	*pOutputTick=tXColorTest.mTick;
	*pOutputColorIdx=tXColorTest.mColor;
}

int XEnterOrCloseColorMode(int tInputEnterOrClose)
{
	static XSurface *pTempSurface=NULL;

//	printf("\nDamon ==> enter color mode : %d \n\n", tInputEnterOrClose);

	if(tInputEnterOrClose)
	{

		if(pTempSurface==NULL)
		{
			pTempSurface=Szbh_SurfaceCreate(dXScreenWidth, dXScreenHeight);
		}
		
		if(pTempSurface!=NULL)
		{
			int i=0, j=0, k=0;
			int tTempStepWidth=dXScreenWidth/10;
			unsigned char tTempBitColor=255/10;
			unsigned int tTempColor=0;
			unsigned int *pTempData=(unsigned int *)(pTempSurface->pVirAddr);
			unsigned int *pTempDstData=NULL;
			for(i=0; i<dXScreenWidth; i+=tTempStepWidth)
			{
				tTempBitColor += (255/10);
				tTempColor=0xff000000+((tTempBitColor<<16)&0x00ff0000)+((tTempBitColor<<8)&0x0000ff00)+tTempBitColor;
				for(k=0; k<tTempStepWidth; k++)
				{
					for(j=0; j<dXScreenHeight; j++)
					{
						pTempDstData=pTempData+(j*dXScreenWidth+(i+k));
					
						*pTempDstData = tTempColor;
					}
				}
			}
		}

	
		if(tXCheckColorFlag==1)
		{
			tXCheckColorFlag=2;
			tXCheckColorIndex=0;
		/*	X2DRect tTempRect={0, 0, dXScreenWidth, dXScreenHeight};
			Szbh_LayerClear(dXLayerId_Top, tTempRect, 0);
		
			Szbh_LayerRender(dXLayerId_Top);*/
		}
		if(tXCheckColorFlag==2)
		{//printf("Damon ==> color idx : %d \n", tXColorTest.mColor);
			if(tXColorTest.mColor==0)
			{
				X2DRect tTempRect={0, 0, dXScreenWidth, dXScreenHeight};
				int tTempTotleCnt=sizeof(pXCheckColor)/sizeof(pXCheckColor[0]);
				if(tXCheckColorIndex<tTempTotleCnt)
				{
					if(tXCheckColorIndex<6)
					{
						Szbh_LayerClear(dXLayerId_Top, tTempRect, pXCheckColor[tXCheckColorIndex]);
						Szbh_LayerRender(dXLayerId_Top);
					}else
					{
						// =====
						if(pTempSurface!=NULL)
						{
							Szbh_LayerShowSurface(dXLayerId_Top, pTempSurface, 0, 0);
							Szbh_LayerRender(dXLayerId_Top);
						}
					}
					
					tXCheckColorIndex++;
					
					if(tXCheckColorIndex>=tTempTotleCnt)
						tXCheckColorIndex=0;
				}
			}else if(tXColorTest.mColor>=1 && tXColorTest.mColor<=7)
			{
				if(tXColorTest.mColor<7)
				{
					X2DRect tTempRect={0, 0, dXScreenWidth, dXScreenHeight};
					Szbh_LayerClear(dXLayerId_Top, tTempRect, pXCheckColor[tXColorTest.mColor-1]);
					Szbh_LayerRender(dXLayerId_Top);
				}else
				{
					if(pTempSurface!=NULL)
					{
						Szbh_LayerShowSurface(dXLayerId_Top, pTempSurface, 0, 0);
						Szbh_LayerRender(dXLayerId_Top);
					}
				}
			}
		}
	}else
	{
		tXCheckColorFlag=0;
	}

	return 0;
}

