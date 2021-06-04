
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <dirent.h>
#include <string.h>
#include <pthread.h>
#include <sys/stat.h>
#include <sys/types.h>



#include "SzbhApi.h"
#include "XVehiclePlayerDefine.h"
#include "XConfigFile.h"
#include "XSectionFile.h"
#include "XLocalPlayer.h"


#define dXSdVideoDir		"/usrdata/sdcard0/"
//#define dXSdVideoDir		"/usrdata/video/"

#define dXVideoNumMax	50

static int tXLocalVideoCnt=0;
static char *pXLocalVideo[dXVideoNumMax];
static int tXLocalVideoPlayIdx=0;


static char *pXVideoTail[9]={".ts", ".avi", ".mp4", ".mkv", ".flv", ".3pg", ".mov", ".dat", ".vob"};
static int tXVideoTailType=9;


static unsigned int tXLocalPlayerHandle=0;
static int gXShowMirror = 0;

void __XLocalPlayerCallBack(XPlayerEventArg *pInputEvent)
{
	if(pInputEvent!=NULL)
	{
		if(pInputEvent->mEvent==PLAYER_EVENT_EOF || pInputEvent->mEvent==PLAYER_EVENT_ERROR)
		{
			printf("Jamon ==>> player callback eof or error !!!\n");

			if(tXLocalVideoCnt>1)
			{
				if(tXLocalVideoPlayIdx>=tXLocalVideoCnt)
					tXLocalVideoPlayIdx=0;			

				char pTempStr[128]={0};
				snprintf(pTempStr, sizeof(pTempStr)-1, "%s%s", dXSdVideoDir, pXLocalVideo[tXLocalVideoPlayIdx]);
				Szbh_PlayerStart(tXLocalPlayerHandle, pTempStr);
			}
		}
	}
}

int XLocalPlayerStartPlay(void)
{	
	if(tXLocalVideoCnt<=0)
		return -1;

	if(tXLocalVideoPlayIdx>=tXLocalVideoCnt)
		tXLocalVideoPlayIdx=0;

	XConfigSection *pTempSection = NULL;
	if(gXShowMirror == 0)
	{
		pTempSection = XConfigGetSection("video_right");
	}else
	{
		pTempSection = XConfigGetSection("video_left");
	}
	
	if(pTempSection!=NULL)
	{
		X2DRect tTempVideoRect;
		tTempVideoRect.x=pTempSection->mXPos;
		tTempVideoRect.y=pTempSection->mYPos;
		tTempVideoRect.width=pTempSection->mWidth;
		tTempVideoRect.height=pTempSection->mHeight;

		if(Szbh_PlayerCreate(&tTempVideoRect, &tXLocalPlayerHandle, __XLocalPlayerCallBack)!=0)
		{
			tXLocalPlayerHandle=0;
			return -2;
		}

		char pTempStr[128]={0};
		snprintf(pTempStr, sizeof(pTempStr)-1, "%s%s", dXSdVideoDir, pXLocalVideo[tXLocalVideoPlayIdx]);
		Szbh_PlayerStart(tXLocalPlayerHandle, pTempStr);

		if(tXLocalVideoCnt==1)
		{
			Szbh_PlayerSetLoop(tXLocalPlayerHandle, true);
		}

		Szbh_GpioWriteData(dXGpioNumMute, 0);	// close audio mute

		tXLocalVideoPlayIdx++;
		
		return 0;
	}

	return -3;
}

void *XLocalPlayerProc(void *pInputArg)
{
	int tTempBreak=0;

	while(!tTempBreak)
	{
		if(tXLocalVideoCnt>0)
		{
			printf("Jamon ==>> local video play file cnt = %d \n", tXLocalVideoCnt);
			if(XLocalPlayerStartPlay()==0)
			{
				tTempBreak=1;
			}else
			{
				sleep(1);
			}
		}else if(access(dXSdVideoDir, F_OK)==0)
		{
			struct dirent **pTempNameList;
			int tTempNum=scandir(dXSdVideoDir, &pTempNameList, NULL, NULL);
		//	printf("Jamon ==>> scandir num : %d \n", tTempNum);
			if(tTempNum>0)
			{
				int i=0, j=0;
				for(i=0; i<tTempNum; i++)
				{
					if(strcmp(pTempNameList[i]->d_name, ".")==0 || strcmp(pTempNameList[i]->d_name, "..")==0 || pTempNameList[i]->d_type==DT_DIR)
					{
						free(pTempNameList[i]);
						continue;
					}

					char *pTempTail=strrchr(pTempNameList[i]->d_name, '.');
					if(pTempTail!=NULL && strlen(pTempTail)>=3)
					{
					//	printf("Jamon ==>> file tail : %s \n", pTempTail);
					
						for(j=0; j<tXVideoTailType; j++)
						{
							if(strcasecmp(pXVideoTail[j], pTempTail)==0)
							{
								int tTempFileLen=strlen(pTempNameList[i]->d_name);
								if(tXLocalVideoCnt<dXVideoNumMax)
								{
									pXLocalVideo[tXLocalVideoCnt]=(char *)malloc(tTempFileLen+1);
									if(pXLocalVideo[tXLocalVideoCnt]!=NULL)
									{
										memset(pXLocalVideo[tXLocalVideoCnt], 0, tTempFileLen+1);
										memcpy(pXLocalVideo[tXLocalVideoCnt], pTempNameList[i]->d_name, tTempFileLen);
										printf("Jamon ==>> video file %d : %s \n", tXLocalVideoCnt, pXLocalVideo[tXLocalVideoCnt]);

										tXLocalVideoCnt++;
									}
								}
								break;
							}
						}
					}

					free(pTempNameList[i]);
				}
				free(pTempNameList);
			}

			if(tXLocalVideoCnt<=0)
			{
				sleep(2);
			}			
		}else
		{
			sleep(2);
		}
	}


	return NULL;
}

int XLocalPlayerSetMirror(unsigned char tInputMirror)
{
	int tTempRet=-1;

	if(tXLocalPlayerHandle>0)
	{
		if(gXShowMirror!=tInputMirror)
		{
			XConfigSection *pTempSection=NULL;
			if(tInputMirror==0)
			{
				pTempSection=XConfigGetSection("video_right");
			}else
			{
				pTempSection=XConfigGetSection("video_left");
			}

			if(pTempSection!=NULL)
			{
				X2DRect tTempVideoRect;
				tTempVideoRect.x=pTempSection->mXPos;
				tTempVideoRect.y=pTempSection->mYPos;
				tTempVideoRect.width=pTempSection->mWidth;
				tTempVideoRect.height=pTempSection->mHeight;
				
				if(Szbh_PlayerSetWinRect(tXLocalPlayerHandle, &tTempVideoRect)==0)
				{
					gXShowMirror=tInputMirror;
					tTempRet=0;
				}else
				{
					printf("Jamon ==>> set player win rect failed !!!\n");
				}
			}
		}
	}

	return tTempRet;
}

void XLocalPlayerInit(void)
{
	if(access("/usrdata/video", F_OK)!=0)
	{
		mkdir("/usrdata/video", 0777);
		system("chmod 777 /usrdata/video");
	}

	pthread_t tTempHandle;
	pthread_create(&tTempHandle, NULL, XLocalPlayerProc, NULL);
}

