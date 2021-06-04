

#ifndef __X_CONTROL_PROTOCOL_H__
#define __X_CONTROL_PROTOCOL_H__


typedef struct{
	unsigned char mHead;
	unsigned char mPackageLen;
	unsigned char mTriggerFlag;
	unsigned char mStartStation;
	unsigned char mEndStation;
	unsigned char mCurStation;
	unsigned char mNextStation;
//	unsigned char mNotStopCode;
	unsigned char mOpenTriggerFLag;
	unsigned char mEmergencyCode;
	unsigned char mVolume;
	unsigned char mTemplate;
	unsigned char mLineNum;
	unsigned char mYear;
	unsigned char mMonth;
	unsigned char mDay;
	unsigned char mHour;
	unsigned char mMinute;
	unsigned char mSeconds;
	unsigned char mScreenLight;
	unsigned char mEmergencyType;
	unsigned short mEmergencyLen;
	unsigned char mTail;
	ushort pMsgData[512];
}XMediaCtrl;

int XCtrlProtocolInit(void);
int XCtrlProtocolMsgGet(XMediaCtrl *pOutputMsg);
int XCtrlProtocolGetLocalIp(char *pInputEthInf, char *pOutputIp);


#endif


