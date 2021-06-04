

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <net/if.h>
#include <arpa/inet.h>
#include <sys/ioctl.h>

#include <pthread.h>

#include "XVehiclePlayerDefine.h"
#include "XControlProtocol.h"

#if (defined dXCustomer_IptvDrmd || defined dXCustomer_XiAn5L)
#include "XProcessDrmd.h"
#endif

#define dXTimsGroupIp	"225.1.1.41"
#define dXTimsGroupPort	4001


static int tXSockFd=-1;

#define dXCtrlMsgMax	10
typedef struct{
	XMediaCtrl *pMsg;
	int mHead;
	int mTail;
}XCtrlMsgQueue;
static XCtrlMsgQueue *pXCtrlMsgQueue=NULL;
static pthread_mutex_t tXCtrlMsgLock;

static int XCtrlProtocolSocketInit(void);

int XCtrlProtocolMsgAdd(XMediaCtrl *pInputMsg)
{
	if(pInputMsg==NULL || pXCtrlMsgQueue==NULL || pXCtrlMsgQueue->pMsg==NULL)
		return -1;

	pthread_mutex_lock(&tXCtrlMsgLock);
	if((pXCtrlMsgQueue->mTail+1)%dXCtrlMsgMax==pXCtrlMsgQueue->mHead)
	{
		printf("Damon ==> [%s][%d] : msg queue is full \n", __FUNCTION__, __LINE__);
		pthread_mutex_unlock(&tXCtrlMsgLock);
		return -2;
	}

	memcpy(pXCtrlMsgQueue->pMsg+pXCtrlMsgQueue->mTail, pInputMsg, sizeof(XMediaCtrl));
	pXCtrlMsgQueue->mTail=(pXCtrlMsgQueue->mTail+1)%dXCtrlMsgMax;
	pthread_mutex_unlock(&tXCtrlMsgLock);

	return 0;
}

int XCtrlProtocolMsgGet(XMediaCtrl *pOutputMsg)
{
	if(pOutputMsg==NULL || pXCtrlMsgQueue==NULL || pXCtrlMsgQueue->pMsg==NULL)
		return -1;

	pthread_mutex_lock(&tXCtrlMsgLock);
	if(pXCtrlMsgQueue->mHead==pXCtrlMsgQueue->mTail)
	{
	//	printf("Damon ==> [%s][%d] : msg queue is empty \n", __FUNCTION__, __LINE__);
		pthread_mutex_unlock(&tXCtrlMsgLock);
		return -2;
	}

	memcpy(pOutputMsg, pXCtrlMsgQueue->pMsg+pXCtrlMsgQueue->mHead, sizeof(XMediaCtrl));
	pXCtrlMsgQueue->mHead=(pXCtrlMsgQueue->mHead+1)%dXCtrlMsgMax;
	pthread_mutex_unlock(&tXCtrlMsgLock);

	return 0;
}

