


#ifndef __X_SCROLL_TEXT_H__
#define __X_SCROLL_TEXT_H__

int XScrollTextIsRun(void);

int XScrollTextStart(unsigned int tInputFontHandle, char *pInputString, unsigned int tInputFontColor, unsigned int tInputBackColor, X2DRect *pInputShowRect);
int XScrollTextStop(void);

void XScrollTextPause(void);
void XScrollTextResume(void);



#endif


