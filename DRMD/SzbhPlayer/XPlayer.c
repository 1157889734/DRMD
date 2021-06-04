

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/time.h>
#include <time.h>
#include <errno.h>
#include <pthread.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <net/if.h>
#include <arpa/inet.h>
#include <sys/ioctl.h>

#include "SzbhApi.h"
#include "XVehiclePlayerDefine.h"
#include "XConfigFile.h"
#include "XSectionFile.h"
#include "XPlayer.h"
#include "XTsDemux.h"

#define dXUseNewTsBuf

extern char pXPlayerUrl[128];
extern u32 tXPlayerHandle;

extern int XGetTick(unsigned int *pOutputMs);
extern int XCheckScreenSave(void);

static unsigned int tXPlayerCheckPrevTick=0;


#ifdef dXUseNewTsBuf
#define dXTsBufferLenMax	(188*14)
static char *pXTsBuffer=NULL;
static int tXTsBufIndex=0;

static char pXRecvTsBuf[188*10+1]={0};
#endif


static pthread_t tXVideoCheckThread=0;
static volatile int tXVideoCheckFlag=0;


void XTsPlayerTickReset(unsigned int tInputCurTick)
{
	tXPlayerCheckPrevTick=tInputCurTick;
}

int XTsPlayerSocketInit(void)
{

#define dXVideoGroupIp	"225.1.1.40"
#define dXVideoGroupPort	4000

	int tTempSockFd;
	struct sockaddr_in tTempAddr;
	struct ip_mreq tTempIpmr;

	tTempSockFd=socket(AF_INET, SOCK_DGRAM, 0);
	if(tTempSockFd<0)
	{
		printf("Damon ==> [%s][%d] : Error create socket failed \n", __FUNCTION__, __LINE__);		
		close(tTempSockFd);
		return -1;
	}

	int tTempVal=1;
	if(setsockopt(tTempSockFd, SOL_SOCKET, SO_REUSEADDR, &tTempVal, sizeof(tTempVal))<0)
	{
		printf("Damon ==> [%s][%d] : Error setsocketopt failed \n", __FUNCTION__, __LINE__);		
		close(tTempSockFd);
		return -1;
	}


	memset((void *)&tTempIpmr, 0, sizeof(tTempIpmr));
	tTempIpmr.imr_multiaddr.s_addr=inet_addr(dXVideoGroupIp);
	tTempIpmr.imr_interface.s_addr=htonl(INADDR_ANY);
	if(setsockopt(tTempSockFd, IPPROTO_IP, IP_ADD_MEMBERSHIP, (char *)&tTempIpmr, sizeof(tTempIpmr))<0)
	{
		printf("Damon ==> [%s][%d] : Error setsockopt failed errno=%d \n", __FUNCTION__, __LINE__, errno);
		close(tTempSockFd);
		return -2;
	}

	int tTempOptVal = 1*1024*1024;
	socklen_t tTempOptLen = sizeof(tTempOptVal);
	if(setsockopt(tTempSockFd, SOL_SOCKET, SO_RCVBUF, (char *)&tTempOptVal, tTempOptLen)<0)
	{
		printf("Damon ==>  [%s][%d] : Error setsockopt failed !", __FUNCTION__, __LINE__);
	}

	memset((void *)&tTempAddr, 0, sizeof(tTempAddr));
	tTempAddr.sin_family=AF_INET;
	tTempAddr.sin_addr.s_addr=inet_addr(dXVideoGroupIp); //htonl(INADDR_ANY);
	tTempAddr.sin_port=htons(dXVideoGroupPort);
//	inet_pton(AF_INET, dXTimsGroupIp, &tTempAddr.sin_addr);

	if(bind(tTempSockFd, (struct sockaddr *)&tTempAddr, sizeof(tTempAddr))<0)
	{
		printf("Damon ==> [%s][%d] : Error sock bind failed \n", __FUNCTION__, __LINE__);
		close(tTempSockFd);
		return -3;
	}

	return tTempSockFd;
}

