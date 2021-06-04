
#include <stdio.h>
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
#include "SzbhApi.h"
#include <stdlib.h> 



#include "XVehiclePlayerDefine.h"
#include "XPiscProc.h"
#include "XProcessDrmd.h"


#define dXIptvSendSocket	"/root/socket/iptv2drmd.socket"
#define dXIptvRecvSocket	"/root/socket/drmd2iptv.socket"

#define DRMD_PRE_DELAY_TIME 20

static int tXRecvSocket=-1;
static int tXSendSocket=-1;
static struct sockaddr_un tXSendAddr;

extern int tXDeviceId;
extern int tXShowMirrorOpen;

static mainDevDataSt mainDevData = {0,};
static mainDevDataSt mainDevDataOld = {0,};

static pthread_mutex_t tXSendMutex;

static int XProcessSocketInit(void);

extern int XLocalPlayerSetMirror(unsigned char tInputMirror);
extern int XCheckScreenSave(void);
extern void XAdjustScreenSaveTime(unsigned int tInputSeconds);
extern void XChangeTimeLock();
extern void XChangeTimeUnlock();



void *XProcessRecvDataProc(void *pInputArg)
{
	char pTempRecvBuf[128]={0};
	int tTempReadLen=0;
	struct sockaddr_in tTempSenderAddr;
	socklen_t tTempAddrLen=sizeof(tTempSenderAddr);

	while(tXRecvSocket<0 || tXSendSocket<0)
	{
		usleep(1000*1000);
		int tXProcessSocket = -1;
		int tXProcessSocketCnt = 0;
		if((tXProcessSocket < 0)&&(tXProcessSocketCnt < 5))
		{
			tXProcessSocket = XProcessSocketInit();
			tXProcessSocketCnt++;

		}
		else
		{
			log_save(dXLogLevel_Error, "XProcessSocketInit failed [%s][%d]\n", __FUNCTION__, __LINE__);
			exit(-1);

		}
	}

	while(1)
	{
		tTempReadLen=recvfrom(tXRecvSocket, pTempRecvBuf, sizeof(pTempRecvBuf), 0, (struct sockaddr *)&tTempSenderAddr, &tTempAddrLen);
		printf("Damon ==> drmd read len : %d \n", tTempReadLen);
        if(tTempReadLen > 0)
        {
            if ((pTempRecvBuf[LOCAL_START_OFFSET] == 0xFE) && 
                 (pTempRecvBuf[LOCAL_TYPE_OFFSET] == DRMD2IPTV))
            {
                switch (pTempRecvBuf[LOCAL_CMD_OFFSET])
                {
                    case DRMD_STATUS:
                        set_drmd_display_st(pTempRecvBuf[LOCAL_DATA_OFFSET + 2]);
                    break;

                    default:
                    break;
                }
            }
        }


		
	
		usleep(1000*100);
	}

	return NULL;
}

int XProcessSendData(char *pInputData, int tInputDataLen)
{
	if(tXSendSocket<0)
		return -1;

	return sendto(tXSendSocket, pInputData, tInputDataLen, 0, (struct sockaddr *)&tXSendAddr,sizeof(tXSendAddr));
}

static int XProcessSocketInit(void)
{
	int tTempRet=-1;
	int tTempSocket=-1;
	if(tXRecvSocket<0)
	{
		struct sockaddr_un tTempRecvAddr;

		tTempSocket=socket(PF_LOCAL,SOCK_DGRAM,0);
		if(tTempSocket<0)
		{
			printf("Damon ==> create recv socket failed : [%s]\n", dXIptvRecvSocket);			
			close(tTempSocket);
			return -2;
		}

		unlink(dXIptvRecvSocket);
		bzero(&tTempRecvAddr,sizeof(tTempRecvAddr));
		tTempRecvAddr.sun_family=AF_LOCAL;
		strcpy(tTempRecvAddr.sun_path, dXIptvRecvSocket);

		tTempRet = bind(tTempSocket ,(struct sockaddr *)&tTempRecvAddr ,sizeof(tTempRecvAddr));
		if(tTempRet<0)
		{
			printf("Damon ==> bind socket failed : [%s]\n", dXIptvRecvSocket);
			close(tTempSocket);
			return -3;
		}

		tXRecvSocket=tTempSocket;
	}


	if(tXSendSocket<0)
	{
	        tTempSocket=socket(PF_LOCAL,SOCK_DGRAM,0);
	        if(tTempSocket<0)
	        {
	            printf("Damon ==> create send socket failed : [%s]\n", dXIptvSendSocket);			
				close(tTempSocket);
	            return -4;
	        }

		bzero(&tXSendAddr,sizeof(tXSendAddr));
		tXSendAddr.sun_family=AF_UNIX;
		strcpy(tXSendAddr.sun_path, dXIptvSendSocket);

		tXSendSocket=tTempSocket;
	}

	return 0;
}

int XProcessCommunicateInit(int tInputDevId)
{
	int tTemretXpics = -1;
	int tTemretXpicsCnt = 0;

	if((tTemretXpics < 0) &&(tTemretXpicsCnt < 5))
	{
		tTemretXpics = XProcessSocketInit();
		tTemretXpicsCnt++;

	}
	else
	{
		log_save(dXLogLevel_Error, "XProcessSocketInit failed  [%s][%d]\n", __FUNCTION__, __LINE__);	
		exit(-1);
	}

	memset(&mainDevData, 0, sizeof(mainDevData));
	memset(&mainDevDataOld, 0, sizeof(mainDevDataOld));
	mainDevData.devId=tInputDevId;
	pthread_rwlock_init(&mainDevData.lock, NULL);

	pthread_mutex_init(&tXSendMutex, NULL);

	pthread_t tTempHandle;
	pthread_create(&tTempHandle, NULL, XProcessRecvDataProc, NULL);
	
	return 0;
}