static void *XCtrlProtocolRecvProc(void *pInputArg)
{
	(void)pInputArg;

	struct sockaddr_in tTempSenderAddr;
	char pTempBuf[1024]={0};
	int tTempRedLen=0;
	socklen_t tTempAddrLen=sizeof(tTempSenderAddr);

	while(tXSockFd<0)
	{
		usleep(1000*1000);
		XCtrlProtocolSocketInit();
	}

	int tTempDataPos=0;
	int tTempMsgIdx=0;
	XMediaCtrl tTempCtrlData;
	while(1)
	{
	//	printf("Damon ==> start read pkt ... \n");
		tTempRedLen=recvfrom(tXSockFd, pTempBuf, sizeof(pTempBuf), 0, (struct sockaddr *)&tTempSenderAddr, &tTempAddrLen);
	//	printf("Damon ==> tims red len=%d %d \n", tTempRedLen, tTempDataPos);
		if(tTempRedLen>0)
		{
		/*	printf(" recv : 0x%x 0x%x 0x%x 0x%x - 0x%x 0x%x 0x%x 0x%x \n", pTempBuf[0], pTempBuf[1], pTempBuf[2], pTempBuf[3],
				pTempBuf[13], pTempBuf[14], pTempBuf[15], pTempBuf[16]);*/
			//  ==== parse protocol data ====
			int i=0;
			for(i=0; i<tTempRedLen; i++)
			{
				switch(tTempDataPos)
				{
					case 0:
						{
							if(pTempBuf[i]==0x7E)
							{
								memset(&tTempCtrlData, 0, sizeof(XMediaCtrl));
								tTempDataPos=1;
								tTempCtrlData.mHead=pTempBuf[i];
							}
						}
						break;
					case 1:
						{
							if(pTempBuf[i]==0x80/*0x0E*/)
							{
								tTempDataPos=2;
								tTempCtrlData.mPackageLen=pTempBuf[i];
							}else
							{
								tTempDataPos=0;
							}
						}
						break;
					case 2:
						{
							tTempCtrlData.mTriggerFlag=pTempBuf[i];
							tTempDataPos++;
						}
						break;
					case 3:
						{
							tTempCtrlData.mStartStation=pTempBuf[i];
							tTempDataPos++;
						}
						break;
					case 4:
						{
							tTempCtrlData.mEndStation=pTempBuf[i];
							tTempDataPos++;
						}
						break;
					case 5:
						{
							tTempCtrlData.mCurStation=pTempBuf[i];
							tTempDataPos++;
						}
						break;
					case 6:
						{
							tTempCtrlData.mNextStation=pTempBuf[i];
							tTempDataPos++;
						}
						break;
					case 8:
						{
							tTempCtrlData.mEmergencyCode=pTempBuf[i];
							tTempDataPos++;
						}
						break;
					case 9:
						{
							tTempCtrlData.mOpenTriggerFLag=pTempBuf[i];
							tTempDataPos++;							
						}
						break;
					case 14:
						{
							tTempCtrlData.mVolume=pTempBuf[i];
							tTempDataPos++;
						}
						break;
					case 15:
						{
							tTempCtrlData.mTemplate=pTempBuf[i];
							tTempDataPos++;
						}
						break;
					case 16:
						{
							tTempCtrlData.mLineNum=pTempBuf[i];
							tTempDataPos++;
						}
						break;
					case 17:
						{
							tTempCtrlData.mYear=pTempBuf[i];
							tTempDataPos++;
						}
						break;
					case 18:
						{
							tTempCtrlData.mMonth=pTempBuf[i];
							tTempDataPos++;
						}
						break;
					case 19:
						{
							tTempCtrlData.mDay=pTempBuf[i];
							tTempDataPos++;
						}
						break;
					case 20:
						{
							tTempCtrlData.mHour=pTempBuf[i];
							tTempDataPos++;
						}
						break;
					case 21:
						{
							tTempCtrlData.mMinute=pTempBuf[i];
							tTempDataPos++;
						}
						break;
					case 22:
						{
							tTempCtrlData.mSeconds=pTempBuf[i];
							tTempDataPos++;
						}
						break;
					case 42:
						{
							tTempCtrlData.mScreenLight=pTempBuf[i];
							tTempDataPos++;							
						}
						break;
					case 127:
						{
							tTempCtrlData.mEmergencyType=pTempBuf[i];
							tTempDataPos++;
						}
						break;
					case 128:
						{
							tTempCtrlData.mEmergencyLen=(pTempBuf[i]&0x00ff);
							tTempDataPos++;
						}
						break;
					case 129:
						{
							tTempCtrlData.mEmergencyLen |= ((pTempBuf[i]<<8)&0xff00);
							tTempDataPos++;
					//	printf("Damon ==> emerge len : %d \n", tTempCtrlData.mEmergencyLen);
						}
						break;
					case 130:
						{
							if(pTempBuf[i]==0x0D)
							{
								tTempCtrlData.mTail=pTempBuf[i];
								if(tTempCtrlData.mEmergencyType==0 || tTempCtrlData.mEmergencyLen==0)
								{
									tTempDataPos=0;
									XCtrlProtocolMsgAdd(&tTempCtrlData);	// add no emergency msg
								//	printf("Damon ==> add no emergency msg \n");

								#if (defined dXCustomer_IptvDrmd || defined dXCustomer_XiAn5L)
									if(pTempBuf[0]==0x7E && pTempBuf[1]==0x80)
									{
										XStationMsgProc((unsigned char *)pTempBuf, tTempRedLen);
									}
								#endif
								}else
								{
									tTempDataPos++;
									tTempMsgIdx=0;
								}
							}else
							{
								tTempDataPos=0;
							}							
						}
						break;
					default:
						{
							if(tTempDataPos>131)
							{
								tTempDataPos=0;	// error
							}else if(tTempDataPos==131)
							{
						//	printf("Damon ==> idx : %d %d \n", tTempMsgIdx, tTempCtrlData.mEmergencyLen*2);
								if(tTempMsgIdx<tTempCtrlData.mEmergencyLen*2)
								{
									if(tTempMsgIdx/2<sizeof(tTempCtrlData.pMsgData))
									{
										if(tTempMsgIdx%2==0)
											tTempCtrlData.pMsgData[tTempMsgIdx/2]=(pTempBuf[i]&0x00ff);
										else
											tTempCtrlData.pMsgData[tTempMsgIdx/2] |= ((pTempBuf[i]<<8)&0xff00);
									}
									tTempMsgIdx++;

									if(tTempMsgIdx==tTempCtrlData.mEmergencyLen*2)
									{
										tTempDataPos=0;
										XCtrlProtocolMsgAdd(&tTempCtrlData);	// add emergency msg
									//	printf("Damon ==> add emergency msg \n");

									#ifdef dXCustomer_IptvDrmd
										if(pTempBuf[0]==0x7E && pTempBuf[1]==0x80)
										{
											XStationMsgProc((unsigned char *)pTempBuf, tTempRedLen);
										}
									#endif
									}
								}else
								{
									tTempDataPos=0;	// error msg data , reset pos
								}
							}else
							{
								tTempDataPos++;
							}
						}
						break;
				}
			}
			
		}else
		{
			usleep(1000*10);
		}
	}

	return NULL;
}

