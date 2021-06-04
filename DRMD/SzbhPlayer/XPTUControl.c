
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/select.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <net/if.h>
#include <arpa/inet.h>
#include <sys/ioctl.h>


#include "SzbhCommon.h"
#include "SzbhApi.h"
#include "XVehiclePlayerDefine.h"
#include "XSectionFile.h"
#include "XPTUControl.h"
#include "XProcessDrmd.h"


#define dXBroadcasePort	20000

//#define dXTcpServerIp	"10.10.30.136"
#define dXTcpServerPort	20001//9990
static char pXTcpServerIp[16]={0};

static int tXBroadcastSock=-1;

static volatile int tXTcpSock=-1;
static pthread_t tXTcpHandle;

static unsigned char *pXPTUTcpBuf=NULL;
static int tXPTUTcpBufIdx=0;

#define dXPTUMsgMax		10
typedef struct{
	XPTUCtrl *pMsg;
	int mHead;
	int mTail;
}XPTUMsgQueue;
static XPTUMsgQueue *pXPTUMsgQueue=NULL;
static pthread_mutex_t tXPTUMsgLock;


extern int tXBacklightHz;

extern char pXSystemImgVer[12];

extern void XSetScreenSaveOrCloseTimeout(int tInputScreenSaveOrClose, int tInputTimeout);
extern int XGetScreenSaveOrCloseTimeout(int tInputScreenSaveOrClose);
extern int XGetScreenSaveFlag();

extern void XSetColorMode(int tInputOpenOrClose, int tInputTick, int tInputColorIdx);
extern void XGetColorMode(int *pOutputOpenOrClose, int *pOutputTick, int *pOutputColorIdx);

static int XPTUCtrlMsgAdd(XPTUCtrl *pInputMsg)
{
	if(pInputMsg==NULL || pXPTUMsgQueue==NULL || pXPTUMsgQueue->pMsg==NULL)
		return -1;

	pthread_mutex_lock(&tXPTUMsgLock);
	if(((pXPTUMsgQueue->mTail+1)%dXPTUMsgMax)==pXPTUMsgQueue->mHead)
	{
		printf("Damon ==> [%s][%d] : msg queue is full \n", __FUNCTION__, __LINE__);
		pthread_mutex_unlock(&tXPTUMsgLock);
		return -2;
	}

	memcpy(pXPTUMsgQueue->pMsg+pXPTUMsgQueue->mTail, pInputMsg, sizeof(XPTUCtrl));
	pXPTUMsgQueue->mTail=(pXPTUMsgQueue->mTail+1)%dXPTUMsgMax;
	pthread_mutex_unlock(&tXPTUMsgLock);

	return 0;
}

static int XPTUCtrlMsgGet(XPTUCtrl *pOutputMsg)
{
	if(pOutputMsg==NULL || pXPTUMsgQueue==NULL || pXPTUMsgQueue->pMsg==NULL)
		return -1;

	pthread_mutex_lock(&tXPTUMsgLock);
	if(pXPTUMsgQueue->mHead==pXPTUMsgQueue->mTail)
	{
	//	printf("Damon ==> [%s][%d] : msg queue is empty \n", __FUNCTION__, __LINE__);
		pthread_mutex_unlock(&tXPTUMsgLock);
		return -2;
	}

	memcpy(pOutputMsg, pXPTUMsgQueue->pMsg+pXPTUMsgQueue->mHead, sizeof(XPTUCtrl));
	pXPTUMsgQueue->mHead=(pXPTUMsgQueue->mHead+1)%dXPTUMsgMax;
	pthread_mutex_unlock(&tXPTUMsgLock);

	return 0;
}

int XPTUCtrlSendData(int tInputFd, unsigned char *pInputData, int tInputDataLen)
{
	int tTempRet=0;
	int tTempWriteLen=0;
	int tTempWriteOk=0;
	int tTempCount=0;

	if(tInputFd<0)
		return -3;

	while(tTempWriteOk<tInputDataLen)
	{
		tTempWriteLen=write(tInputFd, pInputData+tTempWriteOk, tInputDataLen-tTempWriteOk);
		if(tTempWriteLen<0)
		{
			tTempRet=-1;
			break;
		}else if(tTempWriteLen>0)
		{
			tTempWriteOk += tTempWriteLen;
		}else
		{
			tTempCount++;
			if(tTempCount>=5)
			{
				tTempRet=-2;
				break;
			}
			usleep(5);
		}
	}

	return tTempRet;
}

static int XPTUGetMac(unsigned char *pOutputMac)
{
	if(tXTcpSock<0 || pOutputMac==NULL)
		return -1;

	struct ifreq tTempIfrMac;

	memset(&tTempIfrMac, 0, sizeof(tTempIfrMac));
	strncpy(tTempIfrMac.ifr_name, "eth0", sizeof(tTempIfrMac.ifr_name)-1);
	if(ioctl(tXTcpSock, SIOCGIFHWADDR, &tTempIfrMac)<0)
	{
		printf("Damon ==> get mac ioctl failed !\n");
		return -2;
	}

	pOutputMac[0]=(unsigned char)tTempIfrMac.ifr_hwaddr.sa_data[0];
	pOutputMac[1]=(unsigned char)tTempIfrMac.ifr_hwaddr.sa_data[1];
	pOutputMac[2]=(unsigned char)tTempIfrMac.ifr_hwaddr.sa_data[2];
	pOutputMac[3]=(unsigned char)tTempIfrMac.ifr_hwaddr.sa_data[3];
	pOutputMac[4]=(unsigned char)tTempIfrMac.ifr_hwaddr.sa_data[4];
	pOutputMac[5]=(unsigned char)tTempIfrMac.ifr_hwaddr.sa_data[5];

	return 0;
}

