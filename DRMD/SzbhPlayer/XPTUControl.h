

#ifndef __X_PTU_CONTROL_H__
#define __X_PTU_CONTROL_H__

#define dXPTUDataLenMax	64

#define UPDATE_START 0x1500 // 升级开始
#define UPDATE_END 0x1600 // 升级结束


typedef struct {
	unsigned char mHeader;
	unsigned short mCmd;
	unsigned int mPackLen;
	unsigned int mDataLen;
	unsigned char pData[dXPTUDataLenMax];
	unsigned char mCrc;
	unsigned char mTail;
}XPTUCtrl;

int XPTUControlInit(void);
int XPTUControlMessageHandle(void);

#endif