/*************************************/
/*************************************/
/*static unsigned char osd_lcd_st=LCD_IDLE;
static unsigned char get_osd_lcd_st(void){
	return osd_lcd_st;
}
static void set_osd_lcd_st(unsigned char status){
	osd_lcd_st = status;
}*/


static unsigned char drmd_display_st=0;
static unsigned char get_drmd_display_st(void){
	return drmd_display_st;
}


void set_drmd_display_st(unsigned char status){
	drmd_display_st = status;
}


static unsigned char head = 0xfe;
static unsigned char type = IPTV2DRMD;
static unsigned char cmd = SET_LEAVE;
static unsigned char len = 18;
static unsigned char start = 1;
static unsigned char current = 1;
static unsigned char next = 0;
static unsigned char end = 28;
static unsigned char door = 1;
static unsigned char doorErr=0;
static unsigned char chgSt=0;
static unsigned char emeId = 0;
static unsigned char mirror = 0;
static unsigned char skipId[8] = {0,0,0,0,0,0,0,0};
static unsigned short cur_distance=0;
static unsigned short next_distance=0;
static unsigned short speed=0;
static unsigned char pisc_id=1;
static unsigned char sum = 0;
static unsigned char xorSum = 0;
static unsigned char tail = 0xff;



static int target(unsigned char *dst)
{
	int i,j;
	if(NULL==dst)
	{
		perror("target");
		return -1;
	}
	i = 0;
	len = 0;
	dst[i++] = head;
	dst[i++] = type;
	dst[i++] = cmd;
	dst[i++] = 0;
	dst[i++] = start;
	dst[i++] = current;
	dst[i++] = next;
	dst[i++] = end;
	dst[i++] = door;
       dst[i++] = doorErr;
	dst[i++] = emeId;
       dst[i++] = mirror;
       dst[i++] = chgSt;
	dst[i++] = skipId[0];
	dst[i++] = skipId[1];
	dst[i++] = skipId[2];
	dst[i++] = skipId[3];
	dst[i++] = skipId[4];
	dst[i++] = skipId[5];
        dst[i++] = skipId[6];
        dst[i++] = skipId[7];

#ifdef dXCustomer_XiAn5L
	dst[i++]=(cur_distance&0xff);
	dst[i++]=(cur_distance>>8)&0xff;
	dst[i++]=(next_distance&0xff);
	dst[i++]=(next_distance>>8)&0xff;	
	dst[i++]=(speed&0xff);
	dst[i++]=(speed>>8)&0xff;
	dst[i++]=pisc_id;
#endif

	len = i-1;
	dst[3] = len;
	
	
	for(j=1,sum=0;j<=len;j++)
	{
		sum += dst[j];
	}
	dst[i++] = sum;
	for(j=1,xorSum=0;j<=len;j++)
	{
		xorSum ^= dst[j];
	}
	dst[i++] = xorSum;
	
	dst[i++] = tail;
	
	return i;
}

int setCmd(int val)
{
    if(cmd ==SET_IP_CONFLICT_ON)
    {
        if(val ==SET_IP_CONFLICT_OFF)
            cmd =val;
    }
    else
         cmd =val;
    
    return 0;
}

unsigned char getDisplayCmd(void)
{
	return cmd;
}

void setMirror(int val)
{
	mirror = val;
}

void setStartStation(int val)
{
	start = val;
}
void setCurrentStation(int val)
{
	current = val;
}
void setNextStation(int val)
{
	next = val;
}
void setEndStation(int val)
{
	end = val;
}
void setDoor(int val)
{
	door = val;
}

void setDoorErr(int val)
{
	doorErr = val;
}

void setChgSt(int val)
{
	chgSt = val;
}

void setEmeId(int val)
{
	emeId = val;
}

void setNextDistance(int val)
{
	next_distance =val;

}
int getChgSt(void)
{
	return chgSt;
}

void setNextTimeParam(unsigned short tInputCurDis, unsigned short tInputNextDis, unsigned short tInputSpeed)
{
	cur_distance=tInputCurDis;
	next_distance=tInputNextDis;
	speed=tInputSpeed;
}

void setPiscId(int val)
{
	pisc_id=val;
}
	
int setSkipBuf(unsigned char *buf,int len)
{
	int i;
	if(8!=len)
	{
		perror("len is error");
		return -1;
	}
	for(i=0;i<len;i++)
	{
		skipId[i] = buf[i];
	}
	return 1;
}

int sendDrmdPacket()
{
	int ret;
	unsigned char buf[512]={0};

	pthread_mutex_lock(&tXSendMutex);
	ret = target(buf);
	if(ret>0)
	{
		//DrmdWriteData(buf,ret);
		
		ret=XProcessSendData((char *)buf, ret);
		printf("send:cmd=%d\n",buf[2]);
	}
	pthread_mutex_unlock(&tXSendMutex);

	return ret;
}

int crcTime(unsigned char year,unsigned char month,unsigned char date,unsigned char hour,unsigned char minute,unsigned char second)
{
    //printf("----TYPE_PISC time ----- \n");
    //struct rtc_time tm;
    struct tm tm;
    struct timeval tv;
    time_t timep;
    int pisctime;

    pisctime = time((time_t*)NULL);
    //pthread_rwlock_rdlock(&mainDevData.lock); 
    //if((pisctime>mainDevData.crcTime +600)||(time_init==0))
    if((pisctime>mainDevData.crcTime +300)
        ||(mainDevData.year_l   != year)
        ||(mainDevData.month   != month)
        ||(mainDevData.date      != date)
        ||(mainDevData.hour      != hour)
        ||(mainDevData.minute   != minute))
    {                                                           
        mainDevData.year_l  = year;
        mainDevData.month  = month;
        mainDevData.date     = date;
        mainDevData.hour     = hour;
        mainDevData.minute  = minute;
        mainDevData.second  = second;
            
        //tm.tm_year = (mainDevData.year_h<<8) + mainDevData.year_l  - 1900;
        tm.tm_year = mainDevData.year_l +2000 - 1900;
        tm.tm_mon = mainDevData.month - 1;
        tm.tm_mday = mainDevData.date ;
        tm.tm_hour = mainDevData.hour;
        tm.tm_min = mainDevData.minute ; 
        tm.tm_sec = mainDevData.second ;
        

        timep = mktime(&tm);
        tv.tv_sec = timep;
        tv.tv_usec = 0;                

        if(settimeofday(&tv,(struct timezone *) 0) <0)
        {
            perror("settimeofday err \n");
        }                

        printf("now time:%4d-%02d-%02d %02d:%02d:%02d\n",tm.tm_year+1900,tm.tm_mon+1,tm.tm_mday,tm.tm_hour,tm.tm_min,tm.tm_sec);
        mainDevData.crcTime = time((time_t*)NULL);
        mainDevData.netTime = time((time_t*)NULL);
        //pthread_rwlock_unlock(&mainDevData.lock);
    }

	return 0;
}