static int XPTUTcpClientInit(char *pInputServerIp, int tInputPort)
{
	if(pInputServerIp==NULL || tInputPort<2048)
		return -1;

	int tTempSock=0;
	tTempSock=socket(AF_INET, SOCK_STREAM, 0);
	if(tTempSock<0)
	{
		printf("Damon ==> [%s][%d] : Error create sock failed !\n", __FUNCTION__, __LINE__);
		close(tTempSock);
		return -2;
	}

	struct sockaddr_in tTempAddr;
	memset(&tTempAddr, 0, sizeof(tTempAddr));
	tTempAddr.sin_family=AF_INET;
	tTempAddr.sin_port=htons(tInputPort);
	tTempAddr.sin_addr.s_addr=inet_addr(pInputServerIp);

	if(connect(tTempSock, (struct sockaddr *)&tTempAddr, sizeof(tTempAddr))<0)
	{
		printf("Damon ==> [%s][%d] : Error connect failed !\n", __FUNCTION__, __LINE__);		
		close(tTempSock);
		return -3;
	}

	return tTempSock;
}

static int XPTUGetDataCrc(unsigned char *pInputBuf, int tInputLen)
{
	int i=0;
	unsigned char tTempCrc=0;
	unsigned char *pTempData=pInputBuf;

	for(i=1; i<tInputLen+1; i++)
	{
		if(i%2==0)
			tTempCrc += pTempData[i-1]*3;
		else
			tTempCrc += pTempData[i-1];
	}

	tTempCrc /= 11;
	return tTempCrc;
}