int XTsPlayerCheckVideoPlay(X2DRect tInputRect)
{
	static u8 *pTempCurData=NULL;
	static u8 *pTempPrevData=NULL;
	static int tTempPrevOutLen=0;
	static int tTempRgbDataLen=0;
	int tTempCurOutLen=0;
//	static u32 tTempPrevTick=0;
	u32 tTempCurTick;


	if(tInputRect.width<=0 || tInputRect.height<=0)
	{
		printf("Damon ==> input param valid ! \n");
		return -1;
	}


	if(XGetTick(&tTempCurTick)==0)
	{
		if(tTempCurTick>tXPlayerCheckPrevTick)
		{
			if(tTempCurTick-tXPlayerCheckPrevTick>=2*1000*60)
			{
				printf("Damon ==> system time have been adjuest = %u %u \n", tTempCurTick, tXPlayerCheckPrevTick);
				tXPlayerCheckPrevTick=tTempCurTick;
				return 0;
			}
		}else
		{
			tTempCurTick=tXPlayerCheckPrevTick;
			return 0;
		}
	
	
		if(pTempCurData==NULL)
		{
			tTempRgbDataLen=1920*1080*3+54+10;
			pTempCurData=(u8 *)malloc(tTempRgbDataLen);
			if(pTempCurData==NULL)
			{
				printf("Damon ==> not enought memory !\n");
				return -2;
			}
		}

		if(pTempPrevData==NULL)
		{
			pTempPrevData=(u8 *)malloc(tTempRgbDataLen);
			if(pTempPrevData==NULL)
			{
				printf("Damon ==> not enought memory !\n");
				return -3;
			}
		}


		if(Szbh_TsPlayerCapture(pTempCurData, tTempRgbDataLen, &tTempCurOutLen)==0)
		{
			printf("Damon ==> capture :  %d %d %d \n", tTempRgbDataLen, tTempPrevOutLen, tTempCurOutLen);

			if(tTempPrevOutLen==0)
			{
				memcpy(pTempPrevData, pTempCurData, tTempCurOutLen);
				tTempPrevOutLen=tTempCurOutLen;

			//	tTempPrevTick=tTempCurTick;
				if(tXPlayerCheckPrevTick<tTempCurTick)
					tXPlayerCheckPrevTick=tTempCurTick;
				return 0;
			}

			if(tTempCurOutLen==tTempPrevOutLen)
			{

				int i=0, tTempSameCount=0;
				for(i=0; i<tTempCurOutLen; i++)
				{
					if(pTempPrevData[i]==pTempCurData[i])
						tTempSameCount++;
				}

				if(tTempSameCount==tTempCurOutLen)
				{
				printf("Damon ==> play tick : %u %u \n", tTempCurTick, tXPlayerCheckPrevTick);
					if((tTempCurTick>tXPlayerCheckPrevTick) && ((tTempCurTick-tXPlayerCheckPrevTick)>1*60*1000))
					{
						printf("Damon ==> video frame not change !\n");
						return 1;
					}
				}else
				{
					memcpy(pTempPrevData, pTempCurData, tTempCurOutLen);
				//	tTempPrevTick=tTempCurTick;
					tXPlayerCheckPrevTick=tTempCurTick;
				}
			}else
			{
				memcpy(pTempPrevData, pTempCurData, tTempCurOutLen);
				tTempPrevOutLen=tTempCurOutLen;
				
			//	tTempPrevTick=tTempCurTick;
				tXPlayerCheckPrevTick=tTempCurTick;
			}
		}
	}

	return 0;
}


void *XTsPlayerCheckThread(void *pInputArg)
{
	while(1)
	{
		if(tXVideoCheckFlag)
		{
			X2DRect tTempVideoRect={0, 0, 0, 0};
			XConfigSection *pTempVideoSection=XConfigGetSection("video");
			if(pTempVideoSection)
			{
				tTempVideoRect.x=pTempVideoSection->mXPos;
				tTempVideoRect.y=pTempVideoSection->mYPos;
				tTempVideoRect.width=pTempVideoSection->mWidth;
				tTempVideoRect.height=pTempVideoSection->mHeight;

				if(XTsPlayerCheckVideoPlay(tTempVideoRect)==1)
				{
					printf("Damon ==> reboot system !\n");
					sync();sleep(1);
					system("reboot");
				}
			}

			tXVideoCheckFlag=0;
		}

		sleep(1);
	}

	return NULL;
}


