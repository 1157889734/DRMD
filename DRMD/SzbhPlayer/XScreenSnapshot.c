

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "SzbhCommon.h"
#include "SzbhApi.h"

#include "XScreenSnapshot.h"

typedef struct tagBmpFileHeader
{
	u16 mType;
	u32 mSize;
	u16 mReserved1;
	u16 mReserved2;
	u32 mOffset;
}BmpFileHeader;

typedef struct tagBmpInfoHeader{
	u32 mSize;
	u32 mWidth;
	u32 mHeight;
	u16 mPlanes;
	u16 mBitCount;
	u32 mCompression;
	u32 mImgSize;
	u32 mBitXPelsPermeter;
	u32 mBitYPelsPermeter;
	u32 mBitClrUsed;
	u32 mBitClrImportant;
}BmpInfoHeader;

int XScreenSnapshotEncodeBmp(ELayerId tInputLayerId, X2DRect tInputRect, char *pInputFileName)
{
	XSurface *pTempSurface=XScreenSnapshot(tInputLayerId, tInputRect);
	if(pTempSurface==NULL || pInputFileName==NULL)
		return -1;

	int tTempFd=open(pInputFileName, O_RDWR | O_TRUNC | O_CREAT, 0777);
	if(tTempFd<0)
	{
		printf("Damon ==> file failed !\n");
		goto error1;
	}

	printf("Damon ==> save pic width=%d, height=%d \n", pTempSurface->mWidth, pTempSurface->mHeight);

	int tTempRgbSize=pTempSurface->mWidth*pTempSurface->mHeight*4;
	char pTempHeader[54]={0};
	int tTempOffset=0;

	pTempHeader[0]='B';
	pTempHeader[1]='M';
	tTempOffset += 2;

	// file size
	XCommonPut32DateInBufferByHigh((unsigned char *)pTempHeader+tTempOffset, sizeof(pTempHeader)+tTempRgbSize);
	tTempOffset += 4;

	// reserved 
	tTempOffset += 4;

	// header size
	XCommonPut32DateInBufferByHigh((unsigned char *)pTempHeader+tTempOffset, 54);
	tTempOffset += 4;

	// rgb size
	XCommonPut32DateInBufferByHigh((unsigned char *)pTempHeader+tTempOffset, 40);
	tTempOffset += 4;

	// pic width	
	XCommonPut32DateInBufferByHigh((unsigned char *)pTempHeader+tTempOffset, pTempSurface->mWidth);
	tTempOffset += 4;
	
	// pic width	
	XCommonPut32DateInBufferByHigh((unsigned char *)pTempHeader+tTempOffset, pTempSurface->mHeight);
	tTempOffset += 4;

	// planes
	XCommonPut16DateInBufferByHigh((unsigned char *)pTempHeader+tTempOffset, 1);
	tTempOffset += 2;

	// bit counts
	XCommonPut16DateInBufferByHigh((unsigned char *)pTempHeader+tTempOffset, 32);
	tTempOffset += 2;

	// bit compression
	XCommonPut32DateInBufferByHigh((unsigned char *)pTempHeader+tTempOffset, 0);
	tTempOffset += 4;

	// size image
	XCommonPut32DateInBufferByHigh((unsigned char *)pTempHeader+tTempOffset, tTempRgbSize);
	tTempOffset += 4;

/*	XCommonPut32DateInBufferByHigh((unsigned char *)pTempHeader+tTempOffset, 0x1ec2);
	tTempOffset += 4;

	XCommonPut32DateInBufferByHigh((unsigned char *)pTempHeader+tTempOffset, 0x1ec2);
	tTempOffset += 4;*/
	
	int tTempCount=0;
	if((tTempCount=write(tTempFd, pTempHeader, sizeof(pTempHeader)))!=sizeof(pTempHeader))
	{
		printf("Damon ==> write bmp info failed !\n");
		goto error2;
	}


	unsigned char *pTempDstBuf=(unsigned char *)malloc(tTempRgbSize);
	unsigned char *pTempSrcPt=(unsigned char *)pTempSurface->pVirAddr;
	unsigned char *pTempDstPt=pTempDstBuf;

	int i=0;
	for(i=pTempSurface->mHeight-1; i>=0; i--)
	{
		memcpy(pTempDstPt, pTempSrcPt+i*pTempSurface->mWidth*4, pTempSurface->mWidth*4);
		pTempDstPt += pTempSurface->mWidth*4;
 	}

	if(write(tTempFd, pTempDstBuf, tTempRgbSize)!=tTempRgbSize)
	{
		printf("Damon ==> write bmp data failed !\n");
		goto error3;
	}

	free(pTempDstBuf);

	Szbh_SurfaceDestroy(pTempSurface);
	pTempSurface=NULL;

	close(tTempFd);

	return 0;

error3:
	if(pTempDstBuf)
		free(pTempDstBuf);
error2:
	if(tTempFd>0)
		close(tTempFd);
error1:
	if(pTempSurface)
		Szbh_SurfaceDestroy(pTempSurface);
	pTempSurface=NULL;
	return -2;
}


XSurface *XScreenSnapshot(ELayerId tInputLayerId, X2DRect tInputRect)
{
	XSurface *pTempRet=Szbh_LayerGetSurface(tInputLayerId, &tInputRect);

	return pTempRet;
}

int XScreenSnapshotOutFile(EPicType tInputPicType, char *pInputFileName)
{
	return Szbh_DisplaySnapshot(tInputPicType, pInputFileName);
}

