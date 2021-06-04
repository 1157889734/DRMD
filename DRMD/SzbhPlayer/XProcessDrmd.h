

#ifndef __X_PROCESS_DRMD_H__
#define __X_PROCESS_DRMD_H__


#define SET_IP_OSD 0x01
#define SET_MAC 0x02
#define SET_LIGHT 0x03
#define SET_CONTRAST 0x04
#define SET_LCD 0x05
#define SET_485 0x06
#define SET_PWM 0x07

#define LCD_IDLE 0
#define LCD_ON 1
#define LCD_OFF 2   

#define RS485_ON 1
#define RS485_OFF 0  

#define LOCAL_START_OFFSET 0
#define LOCAL_TYPE_OFFSET 1
#define LOCAL_CMD_OFFSET 2
#define LOCAL_LEN_OFFSET 3
#define LOCAL_DATA_OFFSET 4

#define DRMD_STATUS 0x00



#define IPTV2DRMD 0x03
#define DRMD2IPTV 0x04

#define SEND_DRMD_STATUS 0x00  //  发送状态给MAIN

#define SET_IDLE 0x00
#define SET_LEAVE 0x01
#define SET_PRE 0x02
#define SET_ARRIVED 0x03
#define SET_OPEN 0x04
#define SET_CLOSE 0x05
#define SET_EME 0x06
#define SET_NOSIGNAL 0x07
#define SET_CLOSE_LCD 0x08
#define SET_UPDATE_ING 0x09
#define SET_UPDATE_OK 0x0A
#define SET_UPDATE_ERROR 0x0B
#define SET_DOOR_ERROR 0x0C
#define SET_DOOR_OK 0x0D
#define SET_IP_CONFLICT_ON 0x0E
#define SET_IP_CONFLICT_OFF 0x0F
#define SET_CYCLE_TEST_ON 0x10
#define SET_CYCLE_TEST_OFF 0x11
#define SET_HAVE_SIGNAL	0x12


#define CHG_STATION 1
#define CHG_RUNDIR 2
#define CHG_DOOR 3
#define CHG_DOOR_ERR 4
#define CHG_LEAVE 5
#define CHG_ARRIVE 6
#define CHG_SYNC 7
#define CHG_EME  8
#define CHG_DIS 9
#define CHG_UI  10


typedef struct
{
    unsigned char leaveFlag;
    unsigned char arrivedFlag;

    unsigned char startId;
    unsigned char currentId;
    unsigned char nextId;
    unsigned char endId;

    unsigned char emeId;
    unsigned char pisc_dev_id;
    unsigned char masterFlag;
    unsigned char key;    
    unsigned char openleft;
    unsigned char openright;
    unsigned char openleftPulse;
    unsigned char openrightPulse;
    unsigned char up;
    unsigned char down;
    unsigned char closeleft;
    unsigned char closeright;
    unsigned char doorerror;
    unsigned char skipActive;
    unsigned char upDownFlag;

    unsigned char skipStation[8];

    unsigned char chgFlag;
    unsigned char preFlag;
    unsigned char nosignal;       // wr 
    unsigned char closeLcd;      // wr 
    unsigned char devId;
    unsigned char doorflag;      // 1 开门，0关门     
    unsigned char year_l;
    unsigned char year_h;
    unsigned char month;
    unsigned char date;
    unsigned char hour;
    unsigned char minute;
    unsigned char second;

	unsigned short curDistance;
	unsigned short nextDistance;
	unsigned short speed;

    int leaveTime;
    int nowTime;
    int netTime;          // wr 
    int rs485Time;    
    int crcTime;
    pthread_rwlock_t lock;
}mainDevDataSt;

typedef struct
{
    unsigned char frame_head;   // 0x7e
    unsigned char len;                  // 0x80
    unsigned char signal_runst; // bit:  到站停稳触发  离站触发 开门触发 关门触发 emi jump  到站触发 预报站触发
    unsigned char start;
    unsigned char end;
    unsigned char current;
    unsigned char next; 
    unsigned char jump; 
    unsigned char emergency; 
    unsigned char signal_rundoor;  // bit:上行 下行 开左侧门 开右侧门 上下行有效 主备 激活 越站有效
    unsigned char signal_piscFlag;  // bit: 中控标识位(0是司机室1；1是司机室2)  6 5 4 3 2 1 0
    
    unsigned char reserve_Byte10;
    unsigned char reserve_Byte11;
    unsigned char reserve_Byte12;
    unsigned char reserve_Byte13; // LCD屏音量
    unsigned char reserve_Byte14;// 模板号
    unsigned char reserve_Byte15;// 线路号
    
    unsigned char year;
    unsigned char month;
    unsigned char date;
    unsigned char hour;
    unsigned char minute;
    unsigned char second;

    unsigned char reserve_Byte22_32[11]; //  司机室1 2 客室 1-8 室外温度
    unsigned char skip1_32[4];                      // 越站
    unsigned char skip33_64[4];

    unsigned char drmdLight;    //41 亮度
    
    unsigned char reserve_Byte42_125[84];
    unsigned char reserve_Byte126_128[3];   // 紧急信息类型 紧急信息长度
    unsigned char frame_end;
}__attribute__((packed))VvsRecvSt;


int XProcessCommunicateInit(int tInputDevId);
int XProcessSendData(char *pInputData, int tInputDataLen);
int XStationMsgProc(unsigned char *buf,int len);
void XPiscDataParse(unsigned char *data_buf,int len);
void set_drmd_display_st(unsigned char status);
int getChgSt(void);


int setCmd(int val);
int sendDrmdPacket();


#endif

