
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/wait.h>
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
#include <sys/stat.h>
#include <fcntl.h>
#include <signal.h>

#include "SzbhApi.h"
#include "XVehiclePlayerDefine.h"
#include "XSectionFile.h"
#include "XConfigFile.h"
#include "XMenuShow.h"
#include "XSetupMenu.h"
#include "XPlayer.h"
#include "XScrollText.h"
#include "XControlProtocol.h"
#include "XDeviceState.h"
#include "XPTUControl.h"
#include "XScreenSnapshot.h"

#include "XColorCheck.h"

#if (defined dXCustomer_IptvDrmd || defined dXCustomer_XiAn5L)
#include "XProcessDrmd.h"
#include "XPiscProc.h"
#include "XLocalPlayer.h"
#endif


#define dXUseWhatchDog


char pXPlayerUrl[128]={0};
u32 tXPlayerHandle=0;

XMediaCtrl *pXCurMediaData=NULL;

static u32 tXPrevTick=0;
static u32 tXUpdateTick=0;
//static u32 tXCurTimeSeconds=0;


//char *pXTestScrollText="星期一abcd 123, a;sd'。撒大家龙卡起我们萨其马们六的搜索离开地球五";
/*#ifdef dXCustomer_NanNing2L
char *pXTestScrollText="欢迎乘坐南宁地铁2号线！";
#else
char *pXTestScrollText="欢迎乘坐上海地铁18号线！";
#endif*/


volatile int tXEnterScreenSave=0;
static unsigned int tXScreenSavePrevTick=0;
static int tXScreenSaveTimeout=0, tXScreenCloseTimeout=0;
static pthread_mutex_t gXChangeTimeMutex;


static bool tXLocalFilePlay=false;

//#define dXHwcWdgPin		(4*8+4)
static unsigned int tXPrevWdgTick=0;
static unsigned int tXPrevRunTick=0;

int tXBacklightHz=200;

//#define dXUseAdjustBacklight
#ifdef dXUseAdjustBacklight
static unsigned int tXPrevBacklightTick=0;
#endif

char pXSystemImgVer[12]={0};

static int tXDateShowFresh=0;


extern XColorTest tXColorTest;
extern int tXCheckColorFlag;


int tXDeviceId=11;

int tXShowMirrorOpen=0;

int XGetTick(unsigned int *pOutputMs)
{
	struct timeval tTempTick;

	if(gettimeofday(&tTempTick, NULL)==0)
	{
		*pOutputMs=tTempTick.tv_sec*1000+tTempTick.tv_usec/1000;
		return 0;
	}

	return -1;
}


int XGetWeek(int year, int month, int day)
{
	int aWeek[12] = {0,3,2,5,0,3,5,1,4,6,2,4};
	int week = 0;
	int temp = 0;

	if(year < 1 || month < 1 || month > 12 || day < 1 || day > 31)
	{
		return 0;
	}
	
	year -= month < 3;
	temp = (year + year/4 - year/100 + year/400 + aWeek[month-1] + day) % 7;
	switch(temp)
	{
		case 0: week = 1;break;
		case 1: week = 2;break;
		case 2: week = 3;break;
		case 3: week = 4;break;
		case 4: week = 5;break;
		case 5: week = 6;break;
		case 6: week = 7;break;
		default: week = 0;break;
	}
	
	return week;
}


int XSetSystemTime(int tInputYear, u8 tInputMonth, u8 tInputDay, u8 tInputHour, u8 tInputMinute, u8 tInputSeconds)
{
	struct tm tTempTime;
	struct timeval tTempVal;

	tTempTime.tm_year=tInputYear-1900;
	tTempTime.tm_mon=tInputMonth-1;
	tTempTime.tm_mday=tInputDay;
	tTempTime.tm_hour=tInputHour;
	tTempTime.tm_min=tInputMinute;
	tTempTime.tm_sec=tInputSeconds;
	int tTempWeek=XGetWeek(tInputYear, tInputMonth, tInputDay);
	if(tTempWeek>=1 && tTempWeek<=7)
		tTempTime.tm_wday=tTempWeek-1;

	tTempVal.tv_sec=mktime(&tTempTime);
	tTempVal.tv_usec=0;

//printf("Damon ==> time : %ld \n", tTempVal.tv_sec);
	int tTempRet=settimeofday(&tTempVal, NULL);
//printf("Damon ==> ret : %d \n", tTempRet);
	return tTempRet;
}

struct tm *XGetSystemTime(void)
{
	time_t tTempRet;
	struct tm *tTempNowTime;

	tTempRet=time(&tTempRet);
	if(tTempRet==-1)
		return NULL;

	tTempNowTime=localtime(&tTempRet);

	return tTempNowTime;
}

void XChangeTimeLock()
{
	pthread_mutex_lock(&gXChangeTimeMutex);
}

void XChangeTimeUnlock()
{
	pthread_mutex_unlock(&gXChangeTimeMutex);
}


/*******************************
*tInputTimeout : seconds
*******************************/
void XSetScreenSaveOrCloseTimeout(int tInputScreenSaveOrClose, int tInputTimeout)
{
	if(tInputTimeout<0)
		tInputTimeout=0;

	if(tInputScreenSaveOrClose)
	{
		tXScreenSaveTimeout=tInputTimeout*1000;
	}else
	{
		tXScreenCloseTimeout=tInputTimeout*1000;
	}
}

int XGetScreenSaveOrCloseTimeout(int tInputScreenSaveOrClose)
{
	if(tInputScreenSaveOrClose)
		return (tXScreenSaveTimeout/1000);
	else
		return (tXScreenCloseTimeout/1000);

	return 0;
}

int XGetScreenSaveFlag()
{
	return tXEnterScreenSave;
}

int XChangeTemplate(int tInputTemplateIdx, bool tInputReload)
{
	int tTempRet=-2;

	if(pXCurMediaData->mTemplate==tInputTemplateIdx && tInputReload==false)
		return -1;
printf("Damon ==> template idx =%d \n", tInputTemplateIdx);
	if(XConfigLoadTemplate(tInputTemplateIdx)==0)
	{
		XScrollTextStop();
		if(XShowMainMenu(tInputTemplateIdx)==0)
		{		
			XConfigSection *pTempSection=XConfigGetSection("video");	// switch video rect
			if(pTempSection!=NULL)
			{
				X2DRect tTempVideoRect;
				tTempVideoRect.x=pTempSection->mXPos;
				tTempVideoRect.y=pTempSection->mYPos;
				tTempVideoRect.width=pTempSection->mWidth;
				tTempVideoRect.height=pTempSection->mHeight;

				if(strncmp(pXPlayerUrl, "udp://", 6)==0)
				{
				#ifdef dXUseEsPlayer
					Szbh_EsPlayerSetWinRect(&tTempVideoRect);
				#else
					Szbh_TsPlayerSetWinRect(&tTempVideoRect);
				#endif
				}else
				{
					Szbh_PlayerSetWinRect(tXPlayerHandle, &tTempVideoRect);
				}
			}

			pXCurMediaData->mTemplate=tInputTemplateIdx;

		//	XShowScrollText(dXScrollText);	// show scroll text

			tTempRet=0;
		}
	}

	return tTempRet;
}


int XScreenSaveEnterOrNot(bool tInputEnterOrNot)
{
printf("Damon ==> screen save enter : %d %d \n", tInputEnterOrNot, tXEnterScreenSave);
	if(tInputEnterOrNot==true)
	{
		if(tXEnterScreenSave==0)
		{
			tXEnterScreenSave=1;
		#if (defined dXCustomer_IptvDrmd || defined dXCustomer_XiAn5L)
			XScrollTextPause();
			setCmd(SET_NOSIGNAL);
			sendDrmdPacket();
		#else
			tXDateShowFresh=0;
			
			X2DRect tTempRect={0, 0, dXScreenWidth, dXScreenHeight};
			XScrollTextStop();
			Szbh_LayerClear(dXLayerId_Bottom, tTempRect, 0xff000000);

			char pTempString[64]={0};
			snprintf(pTempString, sizeof(pTempString)-1, "%s/screensave.png", dXResourcePath);
			XSurface *pTempSurface=Szbh_LoadPic(pTempString, dXPicType_Png);
			if(pTempSurface!=NULL)
			{
				printf("Damon ==> show screen save pic !\n");
				Szbh_LayerShowSurfaceWithRect(dXLayerId_Bottom, pTempSurface, NULL, &tTempRect);
				Szbh_SurfaceDestroy(pTempSurface);
				pTempSurface=NULL;

				Szbh_LayerRender(dXLayerId_Bottom);
				usleep(1000*100);
			}
		#endif
		}
	}else
	{
		if(tXEnterScreenSave)
		{
			tXEnterScreenSave=0;

		#if (defined dXCustomer_IptvDrmd || defined dXCustomer_XiAn5L)
//			setCmd(SET_HAVE_SIGNAL);
			sendDrmdPacket();
			XScrollTextResume();
		#else
			X2DRect tTempRect={0, 0, dXScreenWidth, dXScreenHeight};
			Szbh_LayerClear(dXLayerId_Bottom, tTempRect, 0xff000000);
		
			XChangeTemplate(pXCurMediaData->mTemplate, true);

			Szbh_LayerRender(dXLayerId_Bottom);
			usleep(1000*100);
		#endif
		}
	}

	return 0;
}

