
#ifndef __X_SETUP_MENU_H__
#define __X_SETUP_MENU_H__

enum{
	dXIrCode_Power=0xba45ff00,
	dXIrCode_Menu=0xbb44ff00,
	dXIrCode_Up=0xbf40ff00,
	dXIrCode_Down=0xe619ff00,
	dXIrCode_Left=0xf807ff00,
	dXIrCode_Right=0xf609ff00,
	dXIrCode_Enter=0xea15ff00,
	dXIrCode_Exit=0xbc43ff00,
	dXIrCode_Empty=0xe916ff00,
	dXIrCode_Num0=0xf20dff00,
	dXIrCode_Num1=0xf30cff00,
	dXIrCode_Num2=0xe718ff00,
	dXIrCode_Num3=0xa15eff00,
	dXIrCode_Num4=0xf708ff00,
	dXIrCode_Num5=0xe31cff00,
	dXIrCode_Num6=0xa55aff00,
	dXIrCode_Num7=0xbd42ff00,
	dXIrCode_Num8=0xad52ff00,
	dXIrCode_Num9=0xb54aff00
};


int XSetupGetLocalIp(char *pInputEthInf, char *pOutputIp);
int XSetupGetLocalNetMask(char *pInputEthInf, char *pOutputNetMask);
int XSetupGetLocalMacAddr(char *pInputEthInf, char *pOutputMac);


int XSetupMenuInit(void);
int XSetupMenuHandle(unsigned int tInputIrCode);
int XSetupConfigInit(unsigned char *pOutputVol, int *pOutputDeviceId);


#endif

