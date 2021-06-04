#ifndef __PARAM_H__
#define __PARAM_H__

#define PACK_ALIGN __attribute__((packed))

#define DF_STATION_NUM 50
#define DRMD_MIRROR

typedef struct
{
    int r;
    int g;
    int b;
    int a;
} PACK_ALIGN rgba_t;

typedef struct
{
    int x;
    int y;
    int w;
    int h;
} PACK_ALIGN rect_t;

typedef struct
{
    rect_t rectUp;
    //rect_t rectDown;
    rgba_t rgba;
    int fontSize;
} PACK_ALIGN label_info_t;

typedef struct
{
    int stationId;
    int lineNo;
    rect_t rect;
    rect_t rectDown;
} PACK_ALIGN chg_t;

typedef struct
{
    label_info_t widget;                        // 窗口参数
    label_info_t welcomeLable;
    label_info_t updateLable;
    label_info_t background;

    label_info_t SNOutRangeColor;               // 站名颜色参数
    label_info_t SNPassColor;
    label_info_t SNCurrentColor;
    label_info_t SNNextColor;
    label_info_t SNNextNColor;

    label_info_t dateLabel;                     // 时间日期参数
    label_info_t weekChLabel;
    label_info_t weekSplitLabel;
    label_info_t weekEnLabel;
    label_info_t timeLabel;

    label_info_t MDstText;
    label_info_t MDstSNCh;
    label_info_t MDstSNEn;

    label_info_t MNextText;
    label_info_t MNextSNCh;
    label_info_t MNextSNEn;

    label_info_t MCurrentText;

    label_info_t DoorErr;

    label_info_t Imetion;

    int MTrainNum;

} PACK_ALIGN public_info_t;

typedef struct
{
    label_info_t background;
    label_info_t chSN[DF_STATION_NUM];          // 站名文本参数
    label_info_t enSN[DF_STATION_NUM];
    label_info_t site[DF_STATION_NUM];          // 站点图标信息
    label_info_t siteFlash;
    label_info_t siteTime;
    label_info_t track[DF_STATION_NUM];         // 轨道图标信息
    label_info_t jump[DF_STATION_NUM];
    label_info_t trackFlash;
    chg_t chg[DF_STATION_NUM];                  // 所有换乘信息参数
    chg_t sp[DF_STATION_NUM];

    int stationChgNum[DF_STATION_NUM];          // 每个站的换乘个数
    int stationspNum[DF_STATION_NUM];

    int VSNOffsetEn;
    int VSNOffsetCh;
    int siteFlashFrameNum;
    int siteChgFlashFrameNum;
    int trackUpFrameNum;
    int trackDownFrameNum;

} PACK_ALIGN all_info_t;

typedef struct
{

//-----------------预到站或者离站局部放大参数---------------------------------------
    label_info_t background;

    int leaveStationNum;                        // 站名个数
    int trackNum;
    int preOpenFlashNum;

    label_info_t preDstChSN;
    label_info_t preDstEnSN;

    label_info_t chSN[10];                      // 站名文本参数
    label_info_t enSN[10];
    label_info_t chNextSN;
    label_info_t enNextSN;

    label_info_t StationChgM[10];                     // 换乘中间坐标参考参数，多个换乘参考此位置左右移动
    label_info_t site[DF_STATION_NUM];          // 站点图标信息
    label_info_t siteTime;
    label_info_t track[DF_STATION_NUM];         // 轨道图标信息
    label_info_t trackFlash;
    label_info_t doorOpenImg;
    label_info_t mationImg;
    label_info_t doorOpenText;
    label_info_t doorOpenImgM;
    label_info_t doorOpenTextM;
    label_info_t doorCloseImg;
    label_info_t doorCloseText;
    label_info_t doorCloseImgM;
    label_info_t doorCloseTextM;
    label_info_t preNextText;
    label_info_t preExit;
    label_info_t chg[10];
    label_info_t sp[10];

    label_info_t currentSNch;
    label_info_t currentSNen;

    label_info_t nextSNch;
    label_info_t nextSNen;

//-----------------到站开关门参数---------------------------------------
    label_info_t arrSite[DF_STATION_NUM];          // 站点图标信息
    label_info_t arrBackground;
    label_info_t arrOpenChSN;                   // 站名文本参数
    label_info_t arrOpenEnSN;

    label_info_t arrCloseChSN;                  // 站名文本参数
    label_info_t arrCloseEnSN;
    label_info_t arrCloseSNSplit;

    label_info_t arrNextCh;                     // 站名文本参数
    label_info_t arrNextEn;

    label_info_t arrNextTextCh;                 // 站名文本参数
    label_info_t arrNextTextEn;

    label_info_t arrSNText;
    label_info_t arrSNDirL;
    label_info_t arrSNDirR;
    label_info_t arrOpenDirL;
    label_info_t arrOpenDirR;
    label_info_t arrCloseDirL;
    label_info_t arrCloseDirR;

    label_info_t arrOpenDoor;                   // 开门动图
    label_info_t arrOpenDoorM;
    label_info_t arrStopText;
    label_info_t arrOpenText;
    label_info_t arrOpenTextM;
    label_info_t arrRoundFlash;


    label_info_t serviceImg; //服务热线
    label_info_t TipsImg;//温馨提示
    label_info_t exportImg;//出口扶梯
    label_info_t arrCloseDoor;
    label_info_t arrCloseDoorM;
    label_info_t arrCloseText;
    label_info_t arrCloseTextM;
    label_info_t arrChgText;
    label_info_t arrChgLine;
    label_info_t arrSp;

    label_info_t arrErrDoor;
    label_info_t arrErrTextL;
    label_info_t arrErrTextR;

    label_info_t arrExit;
    label_info_t arrTrain;
    label_info_t arrCar[10];
    label_info_t curentCar[10];

    label_info_t arrVerticalBar;


    int siteFlashFrameNum;
    int siteChgFlashFrameNum;
    int trackFlashFrameNum;
    int openFlashFrameNum;
    int openToCloseFrameNum;
    int trainUpFrameNum;
    int trainDownFrameNum;
    int arrOpenFrameNum;
    int arrOpenCycleFrameNum;
    int arrCloseFrameNum;

} PACK_ALIGN part_info_t;


//main protocol
typedef struct
{
    unsigned char head;
    unsigned char type;
    unsigned char cmd;
    unsigned char len;
    unsigned char start;
    unsigned char current;
    unsigned char next;
    unsigned char end;
    unsigned char door;
    unsigned char doorErr;    
    unsigned char emeId;
    unsigned char mirror;
    unsigned char chgSt;
    unsigned char skipId[8];
    //unsigned char keySide;
    unsigned short cur_distance;
    unsigned short next_distance;
    unsigned short speed;
    unsigned char keySide;  // pisc_id
    unsigned char sum;
    unsigned char xorSum;
    unsigned char tail;
} PACK_ALIGN main_receive_t;


#endif