int XPTUControlMessageHandle(void)
{
	XPTUCtrl tTempMsg;
	unsigned char pTempData[128]={0};

	pTempData[0]=0x7E;
	
	if(XPTUCtrlMsgGet(&tTempMsg)==0)
	{
		if(tTempMsg.mCmd!=0x0100)
			printf("Damon ==> cmd : 0x%x \n", tTempMsg.mCmd);
	
		switch(tTempMsg.mCmd)
		{
			case 0x0100:	// device satte
				{
				//	printf("Damon ==> check device state !\n");
					XCommonPut16DateInBufferByLow(pTempData+1, 0x0100);
					XCommonPut32DateInBufferByLow(pTempData+3, 14);
					XCommonPut32DateInBufferByLow(pTempData+7,  1);
					if(XGetScreenSaveFlag()==1)
						pTempData[11]=0x03;
					else
						pTempData[11]=0x01;	// device state  0x01 ok, 0x02 fault, 0x03 sleep
					pTempData[12]=XPTUGetDataCrc(pTempData+1, 11);
					pTempData[13]=0xFE;
					XPTUCtrlSendData(tXTcpSock, pTempData, 14);
				}
				break;
			case 0x0200:	 // get emmc size
				{
					printf("Damon ==> get sdcard size \n");
					XCommonPut16DateInBufferByLow(pTempData+1, 0x0200);
				//	if(access("/dev/mmcblk0p15", F_OK)==0)		// 20 SDK
					if(access("/dev/mmcblk0p13", F_OK)==0)		// 50SDK
					{
						unsigned long long tTempTotalSize=0, tTempFreeSize=0;
						if(XCommonGetDeviceStorage("/usrdata", &tTempTotalSize, &tTempFreeSize)==0)
						{
							printf("Damon ==> sd size : %llu %llu \n", tTempTotalSize, tTempFreeSize);
							XCommonPut32DateInBufferByLow(pTempData+3, 0x1D);
							XCommonPut32DateInBufferByLow(pTempData+7, 0x10);
							XCommonPut64DateInBufferByLow(pTempData+11, tTempTotalSize);
							XCommonPut64DateInBufferByLow(pTempData+19, tTempFreeSize);
							pTempData[27]=XPTUGetDataCrc(pTempData+1, 26);
							pTempData[28]=0xFE;
							XPTUCtrlSendData(tXTcpSock, pTempData, 29);
						}
					}else
					{
						XCommonPut32DateInBufferByLow(pTempData+3, 0x0E);
						XCommonPut32DateInBufferByLow(pTempData+7, 0x01);
						pTempData[11]=0x01;
						pTempData[12]=XPTUGetDataCrc(pTempData+1, 11);
						pTempData[13]=0xFE;
						XPTUCtrlSendData(tXTcpSock, pTempData, 14);
					}
				}
				break;
			case 0x2300:	 // get sdcard size
				{
					printf("Damon ==> get sdcard size \n");
					XCommonPut16DateInBufferByLow(pTempData+1, 0x2300);
					if(access("/dev/mmcblk1", F_OK)==0)
					{
						unsigned long long tTempTotalSize=0, tTempFreeSize=0;
						if(XCommonGetDeviceStorage("/usrdata/sdcard0", &tTempTotalSize, &tTempFreeSize)==0)
						{
							printf("Damon ==> sd size : %llu %llu \n", tTempTotalSize, tTempFreeSize);
							XCommonPut32DateInBufferByLow(pTempData+3, 0x1D);
							XCommonPut32DateInBufferByLow(pTempData+7, 0x10);
							XCommonPut64DateInBufferByLow(pTempData+11, tTempTotalSize);
							XCommonPut64DateInBufferByLow(pTempData+19, tTempFreeSize);
							pTempData[27]=XPTUGetDataCrc(pTempData+1, 26);
							pTempData[28]=0xFE;
							XPTUCtrlSendData(tXTcpSock, pTempData, 29);
						}
					}else
					{
						XCommonPut32DateInBufferByLow(pTempData+3, 0x0E);
						XCommonPut32DateInBufferByLow(pTempData+7, 0x01);
						pTempData[11]=0x01;
						pTempData[12]=XPTUGetDataCrc(pTempData+1, 11);
						pTempData[13]=0xFE;
						XPTUCtrlSendData(tXTcpSock, pTempData, 14);
					}
				}
				break;
			case 0x0300:	 // application version
				{
					XCommonPut16DateInBufferByLow(pTempData+1, 0x0300);
					XCommonPut32DateInBufferByLow(pTempData+3, 0x14);
					XCommonPut32DateInBufferByLow(pTempData+7, 7);
					pTempData[11]=1;
					int tTempMajor=0, tTempMinor=0;
					sscanf(dXSoftwareVersion, "%d.%d", &tTempMajor, &tTempMinor);
					pTempData[12]=tTempMajor;
					pTempData[13]=tTempMinor;
					XCommonPut32DateInBufferByLow(pTempData+14, 0);
					pTempData[18]=XPTUGetDataCrc(pTempData+1,0x14-3);
					pTempData[19]=0xFE;
					XPTUCtrlSendData(tXTcpSock, pTempData, 0x14);
				}
				break;
			case 0x2200:
				{
					printf("\nDamon ==> get image version !!!\n\n");
					// system version
					XCommonPut16DateInBufferByLow(pTempData+1, 0x2200);
					XCommonPut32DateInBufferByLow(pTempData+3, 0x0f);
					XCommonPut32DateInBufferByLow(pTempData+7, 2);
					int tTempMajor=0, tTempMinor=0;
					sscanf(pXSystemImgVer, "%d.%d", &tTempMajor, &tTempMinor);
					pTempData[11]=tTempMajor;
					pTempData[12]=tTempMinor;
					pTempData[13]=XPTUGetDataCrc(pTempData+1,0x0f-3);
					pTempData[14]=0xFE;
					XPTUCtrlSendData(tXTcpSock, pTempData, 0x0f);
				}
				break;
			case 0x0400:	// reboot
				{
					printf("Damon ==> system reboot  !\n");
					sync();
					usleep(1000*1000);
					system("reboot");
				}
				break;
			case 0x0700:	// get brightness
				{
					int tTempBrightness=0;
					if(Szbh_DisplayGetBrightness(&tTempBrightness)==0)
					{
						XCommonPut16DateInBufferByLow(pTempData+1, 0x0700);
						XCommonPut32DateInBufferByLow(pTempData+3, 0x0E);
						XCommonPut32DateInBufferByLow(pTempData+7, 0x01);
						pTempData[11]=tTempBrightness;
						pTempData[12]=XPTUGetDataCrc(pTempData+1, 11);
						pTempData[13]=0xFE;
						XPTUCtrlSendData(tXTcpSock, pTempData, 14);
					}					
				}
				break;
			case 0x0800:	// set brightness
				{
					int tTempBrightness=tTempMsg.pData[0];
					printf("Damon ==> set brightness : %d \n", tTempBrightness);

					XCommonPut16DateInBufferByLow(pTempData+1, 0x0800);
					XCommonPut32DateInBufferByLow(pTempData+3, 0x0E);
					XCommonPut32DateInBufferByLow(pTempData+7, 0x01);
					if(Szbh_DisplaySetBrightness(tTempBrightness)==0)
						pTempData[11]=0x00;
					else
						pTempData[11]=0x01;
					pTempData[12]=XPTUGetDataCrc(pTempData+1, 11);
					pTempData[13]=0xFE;
					XPTUCtrlSendData(tXTcpSock, pTempData, 14);

					if(tTempBrightness>=0 && tTempBrightness<=100)
					{
						char pTempString[16]={0};
						snprintf(pTempString, sizeof(pTempString)-1, "%d", tTempBrightness);
						XSectionSaveValue(dXSetupConfigFile, dXSetupDispSectionName, dXSetupBrightnessKeyName, pTempString);
					}
				}
				break;
			case 0x0900:	// get contrast
				{
					int tTempContrast=0;
					if(Szbh_DisplayGetContrast(&tTempContrast)==0)
					{
						XCommonPut16DateInBufferByLow(pTempData+1, 0x0900);
						XCommonPut32DateInBufferByLow(pTempData+3, 0x0E);
						XCommonPut32DateInBufferByLow(pTempData+7, 0x01);
						pTempData[11]=tTempContrast;
						pTempData[12]=XPTUGetDataCrc(pTempData+1, 11);
						pTempData[13]=0xFE;
						XPTUCtrlSendData(tXTcpSock, pTempData, 14);
					}
				}
				break;
			case 0x0A00:	// set contrast
				{
					int tTempContrast=tTempMsg.pData[0];
					printf("Damon ==> set contrast : %d \n", tTempContrast);

					XCommonPut16DateInBufferByLow(pTempData+1, 0x0A00);
					XCommonPut32DateInBufferByLow(pTempData+3, 0x0E);
					XCommonPut32DateInBufferByLow(pTempData+7, 0x01);
					if(Szbh_DisplaySetContrast(tTempContrast)==0)
						pTempData[11]=0x00;
					else
						pTempData[11]=0x01;
					pTempData[12]=XPTUGetDataCrc(pTempData+1, 11);
					pTempData[13]=0xFE;
					XPTUCtrlSendData(tXTcpSock, pTempData, 14);

					if(tTempContrast>=0 && tTempContrast<=100)
					{
						char pTempString[16]={0};
						snprintf(pTempString, sizeof(pTempString)-1, "%d", tTempContrast);
						XSectionSaveValue(dXSetupConfigFile, dXSetupDispSectionName, dXSetupContrastKeyName, pTempString);
					}
				}
				break;
			case 0x0B00:	// set ip
				{
					unsigned char pTempIp[4]={0};
					char pTempString[64]={0};
					pTempIp[0]=tTempMsg.pData[0];
					pTempIp[1]=tTempMsg.pData[1];
					pTempIp[2]=tTempMsg.pData[2];
					pTempIp[3]=tTempMsg.pData[3];

				#if 0
					snprintf(pTempString, sizeof(pTempString)-1, "echo \"ifconfig eth0 %d.%d.%d.%d\" > /usrdata/ipconfig.sh", pTempIp[0], pTempIp[1], pTempIp[2], pTempIp[3]);
					int tTempRet=system(pTempString);
					usleep(1000*10);
				#else
					int tTempRet=0;
					snprintf(pTempString, sizeof(pTempString)-1, "%d.%d.%d.%d", pTempIp[0], pTempIp[1], pTempIp[2], pTempIp[3]);
					XSectionSaveValue(dXSetupConfigFile, dXSetupIpSectionName, dXSetupIpKeyName, pTempString);
					sync();
				#endif

					XCommonPut16DateInBufferByLow(pTempData+1, 0x0B00);
					XCommonPut32DateInBufferByLow(pTempData+3, 0x0E);
					XCommonPut32DateInBufferByLow(pTempData+7, 0x01);
					if(tTempRet==0)
						pTempData[11]=0x00;
					else
						pTempData[11]=0x01;
					pTempData[12]=XPTUGetDataCrc(pTempData+1, 11);
					pTempData[13]=0xFE;
					XPTUCtrlSendData(tXTcpSock, pTempData, 14);
				}
				break;
			case 0x0C00:	// get mac
				{
					unsigned char pTempMac[8]={0};
					if(XPTUGetMac(pTempMac)==0)
					{
						XCommonPut16DateInBufferByLow(pTempData+1, 0x0C00);
						XCommonPut32DateInBufferByLow(pTempData+3, 0x13);
						XCommonPut32DateInBufferByLow(pTempData+7, 0x06);
						pTempData[11]=pTempMac[0];
						pTempData[12]=pTempMac[1];
						pTempData[13]=pTempMac[2];
						pTempData[14]=pTempMac[3];
						pTempData[15]=pTempMac[4];
						pTempData[16]=pTempMac[5];
						pTempData[17]=XPTUGetDataCrc(pTempData+1, 16);
						pTempData[18]=0xFE;
						XPTUCtrlSendData(tXTcpSock, pTempData, 19);
					}
				}
				break;
			case 0x1000:	// format sdcard
				{
					int tTempRet=-1;
					if(access("/dev/mmcblk1", F_OK)==0)
					{
						char pTempString[64]={0};
						snprintf(pTempString, sizeof(pTempString)-1, "rm -rf /usrdata/sdcard0/*");
					//	printf("Damon ==> format sd : %s \n", pTempString);
						tTempRet=system(pTempString);
					}
					usleep(1000*10);
					sync();

				//	printf("Damon ==> format sd ret : %d \n", tTempRet);

					XCommonPut16DateInBufferByLow(pTempData+1, 0x1000);
					XCommonPut32DateInBufferByLow(pTempData+3, 0x0E);
					XCommonPut32DateInBufferByLow(pTempData+7, 0x01);
					if(tTempRet==0)
						pTempData[11]=0x00;
					else
						pTempData[11]=0x01;
					pTempData[12]=XPTUGetDataCrc(pTempData+1, 11);
					pTempData[13]=0xFE;
					XPTUCtrlSendData(tXTcpSock, pTempData, 14);
				}
				break;
			case 0x1300:	// get screen save timeout
				{
					unsigned short tTempScreenSaveTimeout=0;
					XCommonPut16DateInBufferByLow(pTempData+1, 0x1300);
					XCommonPut32DateInBufferByLow(pTempData+3, 0x0F);
					XCommonPut32DateInBufferByLow(pTempData+7, 0x02);
					tTempScreenSaveTimeout=XGetScreenSaveOrCloseTimeout(1);
					XCommonPut16DateInBufferByLow(pTempData+11, tTempScreenSaveTimeout);
					pTempData[13]=XPTUGetDataCrc(pTempData+1, 12);
					pTempData[14]=0xFE;
					XPTUCtrlSendData(tXTcpSock, pTempData, 15);
				}
				break;
			case 0x1400:	// set screen save timeout
				{
					unsigned short tTempScreenSaveTimeout=0;
					tTempScreenSaveTimeout=XCommonGet16DataFromBufferByHigh(tTempMsg.pData);
					printf("Damon ==> set screen save timeout : %d \n", tTempScreenSaveTimeout);

					XCommonPut16DateInBufferByLow(pTempData+1, 0x1400);
					XCommonPut32DateInBufferByLow(pTempData+3, 0x0E);
					XCommonPut32DateInBufferByLow(pTempData+7, 0x01);					

					char pTempString[16]={0};
					snprintf(pTempString, sizeof(pTempString)-1, "%d", tTempScreenSaveTimeout);
					if(XSectionSaveValue(dXSetupConfigFile, dXSetupDispSectionName, dXSetupScreenSaveTimeoutKeyName, pTempString)==0)
					{
						pTempData[11]=0;
						XSetScreenSaveOrCloseTimeout(1, tTempScreenSaveTimeout);
					}else
					{
						pTempData[11]=1;
					}
					pTempData[12]=XPTUGetDataCrc(pTempData+1, 11);
					pTempData[13]=0xFE;
					XPTUCtrlSendData(tXTcpSock, pTempData, 14);
				}
				break;
			case 0x1100:	// get screen close timeout
				{
					unsigned short tTempScreenSaveTimeout=0;
					XCommonPut16DateInBufferByLow(pTempData+1, 0x1100);
					XCommonPut32DateInBufferByLow(pTempData+3, 0x0F);
					XCommonPut32DateInBufferByLow(pTempData+7, 0x02);
					tTempScreenSaveTimeout=XGetScreenSaveOrCloseTimeout(0);
					XCommonPut16DateInBufferByLow(pTempData+11, tTempScreenSaveTimeout);
					pTempData[13]=XPTUGetDataCrc(pTempData+1, 12);
					pTempData[14]=0xFE;
					XPTUCtrlSendData(tXTcpSock, pTempData, 15);
				}
				break;
			case 0x1200:	// set screen close timeout
				{
					unsigned short tTempScreenSaveTimeout=0;
					tTempScreenSaveTimeout=XCommonGet16DataFromBufferByHigh(tTempMsg.pData);
					printf("Damon ==> set screen close timeout : %d \n", tTempScreenSaveTimeout);
			
					XCommonPut16DateInBufferByLow(pTempData+1, 0x1200);
					XCommonPut32DateInBufferByLow(pTempData+3, 0x0E);
					XCommonPut32DateInBufferByLow(pTempData+7, 0x01);					
			
					char pTempString[16]={0};
					snprintf(pTempString, sizeof(pTempString)-1, "%d", tTempScreenSaveTimeout);
					if(XSectionSaveValue(dXSetupConfigFile, dXSetupDispSectionName, dXSetupScreenCloseTimeoutKeyName, pTempString)==0)
					{
						pTempData[11]=0;
						XSetScreenSaveOrCloseTimeout(0, tTempScreenSaveTimeout);
					}else
					{
						pTempData[11]=1;
					}
					pTempData[12]=XPTUGetDataCrc(pTempData+1, 11);
					pTempData[13]=0xFE;
					XPTUCtrlSendData(tXTcpSock, pTempData, 14);
				}
				break;
			case 0x1700:	// get back light
				{
					int tTempBacklight=0;
					if(Szbh_DisplayGetBacklight(&tTempBacklight)==0)
					{
						XCommonPut16DateInBufferByLow(pTempData+1, 0x1700);
						XCommonPut32DateInBufferByLow(pTempData+3, 0x0E);
						XCommonPut32DateInBufferByLow(pTempData+7, 0x01);
						pTempData[11]=tTempBacklight;
						pTempData[12]=XPTUGetDataCrc(pTempData+1, 11);
						pTempData[13]=0xFE;
						XPTUCtrlSendData(tXTcpSock, pTempData, 14);
					}
				}
				break;
			case 0x1800:	// set back light
				{
					printf("Damon ==> set back light : %d  %d \n", tTempMsg.mDataLen, tTempMsg.pData[0]);
					int tTempBacklight=tTempMsg.pData[0];

					XCommonPut16DateInBufferByLow(pTempData+1, 0x1800);
					XCommonPut32DateInBufferByLow(pTempData+3, 0x0E);
					XCommonPut32DateInBufferByLow(pTempData+7, 0x01);
					if(Szbh_DisplaySetBacklight(tXBacklightHz, tTempBacklight)==0)
						pTempData[11]=0x00;
					else
						pTempData[11]=0x01;
					pTempData[12]=XPTUGetDataCrc(pTempData+1, 11);
					pTempData[13]=0xFE;
					XPTUCtrlSendData(tXTcpSock, pTempData, 14);

					if(tTempBacklight>=0 && tTempBacklight<=100)
					{
						char pTempString[16]={0};
						snprintf(pTempString, sizeof(pTempString)-1, "%d", tTempBacklight);
						XSectionSaveValue(dXSetupConfigFile, dXSetupDispSectionName, dXSetupBacklightKeyName, pTempString);
					}
				}
				break;
			case 0x2000:
				{
					int tTempOpenOrClose=tTempMsg.pData[0];
					int tTempTick=tTempMsg.pData[1];
					int tTempColor=tTempMsg.pData[2];
					XSetColorMode(tTempOpenOrClose, tTempTick, tTempColor);
					printf("Damon ==> test rgb color : %d %d %d \n", tTempOpenOrClose, tTempTick, tTempColor);

					XCommonPut16DateInBufferByLow(pTempData+1, 0x2000);
					XCommonPut32DateInBufferByLow(pTempData+3, 0x0E);
					XCommonPut32DateInBufferByLow(pTempData+7, 0x01);
					pTempData[11]=0x00;
					pTempData[12]=XPTUGetDataCrc(pTempData+1, 11);
					pTempData[13]=0xFE;
					XPTUCtrlSendData(tXTcpSock, pTempData, 14);					
				}
				break;
			case 0x2100:
				{
					 int tTempOpenOrClose=0, tTempTick=0, tTempColor=0;
					 XGetColorMode(&tTempOpenOrClose, &tTempTick, &tTempColor);

					 XCommonPut16DateInBufferByLow(pTempData+1, 0x2100);
					 XCommonPut32DateInBufferByLow(pTempData+3, 0x10);
					 XCommonPut32DateInBufferByLow(pTempData+7, 0x03);
					 pTempData[11]=tTempOpenOrClose;
					 pTempData[12]=tTempTick;
					 pTempData[13]=tTempColor;
					 pTempData[14]=XPTUGetDataCrc(pTempData+1, 13);
					 pTempData[15]=0xFE;
					 XPTUCtrlSendData(tXTcpSock, pTempData, 16);
				}
				break;
            case UPDATE_START:
	            {
	                printf("iptv ==> set UPDATE_START \n");
	                XCommonPut16DateInBufferByLow(pTempData + 1, UPDATE_START);
	                XCommonPut32DateInBufferByLow(pTempData + 3, 0x0E);
	                XCommonPut32DateInBufferByLow(pTempData + 7, 0x01);
	                pTempData[11] = 0x00;
	                pTempData[12] = XPTUGetDataCrc(pTempData + 1, 11);
	                pTempData[13] = 0xFE;
	                XPTUCtrlSendData(tXTcpSock, pTempData, 14);
	                setCmd(SET_UPDATE_ING);
	                sendDrmdPacket();
	            }
	            break;            
            case UPDATE_END:
	            {
	                printf("iptv ==> set UPDATE_START \n");
	                XCommonPut16DateInBufferByLow(pTempData + 1, UPDATE_END);
	                XCommonPut32DateInBufferByLow(pTempData + 3, 0x0E);
	                XCommonPut32DateInBufferByLow(pTempData + 7, 0x01);
	                pTempData[11] = 0x00;
	                pTempData[12] = XPTUGetDataCrc(pTempData + 1, 11);
	                pTempData[13] = 0xFE;
	                XPTUCtrlSendData(tXTcpSock, pTempData, 14);
	                setCmd(SET_UPDATE_OK);
	                sendDrmdPacket();
	                sync();
	            }
	            break;				
			default :
				break;
		}
	}

	return 0;
}