void XAdjustScreenSaveTime(unsigned int tInputSeconds)
{
	tXScreenSavePrevTick=tInputSeconds*1000;
}

int XCheckScreenSave(void)
{
#ifndef dXCustomer_XiAn5L
	if(tXLocalFilePlay==true)
		return 0;
#endif

	unsigned int tTempCurTick=0;
	if(XGetTick(&tTempCurTick)==0)
		tXScreenSavePrevTick=tTempCurTick;
	else 
		return 0;

	if(tXEnterScreenSave)
	{
	printf("Damon ==> check screen tick : %u %u \n", tXScreenSavePrevTick, tTempCurTick);
		XScreenSaveEnterOrNot(false);
	}

	return 0;
}


void XAdjustTime(void)
{
	static int tTempFirstAdjuestTime=0;

	static int tTempPrevYear=0, tTempPrevMon=0, tTempPrevDay=0;
	static int tTempPrevWeek=0;

	unsigned int tTempMs=0;
	if(XGetTick(&tTempMs)!=0)
		return ;

//printf("Damon ==> date refresh : %d \n", tXDateShowFresh);

	tXPrevTick=tTempMs;
//	tXCurTimeSeconds=pXCurMediaData->mHour*3600+pXCurMediaData->mMinute*60+pXCurMediaData->mSeconds;

	char pTempString[64]={0};
	if(tXEnterScreenSave==0)
	{
		snprintf(pTempString, sizeof(pTempString)-1, "%02d:%02d:%02d", pXCurMediaData->mHour, pXCurMediaData->mMinute, pXCurMediaData->mSeconds);
	//	XShowTime(pTempString);
		XShowTime2(pXCurMediaData->mHour, pXCurMediaData->mMinute, pXCurMediaData->mSeconds);
	}


	{

		struct tm *pTempNowTime=XGetSystemTime();
		if(pTempNowTime!=NULL)
		{
			if(tTempPrevYear!=pXCurMediaData->mYear || tTempPrevMon!=pXCurMediaData->mMonth ||	tTempPrevDay!=pXCurMediaData->mDay
				|| pTempNowTime->tm_hour!=pXCurMediaData->mHour || pTempNowTime->tm_min!=pXCurMediaData->mMinute || pTempNowTime->tm_sec!=pXCurMediaData->mSeconds)
			{
				// change system time 
				printf("Damon ==> change system time : %d %d %d \n", pXCurMediaData->mYear, pXCurMediaData->mMonth, pXCurMediaData->mDay);
				XSetSystemTime(pXCurMediaData->mYear+2000, pXCurMediaData->mMonth, pXCurMediaData->mDay, 
						pXCurMediaData->mHour, pXCurMediaData->mMinute, pXCurMediaData->mSeconds);
				/*	sprintf(pTempString, "date %02d%02d%02d%02d%04d.%02d > /dev/null", pXCurMediaData->mMonth, pXCurMediaData->mDay, 
						pXCurMediaData->mHour, pXCurMediaData->mMinute, pXCurMediaData->mYear+2000, pXCurMediaData->mSeconds);	// date MMddHHmmYYYY.SS
					system(pTempString);*/			
			}
		}
	}


	if((tXEnterScreenSave==0) && (tXDateShowFresh==0 || tTempPrevYear!=pXCurMediaData->mYear || tTempPrevMon!=pXCurMediaData->mMonth ||  tTempPrevDay!=pXCurMediaData->mDay))
	{
		snprintf(pTempString, sizeof(pTempString)-1, "%04d-%02d-%02d", pXCurMediaData->mYear+2000, pXCurMediaData->mMonth, pXCurMediaData->mDay);
		XShowSectionString("date", pTempString);
		tTempPrevYear=pXCurMediaData->mYear;
		tTempPrevMon=pXCurMediaData->mMonth;
		tTempPrevDay=pXCurMediaData->mDay;

	}


	if(tTempFirstAdjuestTime==0)
	{
		unsigned int tTempAdjustTime=0;
		if(XGetTick(&tTempAdjustTime)==0)
		{
			tXPrevTick=tTempAdjustTime;
			tXScreenSavePrevTick=tTempAdjustTime;
			XTsPlayerTickReset(tTempAdjustTime);

		printf("Damon ==> adjust time : %u %u \n", tTempAdjustTime, tTempMs);

			tTempFirstAdjuestTime=1;
		}
	}
	
	usleep(1000*1);

#if 0
	struct tm *pTempNowTime=XGetSystemTime();
	if(pTempNowTime!=NULL && tTempPrevWeek!=pTempNowTime->tm_wday)
	{
		XShowWeek(pTempNowTime->tm_wday);
		tTempPrevWeek=pTempNowTime->tm_wday;
	}
#else
	int tTempWeek=XGetWeek(pXCurMediaData->mYear+2000, pXCurMediaData->mMonth, pXCurMediaData->mDay);
	if(tTempWeek>0 && (tTempPrevWeek!=tTempWeek || tXDateShowFresh==0) && (tXEnterScreenSave==0))
	{
		XShowWeek(tTempWeek-1);
		tTempPrevWeek=tTempWeek;

		static int tTempCount=0;
		if(tXDateShowFresh==0 && tTempCount++>3)
		{
			tXDateShowFresh=1;
			tTempCount=0;
		}
	}
#endif

	Szbh_LayerRender(dXLayerId_Bottom);

}

void XHwcWhatchdog()
{
#ifdef dXHwcWdgPin
	static int tTempFlag=0;

	Szbh_GpioWriteData(dXHwcWdgPin, tTempFlag);

	tTempFlag=(tTempFlag+1)%2;
#endif
}

void XRunLedOnOff(void)
{
	static int tTempLedStatus=0;

	Szbh_GpioWriteData(dXGpioNumRunLed, tTempLedStatus);

	tTempLedStatus = (tTempLedStatus+1)%2;
}