void *XTsPlayerThread(void *pInputArg)
{

	(void)pInputArg;


	int tTempSockFd=XTsPlayerSocketInit();
	if(tTempSockFd<0)
	{
		printf("Damon ==> ts player start failed !\n");
		return NULL;
	}

	int tTempRet=0;
	struct sockaddr_in tTempSenderAddr;
	socklen_t tTempAddrLen=sizeof(tTempSenderAddr);
	char *pTempBuf=NULL;
	int tTempBufLen=0, tTempReadLen=0;
#ifndef dXUseNewTsBuf
	int tTempTsBufCount=0;
#endif

/*	int tTempCheck=0;
	X2DRect tTempVideoRect={0, 0, 0, 0};
	XConfigSection *pTempVideoSection=XConfigGetSection("video");
	if(pTempVideoSection)
	{
		tTempVideoRect.x=pTempVideoSection->mXPos;
		tTempVideoRect.y=pTempVideoSection->mYPos;
		tTempVideoRect.width=pTempVideoSection->mWidth;
		tTempVideoRect.height=pTempVideoSection->mHeight;

		tTempCheck=1;
	}*/
	

//int i=0;
	printf("Damon ==> start recv video ts buffer ...... \n");
	while(1)
	{
#ifdef dXUseNewTsBuf
		tTempReadLen = recvfrom(tTempSockFd, pXRecvTsBuf, 188*10, 0, (struct sockaddr *)&tTempSenderAddr, &tTempAddrLen);
		if(tTempReadLen>0)
		{
			if(tTempReadLen%188!=0)
			{
				printf("Damon ==> recv ts data is error : %d \n", tTempReadLen);
				continue;
			}

			// check screen save
			XCheckScreenSave();


		#if 1
			// add check video frame 
		//	if(tTempCheck==1)
			{
				static u32 tTempPrevTick=0;
				u32 tTempCurTick=0;
				if(XGetTick(&tTempCurTick)==0)
				{
			//	printf("Damon ==> recv ts data len : %d %d \n", tTempReadLen, tTempCurTick);
					if(tTempCurTick-tTempPrevTick>15*1000)
					{
					/*	if(XTsPlayerCheckVideoPlay(tTempVideoRect)==1)
						{
							printf("Damon ==> reboot system !\n");
							sync();sleep(1);
							system("reboot");
						}*/
						tXVideoCheckFlag=1;
					
						tTempPrevTick=tTempCurTick;
					}
				}
			}
		#endif
			

			if(pXTsBuffer!=NULL)
			{
				if(tXTsBufIndex+tTempReadLen<=dXTsBufferLenMax)
				{
					memcpy(pXTsBuffer+tXTsBufIndex, pXRecvTsBuf, tTempReadLen);
					tXTsBufIndex+=tTempReadLen;
				}else
				{
					int tTempCopyLen=dXTsBufferLenMax-tXTsBufIndex;
					memcpy(pXTsBuffer+tXTsBufIndex, pXRecvTsBuf, tTempCopyLen);
					tXTsBufIndex+=tTempCopyLen;
					tTempRet=Szbh_TsPlayerGetBuffer(&pTempBuf, &tTempBufLen);
					if(tTempRet==0)
					{
						if(tTempBufLen>=tXTsBufIndex)
						{
							memcpy(pTempBuf, pXTsBuffer, tXTsBufIndex);
							if(Szbh_TsPlayerPutBuffer(tXTsBufIndex)!=0)
								printf("Damon ==> Error put ts buffer failed !\n");
						}else
						{
							printf("Damon ==> ts buffer data len if failed !\n");
						}
					}else
					{
						if(tTempRet!=-3)	// switch program
						{
							printf("Damon ==> Error get ts buffer failed !\n");
							Szbh_TsPlayerResetBuffer();
						}
					}

					tXTsBufIndex=0;
					if((tTempReadLen-tTempCopyLen>0) && (tTempReadLen-tTempCopyLen<dXTsBufferLenMax))
					{
						memcpy(pXTsBuffer, pXRecvTsBuf+tTempCopyLen, tTempReadLen-tTempCopyLen);
						tXTsBufIndex+=(tTempReadLen-tTempCopyLen);
					}
				}

				if(tXTsBufIndex>=dXTsBufferLenMax)
				{
					tTempRet=Szbh_TsPlayerGetBuffer(&pTempBuf, &tTempBufLen);
					if(tTempRet==0)
					{
						if(tTempBufLen>=tXTsBufIndex)
						{
							memcpy(pTempBuf, pXTsBuffer, tXTsBufIndex);
							if(Szbh_TsPlayerPutBuffer(tXTsBufIndex)!=0)
								printf("Damon ==> Error put ts buffer failed !\n");
						}else
						{
							printf("Damon ==> ts buffer data len if failed !\n");
						}
					}else
					{
						if(tTempRet!=-3)	// switch program
						{
							printf("Damon ==> Error get ts buffer failed !\n");
							Szbh_TsPlayerResetBuffer();
						}
					}

					tXTsBufIndex=0;
				}
			}
		}else
		{
			printf("Damon ==> Error recv ts data failed !\n");
			usleep(1000*10);
		}
#else
		tTempRet=Szbh_TsPlayerGetBuffer(&pTempBuf, &tTempBufLen);
		if(tTempRet)
		{
			if(tTempTsBufCount++>10)
				printf("Damon ==> ts buf com too fast !\n");
			usleep(1000*10);
			continue;
		}


		tTempTsBufCount=0;
		tTempReadLen = recvfrom(tTempSockFd, pTempBuf, tTempBufLen, 0, (struct sockaddr *)&tTempSenderAddr, &tTempAddrLen);

#if 0	// test
if(tTempReadLen>0)
{printf("Damon ==> read len = %d \n", tTempReadLen);
	if(tTempReadLen>50*188)
		printf("Damon ==> read len = %d \n", tTempReadLen);

	if(tTempReadLen%188!=0)
		printf("Damon ==> ts data error len = %d \n", tTempReadLen);

	for(i=0; i<tTempReadLen; i+=188)
	{
		if(pTempBuf[i]!=0x47)
			printf("Damon ==> ts data header error !\n");
	}

	unsigned int tTempCnt=0, tTempErrCnt=0;
	if(Szbh_TsPlayerGetPacketNum(&tTempCnt, &tTempErrCnt)==0)
	{
		if(tTempErrCnt>0)
		{
			printf("Damon ==> ts packet cnt : %d %d \n", tTempCnt, tTempErrCnt);
			Szbh_TsPlayerResetBuffer();
		}
	}
}
#endif

		if(tTempReadLen>0)
		{
			// check screen save
			XCheckScreenSave();


			// add check video frame 
			if(tTempCheck==1)
			{
				static u32 tTempPrevTick=0;
				u32 tTempCurTick=0;
				if(XGetTick(&tTempCurTick)==0)
				{
					if(tTempCurTick-tTempPrevTick>15*1000)
					{
						if(XTsPlayerCheckVideoPlay(tTempVideoRect)==1)
						{
							printf("Damon ==> reboot system !\n");
							sync();sleep(1);
							system("reboot");
						}
						tTempPrevTick=tTempCurTick;
					}
				}
			}
		
			tTempRet=Szbh_TsPlayerPutBuffer(tTempReadLen);
			if(tTempRet)
				printf("Damon ==> ts put buf error !\n");
		}else
		{
			printf("Damon ==> no data revice !!!\n");
			usleep(1000*10);
		}
#endif
	}

	return NULL;
}

