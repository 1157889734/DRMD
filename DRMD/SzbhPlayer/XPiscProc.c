
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netinet/in.h>
#include <errno.h>
#include <unistd.h>
#include <strings.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <linux/un.h>
#include <time.h>
#include <sys/time.h>

#include "XSectionFile.h"
#include "XVehiclePlayerDefine.h"

#include "XProcessDrmd.h"
#include "XPiscProc.h"
#include "SzbhApi.h"



#define dXPiscGroupIp		"225.1.1.13"
#define dXPiscGroupPort	50152

extern int XCtrlProtocolGetLocalIp(char *pInputEthInf, char *pOutputIp);

static int tXPiscSockFd=-1;

struct sockaddr_in gXSendAddr;
static int gXDeviceId = 0;


int XPiscSocketInit()
{
	int tTempSockFd;
	struct sockaddr_in tTempAddr;
	struct ip_mreq tTempIpmr;

	char pTempLocalIp[20]={0};
	if(XCtrlProtocolGetLocalIp("eth0", pTempLocalIp)<0)
	{
		printf("Damon ==> [%s][%d] : Error get local ip failed \n", __FUNCTION__, __LINE__);
		return -1;
	}

	printf("Damon ==> local ip : %s \n", pTempLocalIp);
	
	tTempSockFd=socket(AF_INET, SOCK_DGRAM, 0);
	if(tTempSockFd<0)
	{
		
		close(tTempSockFd);
		printf("Damon ==> [%s][%d] : Error create socket failed \n", __FUNCTION__, __LINE__);
		return -2;
	}

	int tTempVal=1;
	if(setsockopt(tTempSockFd, SOL_SOCKET, SO_REUSEADDR, &tTempVal, sizeof(tTempVal))<0)
	{
		
		close(tTempSockFd);
		printf("Damon ==> [%s][%d] : Error setsocketopt failed \n", __FUNCTION__, __LINE__);		
		return -2;
	}


	memset((void *)&tTempIpmr, 0, sizeof(tTempIpmr));
	tTempIpmr.imr_multiaddr.s_addr=inet_addr(dXPiscGroupIp);
	tTempIpmr.imr_interface.s_addr=inet_addr(pTempLocalIp);//htonl(INADDR_ANY);
	if(setsockopt(tTempSockFd, IPPROTO_IP, IP_ADD_MEMBERSHIP, (char *)&tTempIpmr, sizeof(tTempIpmr))<0)
	{
		printf("Damon ==> [%s][%d] : Error setsockopt failed errno=%d \n", __FUNCTION__, __LINE__, errno);
		close(tTempSockFd);
		return -3;
	}

	memset((void *)&tTempAddr, 0, sizeof(tTempAddr));
	tTempAddr.sin_family=AF_INET;
	tTempAddr.sin_addr.s_addr=inet_addr(dXPiscGroupIp); //htonl(INADDR_ANY);
	tTempAddr.sin_port=htons(dXPiscGroupPort);
//	inet_pton(AF_INET, dXTimsGroupIp, &tTempAddr.sin_addr);

	if(bind(tTempSockFd, (struct sockaddr *)&tTempAddr, sizeof(tTempAddr))<0)
	{
		printf("Damon ==> [%s][%d] : Error sock bind failed \n", __FUNCTION__, __LINE__);
		close(tTempSockFd);
		return -4;
	}

	tXPiscSockFd=tTempSockFd;

	memset(&gXSendAddr, 0, sizeof(gXSendAddr));
	gXSendAddr.sin_family=AF_INET;    
	gXSendAddr.sin_addr.s_addr=inet_addr(dXPiscGroupIp);    
	gXSendAddr.sin_port=htons(dXPiscGroupPort); 

	return tXPiscSockFd;
}