void XProcessCtrlMsg(XMediaCtrl *pInputMsg)
{
	if(pInputMsg==NULL)
		return ;

	// exit screen save
	XCheckScreenSave();


/*	printf("Damon ==> %d %d - %d %d - %d %d - %d %d \n", pXCurMediaData->mTriggerFlag, pInputMsg->mTriggerFlag,
			pXCurMediaData->mCurStation, pInputMsg->mCurStation, pXCurMediaData->mNextStation, pInputMsg->mNextStation,
			pXCurMediaData->mEndStation, pInputMsg->mEndStation);*/

	// change template
	if(pXCurMediaData->mTemplate!=pInputMsg->mTemplate)
	{
		XChangeTemplate(pInputMsg->mTemplate, false);
	//	pXCurMediaData->mTemplate=pInputMsg->mTemplate;
		pXCurMediaData->mTriggerFlag=0;

		tXDateShowFresh=0;
	}

	// station trigger
	if((pInputMsg->mOpenTriggerFLag&0xC0)!=(pXCurMediaData->mOpenTriggerFLag&0xC0))
	{
		if(tXEnterScreenSave<=0)
		{
			XShowStation(pXCurMediaData->mTemplate, 0x01, pInputMsg->mCurStation, pInputMsg->mNextStation, pInputMsg->mEndStation);
			pXCurMediaData->mOpenTriggerFLag=pInputMsg->mOpenTriggerFLag;
		}
	}
	
	static int tTempVolChangeFlag=0;
	if((pInputMsg->mTriggerFlag & 0x02) || (pInputMsg->mTriggerFlag & 0x01))
	{
		if(pXCurMediaData->mTriggerFlag!=pInputMsg->mTriggerFlag 
			|| pXCurMediaData->mCurStation!=pInputMsg->mCurStation
			|| pXCurMediaData->mNextStation!=pInputMsg->mNextStation
			|| pXCurMediaData->mEndStation!=pInputMsg->mEndStation)
		{
			if(tXEnterScreenSave<=0)
			{
				if(XShowStation(pXCurMediaData->mTemplate, pInputMsg->mTriggerFlag, pInputMsg->mCurStation, pInputMsg->mNextStation, pInputMsg->mEndStation)==0)
				{
				//	pXCurMediaData->mTriggerFlag=pInputMsg->mTriggerFlag;
					pXCurMediaData->mCurStation=pInputMsg->mCurStation;
					pXCurMediaData->mNextStation=pInputMsg->mNextStation;
					pXCurMediaData->mEndStation=pInputMsg->mEndStation;				
				}
			}
		}

		if(tTempVolChangeFlag==0)
		{
			XPlayerSetVolume(0);
			tTempVolChangeFlag=1;
		}
	}else
	{
		if(tTempVolChangeFlag==1)
		{
			XPlayerSetVolume(pXCurMediaData->mVolume);
			tTempVolChangeFlag=0;
		}
	}

	//emergency trigger
/*printf("Damon ==> trigger falg : %d %d - %d %d \n", pXCurMediaData->mTriggerFlag, pInputMsg->mTriggerFlag,
	pInputMsg->mEmergencyCode, pXCurMediaData->mEmergencyCode);*/
	if(pInputMsg->mTriggerFlag & 0x08)	 
	{
		if((pXCurMediaData->mTriggerFlag & 0x08)==0 || (pInputMsg->mEmergencyCode!=pXCurMediaData->mEmergencyCode))
		{
			XShowEmergencyCode(pInputMsg->mEmergencyCode);
			Szbh_LayerRender(dXLayerId_Top);
			pXCurMediaData->mEmergencyCode=pInputMsg->mEmergencyCode;

			XPlayerSetVolume(0);
		}
	}else {
		if(pXCurMediaData->mTriggerFlag & 0x08)
		{
			X2DRect tTempRect={0, 0, dXFramebufWidth, dXFramebufHeight};
			Szbh_LayerClear(dXLayerId_Top, tTempRect, 0);
			Szbh_LayerRender(dXLayerId_Top);

			XPlayerSetVolume(pXCurMediaData->mVolume);
		}
		pXCurMediaData->mEmergencyCode=0;
	}
	pXCurMediaData->mTriggerFlag=pInputMsg->mTriggerFlag;
	

	// set player volume
	if(pXCurMediaData->mEmergencyCode<=0)
	{
		if(pXCurMediaData->mVolume!=pInputMsg->mVolume)
		{
		printf("Damon ==> set volume : %d %d \n", pInputMsg->mVolume, pInputMsg->mVolume);
			if(pInputMsg->mVolume>=0 && pInputMsg->mVolume<=100)
			{
			/*	if(strncmp(pXPlayerUrl, "udp://", 6)==0)
				{
					Szbh_TsPlayerSetVolume(pInputMsg->mVolume);
				}else
				{
					Szbh_PlayerSetVolume(tXPlayerHandle, pInputMsg->mVolume);
				}*/

				if((pInputMsg->mTriggerFlag & 0x03)==0)
					XPlayerSetVolume(pInputMsg->mVolume);
				
				pXCurMediaData->mVolume=pInputMsg->mVolume;
			}
		}
	}

#ifndef dXUseNtpAdjustTime
	// adjust time 
	{
		if(pXCurMediaData->mYear!=pInputMsg->mYear || pXCurMediaData->mMonth!=pInputMsg->mMonth
			|| pXCurMediaData->mDay!=pInputMsg->mDay || pXCurMediaData->mHour!=pInputMsg->mHour
			|| pXCurMediaData->mMinute!=pInputMsg->mMinute || pXCurMediaData->mSeconds!=pInputMsg->mSeconds)
		{
			pXCurMediaData->mYear=pInputMsg->mYear;
			pXCurMediaData->mMonth=pInputMsg->mMonth;
			pXCurMediaData->mDay=pInputMsg->mDay;
			pXCurMediaData->mHour=pInputMsg->mHour;
			pXCurMediaData->mMinute=pInputMsg->mMinute;
			pXCurMediaData->mSeconds=pInputMsg->mSeconds;
			XAdjustTime();
		}
	}
#endif


	// change emergency message
	if(pInputMsg->mEmergencyType!=0)
	{
		if(pInputMsg->mEmergencyType==0x01)	// message
		{
			if(pXCurMediaData->mEmergencyType==0x02)
			{
				XScrollTextStop();
				pXCurMediaData->mEmergencyLen=0;
				usleep(1000*10);
			}
		
			if(pInputMsg->mEmergencyLen>0)
			{
				if(!(pXCurMediaData->mEmergencyLen==pInputMsg->mEmergencyLen
					&& memcmp(pXCurMediaData->pMsgData, pInputMsg->pMsgData, (pInputMsg->mEmergencyLen-1)*2)==0))
				{
					XShowEmergencyMsg(pInputMsg->pMsgData, pInputMsg->mEmergencyLen);
					Szbh_LayerRender(dXLayerId_Top);

					memcpy(pXCurMediaData->pMsgData, pInputMsg->pMsgData, (pInputMsg->mEmergencyLen-1)*2);
					pXCurMediaData->mEmergencyLen=pInputMsg->mEmergencyLen;
				}
			}
		}else if(pInputMsg->mEmergencyType==0x02)	// scroll text
		{

			if(pXCurMediaData->mEmergencyType==0x01)
			{
				X2DRect tTempRect={0, 0, dXScreenWidth, dXScreenHeight};
				Szbh_LayerClear(dXLayerId_Top, tTempRect, 0);
				Szbh_LayerRender(dXLayerId_Top);
				pXCurMediaData->mEmergencyLen=0;
				usleep(1000*10);
			}
		
			if(pInputMsg->mEmergencyLen>1)
			{
				if(!(pXCurMediaData->mEmergencyLen==pInputMsg->mEmergencyLen
					&& memcmp(pXCurMediaData->pMsgData, pInputMsg->pMsgData, (pInputMsg->mEmergencyLen-1)*2)==0))
				{
				printf("Damon ==> emergen unicode len=%d \n", pInputMsg->mEmergencyLen);
					if(pInputMsg->mEmergencyLen<256)
					{
						int tTempSwitchLen=pInputMsg->mEmergencyLen*6+1;
						unsigned char *pTempString=(unsigned char *)malloc(tTempSwitchLen);
						if(pTempString!=NULL)
						{
							memset(pTempString, 0, tTempSwitchLen);
							if(XUnicodeToUtf8(pInputMsg->pMsgData, pInputMsg->mEmergencyLen, pTempString, tTempSwitchLen, NULL)==0)
							{
						//	printf("Damon ==> scroll text : %s \n", pTempString);
								XShowScrollText((char *)pTempString);
							}
							free(pTempString);

							memcpy(pXCurMediaData->pMsgData, pInputMsg->pMsgData, (pInputMsg->mEmergencyLen-1)*2);
							pXCurMediaData->mEmergencyLen=pInputMsg->mEmergencyLen;
						}else
						{
							printf("==============================\n");
							printf("====Error no enough memory====\n");
							printf("==============================\n");
						}
					}else
					{
						printf("Damon ==> Error emergen data len is error !\n");
					}
				}
			}
		}

		pXCurMediaData->mEmergencyType=pInputMsg->mEmergencyType;
	}else
	{
		// close emergen
		if(pXCurMediaData->mEmergencyType==0x01)
		{
			X2DRect tTempRect={0, 0, dXScreenWidth, dXScreenHeight};
			Szbh_LayerClear(dXLayerId_Top, tTempRect, 0);
			Szbh_LayerRender(dXLayerId_Top);
		}else if(pXCurMediaData->mEmergencyType==0x02)
		{
			XScrollTextStop();
		}
		pXCurMediaData->mEmergencyType=0;
		pXCurMediaData->mEmergencyLen=0;
	}

	// set brightness
	if(pInputMsg->mScreenLight!=pXCurMediaData->mScreenLight)
	{
		Szbh_DisplaySetBrightness(pInputMsg->mScreenLight);
		pXCurMediaData->mScreenLight=pInputMsg->mScreenLight;
	}

//	Szbh_LayerRender(dXLayerId_Bottom);
}

int XTickHandle(void)
{
	unsigned int tTempCurTime=0;

	if(XGetTick(&tTempCurTime)==0)
	{

		if(tTempCurTime-tXPrevRunTick>300)
		{
			tXPrevRunTick=tTempCurTime;
			XRunLedOnOff();
		}
		

	#ifdef dXUseAdjustBacklight
		if(tTempCurTime-tXPrevBacklightTick>=2000)
		{
			unsigned char pTempData[2]={0, 0};
			if(Szbh_I2cReadData(2, 0x52, 0, 0, pTempData, 2)==0)
			{
				int tTempSampleVal=((pTempData[0]<<8)&0xff00)+pTempData[1];
				int tTempBacklight=tTempSampleVal*100/4096;
				tTempBacklight=100-tTempBacklight;
			//	printf("Damon ==> set backlight=%d - [%d] [%0.3fV]\n", tTempBacklight, tTempSampleVal, (float)tTempSampleVal*2.987/4096);

				if(tTempBacklight<10)
					tTempBacklight=10;
				Szbh_DisplaySetBacklight(tXBacklightHz, tTempBacklight);
			}

			tXPrevBacklightTick=tTempCurTime;
		}
	#endif
		

		XChangeTimeLock();
	#ifndef dXCustomer_Image
		// check screen save
		#ifndef dXCustomer_XiAn5L
		if(tXLocalFilePlay==false)
		#endif
		{
			static int tTempCloseLcd=0;
			if(tXEnterScreenSave)
			{
				if((tTempCurTime>tXScreenSavePrevTick) && (tTempCurTime-tXScreenSavePrevTick)>=tXScreenCloseTimeout /*dXLcdCloseTimeOut*/)
				{
					// close lcd power and lcd en
					if(tTempCloseLcd==0)
					{
						//Szbh_GpioWriteData(/*5*8+1*/dXGpioNumLcdEn, 0);
						//Szbh_GpioWriteData(/*5*8+3*/dXGpioNumLcdPower, 0);

					//	printf("\n\nDamon ==> close lcd power \n\n");
						tTempCloseLcd=1;

					#if (defined dXCustomer_IptvDrmd || defined dXCustomer_XiAn5L)
//						setCmd(SET_CLOSE_LCD);
						sendDrmdPacket();
					#endif
					}
				}
			}
			else
			{
				if(tTempCloseLcd==1)
				{
					Szbh_GpioWriteData(/*5*8+1*/dXGpioNumLcdEn, 1);
					Szbh_GpioWriteData(/*5*8+3*/dXGpioNumLcdPower, 1);
					
					printf("\n\nDamon ==> open lcd power \n\n");
					tTempCloseLcd=0;
					tXScreenSavePrevTick=tTempCurTime;
				}
			
				if(tTempCurTime>tXScreenSavePrevTick)
				{
					if((tTempCurTime-tXScreenSavePrevTick)>=tXScreenSaveTimeout/*dXScreenSaveTimeOut*/)
					{
						if((tTempCurTime-tXScreenSavePrevTick)<=tXScreenSaveTimeout/*dXScreenSaveTimeOut*/+(1*1000*60))
						{
							XScreenSaveEnterOrNot(true);

							if(pXCurMediaData!=NULL)
								pXCurMediaData->mTriggerFlag=0;
						}else
						{	// sync time from server
							tXScreenSavePrevTick=tTempCurTime;							
						}
					}
				}else
				{
					tXScreenSavePrevTick=tTempCurTime;
				}
			}
		}
	#endif

		if(tTempCurTime<tXScreenSavePrevTick)
			tXScreenSavePrevTick=tTempCurTime;

		XChangeTimeUnlock();


		// hardware whatch dog
		if(tTempCurTime-tXPrevWdgTick>=1200)
		{
		#ifdef dXHwcWdgPin
			XHwcWhatchdog();
		#endif

		#ifdef dXUseWhatchDog
		//	 printf("Damon ==> clear watch dog !\n");
			 Szbh_WdgClear();
		#endif
		
			tXPrevWdgTick=tTempCurTime;
		}


		if(tXCheckColorFlag>0)
		{
			 if((tTempCurTime-tXUpdateTick)>=(tXColorTest.mTick*1000) || tTempCurTime<tXUpdateTick)
			 {			
				// check screen color
				XEnterOrCloseColorMode(1);

				tXUpdateTick=tTempCurTime;
			 }
		}

		if((tTempCurTime-tXPrevTick)>=1*1000 || tTempCurTime<tXPrevTick)
		{								
			tXPrevTick=tTempCurTime;
		}
	}

	return 0;
}


