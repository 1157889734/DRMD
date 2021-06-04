#ifndef __DRMD_UI_H__
#define __DRMD_UI_H__
int DrmdInit();
int DrmdReadData(unsigned char *buf,int len);
int DrmdWriteData(unsigned char *buf,int len);
int target(unsigned char *dst,int type,int cmd,unsigned char *buf,int bufLen);

//#define DRMD_MIRROR

#define MAIN2DRMD 0x03
#define DRMD2MAIN 0x04

#define SEND_DRMD_STATUS 0x00  //  ·¢ËÍ×´Ì¬¸øMAIN

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
#define SET_UP_DOWN_CHG 0x12

#define VERSION_MAX 1
#define VERSION_MIN 0

int getPacket(unsigned char *buf,int length);
#endif
