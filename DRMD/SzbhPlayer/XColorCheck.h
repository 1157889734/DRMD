

#ifndef __X_COLOR_CHECK_H__
#define __X_COLOR_CHECK_H__

void XSetColorMode(int tInputOpenOrClose, int tInputTick, int tInputColorIdx);
void XGetColorMode(int *pOutputOpenOrClose, int *pOutputTick, int *pOutputColorIdx);
int XEnterOrCloseColorMode(int tInputEnterOrClose);


#endif