void *XTsPlayerProgInit(void *pInputArg)
{
	while(1)
	{
		if(Szbh_TsPlayerProgInit()==0)
		{
			printf("Damon ==> ts play prog init success !\n");

			// set player volume
			char pTempString[12]={0};
			if(XSectionGetValue(dXSetupConfigFile, dXSetupDispSectionName, dXSetupVolumeKeyName, pTempString, sizeof(pTempString)-1)==0)
			{
				int tTempVolume=atoi(pTempString);
				if(tTempVolume>=0 && tTempVolume<=100)
					Szbh_TsPlayerSetVolume(tTempVolume);
			}

			Szbh_GpioWriteData(/*6*8+3*/dXGpioNumMute, 0);
			
			break;
		}
		
		sleep(1);
	}

	return NULL;
}

int XPlayerSetVolume(int tInputVolume)
{
	int tTempRet=-1;

	if(strncmp(pXPlayerUrl, "udp://", 6)==0)
	{
	#ifdef dXUseEsPlayer
		tTempRet=Szbh_EsPlayerSetVolume(tInputVolume);
	#else
		tTempRet=Szbh_TsPlayerSetVolume(tInputVolume);
	#endif
	}else
	{
		if(tXPlayerHandle!=0)
			tTempRet=Szbh_PlayerSetVolume(tXPlayerHandle, tInputVolume);
	}

	if(tTempRet==0)
	{
		// save volume
		char pTempString[12]={0};
		sprintf(pTempString, "%d", tInputVolume);
		XSectionSaveValue(dXSetupConfigFile, dXSetupDispSectionName, dXSetupVolumeKeyName, pTempString);
	}

	return tTempRet;
}

