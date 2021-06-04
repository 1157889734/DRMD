#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "myudpforrcvcontrol.h"
#include <QMainWindow>
#include <QLabel>
#include <QFont>
#include <QPen>
#include <QTime>
#include <QTimer>
#include <QLabel>
#include <QPixmap>
#include <QWidget>
#include "myqthread.h"
#include "datadefine.h"
#include "drmd_ui.h"
#include "unistd.h"
#include "param.h"
#include "malloc.h"
#include "stdio.h"
#include "vertical_ch.h"
#include "vertical_en.h"

namespace Ui
{
    class MainWindow;
}

#define DEFINE_STATION_NUM 50

class MainWindow : public QMainWindow
{
    Q_OBJECT
    
public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
    
    void InitEnv();
    void readIpconfig(void);
    void labelInit(QLabel &label, label_info_t info_t, int type, QString str, int widgetNum, QString fontStr, bool blod);
    void publicInit();
    void allReInit();
    void allInit();
    void preInit();
    void preReInit();
    void arrInit();
    void arrReInit();
    void mirrorInfo(label_info_t *info);
    //    void allmirroInfo(all_info_t *info);
    void initDipaly();
    void imageInit();
    void screenshot(void);
    void flashProc();
    void setSiteTime(int uiIndex);
    
    void allUp();
    void allDown();
    void allProc();
    
    void preUp();
    void preDown();
    void preProc();
    
    void arrProc();
    bool checkStation(int start, int current, int next, int end, int station1, int station2);
    void setFont(QLabel &label, QString name, bool bold, int size, int r, int g, int b);
    void chgToWidget(int index);
    void ipConflictProc(int flag);
    void readLabelAttr(label_info_t *ps, QSettings *set, QString path, int labelType, QString language, int index, bool debug);
    
    /*------Ui Init parameter------*/
    /***images path***/
    QString filePath = QString("/usrdata/disp/drmd/");
    QString imageDir = QString("/usrdata/disp/drmd/images/");
    QString welcomePath = QString(imageDir + "welcome.png");
    
    QString allBgPath = QString(imageDir + "ui_all/BG/bg.png");
    QString allSitePath = QString(imageDir + "ui_all/SITE/");
    QString allTrackPath = QString(imageDir + "ui_all/TRACK/");
    QString allChgPath = QString(imageDir + "ui_all/CHANGE/line/");
    QPixmap originalPixmap;
    
    int imageInitFlag;
    int timeflag = 0;
    
    /******1. 公共控件&变量参数******/
    int widgetIndex;
    QWidget widget[10];
    QLabel  welcomeLable;
    QLabel  updateLable;
    
    int nextChgFlag;               // 下一站是否为换乘站
    
    //---标题栏共同属性控件
    QLabel dateLabel;               // 日期时间
    QLabel weekChLabel;
    QLabel weekSplitLabel;
    QLabel weekEnLabel;
    QLabel timeLabel;
    
    QLabel preDateLabel;
    QLabel preWeekChLabel;
    QLabel preWeekSplitLabel;
    QLabel preWeekEnLabel;
    QLabel preTimeLabel;
    
    QLabel arrDateLabel;
    QLabel arrWeekChLabel;
    QLabel arrWeekSplitLabel;
    QLabel arrWeekEnLabel;
    QLabel arrTimeLabel;
    
    QLabel allMDstText;             //  终点站text图片
    QLabel preMDstText;
    QLabel arrMDstText;
    
    QLabel allMDstSNCh;             //  终点站SN
    QLabel allMDstSNEn;
    QLabel preMDstSNCh;
    QLabel preMDstSNEn;
    QLabel arrMDstSNCh;
    QLabel arrMDstSNEn;
    
    QLabel allMDstTextCh;             //  终点站Text
    QLabel allMDstTextEn;
    QLabel preMDstTextCh;
    QLabel preMDstTextEn;
    QLabel arrMDstTextCh;
    QLabel arrMDstTextEn;
    
    QLabel allMNextText;          //  下一站text
    QLabel preMNextText;
    QLabel arrMNextText;
    
    QLabel allMNextSNCh;            //  下一站SN
    QLabel allMNextSNEn;
    QLabel allMNextSNEn2;
    QLabel preMNextSNCh;
    QLabel preMNextSNEn;
    QLabel preMNextSNEn2;
    QLabel arrMNextSNCh;
    QLabel arrMNextSNEn;
    QLabel arrMNextSNEn2;
    
    QLabel allMCurrentText;         //  当前站text图片
    QLabel preMCurrentText;
    QLabel arrMCurrentText;
    
    /******2. all ui ******/
    QLabel allBgLabel;
    
    VERTICALCH *allSNPassCh[50];        // 站名控件
    VERTICALCH *allSNNextCh[50];
    VERTICALEN *allSNPassEn[50];
    VERTICALEN *allSNNextEn[50];
    VERTICALEN *allSNPassEn2[50];
    VERTICALEN *allSNNextEn2[50];
    
    QLabel *allSite[50];
    QLabel allNotRun;
    QLabel *allSiteTime[50];
    QLabel *allTrack[50];
    QLabel *allJump[50];
    QLabel *allStart[50];
    QLabel *allEnd[50];
    QLabel *allChg[50];
    QLabel *allSp[50];
    QLabel allTrackFlash;
    QLabel allSiteFlash;
    QLabel allDoorErr;
    QLabel allImetion;
    QLabel allFlowLed1;
    QLabel allFlowLed2;
    
    /*###2 pre arrive ui */
    QString preBgPath = QString(imageDir + "ui_pre/BG/partBg.png");
    QLabel preBgLabel;
    