void setDeviceId(int tInputDevId)
{
	pthread_rwlock_wrlock(&mainDevData.lock);
	mainDevData.devId=tInputDevId;
	pthread_rwlock_unlock(&mainDevData.lock);
}


void get_pisc_sta(unsigned char *data_buf,int len,int type)
{
    int temp;
    PiscRecvSt *piscRecvPacket = (PiscRecvSt *)data_buf;
//    static unsigned int door_side = 0;  // 1:right 2:left 3:both
//    static unsigned char cmdBackup=0;
      static unsigned int drmdStSyncCnt=0;

	  unsigned int Distance[35]={1476,2076,911,984,1541,2716,897,2220,894,1044,
								 1858,1402,2715,818,1700,1804,1508,1500,1356,1114,
								 895,1129,900,686,1003,1519,978,1482,1403,1283,
								 1087,1170,1032
	  						};
	  unsigned int Temptime[35]={0,3,3,2,2,3,3,3,3,3,
							 3,3,3,4,2,3,3,3,3,3,
							 2,2,3,2,2,3,3,2,3,3,
							 2,2,2,2
							};

	XCheckScreenSave();


    //鎺ユ敹鍒版纭暟鎹? 璁板綍鏈夋晥鏁版嵁鍐呭
    temp = time((time_t*)NULL);
    pthread_rwlock_wrlock(&mainDevData.lock);

    mainDevData.rs485Time = temp;     
    mainDevData.closeLcd = 0;   
    mainDevData.nosignal = 0;
    mainDevData.chgFlag = 0;
    mainDevData.leaveFlag = piscRecvPacket->signal_runst & 0x80;
    mainDevData.arrivedFlag = piscRecvPacket->signal_runst & 0x40;
    mainDevData.startId = piscRecvPacket->start;
    mainDevData.currentId = piscRecvPacket->current;
    mainDevData.nextId = piscRecvPacket->next;
    mainDevData.endId = piscRecvPacket->end;
    mainDevData.emeId = piscRecvPacket->emeId;
    mainDevData.pisc_dev_id = piscRecvPacket->frame_head.src_eq.num;
    mainDevData.masterFlag = piscRecvPacket->signal_mk & 0x80;
    mainDevData.key= piscRecvPacket->signal_mk & 0x40;
    mainDevData.openleft = (piscRecvPacket->signal_rundoor & 0x01)|(piscRecvPacket->signal_rundoor & 0x10);
    mainDevData.openright = (piscRecvPacket->signal_rundoor & 0x02)|(piscRecvPacket->signal_rundoor & 0x20);
    //mainDevData.openleftPulse= piscRecvPacket->signal_rundoor & 0x10;
    //mainDevData.openrightPulse= piscRecvPacket->signal_rundoor & 0x20;
    mainDevData.up = piscRecvPacket->signal_rundoor & 0x80;
    mainDevData.down =  piscRecvPacket->signal_rundoor & 0x40;
    mainDevData.closeleft = piscRecvPacket->signal_rundoor & 0x04;
    mainDevData.closeright = piscRecvPacket->signal_rundoor & 0x08;
	mainDevData.curDistance=piscRecvPacket->cur_distance;
	mainDevData.nextDistance=piscRecvPacket->next_distance;
	mainDevData.speed=piscRecvPacket->speed;

    pthread_rwlock_unlock(&mainDevData.lock);

/*    if((getDisplayCmd()==SET_CLOSE_LCD)||(getDisplayCmd()==SET_NOSIGNAL)
        ||(getDisplayCmd()==SET_IP_CONFLICT_OFF))
    {
        mainDevDataOld.leaveFlag = 0;
        mainDevDataOld.arrivedFlag = 0;
    }*/


    if(getDisplayCmd() != get_drmd_display_st())
    {
        if(++drmdStSyncCnt>5)
        {
            drmdStSyncCnt = 5;
            mainDevData.chgFlag = 1;
            setChgSt(CHG_SYNC);
            printf("drmd st no update\n");
        }
    }
    else
        drmdStSyncCnt = 0;


    if(((mainDevDataOld.up==0)&&(mainDevData.up))||
        ((mainDevDataOld.down==0)&&(mainDevData.down)))
    {
        setCmd(SET_LEAVE);
        mainDevData.chgFlag = 1;
        setChgSt(CHG_RUNDIR);
    }

    // 1.绔欑偣淇℃伅澶勭悊
    int startId,currentId,nextId,endId;
	int imeid;		
    //pthread_rwlock_rdlock(&mainDevData.lock);
    startId = mainDevData.startId;
    currentId = mainDevData.currentId;
    nextId = mainDevData.nextId;
    endId = mainDevData.endId;
	
    //pthread_rwlock_unlock(&mainDevData.lock);
    if(((startId<=currentId&&currentId<=nextId&&nextId<=endId)||
        (startId>=currentId&&currentId>=nextId&&nextId>=endId))&&
        (startId>0 && currentId>0&&nextId>0&&endId>0))
    {
        setStartStation(startId);
        setCurrentStation(currentId);
        setNextStation(nextId);
        setEndStation(endId);

        if((mainDevDataOld.startId  != mainDevData.startId)
            ||(mainDevDataOld.endId != mainDevData.endId))
        {
            mainDevData.chgFlag = 1;
            setChgSt(CHG_STATION);
        }
        if(((mainDevDataOld.currentId!= mainDevData.currentId)||(mainDevDataOld.nextId!= mainDevData.nextId))
            &&(mainDevData.leaveFlag||mainDevData.arrivedFlag))
        {
            mainDevData.chgFlag = 1;
            setChgSt(CHG_STATION);
        }
        //printf("start:%d current:%d next:%d end:%d\n",startId,currentId,nextId,endId);
    }

    if(((mainDevDataOld.up==0)&&(mainDevData.up))||
        ((mainDevDataOld.down==0)&&(mainDevData.down)))
    {
        setCmd(SET_LEAVE);
        mainDevData.chgFlag = 1;
        setChgSt(CHG_RUNDIR);
    }

    // 2. 绂荤珯澶勭悊
    int leaveFlag,leaveFlagOld;
    int leaveTime;

    //pthread_rwlock_rdlock(&mainDevData.lock);
    leaveFlag = mainDevData.leaveFlag;
    leaveFlagOld = mainDevDataOld.leaveFlag;
    //pthread_rwlock_unlock(&mainDevData.lock);
    if(0==leaveFlagOld && leaveFlag>0)
    {
        //璁剧疆绂荤珯骞挎挱
        leaveTime = time((time_t*)NULL);
        //pthread_rwlock_wrlock(&mainDevData.lock);
        mainDevData.leaveTime = leaveTime;
        mainDevData.preFlag = 1;  // 棰勫埌绔欒浆鎹㈡爣蹇?
        mainDevData.chgFlag = 1; // 鐘舵�佹敼鍙樻爣蹇?
        setChgSt(CHG_LEAVE);
        //pthread_rwlock_unlock(&mainDevData.lock);
        printf("leaveFlag\n");
		
		setNextTimeParam(mainDevData.curDistance, mainDevData.nextDistance, mainDevData.speed);
        setCmd(SET_LEAVE);
    }


    // 3. 棰勫埌绔欏鐞?
    #if 0
    int preFlag;
    int nowTime;
    //pthread_rwlock_rdlock(&mainDevData.lock);
    preFlag = mainDevData.preFlag;
    leaveTime = mainDevData.leaveTime;
    //pthread_rwlock_unlock(&mainDevData.lock);
    if(preFlag>0)
    {
        nowTime = time((time_t*)NULL);
        if(leaveTime+DRMD_PRE_DELAY_TIME<=nowTime)   //  pre delay time
        {
            //璁剧疆棰勫埌绔?
            //pthread_rwlock_wrlock(&mainDevData.lock);
            mainDevData.preFlag = 0;
            mainDevData.chgFlag = 1;
            //pthread_rwlock_unlock(&mainDevData.lock);
            printf("preFlag\n");
            setCmd(SET_PRE);
        }
    }
    #endif

    // 4. 鍒扮珯澶勭悊
    int arrivedFlag;
    int arrivedFlagOld;
    //pthread_rwlock_rdlock(&mainDevData.lock);
    arrivedFlag = mainDevData.arrivedFlag;
    arrivedFlagOld = mainDevDataOld.arrivedFlag;
    //pthread_rwlock_unlock(&mainDevData.lock);
    if(0==arrivedFlagOld && arrivedFlag>0)
    {
        //pthread_rwlock_wrlock(&mainDevData.lock);
        mainDevData.preFlag = 0;	
        mainDevData.chgFlag = 1;
        setChgSt(CHG_ARRIVE);
        //pthread_rwlock_unlock(&mainDevData.lock);
        setCmd(SET_ARRIVED);
        printf("arrivedFlag\n");
    }

    // 5. 寮�闂ㄤ晶澶勭悊
    int devId,pisc_id,openleft,openright,doorflag;//leftPulse,rightPulse;
    //pthread_rwlock_rdlock(&mainDevData.lock);
#ifdef dXCustomer_XiAn5L
	devId = mainDevData.devId;
#else
	devId = mainDevData.devId-10;
#endif
    pisc_id =  mainDevData.pisc_dev_id;
    openleft = mainDevData.openleft;
    openright = mainDevData.openright;
    doorflag = mainDevData.doorflag;
    //leftPulse = mainDevData.openleftPulse;
    //rightPulse = mainDevData.openrightPulse;
    //pthread_rwlock_unlock(&mainDevData.lock);   

    if((mainDevData.key>1)&&(pisc_id==2))
    {
        if((openleft==0) && (openright>0)){
            openleft = openright;
            openright = 0;
        }
        else{
            if((openleft>0) && (openright==0)){
                openright = openleft;
                openleft = 0;
            }
        }
    }

    if(openleft>0 && openright>0) // 涓や晶寮�闂?
        doorflag = 1;
    else if(openleft>0) // 寮�宸﹂棬
    {
        if(devId%2==1) // 濂囨暟
        {
            if(devId<30)
                doorflag = 0;
            else
                doorflag = 1;
        }
        else
        {             
            if(devId<30)
                doorflag = 1;
            else
                doorflag = 0;
        }
    }
    else if(openright>0)
    {
                      
        if(devId%2==1) // 濂囨暟
        {
            if(devId<30)
                doorflag = 1;
            else
                doorflag = 0;
        }
        else
        {     
            if(devId<30)
                doorflag = 0;
            else
                doorflag = 1;
        }
    }
    else
        doorflag = 0;

    //printf("doorflag:%d \n",doorflag);
    
    //pthread_rwlock_wrlock(&mainDevData.lock);
    mainDevData.doorflag = doorflag;	
    //pthread_rwlock_unlock(&mainDevData.lock);
    setDoor(doorflag);    


#ifdef dXCustomer_XiAn5L
// set mirror
	if(tXShowMirrorOpen==1)
	{
		static unsigned char tTempPrevMirror=0;
		
	  if(((startId<=currentId&&currentId<=nextId&&nextId<=endId)||
        (startId>=currentId&&currentId>=nextId&&nextId>=endId))&&
        (startId>0 && currentId>0&&nextId>0&&endId>0))
	  	{
			if((mainDevData.pisc_dev_id==1 && mainDevData.devId<30 && mainDevData.devId%2==1)
				|| (mainDevData.pisc_dev_id==1 && mainDevData.devId>30 && mainDevData.devId%2==0)
				|| (mainDevData.pisc_dev_id==2 && mainDevData.devId<30 && mainDevData.devId%2==0)
				|| (mainDevData.pisc_dev_id==2 && mainDevData.devId>30 && mainDevData.devId%2==1))
			{
				setMirror(1);
			}else
			{
				setMirror(0);			
			}

			if(tTempPrevMirror!=mirror)
			{
				if(XLocalPlayerSetMirror(mirror)==0)
				{
					tTempPrevMirror=mirror;
				}
			}
        }	
	}else
	{
		setMirror(0);
	}

	setPiscId(mainDevData.pisc_dev_id);
#endif



    // 6.鍏抽棬
    // 7.绱ф�ュ箍鎾
 
	imeid =  mainDevData.emeId;
	if(mainDevData.emeId != mainDevDataOld.emeId)
	{	
		mainDevData.chgFlag = 1;
	}	
	setEmeId(imeid);


    // 8. 闂ㄩ殧绂昏В閿佸鐞?
    int carId,doorId;
    int doorErr = 0;
        
    carId = mainDevData.devId/10 + 1;
    doorId = mainDevData.devId%10;

    if(doorId<9){
        if(piscRecvPacket->car1_8_door1_8_unlock[carId-1] &  (0x01<<(doorId-1)))
            doorErr=1;
        if (piscRecvPacket->car1_8_door_isolation[(carId-1)*2] & (0x01<<(doorId-1)))
            doorErr=1;
    }
    else{
        if((carId<=4)&&(piscRecvPacket->car1_8_door9_10_unlock[0] & (0x01<<((carId-1)*2+doorId-9))))
            doorErr=1;

        if((carId > 4)&&(piscRecvPacket->car1_8_door9_10_unlock[1] & (0x01<<((carId-5)*2+doorId-9))))
            doorErr=1;

        if (piscRecvPacket->car1_8_door_isolation[(carId-1)*2+1] & (0x01<<(doorId-9)))
            doorErr=1;
    }
    if(doorErr){
        if(mainDevData.doorerror == 0)
        {
            mainDevData.chgFlag = 1;
            setChgSt(CHG_DOOR_ERR);
        }
    }
    else
    {
        if(mainDevData.doorerror == 1)
        {
            mainDevData.chgFlag = 1;
            setChgSt(CHG_DOOR_ERR);
        }
    }
    
    setDoorErr(doorErr);
    mainDevData.doorerror = doorErr;

    // 9.瓒婄珯澶勭悊
    unsigned char  skip_buf[8];
    unsigned char  skip_cnt,skip_buf_offset,skip_bit_offset;

    for(skip_cnt=0;skip_cnt<4;skip_cnt++)
    {
        skip_buf[skip_cnt] = piscRecvPacket->skip1_32[skip_cnt];
    }

    for(skip_cnt=4;skip_cnt<8;skip_cnt++)
    {
        skip_buf[skip_cnt] = piscRecvPacket->skip33_64[skip_cnt - 4];
    }

    if(((startId<=currentId&&currentId<=nextId&&nextId<=endId)||
        (startId>=currentId&&currentId>=nextId&&nextId>=endId))&&
        (startId>0 && currentId>0&&nextId>0&&endId>0))
    {
        if((nextId >(currentId+1))&& (startId<=endId)) // up
        {
            for(skip_cnt=currentId+1;skip_cnt<nextId;skip_cnt++)
            {
                skip_buf_offset = (skip_cnt-1)/8;
                skip_bit_offset = (skip_cnt-1)%8;
                skip_buf[skip_buf_offset] = skip_buf[skip_buf_offset] |(1<<skip_bit_offset);
            }
        }

        if(((nextId+1)<currentId)&& (startId>=endId)) // down
        {
            for(skip_cnt=nextId+1;skip_cnt<currentId;skip_cnt++)
            {
                skip_buf_offset =   (skip_cnt-1)/8;
                skip_bit_offset = (skip_cnt-1)%8;
                skip_buf[skip_buf_offset] = skip_buf[skip_buf_offset] |(1<<skip_bit_offset);
            }
        }        
    }
    setSkipBuf(skip_buf, 8);
	static int tmp_flag = 1;
	int tmp_time;
	if(mainDevData.curDistance!=mainDevDataOld.curDistance || 
		mainDevData.nextDistance!=mainDevDataOld.nextDistance)
	{
		setNextTimeParam(mainDevData.curDistance, mainDevData.nextDistance, mainDevData.speed);
		if(mainDevData.nextDistance !=	mainDevDataOld.nextDistance)
		{
						
			tmp_time = (mainDevData.nextDistance * Temptime[mainDevData.currentId]) / Distance[mainDevData.currentId - 1];
			if(tmp_time < 1)
				tmp_time = tmp_time + 1;
			
			if(tmp_flag == 1)
			{
				if(tmp_time == 2)
				{					
					mainDevData.chgFlag = 1;
					tmp_flag = 0;
				}
			}
			else
			{
				if(tmp_time == 1)
				{				
					mainDevData.chgFlag = 1;
					tmp_flag = 1;					
				}

			}
		
		
		}		
	}
	
	
	
    // 10.鐘舵�佹湁鍙樺寲锛屽彂閫佷俊鍙风粰鐣岄潰鏄剧ず
    if(mainDevData.chgFlag > 0)
    {   
		sendDrmdPacket();		
        printf("sendDrmdPacket\n");
    }




    // 11.鏁版嵁鍖呭鐞嗗畬锛屽浠芥棫鏁版嵁
    //pthread_rwlock_wrlock(&mainDevData.lock);
    memcpy(&mainDevDataOld,&mainDevData,sizeof(mainDevDataOld));	
   // pthread_rwlock_unlock(&mainDevData.lock);
    
}