void *XPiscProcHandle(void *pInputArg)
{
	while(tXPiscSockFd<0)
	{
		sleep(2);
		int tPiscSocket = -1;
		int tPiscSocketCnt = 0;
		if((tPiscSocket < 0)&&(tPiscSocketCnt <5))
		{
			tPiscSocket = XPiscSocketInit();
			tPiscSocketCnt++;
		}
		else
		{
			log_save(dXLogLevel_Error, "XPiscSocketInit failed  [%s][%d]\n", __FUNCTION__, __LINE__);
			exit(-1);
		}
	}

	struct sockaddr_in tTempSenderAddr;
	char pTempBuf[1024]={0};
	int tTempReadLen=0;
	socklen_t tTempAddrLen=sizeof(tTempSenderAddr);

	printf("Jamon ==>> pisc proc start read data .......\n");
	
	while(1)
	{
		tTempReadLen=recvfrom(tXPiscSockFd, pTempBuf, sizeof(pTempBuf), 0, (struct sockaddr *)&tTempSenderAddr, &tTempAddrLen);
		if(tTempReadLen>0)
		{
			XPiscDataParse((unsigned char *)pTempBuf, tTempReadLen);
		}else
		{
			usleep(1000*100);
		}
	}

	return NULL;
}


void *XPiscProcSendState(void *pInputArg)
{
	unsigned char pTempSendData[27]={0};

	sleep(2);
	while(1)
	{

		if(tXPiscSockFd>0)
		{
			pTempSendData[0] = 0x7E;
			pTempSendData[1] = 0xff;
			pTempSendData[2] = 0xff;
			pTempSendData[3] = 0xff;
			pTempSendData[4] = 0xff;
			pTempSendData[5] = 0xff;
			pTempSendData[6] = 0xff;
			pTempSendData[7] = 0xff;
			pTempSendData[8] = 0xff;

			pTempSendData[9] = (unsigned char)(((gXDeviceId+10) / 10) & 0xff);
			pTempSendData[10] = (unsigned char)((((gXDeviceId+10) / 10) >> 8) & 0xff);

			pTempSendData[11] = 0x07;
			pTempSendData[12] = (unsigned char)(gXDeviceId %10);

		#ifdef dXCustomer_XiAn5L
			pTempSendData[13] = gXDeviceId;
			pTempSendData[14] = 7;
			pTempSendData[15] = 16;
			pTempSendData[16] = 172;
		#endif

			pTempSendData[17] = 0x01;
			pTempSendData[18] = 0x00;

			pTempSendData[19] = 0x06;
			pTempSendData[20] = 0x00;

			char pTempString[12] = {0};
			int tTempLight = 0;
			if(0 == XSectionGetValue(dXSetupConfigFile, dXSetupDispSectionName, dXSetupBrightnessKeyName, pTempString, sizeof(pTempString)))
			{
				tTempLight = atoi(pTempString);
			}

			int tTempVerHight = 0, tTempVerLow = 0;
			sscanf(dXSoftwareVersion, "%d.%d", &tTempVerHight, &tTempVerLow);

			pTempSendData[21] = 0;
			pTempSendData[22] = tTempLight;
			pTempSendData[23] = 0;
			pTempSendData[24] = 0;
			pTempSendData[25] = tTempVerLow;
			pTempSendData[26] = tTempVerHight;

			if(sendto(tXPiscSockFd, pTempSendData, 27, 0, (struct sockaddr *)&gXSendAddr,sizeof(struct sockaddr)) < 0)
			{
				printf("Jamon ==>>> send data failed !!\n");
				usleep(1000*500);
			}

		}

		sleep(1);
	}

	return NULL;
}


int XPiscProcInit(int tInputDevId)
{
	gXDeviceId = tInputDevId;

	system("route add -net 225.1.1.13 netmask 255.255.255.255 eth0");
	usleep(1000*10);
	int tTemretXpics = -1;
	int tTemretXpicsCnt = 0;
	if((tTemretXpics < 0)&&(tTemretXpicsCnt <5))
	{		
		tTemretXpics = XPiscSocketInit();
		tTemretXpicsCnt++;
	}
	else
	{
		log_save(dXLogLevel_Error, "XPiscSocketInit failed [%s][%d]\n", __FUNCTION__, __LINE__);
		exit(-1);
	}

	pthread_t tTempHandle;
	pthread_create(&tTempHandle, NULL, XPiscProcHandle, NULL);

	usleep(1000*100);
	pthread_create(&tTempHandle, NULL, XPiscProcSendState, NULL);

	return 0;
}