    QLabel preNextSNCh;
    QLabel preNextSNEn;
    
    QLabel *preSNCh[10];
    QLabel *preSNEn[10];
    QLabel *preSNEn2[10];
    QLabel *preTrack[10];
    QLabel preNotRun;
    QLabel *preSite[10];
    QLabel *preJump[10];
    QLabel *preSiteTime[10];
    
    int preSiteSt;
    QLabel preSiteFlash;
    
    QString preSitePath = QString(imageDir + "ui_pre/SITE/");
    QString preTrackPath = QString(imageDir + "ui_pre/TRACK/");
    
    QString preChgPath = QString(imageDir + "ui_pre/CHANGE/line/");
    QLabel preChg[20];                     // 换乘数字图标
    QLabel preSp[20];
    
    QLabel premationImg;                   //预到站二维码区域
    
    QLabel preOpenImg;                     // 预到开门提示
    QLabel preOpenText;
    QLabel preOpenImgM;                     // 预到开门提示
    QLabel preOpenTextM;
    QLabel preCloseImg;                    // 预到关门提示
    QLabel preCloseText;
    QLabel preCloseImgM;                    // 预到关门提示
    QLabel preCloseTextM;
    QLabel preNextText;
    QLabel preExit;
    QLabel preDoorErr;
    QLabel preImetion;

    
    int preSnIndex = 0;
    
    /*###3 arrive ui init*/
    QString arrSitePath = QString(imageDir + "ui_pre/SITE/");
    QLabel arrBgLabel;
    QLabel arrChgText;
    QLabel arrChgLine;
    QLabel arrSp;
    
    //QLabel arrMDstTextCh;
    //QLabel arrMDstTextEn;
    QLabel arrSNText;
    QLabel arrOpenDoor;
    QLabel arrOpenText;
    QLabel arrOpenDoorM;
    QLabel arrOpenTextM;
    
    QLabel arrChSN;
    QLabel arrEnSN;
    
    QLabel arrStopText;
    
    QLabel serviceImg;  //服务热线
    QLabel TipsImg;     //温馨提示
    QLabel exportImg;   //出口扶梯
    QLabel arrCloseDoor;                     // 关门提示
    QLabel arrCloseText;
    QLabel arrCloseDoorM;
    QLabel arrCloseTextM;
    QLabel arrDoorErr;
    QLabel arrImetion;

    
    QLabel arrExit;
    QLabel arrTrain;
    QWidget arrTrainWidget;
    QLabel arrCar[10];
    QLabel curentCar[10];
    int carDispFlag;
    int car_chg_time;
    int carDispSt;
    int tmpAllSite;
    int tmpPreSite;
    
    /*drmd info ui */
    QLabel ipLabel;
    QLabel ipConflictLabel;
    QLabel macLabel;
    QLabel versionLabel;
    QLabel carIdLabel;
    QLabel devIdLabel;
    
    /*color test */
    QLabel holeDisp;
    QLabel grayScale16[16];
    QLabel grayScale32[32];
    QLabel grayScale64[64];
    unsigned char cycleMode;
    unsigned char cycleTime;
    unsigned char cycleIndex;
    unsigned char colorDispIndex;
    unsigned char colorDispUpDown;
    
    /*public parameter*/
    int updateFlag = 0;
    int doorIndex = 0;
    int startId = 1;
    int currentId = 1;
    int nextId = 2;
    int endId = 34;
    int StationMaxNumber = DEFINE_STATION_NUM;
    int gRunDir = 1;
    unsigned char run_st;
    int movieIndex = 0;
    int movieFrameNum = 0;
    bool doorFlag = false;
    bool doorCycleFlag = false;
    bool preFlag = false;
    bool arrivedFlag = false;
    bool mirrorFlag = false;
    BYTE skipStation[10];
    
    int arrToLeaveFlag = 0;
    int leaveSt; // 离站状态，应用与多个离站界面循环切换
    int leave_chg_time; // 离站界面之间切换时间
    int arrOpenFlag = 0; // 到站开门标志
    int change_time;
    int PrechageTime;
    
private:
    Ui::MainWindow *ui;
    
    QString movieAllTrackFile;
    QString moviePreTrackFile;
    QTimer *psysTimer;
    QMovie *movieAllTrack;
    QMovie *moviePartDoor;
    QMovie *moviePreTrack;
    QMovie *movieArrDoor;
    QMovie *movieArrFlash;
    QMovie *arrTrainMovie;
    QMovie *arrCarMovie;
    
    QStringList weeklist;
    QStringList weekchlist;
    
    int stationNum;                        // 当前的最大站号
    int ip4;
    QString macStr;
    
    QList<QString>stationChName;
    QList<QString>stationEnName;
    QList<QString>stationEnName2;
    QList<int>stationEnDispLines;
    
    QList<int>tmpSiteTime;
    QList<int>siteTime;
    QList<int>distance;


    
    //    QList<QString>changeLineCh;
    //    QList<QString>changeLineEn;
    
protected:
signals:

private slots:
    //void syndistance();
    void running_trigger();
    void sysproc();
    void setWelcome();
    void setInfo();
    void setColorTest(BYTE mode, BYTE time, BYTE index);
    void setUpdate(int update);
    void MainDisplayType(BYTE);
    void TimeoutProc();
    void dispArrCar();
    
private:
    class MyUdpforRcvControl mycontroludp;
    class myqthread mytimer;
    class myqthread mytimer2;
    class myqthread mytimer3;
    
};

#endif // MAINWINDOW_H