void MyIrEventCallBack(unsigned int tInputIrCode, EIrEvent tInputEvent)
{
	printf("Damon ==> ir event : 0x%x - %d \n", tInputIrCode, tInputEvent);
	if(tInputEvent==dXIrEvent_Down)
	{
	/*	if(tInputIrCode==0xea15ff00)
		{
			if(tXCheckColorFlag==0)
			{
				tXCheckColorFlag=1;
			}
		}*/

		if(tInputIrCode==dXIrCode_Power)
		{
			Szbh_GpioWriteData(/*5*8+1*/dXGpioNumLcdEn, 0);		
			Szbh_GpioWriteData(/*5*8+3*/dXGpioNumLcdPower, 0);
			
			system("killall myloader");
			system("killall usb_hotplug");
			sync();
			sleep(1);
			system("reboot");
			
		}else
		{
		printf("Damon ==> enter setup menu !\n");
			XSetupMenuHandle(tInputIrCode);
			Szbh_LayerRender(dXLayerId_Top);
		}
	}
	XCheckScreenSave();
}


int XIpAddrInit(void)
{
#define dXGpioNumAddr0		3*8+6
#define dXGpioNumAddr1		3*8+5
#define dXGpioNumAddr2		3*8+4
#define dXGpioNumAddr3		4*8+2
#define dXGpioNumAddr4		4*8+1
#define dXGpioNumAddr5		4*8+0
#define dXGpioNumAddr6		3*8+7


	// gpio share
	{
		unsigned int tTempRegAddr=0;
		unsigned int tTempRegVal=0; 

		tTempRegAddr=0xF8A21000+0x070;
		if(Szbh_ReadRegister(tTempRegAddr, &tTempRegVal)==0)
		{
			tTempRegVal &= 0xfffffff8;
			tTempRegVal |= 0x04;
			Szbh_WriteRegister(tTempRegAddr, tTempRegVal);
		}
	}

	int pTempGpioNum[7]={
		dXGpioNumAddr0, dXGpioNumAddr1, dXGpioNumAddr2, dXGpioNumAddr3,
		dXGpioNumAddr4, dXGpioNumAddr5, dXGpioNumAddr6
	};

	int tTempAddrNum=sizeof(pTempGpioNum)/sizeof(int);

	int i=0;
	for(i=0; i<tTempAddrNum; i++)
	{
		if(Szbh_GpioSetInput(pTempGpioNum[i])!=0)
			printf("Damon ==> Error set ip addr gpio dir failed : %d %d \n", i, pTempGpioNum[i]);
		usleep(1000*10);
	}

	int pTempData[7]={1, 1, 1, 1, 1, 1, 1};
	int tTempReadVal=0;
	for(i=0; i<tTempAddrNum; i++)
	{
		if(Szbh_GpioReadData(pTempGpioNum[i], &tTempReadVal)==0)
		{
			pTempData[i]=tTempReadVal;
		}else
		{
			printf("Damon ==> Error read ip addr gpio val failed : %d \n", pTempGpioNum[i]);
		}
		usleep(1000*10);
	}

	int tTempCardNum=0, tTempLocateNum=0;
#ifdef dXCustomer_IptvDrmd
	if(pTempData[0]==0 && pTempData[1]==0 && pTempData[2]==0)
		tTempCardNum=1;
	else if(pTempData[0]==0 && pTempData[1]==0 && pTempData[2]==1)
		tTempCardNum=2;
	else if(pTempData[0]==0 && pTempData[1]==1 && pTempData[2]==0)
		tTempCardNum=3;
	else if(pTempData[0]==0 && pTempData[1]==1 && pTempData[2]==1)
		tTempCardNum=4;
	else if(pTempData[0]==1 && pTempData[1]==0 && pTempData[2]==0)
		tTempCardNum=5;
	else if(pTempData[0]==1 && pTempData[1]==0 && pTempData[2]==1)
		tTempCardNum=6;
	else if(pTempData[0]==1 && pTempData[1]==1 && pTempData[2]==0)
		tTempCardNum=7;
	else 
	{
		printf("Damon ==> no this card number !\n");
		return -1;
	}

	if(pTempData[3]==0)
		tTempLocateNum=1;
	else if(pTempData[3]==1)
		tTempLocateNum=2;
#elif (defined dXCustomer_XiAn5L)
	if(pTempData[0]==0 && pTempData[1]==1 && pTempData[2]==1)
		tTempCardNum=1;
	else if(pTempData[0]==1 && pTempData[1]==0 && pTempData[2]==1)
		tTempCardNum=2;
	else if(pTempData[0]==0 && pTempData[1]==0 && pTempData[2]==1)
		tTempCardNum=3;
	else if(pTempData[0]==1 && pTempData[1]==1 && pTempData[2]==0)
		tTempCardNum=4;
	else if(pTempData[0]==0 && pTempData[1]==1 && pTempData[2]==0)
		tTempCardNum=5;
	else if(pTempData[0]==1 && pTempData[1]==0 && pTempData[2]==0)
		tTempCardNum=6;
	else 
	{
		printf("Damon ==> no this card number !\n");
		return -1;
	}

	if(pTempData[3]==0 && pTempData[4]==1 && pTempData[5]==1)
		tTempLocateNum=1;
	else if(pTempData[3]==1 && pTempData[4]==0 && pTempData[5]==1)
		tTempLocateNum=2;
	else if(pTempData[3]==0 && pTempData[4]==0 && pTempData[5]==1)
		tTempLocateNum=3;
	else if(pTempData[3]==1 && pTempData[4]==1 && pTempData[5]==0)
		tTempLocateNum=4;
	else if(pTempData[3]==0 && pTempData[4]==1 && pTempData[5]==0)
		tTempLocateNum=5;
	else if(pTempData[3]==1 && pTempData[4]==0 && pTempData[5]==0)
		tTempLocateNum=6;
	else if(pTempData[3]==0 && pTempData[4]==0 && pTempData[5]==0)
		tTempLocateNum=7;
	else if(pTempData[3]==1 && pTempData[4]==1 && pTempData[5]==1)
		tTempLocateNum=8;
	else
	{
		printf("Damon ==> no this drmd number !\n");
		return -1;

	}
	
#else
	if(pTempData[0]==0 && pTempData[1]==0 && pTempData[2]==0)
		tTempCardNum=1;
	else if(pTempData[0]==0 && pTempData[1]==0 && pTempData[2]==1)
		tTempCardNum=2;
	else if(pTempData[0]==0 && pTempData[1]==1 && pTempData[2]==0)
		tTempCardNum=3;
	else if(pTempData[0]==0 && pTempData[1]==1 && pTempData[2]==1)
		tTempCardNum=4;
	else if(pTempData[0]==1 && pTempData[1]==0 && pTempData[2]==0)
		tTempCardNum=5;
	else if(pTempData[0]==1 && pTempData[1]==0 && pTempData[2]==1)
		tTempCardNum=6;
	else 
	{
		printf("Damon ==> no this card number !\n");
		return -1;
	}

	if(pTempData[3]==0 && pTempData[4]==0 && pTempData[5]==0)
		tTempLocateNum=1;
	else if(pTempData[3]==0 && pTempData[4]==0 && pTempData[5]==1)
		tTempLocateNum=2;
	else if(pTempData[3]==0 && pTempData[4]==1 && pTempData[5]==0)
		tTempLocateNum=3;
	else if(pTempData[3]==0 && pTempData[4]==1 && pTempData[5]==1)
		tTempLocateNum=4;
	else if(pTempData[3]==1 && pTempData[4]==0 && pTempData[5]==0)
		tTempLocateNum=5;
	else if(pTempData[3]==1 && pTempData[4]==0 && pTempData[5]==1)
		tTempLocateNum=6;
	else if(pTempData[3]==1 && pTempData[4]==1 && pTempData[5]==0)
		tTempLocateNum=7;
	else if(pTempData[3]==1 && pTempData[4]==1 && pTempData[5]==1)
		tTempLocateNum=8;
#endif

	char pTempGetIpAddr[20]={0};
	char pTempSetIpAddr[20]={0};
	int tTempNetLocate=0;
#ifdef dXCustomer_ShangHai18L
	if(tTempCardNum==1)
		snprintf(pTempSetIpAddr, sizeof(pTempSetIpAddr)-1, "172.16.7.%d", tTempLocateNum);
	else if(tTempCardNum>1)
		snprintf(pTempSetIpAddr, sizeof(pTempSetIpAddr)-1, "172.16.7.%d%d", (tTempCardNum-1), tTempLocateNum);
	tTempNetLocate=7;
#elif (defined dXCustomer_NanNing2L) || (defined dXCustomer_NanNing4L)
	if(tTempCardNum==0)
		snprintf(pTempSetIpAddr, sizeof(pTempSetIpAddr)-1, "192.168.101.%d", tTempLocateNum);
	else if(tTempCardNum>=1)
		snprintf(pTempSetIpAddr, sizeof(pTempSetIpAddr)-1, "192.168.101.%d%d", tTempCardNum, tTempLocateNum);
	tTempNetLocate=101;
#elif (defined dXCustomer_IptvDrmd)
	snprintf(pTempSetIpAddr, sizeof(pTempSetIpAddr)-1, "172.16.101.%d%d", tTempCardNum, tTempLocateNum);
	tTempNetLocate=101;

	tXDeviceId=tTempCardNum*10+tTempLocateNum;
#elif (defined dXCustomer_XiAn5L)
	if(tTempCardNum==1)
		snprintf(pTempSetIpAddr, sizeof(pTempSetIpAddr)-1, "172.16.7.%d", tTempLocateNum);
	else if(tTempCardNum>1)
		snprintf(pTempSetIpAddr, sizeof(pTempSetIpAddr)-1, "172.16.7.%d%d", (tTempCardNum-1), tTempLocateNum);
	tTempNetLocate=7;
	tXDeviceId=(tTempCardNum-1)*10+tTempLocateNum;	
#else
	if(tTempCardNum==0)
		snprintf(pTempSetIpAddr, sizeof(pTempSetIpAddr)-1, "192.168.1.%d", tTempLocateNum);
	else if(tTempCardNum>=1)
		snprintf(pTempSetIpAddr, sizeof(pTempSetIpAddr)-1, "192.168.1.%d%d", tTempCardNum, tTempLocateNum);
	tTempNetLocate=1;
#endif


	if(XSectionGetValue(dXSetupConfigFile, dXSetupIpSectionName, dXSetupIpKeyName, pTempGetIpAddr, sizeof(pTempGetIpAddr)-1)!=0)
		strcpy(pTempGetIpAddr, "0.0.0.0");

	if(strcmp(pTempGetIpAddr, pTempSetIpAddr)!=0)
	{
		if(XSectionSaveValue(dXSetupConfigFile, dXSetupIpSectionName, dXSetupIpKeyName, pTempSetIpAddr)!=0)
		{
			printf("Damon ==> save ip addr failed !\n");
			return -2;
		}
	}

#ifdef dXNetmask
	char pTempMaskAddr[20]={0};
	char pTempSetMaskAddr[20]={0};
	if(XSectionGetValue(dXSetupConfigFile, dXSetupIpSectionName, dXSetupMaskKeyName, pTempMaskAddr, sizeof(pTempMaskAddr)-1)!=0)
		strcpy(pTempGetIpAddr, "0.0.0.0");

	sprintf(pTempSetMaskAddr, "%s", dXNetmask);
	if(strcmp(pTempMaskAddr, pTempSetMaskAddr)!=0)
	{
		if(XSectionSaveValue(dXSetupConfigFile, dXSetupIpSectionName, dXSetupMaskKeyName, pTempSetMaskAddr)!=0)
		{
			printf("Damon ==> save mask addr failed !\n");
			return -2;
		}
	}
#endif

	char pTempMacAddr[24]={0};
	if(XSectionGetValue(dXSetupConfigFile, dXSetupIpSectionName, dXSetupMacKeyName, pTempMacAddr, sizeof(pTempMacAddr)-1)==0)
	{
		int tTempHight=tTempNetLocate/16;
		int tTempLow=tTempNetLocate%16;
		if(tTempHight>=0 && tTempHight<=9)
			pTempMacAddr[12]='0'+tTempHight;
		else if(tTempHight>9)
			pTempMacAddr[12]='A'+(tTempHight-10);
		if(tTempLow>=0 && tTempLow<=9)
			pTempMacAddr[13]='0'+tTempLow;
		else if(tTempLow>9)
			pTempMacAddr[13]='A'+(tTempLow-10);

		tTempHight=(tTempCardNum*10+tTempLocateNum)/16;
		tTempLow=(tTempCardNum*10+tTempLocateNum)%16;
		if(tTempHight>=0 && tTempHight<=9)
			pTempMacAddr[15]='0'+tTempHight;
		else if(tTempHight>9)
			pTempMacAddr[15]='A'+(tTempHight-10);
		if(tTempLow>=0 && tTempLow<=9)
			pTempMacAddr[16]='0'+tTempLow;
		else if(tTempLow>9)
			pTempMacAddr[16]='A'+(tTempLow-10);
		
		if(XSectionSaveValue(dXSetupConfigFile, dXSetupIpSectionName, dXSetupMacKeyName, pTempMacAddr))
		{
			printf("Damon ==> save mac addr failed !\n");
			return -3;
		}
	}

	sync();

	return 0;
}