int XPlayerLocalVideoPauseOrResume(bool tInputPauseOrResume)
{
	int tTempRet=0;

	if(strncmp(pXPlayerUrl, "udp://", 6))
	{
		if(tInputPauseOrResume)
			tTempRet=Szbh_PlayerPause(tXPlayerHandle);
		else
			tTempRet=Szbh_PlayerResume(tXPlayerHandle);
	}

	return tTempRet;
}


void XLocalPlayerCallBack(XPlayerEventArg *pInputEvent)
{
	static int tTempPlayerFlag=1;

	if(pInputEvent!=NULL)
	{
		if(pInputEvent->mEvent==PLAYER_EVENT_EOF)
		{
			printf("Damon ==> player callback event : %d \n", pInputEvent->mEvent);
			
			if(tTempPlayerFlag==0)
				Szbh_PlayerStart(tXPlayerHandle, "/usrdata/appdir/100001.mp4");
			else
				Szbh_PlayerStart(tXPlayerHandle, "/usrdata/appdir/100002.mp4");

			tTempPlayerFlag = (tTempPlayerFlag+1)%2;
		}
	}
}


void XPlayVideo(char *pInputVideo)
{
	XConfigSection *pTempSection=XConfigGetSection("video");
	if(pTempSection!=NULL)
	{
		X2DRect tTempVideoRect;
		tTempVideoRect.x=pTempSection->mXPos;
		tTempVideoRect.y=pTempSection->mYPos;
		tTempVideoRect.width=pTempSection->mWidth;
		tTempVideoRect.height=pTempSection->mHeight;

		strncpy(pXPlayerUrl, pInputVideo, sizeof(pXPlayerUrl)-1);
		if(strncmp(pXPlayerUrl, "udp://", 6)==0)
		{
		#ifdef dXUseEsPlayer
			if(Szbh_EsPlayerCreate(&tTempVideoRect)==0)
			{
			#ifdef dXUseFfmpegTsDemux
				extern int XTsFfmpegDemuxInit(void);
				XTsFfmpegDemuxInit();
			#else
				XTsDemuxInit();
			#endif
			}
		#else
			if(Szbh_TsPlayerCreate(&tTempVideoRect)==0)
			{
				pthread_t tTempHandle;
				pthread_create(&tTempHandle, NULL, XTsPlayerThread, NULL);
				pthread_detach(tTempHandle);

			#if 1
				pthread_t tTempPlayProgHandle;
				pthread_create(&tTempPlayProgHandle, NULL, XTsPlayerProgInit, NULL);
				pthread_detach(tTempPlayProgHandle);
			#else
				if(Szbh_TsPlayerProgInit()<0)
				{
					printf("Damon ==> [%s][%d] : set ts prog failed \n", __FUNCTION__, __LINE__);
				}
			#endif
			}
		#endif
		}else
		{
			if(Szbh_PlayerCreate(&tTempVideoRect, &tXPlayerHandle, NULL/*XLocalPlayerCallBack*/)!=0)
			{
				tXPlayerHandle=0;
				return;
			}

			Szbh_PlayerStart(tXPlayerHandle, pInputVideo);
			Szbh_PlayerSetLoop(tXPlayerHandle, true);

			// set player volume
			char pTempString[12]={0};
			if(XSectionGetValue(dXSetupConfigFile, dXSetupDispSectionName, dXSetupVolumeKeyName, pTempString, sizeof(pTempString)-1)==0)
			{
				int tTempVolume=atoi(pTempString);
				if(tTempVolume>=0 && tTempVolume<=100)
					XPlayerSetVolume(tTempVolume);
			}
			
			Szbh_GpioWriteData(/*6*8+3*/dXGpioNumMute, 0);	// close audio mute
		}
	}


#ifdef dXUseNewTsBuf
	if(pXTsBuffer==NULL)
	{
		pXTsBuffer=(char *)malloc(dXTsBufferLenMax+1);
		if(pXTsBuffer==NULL)
		{
			printf("Damon ==> Error malloc ts buffer failed !\n");
		}
	}
#endif

	if(tXVideoCheckThread==0)
	{
		pthread_create(&tXVideoCheckThread, NULL, XTsPlayerCheckThread, NULL);
		pthread_detach(tXVideoCheckThread);
	}

}


