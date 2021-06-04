

#ifndef __X_MENU_SHOW_H__
#define __X_MENU_SHOW_H__

int XUnicodeToUtf8(ushort *pInputUnicode, int tInputUnicodeLen, unsigned char *pOutputData, int tInputDataLen, int *pOutputLen);

int XShowFontInit(const char *pInputFontName);
int XShowMainMenu(int tInputTemplateId);
int XShowStation(int tInputCurTemplate, u8 tInputTrigger, u8 tInputCurStation, u8 tInputNextStation, u8 tInputEndStation);

int XShowTime2(int tInputHour, int tInputMin, int tInputSec);
int XShowTime(char *pInputTime);
int XShowWeek(int tInputWeek);
int XShowSectionString(char *pInputSectionName, char *pInputText);

int XShowScrollText(char *pInputString);

int XShowEmergencyCode(int tInputEmergCode);
int XShowEmergencyMsg(unsigned short *pInputUnicode, int tInputDataLen);


#endif


