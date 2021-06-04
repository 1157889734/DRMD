

#include <stdio.h>
#include <string.h>
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
#include "XVehiclePlayerDefine.h"
#include "XControlProtocol.h"
#include "XDeviceState.h"

#define dXDeviceGroupIp	"225.1.1.42"
#define dXDeviceGroupPort	4002

#define dXdeviceLocalPort	9879

static int tXDeviceSockFd=-1;
static struct sockaddr_in tXPeerAddr;

static int tXDeviceNumber=0;

static unsigned char pXDeviceIp[4]={0};


int XDeviceSendDataProtocolV2_2(void)
{
	if(tXDeviceSockFd<0)
		return -1;

	int tTempCardNum=0, tTempLocateNum=0;

	unsigned char pTempSendData[46]={0};
	memset(pTempSendData, 0, sizeof(pTempSendData));

	pTempSendData[0]=0x7E;
	memset(pTempSendData+1, 0xff, 8);

	tTempCardNum=pXDeviceIp[3]/10;
	tTempLocateNum=pXDeviceIp[3]%10;
	pTempSendData[9]=(unsigned char)(tTempCardNum & 0xff);
	pTempSendData[10]=(unsigned char)((tTempCardNum >> 8) & 0xff);
	pTempSendData[11]=0x07;
	pTempSendData[12]=(unsigned char)tTempLocateNum;
	pTempSendData[13]=pXDeviceIp[0];
	pTempSendData[14]=pXDeviceIp[1];
	pTempSendData[15]=pXDeviceIp[2];
	pTempSendData[16]=pXDeviceIp[3];

	pTempSendData[17]=0;
	pTempSendData[18]=0x7d;

	if(tTempCardNum>0 && tTempCardNum<=8)
		pTempSendData[21+tTempCardNum]=(1<<(tTempLocateNum-1));

	if(sendto(tXDeviceSockFd, pTempSendData, 45, 0, (struct sockaddr *) &tXPeerAddr, sizeof(struct sockaddr_in)) < 0)
	{
		printf("Damon ==> send device state failed !\n");
		return -2;
	}	

	return 0;
}

int XDeviceSendDataProtocolV2_0(void)
{
	if(tXDeviceSockFd<0)
		return -1;

	int tTempBrightness=0;
	unsigned char pTempSendData[12]={0};

	pTempSendData[0]=(unsigned char)tXDeviceNumber;
	pTempSendData[1]=1;
	if(Szbh_DisplayGetBrightness(&tTempBrightness)==0)
		pTempSendData[2]=tTempBrightness;
	else
		pTempSendData[2]=0;
	if(sendto(tXDeviceSockFd, pTempSendData, 8, 0, (struct sockaddr *) &tXPeerAddr, sizeof(struct sockaddr_in)) < 0)
	{
		printf("Damon ==> send device state failed !\n");
		return -2;
	}

	return 0;
}

	
void *XDeviceStateProc(void *pInputArg)
{
	while(1)
	{

		if(strcmp(dXDeviceStatusProtocol, "V2.1")==0)
		{
			if(XDeviceSendDataProtocolV2_0()<0)
			{
				sleep(1);
			}
		}else if(strcmp(dXDeviceStatusProtocol, "V2.2")==0)
		{
			if(XDeviceSendDataProtocolV2_2()<0)
			{
				sleep(1);
			}
		}

//	printf("Damon ==> send data \n");
		usleep(1000*1000);
	}

	return NULL;
}

int XDeviceStateInit(void)
{
	struct sockaddr_in tTempLocalAddr;
	int tTempSockFd;
	unsigned int tTempSockLen;

	tTempSockFd=socket(AF_INET, SOCK_DGRAM, 0);
	if(tTempSockFd<0)
	{
		printf("Damon ==> socket create failed !\n");		
		close(tTempSockFd);
		return -1;
	}

	tTempSockLen=sizeof(struct sockaddr_in);

	memset(&tXPeerAddr, 0, tTempSockLen);
	tXPeerAddr.sin_family=AF_INET;
	tXPeerAddr.sin_port=htons(dXDeviceGroupPort);
	if(inet_pton(AF_INET, dXDeviceGroupIp, &tXPeerAddr.sin_addr)<0)
	{
		printf("Damon ==> inet_pton failed !\n");
		close(tTempSockFd);
		return -2;
	}

	memset(&tTempLocalAddr, 0, tTempSockLen);
	tTempLocalAddr.sin_family=AF_INET;
	tTempLocalAddr.sin_port=htons(dXdeviceLocalPort);
	tTempLocalAddr.sin_addr.s_addr=INADDR_ANY;
	if(bind(tTempSockFd, (struct sockaddr *)&tTempLocalAddr, sizeof(struct sockaddr_in))<0)
	{
		printf("Damon ==> bind failed  !\n");
		close(tTempSockFd);
		return -3;
	}

	char pTempString[16]={0};
	if(XCtrlProtocolGetLocalIp("eth0", pTempString)==0)
	{
		int pTempAddr[4]={0};
		sscanf(pTempString, "%d.%d.%d.%d", (pTempAddr+0), (pTempAddr+1), (pTempAddr+2), (pTempAddr+3));
		pXDeviceIp[0]=(unsigned char)pTempAddr[0];
		pXDeviceIp[1]=(unsigned char)pTempAddr[1];
		pXDeviceIp[2]=(unsigned char)pTempAddr[2];
		pXDeviceIp[3]=(unsigned char)pTempAddr[3];
		
		char *pTempPt=strrchr(pTempString, '.');
		if(pTempPt!=NULL)
		{
			pTempPt++;
			while(pTempPt && *pTempPt)
			{
				tXDeviceNumber=tXDeviceNumber*10+(*pTempPt-'0');
				pTempPt++;
			}
		}
		printf("Damon ==> device number : %d \n", tXDeviceNumber);
	}


	pthread_t tTempHandle;
	if(pthread_create(&tTempHandle, NULL, XDeviceStateProc, NULL)!=0)
	{
		printf("Damon ==> thread create  failed  !\n");
		close(tTempSockFd);
		return -4;
	}
	pthread_detach(tTempHandle);

	tXDeviceSockFd=tTempSockFd;

	return 0;
}