void *XPTUTcpProc(void *pInputArg)
{
	if(tXTcpSock>0)
	{
		return NULL;
	}

	tXTcpSock=XPTUTcpClientInit(/*dXTcpServerIp*/pXTcpServerIp, dXTcpServerPort);
	if(tXTcpSock<0)
	{
		tXTcpSock=-1;
		return NULL;
	}

printf("Damon ==> tcp start read data \n");
	char pTempBuf[128]={0};
	int tTempReadLen=0;
	int tTempPos=0;
	XPTUCtrl tTempPackData;
	int tTempDataIdx=0;

	fd_set tTempReadFds;
	struct timeval tTempTimeout;
	int tTempRet=0;

	
	while(1)
	{

		FD_ZERO(&tTempReadFds);
		FD_SET(tXTcpSock, &tTempReadFds);
		tTempTimeout.tv_sec=5;
		tTempTimeout.tv_usec=0;
		
		tTempRet=select(tXTcpSock+1, &tTempReadFds, NULL, NULL, &tTempTimeout);
//	printf("Damon ==> select ret = %d \n", tTempRet);
		if(tTempRet<=0)
		{
			printf("Damon ==> [%s][%d] : select time out ret = %d  \n", __FUNCTION__, __LINE__, tTempRet);
			close(tXTcpSock);
			tXTcpSock=-1;
			break;
		}

		if(!FD_ISSET(tXTcpSock, &tTempReadFds))
		{
			usleep(1000*100);
			continue;
		}
	
		tTempReadLen=read(tXTcpSock, pTempBuf, sizeof(pTempBuf));
//	printf("Damon ==> tcp read len : %d \n", tTempReadLen);
		if(tTempReadLen>0)
		{
			int i=0;
			for(i=0; i<tTempReadLen; i++)
			{
				switch(tTempPos)
				{
					case 0:
						{
							if(pTempBuf[i]==0x7E)
							{
								memset(&tTempPackData, 0, sizeof(tTempPackData));
								tTempPackData.mHeader=pTempBuf[i];
								tTempDataIdx=0;

								tXPTUTcpBufIdx=0;
								if(tXPTUTcpBufIdx<128)
									pXPTUTcpBuf[tXPTUTcpBufIdx++]=pTempBuf[i];
								
								tTempPos=1;
							}else
								tTempPos=0;
						}
						break;
					case 1:
						{
							if(tXPTUTcpBufIdx<128)
								pXPTUTcpBuf[tXPTUTcpBufIdx++]=pTempBuf[i];
							
							tTempPackData.mCmd = (unsigned short)((pTempBuf[i]<<8)&0xff00);
							tTempPos++;
						}
						break;
					case 2:
						{
							if(tXPTUTcpBufIdx<128)
								pXPTUTcpBuf[tXPTUTcpBufIdx++]=pTempBuf[i];

							tTempPackData.mCmd += (unsigned short)(pTempBuf[i]&0xff);
							tTempPos++;							
						}
						break;
					case 3:
						{
							if(tXPTUTcpBufIdx<128)
								pXPTUTcpBuf[tXPTUTcpBufIdx++]=pTempBuf[i];
							
							tTempPackData.mPackLen = (unsigned int)((pTempBuf[i]<<24)&0xff000000);
							tTempPos++;
						}
						break;
					case 4:
						{
							if(tXPTUTcpBufIdx<128)
								pXPTUTcpBuf[tXPTUTcpBufIdx++]=pTempBuf[i];
							
							tTempPackData.mPackLen += (unsigned int)((pTempBuf[i]<<16)&0xff0000);
							tTempPos++;
						}
						break;					
					case 5:
						{
							if(tXPTUTcpBufIdx<128)
								pXPTUTcpBuf[tXPTUTcpBufIdx++]=pTempBuf[i];
							
							tTempPackData.mPackLen += (unsigned int)((pTempBuf[i]<<8)&0xff00);
							tTempPos++;
						}
						break;
					case 6:
						{
							if(tXPTUTcpBufIdx<128)
								pXPTUTcpBuf[tXPTUTcpBufIdx++]=pTempBuf[i];
							
							tTempPackData.mPackLen += (unsigned int)(pTempBuf[i]&0xff);
							tTempPos++;
						}
						break;
					case 7:
						{
							if(tXPTUTcpBufIdx<128)
								pXPTUTcpBuf[tXPTUTcpBufIdx++]=pTempBuf[i];
							
							tTempPackData.mDataLen = (unsigned int)((pTempBuf[i]<<24)&0xff000000);
							tTempPos++;
						}
						break;
					case 8:
						{
							if(tXPTUTcpBufIdx<128)
								pXPTUTcpBuf[tXPTUTcpBufIdx++]=pTempBuf[i];
							
							tTempPackData.mDataLen += (unsigned int)((pTempBuf[i]<<16)&0xff0000);
							tTempPos++;
						}
						break;					
					case 9:
						{
							if(tXPTUTcpBufIdx<128)
								pXPTUTcpBuf[tXPTUTcpBufIdx++]=pTempBuf[i];
							
							tTempPackData.mDataLen += (unsigned int)((pTempBuf[i]<<8)&0xff00);
							tTempPos++;
						}
						break;
					case 10:
						{
							if(tXPTUTcpBufIdx<128)
								pXPTUTcpBuf[tXPTUTcpBufIdx++]=pTempBuf[i];
							
							tTempPackData.mDataLen += (unsigned int)(pTempBuf[i]&0xff);
							tTempPos++;
						}
						break;
					default:
						{
							if(tXPTUTcpBufIdx<128)
								pXPTUTcpBuf[tXPTUTcpBufIdx++]=pTempBuf[i];
							
							if(tTempDataIdx<tTempPackData.mDataLen)
							{
								if(tTempDataIdx<dXPTUDataLenMax)
									tTempPackData.pData[tTempDataIdx]=pTempBuf[i];
								tTempDataIdx++;
								tTempPos++;
							}else
							{
								if(tTempPos==tTempPackData.mPackLen-2)
								{
									tTempPackData.mCrc=pTempBuf[i];
									tTempPos++;
								}else if(tTempPos==tTempPackData.mPackLen-1)
								{
									tTempPackData.mTail=pTempBuf[i];
								/*	printf("Damon ==> recv ptu pack : header=0x%x, cmd==0x%x, packLen=0x%x, dataLen=0x%x, crc=0x%x, tail=0x%x\n", 
										tTempPackData.mHeader, tTempPackData.mCmd, tTempPackData.mPackLen, tTempPackData.mDataLen, tTempPackData.mCrc, tTempPackData.mTail);*/
									if(tXPTUTcpBufIdx<128)
									{
										unsigned char tTempCrc=0;
										tTempCrc=XPTUGetDataCrc(pXPTUTcpBuf+1, tTempPackData.mPackLen-3);
										if(tTempCrc==tTempPackData.mCrc)
										{
										//	printf("Damon ==> crc check ok !!!\n");
											if(tTempPackData.mCmd==0x9900)
											{
												char pTempKeepData[13]={0x7e, 0x99, 0x00, 0x00, 0x00, 0x00, 0x0d, 0x00, 0x00, 0x00, 0x00, 0x11, 0xfe};
												write(tXTcpSock, pTempKeepData, sizeof(pTempKeepData));
											}else
											{
												printf("Damon ==> add ptu msg cmd : 0x%x \n", tTempPackData.mCmd);
												XPTUCtrlMsgAdd(&tTempPackData);
											}
										}else
										{
											printf("Damon ==> Error : ptu data crc check failed : 0x%x 0x%x \n", tTempPackData.mCrc, tTempCrc);
										}
									}else
									{
										printf("Damon ==> Warning : ptu data out buffer len !\n");
									}
								
									tTempPos=0;	// one package end									
								}else
								{
									printf("Damon ==> ptu protocol data error !\n");
									tTempPos=0;
								}
							}
						}
						break;
				}
			}
		}else if(tTempReadLen==0)
		{
			sleep(1);
		}else
		{
			printf("Damon ==> [%s][%d] : tcp client close \n", __FUNCTION__, __LINE__);
			close(tXTcpSock);
			tXTcpSock=-1;
			break;
		}
	}

	return NULL;
}