int XShowStartMenu(void)
{
	char pTempImgVerCh[16]={0xE5, 0x9B, 0xBA, 0xE4, 0xBB, 0xB6, 0xE7, 0x89, 0x88, 0xE6, 0x9C, 0xAC, 0x00};
	char pTempVersionCh[16]={0xE8, 0xBD, 0xAF, 0xE4, 0xBB, 0xB6, 0xE7, 0x89, 0x88, 0xE6, 0x9C, 0xAC, 0x00};
	char pTempIpAddrCh[16]={0x49, 0x50, 0xE5, 0x9C, 0xB0, 0xE5, 0x9D, 0x80, 0x00};

	XSurface *pTempSurface=NULL;

	X2DRect tTempRect={0, 0, dXScreenWidth, dXScreenHeight};
#if (defined dXCustomer_IptvDrmd || defined dXCustomer_XiAn5L)
	Szbh_LayerClear(dXLayerId_Top, tTempRect, 0xff000000);
#else
	Szbh_LayerClear(dXLayerId_Bottom, tTempRect, 0xff000000);
#endif

	u32 tTempFontHandle=0;
	if(Szbh_CreateFont(dXResourcePath"/font.ttf", 48, &tTempFontHandle)==0)
	{
		int tTempShowY=50;
		char pTempString[128]={0};
		char pTempImgVer[12]={0};

		if(XSectionGetValue("/etc/imgver.conf", "image", "version", pTempImgVer, sizeof(pTempImgVer)-1)==0)
		{
			snprintf(pTempString, sizeof(pTempString)-1, "%s:%s", pTempImgVerCh, pTempImgVer);
			strcpy(pXSystemImgVer, pTempImgVer);
			pTempSurface=Szbh_LoadString(tTempFontHandle, pTempString, dXColor_White, 0);
			if(pTempSurface!=NULL)
			{
			#if (defined dXCustomer_IptvDrmd || defined dXCustomer_XiAn5L)
				Szbh_LayerShowSurface(dXLayerId_Top, pTempSurface, 50, tTempShowY);
			#else
				Szbh_LayerShowSurface(dXLayerId_Bottom, pTempSurface, 50, tTempShowY);
			#endif

				Szbh_SurfaceDestroy(pTempSurface);
				pTempSurface=NULL;

				tTempShowY += 70;
			}
		}

		memset(pTempString, 0, sizeof(pTempString));
		snprintf(pTempString, sizeof(pTempString)-1, "%s:%s", pTempVersionCh, dXSoftwareVersion);
		pTempSurface=Szbh_LoadString(tTempFontHandle, pTempString, dXColor_White, 0);
		if(pTempSurface!=NULL)
		{
		#if (defined  dXCustomer_IptvDrmd || defined dXCustomer_XiAn5L)
			Szbh_LayerShowSurface(dXLayerId_Top, pTempSurface, 50, tTempShowY);
		#else
			Szbh_LayerShowSurface(dXLayerId_Bottom, pTempSurface, 50, tTempShowY);
		#endif

			Szbh_SurfaceDestroy(pTempSurface);
			pTempSurface=NULL;

			tTempShowY += 70;
		}

		char pTempIpAddr[20]={0};
		memset(pTempString, 0, sizeof(pTempString));
		if(XSectionGetValue(dXSetupConfigFile, dXSetupIpSectionName, dXSetupIpKeyName, pTempIpAddr, sizeof(pTempIpAddr))==0)
		{
			snprintf(pTempString, sizeof(pTempString)-1, "%s:%s", pTempIpAddrCh, pTempIpAddr);
			pTempSurface=Szbh_LoadString(tTempFontHandle, pTempString, dXColor_White, 0);
			if(pTempSurface!=NULL)
			{
			#if (defined dXCustomer_IptvDrmd || defined dXCustomer_XiAn5L)
				Szbh_LayerShowSurface(dXLayerId_Top, pTempSurface, 50, tTempShowY);
			#else
				Szbh_LayerShowSurface(dXLayerId_Bottom, pTempSurface, 50, tTempShowY);
			#endif
			
				Szbh_SurfaceDestroy(pTempSurface);
				pTempSurface=NULL;

				tTempShowY += 70;
			}
		}

	#if (defined dXCustomer_IptvDrmd || defined dXCustomer_XiAn5L)
		Szbh_LayerRender(dXLayerId_Top);
	#else
		Szbh_LayerRender(dXLayerId_Bottom);
	#endif

		Szbh_DestroyFont(tTempFontHandle);
	}

	return 0;
}