int XCtrlProtocolGetLocalIp(char *pInputEthInf, char *pOutputIp)
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

static int XCtrlProtocolSocketInit(void)
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
		printf("Damon ==> [%s][%d] : Error create socket failed \n", __FUNCTION__, __LINE__);
		return -2;
	}

	int tTempVal=1;
	if(setsockopt(tTempSockFd, SOL_SOCKET, SO_REUSEADDR, &tTempVal, sizeof(tTempVal))<0)
	{
		printf("Damon ==> [%s][%d] : Error setsocketopt failed \n", __FUNCTION__, __LINE__);
	}


	memset((void *)&tTempIpmr, 0, sizeof(tTempIpmr));
	tTempIpmr.imr_multiaddr.s_addr=inet_addr(dXTimsGroupIp);
	tTempIpmr.imr_interface.s_addr=inet_addr(pTempLocalIp);//htonl(INADDR_ANY);
	if(setsockopt(tTempSockFd, IPPROTO_IP, IP_ADD_MEMBERSHIP, (char *)&tTempIpmr, sizeof(tTempIpmr))<0)
	{
		printf("Damon ==> [%s][%d] : Error setsockopt failed errno=%d \n", __FUNCTION__, __LINE__, errno);
		close(tTempSockFd);
		return -3;
	}

	memset((void *)&tTempAddr, 0, sizeof(tTempAddr));
	tTempAddr.sin_family=AF_INET;
	tTempAddr.sin_addr.s_addr=inet_addr(dXTimsGroupIp); //htonl(INADDR_ANY);
	tTempAddr.sin_port=htons(dXTimsGroupPort);
//	inet_pton(AF_INET, dXTimsGroupIp, &tTempAddr.sin_addr);

	if(bind(tTempSockFd, (struct sockaddr *)&tTempAddr, sizeof(tTempAddr))<0)
	{
		printf("Damon ==> [%s][%d] : Error sock bind failed \n", __FUNCTION__, __LINE__);
		close(tTempSockFd);
		return -4;
	}

	tXSockFd=tTempSockFd;

	return tXSockFd;
}

int XCtrlProtocolInit(void)
{
	XCtrlProtocolSocketInit();

	pthread_mutex_init(&tXCtrlMsgLock, NULL);
	if(pXCtrlMsgQueue==NULL)
	{
		pXCtrlMsgQueue=(XCtrlMsgQueue *)malloc(sizeof(XCtrlMsgQueue));
		pXCtrlMsgQueue->pMsg=(XMediaCtrl *)malloc(sizeof(XMediaCtrl)*dXCtrlMsgMax);
		pXCtrlMsgQueue->mHead=0;
		pXCtrlMsgQueue->mTail=0;
	}

	pthread_t tTempHandle;
	int tTempRet=pthread_create(&tTempHandle, NULL, XCtrlProtocolRecvProc, NULL);
	if(tTempRet!=0)
	{
		printf("Damon ==> [%s][%d] : Error create thread failed \n", __FUNCTION__, __LINE__);
		close(tXSockFd);
		tXSockFd=-1;
		return -5;
	}


	return 0;
}