void *XPTUBroadcastProc(void *pInputArg)
{
	int tTempReadLen=0;
	struct sockaddr_in tTempFromAddr;
	char pTempBuf[128]={0};
	socklen_t tTempAddrLen=sizeof(tTempFromAddr);

	while(1)
	{
		tTempReadLen=recvfrom(tXBroadcastSock, pTempBuf, sizeof(pTempBuf), 0, (struct sockaddr *)&tTempFromAddr, &tTempAddrLen);
	printf("Damon ==> recv data : %d \n", tTempReadLen);
		if(tTempReadLen>0)
		{
			printf("Damon ==> read len : %d 0x%x 0x%x \n", tTempReadLen, pTempBuf[0], pTempBuf[1]);
			if(tTempReadLen==6 && pTempBuf[0]==0x01 && pTempBuf[1]==0x02 && pTempBuf[2]==0x03
				&& pTempBuf[3]==0x04 && pTempBuf[4]==0x05 && pTempBuf[5]==0x06)
			{
				if(tXTcpSock<0)
				{
					memset(pXTcpServerIp, 0, sizeof(pXTcpServerIp));
					strncpy(pXTcpServerIp, inet_ntoa(tTempFromAddr.sin_addr), sizeof(pXTcpServerIp)-1);
					printf("Damon ==> server ip : %s \n", pXTcpServerIp);
					
					pthread_create(&tXTcpHandle, NULL, XPTUTcpProc, NULL);
				}
			}
		}else
		{
			usleep(100*100);
		}
	}

	return NULL;
}

