

#ifndef __X_CONFIG_FILE_H__
#define __X_CONFIG_FILE_H__

#define dXResourcePath	"/usrdata/disp"

#define dXSectionNameLen		64
#define dXPicNameLen		32

typedef struct	{
	char pSectionName[dXSectionNameLen];

	int mEnable;
	int mType;	// 0 unkonw, 1 string, 2 pic, 3 video
	int mMove;	// 0 static , 1 move
	int mPicType;	// 0 unkonw, 1 bmp, 2 png, 3 jpg, 4 gif
	int mXPos;
	int mYPos;
	int mWidth;
	int mHeight;

	int mFontSize;
	unsigned int mFontColor; 	// argb
	unsigned int mFontBackColor;

	char pPicName[dXPicNameLen];
}XConfigSection;

typedef struct __XSectionNode{
	XConfigSection *pData;
	struct __XSectionNode *pNext;
}XSectionNode;

typedef struct{
	XSectionNode *pHeader;
	XSectionNode *pTail;
	int mCount;
}XSectionList;


int XConfigFileInit(void);
int XConfigLoadTemplate(int tInputTemplateIdx);
int XConfigGetSectionNum(void);
XConfigSection *XConfigGetSection(char *pInputSectionName);
XConfigSection *XConfigGetSectionWithIdx(int tInputIdx);


#endif