void XCustomerGetIpFromDhcp()
{
	system("ifconfig eth0 down");
	sleep(1);
	system("udhcpc -i eth0 &");
	X2DRect tTempShowRect={0, 0, dXScreenWidth, dXScreenHeight};
	Szbh_LayerClear(dXLayerId_Bottom, tTempShowRect, 0xffffffff);

	u32 tTempFontHandle=0;
	if(Szbh_CreateFont(dXResourcePath"/font.ttf", 48, &tTempFontHandle)==0)
	{
		XSurface *pTempShowString=Szbh_LoadString(tTempFontHandle, "公版测试程序，正在获取IP地址......", 0xff000000, 0xffffffff);
		if(pTempShowString)
		{
			int tTempShowX=0, tTempShowY=0;;
			if(dXScreenWidth>pTempShowString->mWidth)
				tTempShowX=(dXScreenWidth-pTempShowString->mWidth)/2;
			if(dXScreenHeight>pTempShowString->mHeight)
				tTempShowY=(dXScreenHeight-pTempShowString->mHeight)/2;

			XSurface *pTempShowSurface=Szbh_SurfaceCreate(pTempShowString->mWidth, pTempShowString->mHeight);
			if(pTempShowSurface!=NULL)
			{
				Szbh_SurfaceFill(pTempShowSurface, NULL, 0xffffffff);
				X2DRect tTempRect={0, 0, 0, 0};
				tTempRect.width=pTempShowString->mWidth;
				tTempRect.height=pTempShowString->mHeight;
				Szbh_SurfaceBlitWithAlpha(pTempShowString, &tTempRect, pTempShowSurface, &tTempRect, EBlitAlpha_Both);
				Szbh_LayerShowSurface(dXLayerId_Bottom, pTempShowSurface, tTempShowX, tTempShowY);

				Szbh_SurfaceDestroy(pTempShowSurface);
			}

			Szbh_SurfaceDestroy(pTempShowString);
		}
		Szbh_DestroyFont(tTempFontHandle);
	}
	Szbh_LayerRender(dXLayerId_Bottom);
	sleep(1);

	char pTempString[20]={0};
	while(1)
	{
		if(XSetupGetLocalIp("eth0", pTempString)==0)
		{
			int i=0;
			int tTempPoint=0;
			for(i=0; i<strlen(pTempString); i++)
			{
				if(pTempString[i]=='.')
					tTempPoint++;
			}
			if(tTempPoint==3)
				break;
		}
		
		sleep(1);
	}
	Szbh_LayerClear(dXLayerId_Bottom, tTempShowRect, 0xffffffff);

	XSectionSaveValue(dXSetupConfigFile, dXSetupIpSectionName, dXSetupIpKeyName, pTempString);

}


void XSystemStartInit(void)
{
#ifdef dXUseAdjustBacklight
	Szbh_ApiInit(dXApiInit_System | dXApiInit_Log | dXApiInit_Framebuffer | dXApiInit_Gpio | dXApiInit_Surface | dXApiInit_Player | dXApiInit_Ir | dXApiInit_I2c);
#else
	Szbh_ApiInit(dXApiInit_System | dXApiInit_Log | dXApiInit_Framebuffer | dXApiInit_Gpio | dXApiInit_Surface | dXApiInit_Player | dXApiInit_Ir);
#endif
	
	usleep(1000*100);

	Szbh_DisplaySetScreenResolution(dXScreenWidth, dXScreenHeight);
//	Szbh_DisplaySetScreenResolution(3840, 2160);
	
	// lcd power and en
#if 1
	Szbh_GpioSetOutput(/*5*8+1*/dXGpioNumLcdEn);
	Szbh_GpioWriteData(/*5*8+1*/dXGpioNumLcdEn, 0);
	usleep(1000*300);	
//	Szbh_GpioSetOutput(/*5*8+1*/dXGpioNumLcdEn);
	Szbh_GpioWriteData(/*5*8+1*/dXGpioNumLcdEn, 1);
	
	Szbh_GpioSetOutput(/*5*8+3*/dXGpioNumLcdPower);
	Szbh_GpioWriteData(/*5*8+3*/dXGpioNumLcdPower, 1);
#endif
	
		// mute
	Szbh_GpioSetOutput(/*6*8+3*/dXGpioNumMute);

	// run led
	Szbh_GpioSetOutput(dXGpioNumRunLed);
}


