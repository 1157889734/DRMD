
#ifndef __X_VEHICLE_PLAYER_H__
#define __X_VEHICLE_PLAYER_H__


#define dXSetupConfigFile	"/usrdata/setup.ini"

#define dXSetupIpSectionName	"ip"
#define dXSetupIpKeyName		"IpAddr"
#define dXSetupMaskKeyName	"NetMask"
#define dXSetupMacKeyName	"MacAddr"

#define dXSetupDispSectionName		"disp"
#define dXSetupBrightnessKeyName		"Brightness"
#define dXSetupContrastKeyName		"Contrast"
#define dXSetupBacklightKeyName		"Backlight"
#define dXSetupVolumeKeyName		"Volume"
#define dXSetupScreenSaveTimeoutKeyName		"ScreenSaveTimeout"
#define dXSetupScreenCloseTimeoutKeyName		"ScreenCloseTimeout"


#define dXGpioNumRunLed			(5*8+0)
#define dXGpioNumLcdEn			(5*8+1)
#define dXGpioNumLcdPower		(5*8+3)
#define dXGpioNumMute			(6*8+3)


//#define dXUseFfmpegTsDemux

#define dXUseEsPlayer

//#define dXUseNtpAdjustTime

#if	0		// NanNing2#
#define dXCustomer_NanNing2L
#define dXScreenWidth	1366
#define dXScreenHeight	768
#define dXUseTsVideoCheck
#define dXNetmask	"255.255.255.0"
#define dXScreenSaveTimeOut	(1*1000*60)	//ms
#define dXLcdCloseTimeOut		(10*1000*60)	//ms
#define dXScrollText	"欢迎乘坐南宁地铁2号线！"
#define dXDeviceStatusProtocol		"V2.1"
#elif 0		// NanNing4#
#define dXCustomer_NanNing4L
#define dXScreenWidth	1366
#define dXScreenHeight	768
#define dXUseTsVideoCheck
#define dXNetmask	"255.255.255.0"
#define dXScreenSaveTimeOut	(1*1000*60)	//ms
#define dXLcdCloseTimeOut		(10*1000*60)	//ms
#define dXScrollText	"欢迎乘坐南宁地铁4号线！"
#define dXDeviceStatusProtocol		"V2.1"
#elif 0		// ShangHai18#
#define dXCustomer_ShangHai18L
#define dXScreenWidth	1920
#define dXScreenHeight	1080
#define dXUseTsVideoCheck
#define dXNetmask	"255.255.0.0"
#define dXScreenSaveTimeOut	(1*1000*60)	//ms
#define dXLcdCloseTimeOut		(10*1000*60)	//ms
#define dXScrollText	"欢迎乘坐上海地铁18号线！"
#define dXDeviceStatusProtocol		"V2.2"
#elif 0
#define dXCustomer_IptvDrmd
#define dXSoftwareVersion		"1.4"
#define dXScreenWidth	1920
#define dXScreenHeight	1080
#define dXUseTsVideoCheck
#define dXNetmask	"255.255.255.0"
#define dXScreenSaveTimeOut	(1*1000*60)	//ms
#define dXLcdCloseTimeOut		(10*1000*60)	//ms
#define dXScrollText	"欢迎乘坐上海地铁18号线！"
#define dXDeviceStatusProtocol		"V2.2"
#elif 1	// XiAn5# and KunMing5#
#define dXCustomer_XiAn5L
#define dXSoftwareVersion		"1.4"
#define dXScreenWidth	1920
#define dXScreenHeight	1080
#define dXUseTsVideoCheck
#define dXNetmask	"255.255.0.0"
#define dXScreenSaveTimeOut	(10*1000*60)	//ms
#define dXLcdCloseTimeOut		(10*1000*60)	//ms
#define dXScrollText	"欢迎乘坐上海地铁18号线！"
#define dXDeviceStatusProtocol		"V2.2"
#else
#define dXCustomer_Image
#define dXScreenWidth	1920
#define dXScreenHeight	1080
#define dXScreenSaveTimeOut	(1*1000*60)	//ms
#define dXLcdCloseTimeOut		(10*1000*60)	//ms
#define dXScrollText	"公版测试程序"
#define dXDeviceStatusProtocol		"V2.2"
#endif


typedef struct{
	int mOpenOrClose;
	int mTick;	// seconds
	int mColor;
}XColorTest;

#endif