int XPTUControlInit(void)
{

	pthread_mutex_init(&tXPTUMsgLock, NULL);
	if(pXPTUMsgQueue==NULL)
	{
		pXPTUMsgQueue=(XPTUMsgQueue *)malloc(sizeof(XPTUMsgQueue));
		pXPTUMsgQueue->pMsg=(XPTUCtrl*)malloc(sizeof(XPTUCtrl)*dXPTUMsgMax);
		pXPTUMsgQueue->mHead=0;
		pXPTUMsgQueue->mTail=0;
	}

	int tTempSock=-1;
	tTempSock=socket(AF_INET, SOCK_DGRAM, 0);
	if(tTempSock<0)
	{
		printf("Damon ==> [%s][%d] : Error create socket failed !\n", __FUNCTION__, __LINE__);
		close(tTempSock);		
		return -1;
	}


	int tTempOpt=1;
	if(setsockopt(tTempSock, SOL_SOCKET, SO_BROADCAST, (char *)&tTempOpt, sizeof(tTempOpt))<0)
	{
		printf("Damon ==> [%s][%d] : Error setsockopt failed !\n", __FUNCTION__, __LINE__);
		close(tTempSock);
		return -2;
	}

	struct sockaddr_in tTempAddr;
	bzero(&tTempAddr, sizeof(struct sockaddr_in));
	tTempAddr.sin_family=AF_INET;
	tTempAddr.sin_addr.s_addr=htonl(INADDR_ANY);
	tTempAddr.sin_port=htons(dXBroadcasePort);
	if(bind(tTempSock, (struct sockaddr *)&tTempAddr, sizeof(struct sockaddr_in))<0)
	{
		printf("Damon ==> [%s][%d] : Error bind failed !\n", __FUNCTION__, __LINE__);
		close(tTempSock);
		return -3;
	}

	pthread_t tTempHandle;
	if(pthread_create(&tTempHandle, NULL, XPTUBroadcastProc, NULL)!=0)
	{
		printf("Damon ==> [%s][%d] : Error thread create  failed  !\n", __FUNCTION__, __LINE__);
		close(tTempSock);
		return -4;
	}
	pthread_detach(tTempHandle);


	tXBroadcastSock=tTempSock;
	if(pXPTUTcpBuf==NULL)
		pXPTUTcpBuf=(unsigned char *)malloc(128);
	tXPTUTcpBufIdx=0;

	return 0;
}