/************* IPTV AND DRMD****************/
void XProcessIptvDrmdCtrlMsg(XMediaCtrl *pInputMsg)
{
	if(pInputMsg==NULL)
		return ;

	// exit screen save
	XCheckScreenSave();


/*	printf("Damon ==> %d %d - %d %d - %d %d - %d %d \n", pXCurMediaData->mTriggerFlag, pInputMsg->mTriggerFlag,
			pXCurMediaData->mCurStation, pInputMsg->mCurStation, pXCurMediaData->mNextStation, pInputMsg->mNextStation,
			pXCurMediaData->mEndStation, pInputMsg->mEndStation);*/

	// change template
	if(pXCurMediaData->mTemplate!=pInputMsg->mTemplate)
	{
	//	pXCurMediaData->mTemplate=pInputMsg->mTemplate;
	//	pXCurMediaData->mTriggerFlag=0;
	}

	// station trigger
	if((pInputMsg->mOpenTriggerFLag&0xC0)!=(pXCurMediaData->mOpenTriggerFLag&0xC0))
	{
		pXCurMediaData->mOpenTriggerFLag=pInputMsg->mOpenTriggerFLag;
	}
	
	static int tTempVolChangeFlag=0;
	if((pInputMsg->mTriggerFlag & 0x02) || (pInputMsg->mTriggerFlag & 0x01))
	{
		if(pXCurMediaData->mTriggerFlag!=pInputMsg->mTriggerFlag 
			|| pXCurMediaData->mCurStation!=pInputMsg->mCurStation
			|| pXCurMediaData->mNextStation!=pInputMsg->mNextStation
			|| pXCurMediaData->mEndStation!=pInputMsg->mEndStation)
		{
			pXCurMediaData->mCurStation=pInputMsg->mCurStation;
			pXCurMediaData->mNextStation=pInputMsg->mNextStation;
			pXCurMediaData->mEndStation=pInputMsg->mEndStation;				
		}

	#ifndef dXCustomer_IptvDrmd
		if(tTempVolChangeFlag==0)
		{
			XPlayerSetVolume(0);
			tTempVolChangeFlag=1;
		}
	#endif
	}else
	{
	#ifdef dXCustomer_IptvDrmd
		if(tTempVolChangeFlag==1)
		{
			XPlayerSetVolume(pXCurMediaData->mVolume);
			tTempVolChangeFlag=0;
		}
	#endif
	}

	//emergency trigger
/*printf("Damon ==> trigger falg : %d %d - %d %d \n", pXCurMediaData->mTriggerFlag, pInputMsg->mTriggerFlag,
	pInputMsg->mEmergencyCode, pXCurMediaData->mEmergencyCode);*/
	if(pInputMsg->mTriggerFlag & 0x08)	 
	{
		if((pXCurMediaData->mTriggerFlag & 0x08)==0 || (pInputMsg->mEmergencyCode!=pXCurMediaData->mEmergencyCode))
		{
			if((pXCurMediaData->mTriggerFlag & 0x08)==0)
				XScrollTextPause();
		
			XShowEmergencyCode(pInputMsg->mEmergencyCode);
			Szbh_LayerRender(dXLayerId_Top);
			pXCurMediaData->mEmergencyCode=pInputMsg->mEmergencyCode;

		#ifndef dXCustomer_IptvDrmd
			XPlayerSetVolume(0);
		#endif
		}
	}else {
	
		if(pXCurMediaData->mTriggerFlag & 0x08)
		{
			X2DRect tTempRect={0, 0, dXFramebufWidth, dXFramebufHeight};
			Szbh_LayerClear(dXLayerId_Top, tTempRect, 0);
			Szbh_LayerRender(dXLayerId_Top);
			
			XScrollTextResume();

		#ifndef dXCustomer_IptvDrmd
			XPlayerSetVolume(pXCurMediaData->mVolume);
		#endif
		}
		pXCurMediaData->mEmergencyCode=0;
	}
	pXCurMediaData->mTriggerFlag=pInputMsg->mTriggerFlag;
	

	// set player volume
#ifndef dXCustomer_IptvDrmd
	if(pXCurMediaData->mEmergencyCode<=0)
#endif
	{
		if(pXCurMediaData->mVolume!=pInputMsg->mVolume)
		{
		printf("Damon ==> set volume : %d %d \n", pInputMsg->mVolume, pInputMsg->mVolume);
			if(pInputMsg->mVolume>=0 && pInputMsg->mVolume<=100)
			{
			#ifndef dXCustomer_IptvDrmd
				if((pInputMsg->mTriggerFlag & 0x03)==0)
			#endif
					XPlayerSetVolume(pInputMsg->mVolume);
				
				pXCurMediaData->mVolume=pInputMsg->mVolume;
			}
		}
	}

#ifndef dXUseNtpAdjustTime
	// adjust time 
	{
		if(pXCurMediaData->mYear!=pInputMsg->mYear || pXCurMediaData->mMonth!=pInputMsg->mMonth
			|| pXCurMediaData->mDay!=pInputMsg->mDay || pXCurMediaData->mHour!=pInputMsg->mHour
			|| pXCurMediaData->mMinute!=pInputMsg->mMinute || pXCurMediaData->mSeconds!=pInputMsg->mSeconds)
		{
			pXCurMediaData->mYear=pInputMsg->mYear;
			pXCurMediaData->mMonth=pInputMsg->mMonth;
			pXCurMediaData->mDay=pInputMsg->mDay;
			pXCurMediaData->mHour=pInputMsg->mHour;
			pXCurMediaData->mMinute=pInputMsg->mMinute;
			pXCurMediaData->mSeconds=pInputMsg->mSeconds;
			XSetSystemTime(pXCurMediaData->mYear+2000, pXCurMediaData->mMonth, pXCurMediaData->mDay, 
					pXCurMediaData->mHour, pXCurMediaData->mMinute, pXCurMediaData->mSeconds);
		}
	}
#endif


	// change emergency message
	if(pInputMsg->mEmergencyType!=0)
	{
		if(pInputMsg->mEmergencyType==0x01)	// message
		{
			if(pXCurMediaData->mEmergencyType==0x02)
			{
				XScrollTextStop();
				pXCurMediaData->mEmergencyLen=0;
				usleep(1000*10);
			}
		
			if(pInputMsg->mEmergencyLen>0)
			{
				if(!(pXCurMediaData->mEmergencyLen==pInputMsg->mEmergencyLen
					&& memcmp(pXCurMediaData->pMsgData, pInputMsg->pMsgData, (pInputMsg->mEmergencyLen-1)*2)==0))
				{
					XScrollTextPause();
					
					XShowEmergencyMsg(pInputMsg->pMsgData, pInputMsg->mEmergencyLen);
					Szbh_LayerRender(dXLayerId_Top);

					memcpy(pXCurMediaData->pMsgData, pInputMsg->pMsgData, (pInputMsg->mEmergencyLen-1)*2);
					pXCurMediaData->mEmergencyLen=pInputMsg->mEmergencyLen;
				}
			}
		}else if(pInputMsg->mEmergencyType==0x02)	// scroll text
		{

			if(pXCurMediaData->mEmergencyType==0x01)
			{
				X2DRect tTempRect={0, 0, dXScreenWidth, dXScreenHeight};
				Szbh_LayerClear(dXLayerId_Top, tTempRect, 0);
				Szbh_LayerRender(dXLayerId_Top);
				pXCurMediaData->mEmergencyLen=0;
				
				XScrollTextResume();
				usleep(1000*10);
			}
		
			if(pInputMsg->mEmergencyLen>1)
			{
				if(!(pXCurMediaData->mEmergencyLen==pInputMsg->mEmergencyLen
					&& memcmp(pXCurMediaData->pMsgData, pInputMsg->pMsgData, (pInputMsg->mEmergencyLen-1)*2)==0))
				{
				printf("Damon ==> emergen unicode len=%d \n", pInputMsg->mEmergencyLen);
					if(pInputMsg->mEmergencyLen<256)
					{
						int tTempSwitchLen=pInputMsg->mEmergencyLen*6+1;
						unsigned char *pTempString=(unsigned char *)malloc(tTempSwitchLen);
						if(pTempString!=NULL)
						{
							memset(pTempString, 0, tTempSwitchLen);
							if(XUnicodeToUtf8(pInputMsg->pMsgData, pInputMsg->mEmergencyLen, pTempString, tTempSwitchLen, NULL)==0)
							{
						//	printf("Damon ==> scroll text : %s \n", pTempString);
								XShowScrollText((char *)pTempString);
							}
							free(pTempString);

							memcpy(pXCurMediaData->pMsgData, pInputMsg->pMsgData, (pInputMsg->mEmergencyLen-1)*2);
							pXCurMediaData->mEmergencyLen=pInputMsg->mEmergencyLen;
						}else
						{
							printf("==============================\n");
							printf("====Error no enough memory====\n");
							printf("==============================\n");
						}
					}else
					{
						printf("Damon ==> Error emergen data len is error !\n");
					}
				}
			}
		}

		pXCurMediaData->mEmergencyType=pInputMsg->mEmergencyType;
	}else
	{
		// close emergen
		if(pXCurMediaData->mEmergencyType==0x01)
		{
			X2DRect tTempRect={0, 0, dXScreenWidth, dXScreenHeight};
			Szbh_LayerClear(dXLayerId_Top, tTempRect, 0);
			Szbh_LayerRender(dXLayerId_Top);

			XScrollTextResume();
		}else if(pXCurMediaData->mEmergencyType==0x02)
		{
			XScrollTextStop();
		}
		pXCurMediaData->mEmergencyType=0;
		pXCurMediaData->mEmergencyLen=0;
	}


	// set brightness
	if(pInputMsg->mScreenLight!=pXCurMediaData->mScreenLight)
	{
		Szbh_DisplaySetBrightness(pInputMsg->mScreenLight);
		pXCurMediaData->mScreenLight=pInputMsg->mScreenLight;
	}

//	Szbh_LayerRender(dXLayerId_Bottom);
}


#if (defined dXCustomer_IptvDrmd || defined dXCustomer_XiAn5L)
void XStartMenuApp()
{
	pid_t tTempPid;
	
	if((tTempPid=fork())<0)
	{
		printf("Damon ==> fork error !\n");
		exit(1);
	}else if(tTempPid==0)
	{
		// child process 
		printf("\nDamon ==> start child process : %d \n", getpid());
		if(access("/usrdata/appdir/SzbhMenu", F_OK)==0 && access("/usrdata/appdir/lib/libQtCore.so.4", F_OK)==0)
		{
			if(execl("/usrdata/appdir/SzbhMenu", "SzbhMenu", "-qws", NULL)<0)
			{
				printf("Damon ==> start szbhmenu failed !!!!\n");
				exit(1);
			}
			printf("Damon ==> child process exit !!!!\n");
		}else
		{
			printf("Damon ==>> Error : not find qt env !!!\n");
			while(1)
			{
				sleep(5);
				if(access("/usrdata/appdir/SzbhMenu", F_OK)==0 && access("/usrdata/appdir/lib/libQtCore.so.4", F_OK)==0)
				{
					break;
				}
			}
			exit(-1);
		}
	}else
	{
		usleep(1000*100);
	}
}


void XSignalHandle(int tInputSigNum)
{
	if(tInputSigNum==SIGCHLD)
	{
	/*	printf("\n\nJamon ==>> recv sigchild signal \n\n");
		int tTempPid = wait(NULL);
		printf("Jamon ==>> exit child : %d \n", tTempPid);*/

	}
}

void *XCheckChildProcess(void *pInputArg)
{
	sleep(5);

	while(1)
	{
		sleep(3);
	
		FILE *pTempFile=popen("ps | grep \'SzbhMenu -qws\' | awk \'{print $1}\'", "r");
		char pTempBuffer[10] = {0};
		if(pTempFile != NULL)
		{
			int tTempChildPid=0;
			int tTempCount = 0;
			while(NULL != fgets(pTempBuffer, 10, pTempFile))
			{
				tTempChildPid = atoi(pTempBuffer);
				if(tTempChildPid>0)
				{
					tTempCount ++;
				}
			//	printf("\n\nJamon ==>> [%s] [%d] \n", pTempBuffer, tTempChildPid);		
			}

			if(tTempCount == 2)
			{
				system("killall SzbhMenu");
				usleep(1000*100);
			
				printf("===========================\n");
				printf("Damon ==> child process exit !\n");
				printf("======= restart menu ========\n");
				printf("===========================\n");
			
				XStartMenuApp();
			}
		
			pclose(pTempFile);
		}
		
	}

	return NULL;
}