void XPiscDataParse(unsigned char *data_buf, int len)
{
    FRAME_HEAD *p_frame_head;
    static int time_init=0;

    p_frame_head = (FRAME_HEAD *)data_buf;

    if((len <= LEN_FRAME_HEAD) || (p_frame_head->dstn_eq.ip[0] !=tXDeviceId && p_frame_head->dstn_eq.ip[0] !=0xff )
        ||(p_frame_head->frame_start != 0x7E))
    {
        //printf("----PiscProc pakect not for drmd----- \n");
        return;
    }

    //printf("---- pakect type :%d \n",p_frame_head->src_eq.type);
    
    switch(p_frame_head->src_eq.type)
    {
        case TYPE_PISC://涓帶

            if((p_frame_head->cmd == 0x0001)&&((data_buf[LEN_FRAME_HEAD]&0x80) == 0x80))
                get_pisc_sta(data_buf, len,type);//鎺ユ敹澶勭悊涓帶鐨勬暟鎹?
                
            if(p_frame_head->cmd == 0x000a)
            {
                //printf("----TYPE_PISC time ----- \n");
                //struct rtc_time tm;
                struct tm tm;
                struct timeval tv;
                time_t timep;
                int pisctime;

                pisctime = time((time_t*)NULL);
                //pthread_rwlock_rdlock(&mainDevData.lock); 
                //if((pisctime>mainDevData.crcTime +600)||(time_init==0))
                if((pisctime>mainDevData.crcTime +300)
                    ||(mainDevData.year_l   != data_buf[LEN_FRAME_HEAD])
                    ||(mainDevData.year_h  != data_buf[LEN_FRAME_HEAD+1])
                    ||(mainDevData.month   != data_buf[LEN_FRAME_HEAD+2])
                    ||(mainDevData.date      != data_buf[LEN_FRAME_HEAD+3])
                    ||(mainDevData.hour      != data_buf[LEN_FRAME_HEAD+4])
                    ||(mainDevData.minute   != data_buf[LEN_FRAME_HEAD+5]))
                {                                                           
                    mainDevData.year_l  = data_buf[LEN_FRAME_HEAD];
                    mainDevData.year_h = data_buf[LEN_FRAME_HEAD+1];
                    mainDevData.month  = data_buf[LEN_FRAME_HEAD+2];
                    mainDevData.date     = data_buf[LEN_FRAME_HEAD+3];
                    mainDevData.hour     = data_buf[LEN_FRAME_HEAD+4];
                    mainDevData.minute  = data_buf[LEN_FRAME_HEAD+5];
                    mainDevData.second  = data_buf[LEN_FRAME_HEAD+6];
                        
                    tm.tm_year = (mainDevData.year_h<<8) + mainDevData.year_l  - 1900;
                    tm.tm_mon = mainDevData.month - 1;
                    tm.tm_mday = mainDevData.date ;
                    tm.tm_hour = mainDevData.hour;
                    tm.tm_min = mainDevData.minute ; 
                    tm.tm_sec = mainDevData.second ;
                    

                    timep = mktime(&tm);
                    tv.tv_sec = timep;
                    tv.tv_usec = 0;                

			XChangeTimeLock();
			XAdjustScreenSaveTime(timep);

                    if(settimeofday(&tv,(struct timezone *) 0) <0)
                    {
                        perror("settimeofday err \n");
                    }
			XChangeTimeUnlock();

                    printf("now time:%4d-%02d-%02d %02d:%02d:%02d\n",tm.tm_year+1900,tm.tm_mon+1,tm.tm_mday,tm.tm_hour,tm.tm_min,tm.tm_sec);
                    mainDevData.crcTime = time((time_t*)NULL);
                    mainDevData.netTime = time((time_t*)NULL);
                    //pthread_rwlock_unlock(&mainDevData.lock);
                }
                if(time_init == 0)
                    time_init = 1;
            }

            break;

        default:
            break;	
    }
	
}