void XIptvDrmdInit()
{

/*	if(signal(SIGCHLD, XSignalHandle)==SIG_ERR)
	{
		printf("==== Damon signale err !!! ========\n");
	}*/

	signal(SIGCHLD, SIG_IGN);

	pthread_mutex_init(&gXChangeTimeMutex, NULL);

	XProcessCommunicateInit(tXDeviceId);

	XStartMenuApp();

	if(access("/root/socket",F_OK)<0)
	{
	    mkdir("/root/socket", S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
	}

	sync();
	sleep(1);

	X2DRect tTempClearRect={0, 0, dXScreenWidth, dXScreenHeight};
	Szbh_LayerClear(dXLayerId_Top, tTempClearRect, 0);
	Szbh_LayerRender(dXLayerId_Top);

	pthread_t tTempHandle;
	pthread_create(&tTempHandle, NULL, XCheckChildProcess, NULL);
}
#endif
/****************** END *********************/

int main(int argc, char *argv[])
{

	if(argc<2)
	{
		printf("usage : ./szbhplayer udp://225.1.1.40:4000 \n");
		return 0;
	}

#if 0
	if(argc<3)
	{
		printf("usr : ./szbhplayer ../test_data/10004.mp4 400 \n");
		return -1;
	}

	tXBacklightHz=atoi(argv[2]);
	printf("Damon ==> back light : %d \n", tXBacklightHz);
	if(tXBacklightHz<=100 || tXBacklightHz>=1000000)
	{
		printf("Damon ==> back light hz is error !\n");
		return -2;
	}
#endif
	

	pXCurMediaData=(XMediaCtrl *)malloc(sizeof(XMediaCtrl));
	memset(pXCurMediaData, 0, sizeof(XMediaCtrl));
	pXCurMediaData->mTemplate=2;

	XSystemStartInit();	

#ifdef dXCustomer_Image
// use dhcp get ip address
	XCustomerGetIpFromDhcp();
#endif


#ifdef dXUseWhatchDog
	if(Szbh_WdgInit()==0)
	{
		if(Szbh_WdgSetEnable(10*1000)==0)	// 10s
		{
			printf("Damon ==> whatch dog enable timeout=10*1000 \n");
			Szbh_WdgClear();
		}
	}
#endif

log_save(dXLogLevel_Info, "start [%s][%d]\n", __FUNCTION__, __LINE__);
log_save(dXLogLevel_Info, "log test template=%d \n", pXCurMediaData->mTemplate);

	// init ip addr
	XIpAddrInit();////////////////////


	// net and display init
	tXScreenSaveTimeout=dXScreenSaveTimeOut;
	tXScreenCloseTimeout=dXLcdCloseTimeOut;
	XSetupConfigInit(&pXCurMediaData->mVolume, &tXDeviceId);///////////
	printf("Damon ==>> device id : %d \n", tXDeviceId);


	XConfigFileInit();		// init template file
	XConfigLoadTemplate(pXCurMediaData->mTemplate);
	

	// show version and ip addr
	XShowStartMenu();
	sleep(3);

#if (defined dXCustomer_IptvDrmd || defined dXCustomer_XiAn5L)
	char pTempStr[8]={0};
	if(XSectionGetValue(dXSetupConfigFile, "mirror", "open", pTempStr, sizeof(pTempStr))==0)
	{
		tXShowMirrorOpen=atoi(pTempStr);
	}
	printf("Jamon ==>> show mirro open : %d \n", tXShowMirrorOpen);

	// load qt application 
	XIptvDrmdInit();
#endif
	

#ifdef dXCustomer_Image
	tXEnterScreenSave=1;
	XSurface *pTempStartSurface=Szbh_LoadPic("/usrdata/appdir/start.png", dXPicType_Png);
	if(pTempStartSurface)
	{
		Szbh_LayerShowSurface(dXLayerId_Bottom, pTempStartSurface, 0, 0);
		Szbh_LayerRender(dXLayerId_Bottom);

		Szbh_SurfaceDestroy(pTempStartSurface);
	}else
	{
		X2DRect tTempClearRect={0, 0, dXScreenWidth, dXScreenHeight};
		Szbh_LayerClear(dXLayerId_Bottom, tTempClearRect, 0xffffff00);
		Szbh_LayerRender(dXLayerId_Bottom);
	}
#else
	if(strncmp(argv[1], "udp://", 6)==0)
	{
		tXLocalFilePlay=false;
		XScreenSaveEnterOrNot(true);
	}else
	{
		tXLocalFilePlay=true;
	#if (defined dXCustomer_IptvDrmd || defined dXCustomer_XiAn5L)
		XScreenSaveEnterOrNot(true);
	#else
		XShowMainMenu(pXCurMediaData->mTemplate);
		XShowScrollText(dXScrollText);
	#endif
	}

#ifdef dXCustomer_XiAn5L
	XLocalPlayerInit();
#else
	XPlayVideo(argv[1]);
#endif

#endif
printf("Damon ==> test app \n");

	usleep(1000*500);
#ifndef dXCustomer_Image
	#ifdef dXCustomer_XiAn5L
		XPiscProcInit(tXDeviceId);
	#else
		int tTemretCtrl = -1;
		int tTemretCtrlcnt = 0;
		if((tTemretCtrl < 0)&&(tTemretCtrlcnt < 5))
		{
			tTemretCtrl = XCtrlProtocolInit();
			tTemretCtrlcnt++;
		}
		else
		{
			log_save(dXLogLevel_Error, "XCtrlProtocolInit failed [%s][%d]\n", __FUNCTION__, __LINE__);
			exit(-1);
		}
		
		int tTemretXdevice = -1;
		int tTemretXdevicecnt = 0;
		if((tTemretXdevice < 0)&&(tTemretXdevicecnt < 5))
		{
			tTemretXdevice = XDeviceStateInit();
			tTemretXdevicecnt++;
		}
		else
		{
			log_save(dXLogLevel_Error, "XCtrlProtocolInit failed [%s][%d]\n", __FUNCTION__, __LINE__);
			exit(-1);
		}
	#endif
#endif


	int tTemretXPTU = -1;
	int tTemretXPTUCnt = 0;

	if((tTemretXPTU < 0 )&&(tTemretXPTUCnt < 5))
	{
		tTemretXPTU = XPTUControlInit();
		tTemretXPTUCnt++;
	}
	else
	{
		log_save(dXLogLevel_Error, "XPTUControlInit failed [%s][%d]\n", __FUNCTION__, __LINE__);
		exit(-1);
	}
	XGetTick(&tXScreenSavePrevTick);


	XSetupMenuInit();
	Szbh_IrStart(MyIrEventCallBack);


#ifdef dXHwcWdgPin
	Szbh_GpioSetOutput(dXHwcWdgPin);
#endif


#ifdef dXUseAdjustBacklight
	unsigned char tTempVal=0x18;
	if(Szbh_I2cWriteData(2, 0x52, 0, 0, &tTempVal, 1)!=0)
		printf("\n\nDamon ==> write data failed !\n\n");
#endif

	memset(&tXColorTest, 0, sizeof(tXColorTest));
	tXColorTest.mTick=1;

	int tTempRet=0;

#ifdef dXCustomer_IptvDrmd
	while(1)
	{
		XMediaCtrl tTempCtrlMsg;
		tTempRet = XCtrlProtocolMsgGet(&tTempCtrlMsg);
		if(tTempRet==0)
		{
		/*	printf("Damon ==> trrige type : 0x%x \n", tTempCtrlMsg.mTriggerFlag);
			printf("Damon ==> date :  %04d-%02d-%02d  %02d:%02d:%02d \n", tTempCtrlMsg.mYear+2000, tTempCtrlMsg.mMonth, tTempCtrlMsg.mDay,
				tTempCtrlMsg.mHour, tTempCtrlMsg.mMinute, tTempCtrlMsg.mSeconds);*/
			XProcessIptvDrmdCtrlMsg(&tTempCtrlMsg);
		}
	
		tTempRet=XPTUControlMessageHandle();
		if(tTempRet!=0)
		{
			printf("Damon ==> process ptu msg failed !\n");
		}

		XTickHandle();
		
		usleep(1000*10);
	}
#elif (defined dXCustomer_XiAn5L)
	while(1)
	{
	//	XMediaCtrl tTempCtrlMsg;
	//	tTempRet = XCtrlProtocolMsgGet(&tTempCtrlMsg);

	
		tTempRet=XPTUControlMessageHandle();
		if(tTempRet!=0)
		{
			printf("Damon ==> process ptu msg failed !\n");
		}

		XTickHandle();
		
		usleep(1000*10);
	}
#else
	while(1)
	{
	#if 0
		char tTempC=getchar();
		switch(tTempC)
		{
			case '2':
				XChangeTemplate(2, false);
				break;
			case '3':
				XChangeTemplate(3, false);
				break;
			case '4':
				XChangeTemplate(4, false);
				break;
			case 's':
				{
				/*	X2DRect tTempRect={100, 200, 1280, 720};
					tTempRet=XScreenSnapshotEncodeBmp(dXLayerId_Bottom, 
						tTempRect, "/usrdata/snapshot.bmp");*/
					tTempRet=XScreenSnapshotOutFile(dXPicType_Bmp, "/usrdata/snapshop.bmp");
					if(tTempRet!=0)
						printf("Damon ==> snapshot failed !\n");
				}
				break;
		}
	#endif

	#ifndef dXCustomer_Image
		XMediaCtrl tTempCtrlMsg;
		tTempRet = XCtrlProtocolMsgGet(&tTempCtrlMsg);
		if(tTempRet==0)
		{
		/*	printf("Damon ==> trrige type : 0x%x \n", tTempCtrlMsg.mTriggerFlag);
			printf("Damon ==> date :  %04d-%02d-%02d  %02d:%02d:%02d \n", tTempCtrlMsg.mYear+2000, tTempCtrlMsg.mMonth, tTempCtrlMsg.mDay,
				tTempCtrlMsg.mHour, tTempCtrlMsg.mMinute, tTempCtrlMsg.mSeconds);*/
			XProcessCtrlMsg(&tTempCtrlMsg);
		}
	#endif

		tTempRet=XPTUControlMessageHandle();
		if(tTempRet!=0)
		{
			printf("Damon ==> process ptu msg failed !\n");
		}

		XTickHandle();

	#if 0
		XIrEvent tTempIrEvent;
		if(Szbh_IrEventGet(&tTempIrEvent)==0)
		{
			MyIrEventCallBack(tTempIrEvent.mCode, tTempIrEvent.mEvent);
		}
	#endif

		usleep(1000*10);
	}
#endif

	return 0;
}