int XStationMsgProc(unsigned char *buf,int len)
{
    int temp;
    VvsRecvSt *vvsRecvPacket = (VvsRecvSt *)buf;
//    static unsigned int door_side = 0;  // 1:right 2:left 3:both
//    static unsigned char cmdBackup=0;
      static unsigned int drmdStSyncCnt=0;

/*    if(get_osd_lcd_st() != LCD_ON)
        setLcd(LCD_ON);*/
//printf("Damon ==> rundoor=0x%x \n", vvsRecvPacket->signal_rundoor);
/*    if((vvsRecvPacket->signal_rundoor & 0x04)==0)  // 非主不处理
        return -1;*/

    //接收到正确数据, 记录有效数据内容
    temp = time((time_t*)NULL);
    pthread_rwlock_wrlock(&mainDevData.lock);

    mainDevData.netTime = temp;
    mainDevData.rs485Time = temp;     
    mainDevData.closeLcd = 0;   
    mainDevData.nosignal = 0;
    mainDevData.chgFlag = 0;
    mainDevData.leaveFlag = vvsRecvPacket->signal_runst & 0x01;
    mainDevData.arrivedFlag = vvsRecvPacket->signal_runst & 0x02;
    mainDevData.startId = vvsRecvPacket->start;
    mainDevData.currentId = vvsRecvPacket->current;
    mainDevData.nextId = vvsRecvPacket->next;
    mainDevData.endId = vvsRecvPacket->end;
    mainDevData.emeId = vvsRecvPacket->emergency;

    if((vvsRecvPacket->signal_piscFlag & 0x80)==0)
        mainDevData.pisc_dev_id = 1;
    else
        mainDevData.pisc_dev_id = 2;
    
    mainDevData.masterFlag = vvsRecvPacket->signal_rundoor & 0x04;
    mainDevData.key= vvsRecvPacket->signal_rundoor & 0x02;
    mainDevData.openleft = (vvsRecvPacket->signal_rundoor & 0x20)|(vvsRecvPacket->signal_rundoor & 0x20);
    mainDevData.openright = (vvsRecvPacket->signal_rundoor & 0x10)|(vvsRecvPacket->signal_rundoor & 0x10);
    //mainDevData.openleftPulse= piscRecvPacket->signal_rundoor & 0x10;
    //mainDevData.openrightPulse= piscRecvPacket->signal_rundoor & 0x20;
    mainDevData.up = vvsRecvPacket->signal_rundoor & 0x80;
    mainDevData.down =  vvsRecvPacket->signal_rundoor & 0x40;
    //mainDevData.closeleft = vvsRecvPacket->signal_rundoor & 0x04;
    //mainDevData.closeright = vvsRecvPacket->signal_rundoor & 0x08;

    pthread_rwlock_unlock(&mainDevData.lock);

    if((getDisplayCmd()==SET_CLOSE_LCD)||(getDisplayCmd()==SET_NOSIGNAL)
        ||(getDisplayCmd()==SET_IP_CONFLICT_OFF))
    {
        mainDevDataOld.leaveFlag = 0;
        mainDevDataOld.arrivedFlag = 0;
    }

    if(getDisplayCmd() != get_drmd_display_st())
    {
        if(++drmdStSyncCnt>20)
        {
            drmdStSyncCnt = 20;
            mainDevData.chgFlag = 1;
            setChgSt(CHG_SYNC);
            printf("drmd st no update\n");
        }
    }
    else
        drmdStSyncCnt = 0;

    if(((mainDevDataOld.up==0)&&(mainDevData.up))||
        ((mainDevDataOld.down==0)&&(mainDevData.down)))
    {
        setCmd(SET_LEAVE);
        mainDevData.chgFlag = 1;
        setChgSt(CHG_RUNDIR);
    }

    // 1.站点信息处理
    int startId,currentId,nextId,endId;
    //pthread_rwlock_rdlock(&mainDevData.lock);
    startId = mainDevData.startId;
    currentId = mainDevData.currentId;
    nextId = mainDevData.nextId;
    endId = mainDevData.endId;
    //pthread_rwlock_unlock(&mainDevData.lock);
    if(((startId<=currentId&&currentId<=nextId&&nextId<=endId)||
        (startId>=currentId&&currentId>=nextId&&nextId>=endId))&&
        (startId>0 && currentId>0&&nextId>0&&endId>0))
    {
        setStartStation(startId);
        setCurrentStation(currentId);
        setNextStation(nextId);
        setEndStation(endId);

        if((mainDevDataOld.startId  != mainDevData.startId)
            ||(mainDevDataOld.endId != mainDevData.endId))
        {
            mainDevData.chgFlag = 1;
            setChgSt(CHG_STATION);
        }
        if(((mainDevDataOld.currentId!= mainDevData.currentId)||(mainDevDataOld.nextId!= mainDevData.nextId))
            &&(mainDevData.leaveFlag||mainDevData.arrivedFlag))
        {
            mainDevData.chgFlag = 1;
            setChgSt(CHG_STATION);
        }
        //printf("start:%d current:%d next:%d end:%d\n",startId,currentId,nextId,endId);
    }

    if(((mainDevDataOld.up==0)&&(mainDevData.up))||
        ((mainDevDataOld.down==0)&&(mainDevData.down)))
    {
        setCmd(SET_LEAVE);
        mainDevData.chgFlag = 1;
        setChgSt(CHG_RUNDIR);
    }

    // 2. 离站处理
    int leaveFlag,leaveFlagOld;
    int leaveTime;

    //pthread_rwlock_rdlock(&mainDevData.lock);
    leaveFlag = mainDevData.leaveFlag;
    leaveFlagOld = mainDevDataOld.leaveFlag;
    //pthread_rwlock_unlock(&mainDevData.lock);
    if(0==leaveFlagOld && leaveFlag>0)
    {
        //设置离站广播
        leaveTime = time((time_t*)NULL);
        //pthread_rwlock_wrlock(&mainDevData.lock);
        mainDevData.leaveTime = leaveTime;
        mainDevData.preFlag = 1;  // 预到站转换标志
        mainDevData.chgFlag = 1; // 状态改变标志
        setChgSt(CHG_LEAVE);
        //pthread_rwlock_unlock(&mainDevData.lock);
        printf("leaveFlag\n");
        setCmd(SET_LEAVE);
    }

    // 3. 预到站处理
    #if 0
    int preFlag;
    int nowTime;
    //pthread_rwlock_rdlock(&mainDevData.lock);
    preFlag = mainDevData.preFlag;
    leaveTime = mainDevData.leaveTime;
    //pthread_rwlock_unlock(&mainDevData.lock);
    if(preFlag>0)
    {
        nowTime = time((time_t*)NULL);
        if(leaveTime+DRMD_PRE_DELAY_TIME<=nowTime)   //  pre delay time
        {
            //设置预到站
            //pthread_rwlock_wrlock(&mainDevData.lock);
            mainDevData.preFlag = 0;
            mainDevData.chgFlag = 1;
            //pthread_rwlock_unlock(&mainDevData.lock);
            printf("preFlag\n");
            setCmd(SET_PRE);
        }
    }
    #endif

    // 4. 到站处理
    int arrivedFlag;
    int arrivedFlagOld;
    //pthread_rwlock_rdlock(&mainDevData.lock);
    arrivedFlag = mainDevData.arrivedFlag;
    arrivedFlagOld = mainDevDataOld.arrivedFlag;
    //pthread_rwlock_unlock(&mainDevData.lock);
    if(0==arrivedFlagOld && arrivedFlag>0)
    {
        //pthread_rwlock_wrlock(&mainDevData.lock);
        mainDevData.preFlag = 0;	
        mainDevData.chgFlag = 1;
        setChgSt(CHG_ARRIVE);
        //pthread_rwlock_unlock(&mainDevData.lock);
        setCmd(SET_ARRIVED);
        printf("arrivedFlag\n");
    }

    // 5. 开门侧处理
    int devId,pisc_id,openleft,openright,doorflag;//leftPulse,rightPulse;
    //pthread_rwlock_rdlock(&mainDevData.lock);
#ifdef dXCustomer_XiAn5L
	devId = mainDevData.devId;
#else
	devId = mainDevData.devId-10;
#endif
    pisc_id =  mainDevData.pisc_dev_id;
    openleft = mainDevData.openleft;
    openright = mainDevData.openright;
    doorflag = mainDevData.doorflag;
    //leftPulse = mainDevData.openleftPulse;
    //rightPulse = mainDevData.openrightPulse;
    //pthread_rwlock_unlock(&mainDevData.lock);   

    if(((mainDevData.key>1)&&(pisc_id==2))
        ||((mainDevData.key==0)&&(pisc_id==1)))
    {
        if((openleft==0) && (openright>0)){
            openleft = openright;
            openright = 0;
        }
        else{
            if((openleft>0) && (openright==0)){
                openright = openleft;
                openleft = 0;
            }
        }
    }

    if(openleft>0 && openright>0) // 两侧开门
        doorflag = 1;
    else if(openleft>0) // 开左门
    {
        if(devId%2==0) // 偶数左
        {
            if(devId<=40)
                doorflag = 0;
            else
                doorflag = 1;
        }
        else
        {
            if(devId<=40)
                doorflag = 1;
            else
                doorflag = 0;
        }
    }
    else if(openright>0)
    {
                      
        if(devId%2==0) // 偶数左
        {
            if(devId<=40)
                doorflag = 1;
            else
                doorflag = 0;
        }
        else
        {     
            if(devId<=40)
                doorflag = 0;
            else
                doorflag = 1;
        }
    }
    else
        doorflag = 0;

    //printf("doorflag:%d \n",doorflag);
    
    //pthread_rwlock_wrlock(&mainDevData.lock);
    mainDevData.doorflag = doorflag;	
    //pthread_rwlock_unlock(&mainDevData.lock);
    setDoor(doorflag);    


#ifdef dXCustomer_XiAn5L
// set mirror
	if(tXShowMirrorOpen==1)
	{
		static unsigned char tTempPrevMirror=0;
	
		if((mainDevData.pisc_dev_id==1 && mainDevData.devId<30 && mainDevData.devId%2==1)
			|| (mainDevData.pisc_dev_id==1 && mainDevData.devId>30 && mainDevData.devId%2==0)
			|| (mainDevData.pisc_dev_id==2 && mainDevData.devId<30 && mainDevData.devId%2==0)
			|| (mainDevData.pisc_dev_id==2 && mainDevData.devId>30 && mainDevData.devId%2==1))
		{
			setMirror(1);
		}else
		{
			setMirror(0);			
		}

		if(tTempPrevMirror!=mirror)
		{
			if(XLocalPlayerSetMirror(mirror)==0)
			{
				tTempPrevMirror=mirror;
			}
		}		
	}else
	{
		setMirror(0);
	}

	setPiscId(mainDevData.pisc_dev_id);
#endif


    // 6.关门
    // 7.紧急广播 
    // 8. 门隔离解锁处理
    // 9.越站处理
    unsigned char  skip_buf[8];
    unsigned char  skip_cnt,skip_buf_offset,skip_bit_offset;

    for(skip_cnt=0;skip_cnt<4;skip_cnt++)
    {
        skip_buf[skip_cnt] = vvsRecvPacket->skip1_32[skip_cnt];
    }

    for(skip_cnt=4;skip_cnt<8;skip_cnt++)
    {
        skip_buf[skip_cnt] = vvsRecvPacket->skip33_64[skip_cnt];
    }

    if(((startId<=currentId&&currentId<=nextId&&nextId<=endId)||
        (startId>=currentId&&currentId>=nextId&&nextId>=endId))&&
        (startId>0 && currentId>0&&nextId>0&&endId>0))
    {
        if((nextId >(currentId+1))&& (startId<=endId)) // up
        {
            for(skip_cnt=currentId+1;skip_cnt<nextId;skip_cnt++)
            {
                skip_buf_offset = (skip_cnt-1)/8;
                skip_bit_offset = (skip_cnt-1)%8;
                skip_buf[skip_buf_offset] = skip_buf[skip_buf_offset] |(1<<skip_bit_offset);
            }
        }

        if(((nextId+1)<currentId)&& (startId>=endId)) // down
        {
            for(skip_cnt=nextId+1;skip_cnt<currentId;skip_cnt++)
            {
                skip_buf_offset =   (skip_cnt-1)/8;
                skip_bit_offset = (skip_cnt-1)%8;
                skip_buf[skip_buf_offset] = skip_buf[skip_buf_offset] |(1<<skip_bit_offset);
            }
        }        
    }
    setSkipBuf(skip_buf, 8);
	

    // 10.状态有变化，发送信号给界面显示
//   printf("Damon ==> chg flag : %d \n", mainDevData.chgFlag);
    if(mainDevData.chgFlag>0)
    {
        sendDrmdPacket();
        printf("sendDrmdPacket\n");
    }

    //11. 时间校正
    crcTime(vvsRecvPacket->year,vvsRecvPacket->month,vvsRecvPacket->date,
    vvsRecvPacket->hour,vvsRecvPacket->minute,vvsRecvPacket->second);

    // 12.数据包处理完，备份旧数据
    //pthread_rwlock_wrlock(&mainDevData.lock);
    memcpy(&mainDevDataOld,&mainDevData,sizeof(mainDevDataOld));	
   // pthread_rwlock_unlock(&mainDevData.lock);

   return 0;
}
/*******************************************************/
/*******************************************************/

