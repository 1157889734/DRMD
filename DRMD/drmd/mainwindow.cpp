#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QtGui>
#include <QDebug>
#include <QPalette>
#include <QTime>
#include <QTimer>
#include <QPainter>
#include <QSettings>
#include <QTabBar>
#include <QMovie>
#include <QProcess>
#include <QFileInfo>
#include <QGraphicsOpacityEffect>

#include "drmd_ui.h"
#include "unistd.h"

#define PREDO
#define PRODUCION           // 公版程序宏定义

#define WELCOME 0           // 界面索引宏定义
#define INFODISP 1
#define COLORDISP 2
#define ALL 3
#define PRE 4
#define ARR 5
#define WIDGET_NUM 6

#define LEAVE_ST_OFF 0      // 显示界面状态宏定义
#define LEAVE_ST_ALL 1
#define LEAVE_ST_PRE 2
#define LEAVE_ST_ARR 3
#define LEAVE_ST_NEX 4

#define ST_OUT 0            // 站点运行状态宏定义
#define ST_SKIP 1
#define ST_PASS 2
#define ST_CURRENT 3
#define ST_NEXT 4
#define ST_NEXTN 5
#define ST_NONE 6

#define LABEL_CHEN_TYPE 0   // 读取配置类型宏定义// example: for xCh xEn yCh yEn ...
#define LABEL_OTHER_TYPE 1

#define LABEL_TYPE_IMAGE 0  // 控件类型宏定义
#define LABEL_TYPE_IMAGE_FLASH 1
#define LABEL_TYPE_TEXT 2

#define NOT_IN_SERVICE 32    //  未开通站
#define NOT_OPEN_ID_32  32
#define NOT_OPEN_ID_33  33
#define NOT_OPEN_ID_34  34



//#define ALL_TEST  // 调试测试打开
//#define PRE_TEST

extern public_info_t gPublicInfo;
extern all_info_t gAllInfo;
extern part_info_t gPartInfo;
extern main_receive_t gMainReceive;

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    qDebug() << "MainWindow";
    qDebug() << "------Version:" << VERSION_MAX << "." << VERSION_MIN << "------";
    /*background clear*/
    ui->setupUi(this);
    QPalette pal;
    pal.setBrush(this->backgroundRole(), QBrush(QColor(0, 0, 0, 0)));
    this->setPalette(pal);
    this->setAutoFillBackground(true);
    readIpconfig();   // read ip config
    initDipaly();     // init display ui
    InitEnv();        // init trigger signal
    qDebug() << "MainWindow end";
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::readLabelAttr(label_info_t *ps, QSettings *set, QString path, int labelType, QString language, int index, bool debug)
{
    QString str, ch, en;
    int i;
    
    if (labelType == LABEL_CHEN_TYPE)
    {
        str = QString("%1%2%3").arg(path).arg("/x").arg(language);
        ps->rectUp.x = set->value(str).toInt();
        str = QString("%1%2%3").arg(path).arg("/y").arg(language);
        ps->rectUp.y = set->value(str).toInt();
        str = QString("%1%2%3").arg(path).arg("/w").arg(language);
        ps->rectUp.w = set->value(str).toInt();
        str = QString("%1%2%3").arg(path).arg("/h").arg(language);
        ps->rectUp.h = set->value(str).toInt();
        str = QString("%1%2%3").arg(path).arg("/r").arg(language);
        ps->rgba.r = set->value(str).toInt();
        str = QString("%1%2%3").arg(path).arg("/g").arg(language);
        ps->rgba.g = set->value(str).toInt();
        str = QString("%1%2%3").arg(path).arg("/b").arg(language);
        ps->rgba.b = set->value(str).toInt();
        str = QString("%1%2%3").arg(path).arg("/px").arg(language);
        ps->fontSize = set->value(str).toInt();
        ch = "Ch";
        en = "En";
        i = index;

        
        if (debug == true)
        {
            if (language.compare(ch) == 0)
            {
                str = QString("%1").arg(ps->rectUp.x);
                qDebug() << i + 1 << "ch.x:" << str;
                str = QString("%1").arg(ps->rectUp.y);
                qDebug() << i + 1 << "ch.y:" << str;
                str = QString("%1").arg(ps->rectUp.w);
                qDebug() << i + 1 << "ch.w:" << str;
                str = QString("%1").arg(ps->rectUp.h);
                qDebug() << i + 1 << "ch.h:" << str;
                str = QString("%1").arg(ps->rgba.r);
                qDebug() << i + 1 << "ch.r:" << str;
                str = QString("%1").arg(ps->rgba.g);
                qDebug() << i + 1 << "ch.g:" << str;
                str = QString("%1").arg(ps->rgba.b);
                qDebug() << i + 1 << "ch.b:" << str;
                str = QString("%1").arg(ps->fontSize);
                qDebug() << i + 1 << "ch.px:" << str;
            }
            
            if (language.compare(en) == 0)
            {
                str = QString("%1").arg(ps->rectUp.x);
                qDebug() << i + 1 << "en.x:" << str;
                str = QString("%1").arg(ps->rectUp.y);
                qDebug() << i + 1 << "en.y:" << str;
                str = QString("%1").arg(ps->rectUp.w);
                qDebug() << i + 1 << "en.w:" << str;
                str = QString("%1").arg(ps->rectUp.h);
                qDebug() << i + 1 << "en.h:" << str;
                str = QString("%1").arg(ps->rgba.r);
                qDebug() << i + 1 << "en.r:" << str;
                str = QString("%1").arg(ps->rgba.g);
                qDebug() << i + 1 << "en.g:" << str;
                str = QString("%1").arg(ps->rgba.b);
                qDebug() << i + 1 << "en.b:" << str;
                str = QString("%1").arg(ps->fontSize);
                qDebug() << i + 1 << "en.px:" << str;
            }
        }
        

    }
    else
    {
        str = QString("%1%2").arg(path).arg("/x");
        ps->rectUp.x = set->value(str).toInt();
        str = QString("%1%2").arg(path).arg("/y");
        ps->rectUp.y = set->value(str).toInt();
        str = QString("%1%2").arg(path).arg("/w");
        ps->rectUp.w = set->value(str).toInt();
        str = QString("%1%2").arg(path).arg("/h");
        ps->rectUp.h = set->value(str).toInt();
        str = QString("%1%2").arg(path).arg("/r");
        ps->rgba.r = set->value(str).toInt();
        str = QString("%1%2").arg(path).arg("/g");
        ps->rgba.g = set->value(str).toInt();
        str = QString("%1%2").arg(path).arg("/b");
        ps->rgba.b = set->value(str).toInt();
        str = QString("%1%2").arg(path).arg("/px");
        ps->fontSize = set->value(str).toInt();

        
        if (debug == true)
        {
            str = QString("%1").arg(ps->rectUp.x);
            qDebug() << "x:" << str;
            str = QString("%1").arg(ps->rectUp.y);
            qDebug() << "y:" << str;
            str = QString("%1").arg(ps->rectUp.w);
            qDebug() << "w:" << str;
            str = QString("%1").arg(ps->rectUp.h);
            qDebug() << "h:" << str;
            str = QString("%1").arg(ps->rgba.r);
            qDebug() << "r:" << str;
            str = QString("%1").arg(ps->rgba.g);
            qDebug() << "g:" << str;
            str = QString("%1").arg(ps->rgba.b);
            qDebug() << "b:" << str;
            str = QString("%1").arg(ps->fontSize);
            qDebug() << "px:" << str;
        }
        

    }
}


void MainWindow::readIpconfig(void)
{
    /*****************(1)netcfg.ini*********************************************************/
    QString netcfgPath = "/usrdata/setup.ini";
    QSettings settings_netcfg(netcfgPath, QSettings::IniFormat);
    settings_netcfg.setIniCodec("UTF8");
    QString str_net;
    str_net = settings_netcfg.value("ip/IpAddr").toString();    /*---ip配置---*/
    str_net = str_net.trimmed();
    qDebug() << "IP:" << str_net;
    mytimer.ipStr = str_net;
    QStringList list = str_net.split(".");
    ip4 = list[3].toInt();
    str_net = settings_netcfg.value("ip/MacAddr").toString();
    str_net = str_net.trimmed();
    qDebug() << "MAC:" << str_net;
    mytimer.macStr = str_net;
    macStr = str_net;
}


void MainWindow::labelInit(QLabel &label, label_info_t info_t, int type, QString str, int widgetNum, QString fontStr, bool blod)
{
    QImage image;
    QPixmap p;
    label.setParent(&widget[widgetNum]);
    label.setGeometry(info_t.rectUp.x, info_t.rectUp.y, info_t.rectUp.w, info_t.rectUp.h);
    
    if (type == LABEL_TYPE_IMAGE)
    {
        if (str.compare("NULL") != 0)
        {
            image.load(str);
            p = QPixmap::fromImage(image.scaled(info_t.rectUp.w, info_t.rectUp.h, Qt::IgnoreAspectRatio));
            label.setPixmap(p);
        }
    }
    else if (type == LABEL_TYPE_TEXT)
    {
        setFont(label, fontStr, blod, info_t.fontSize, info_t.rgba.r, info_t.rgba.g, info_t.rgba.b);
        //        label.setText(str);
    }
    
    label.show();
}


void MainWindow::publicInit()
{
    QString str;
    bool debug = false;
    QString displayPath = filePath + "display.ini";
    QSettings settings_disp(displayPath, QSettings::IniFormat);
    settings_disp.setIniCodec("UTF8");
    public_info_t *ppublic = &gPublicInfo;
    qDebug() << "------------------------------publicInit------------------------------";
    /*******中英文站名文本******/
    StationMaxNumber = DEFINE_STATION_NUM;                              //站名文本
    stationNum = settings_disp.value("StationNum/num").toInt();
    StationMaxNumber = stationNum;                                      // 站总数
    mycontroludp.StationMax = stationNum;
    gPartInfo.leaveStationNum = settings_disp.value("PartSNNum/num").toInt();
    int lsiteTime = 0;
    
    for (int i = 0; i < stationNum; i++) // siteTime
    {
        str.clear();
        str = QString("%1%2").arg("StationArrTime/").arg(i + 1, 2, 10, QLatin1Char('0'));
        lsiteTime = settings_disp.value(str).toInt();
        siteTime.append(lsiteTime);
        tmpSiteTime.append(lsiteTime);
    }
    
    int listDistance = 0;
    
    for (int i = 0; i < stationNum; i++)
    {
        str.clear();
        str = QString("%1%2").arg("StationDistance/").arg(i + 1, 2, 10, QLatin1Char('0'));
        listDistance = settings_disp.value(str).toInt();
        distance.append(listDistance);
    }
    
    for (int i = 0; i < stationNum; i++) // 获取站名
    {
        str.clear();
        str = QString("%1%2").arg("StationNameCh/").arg(i + 1, 2, 10, QLatin1Char('0'));
        str = settings_disp.value(str).toString();
        
        if (str.length() == 0)
        {
            str = QString("%1%2").arg("StationNameCh/").arg(i + 1, 2, 10, QLatin1Char('0'));
            str = settings_disp.value(str).toStringList().join(",");
        }
        
        str = str.trimmed();
        stationChName.append(str);
    }
    
    int dispLines = 0;
    
    for (int i = 0; i < stationNum; i++)
    {
        str.clear();
        str = QString("%1%2").arg("SNEnDispLines/").arg(i + 1, 2, 10, QLatin1Char('0'));
        dispLines = settings_disp.value(str).toInt();
        stationEnDispLines.append(dispLines);
        //qDebug()<<"station:"<<i<<"dispLines:"<<dispLines;
    }
    
    for (int i = 0; i < stationNum; i++) // 获取站名
    {
        str.clear();
        str = QString("%1%2").arg("StationNameEn2/").arg(i + 1, 2, 10, QLatin1Char('0'));
        str = settings_disp.value(str).toString();
        str = str.trimmed();
        stationEnName2.append(str);
    }
    
    for (int i = 0; i < stationNum; i++)
    {
        str.clear();
        str = QString("%1%2").arg("StationNameEn/").arg(i + 1, 2, 10, QLatin1Char('0'));
        str = settings_disp.value(str).toString();
        
        if (str.length() == 0)
        {
            str = QString("%1%2").arg("StationNameEn/").arg(i + 1, 2, 10, QLatin1Char('0'));
            str = settings_disp.value(str).toStringList().join(",");
        }
        
        str = str.trimmed();
        stationEnName.append(str);
    }
    
    qDebug() << "------Widget------";
    readLabelAttr(&ppublic->widget, &settings_disp, "Widget", LABEL_OTHER_TYPE, "NULL", 0, debug);
    
    for (int i = 0; i < WIDGET_NUM; i++)
    {
        widget[i].setParent(ui->centralWidget);
        widget[i].setGeometry(ppublic->widget.rectUp.x, ppublic->widget.rectUp.y,
                              ppublic->widget.rectUp.w, ppublic->widget.rectUp.h);
    }
    
    qDebug() << "------Welcome------";
    readLabelAttr(&ppublic->welcomeLable, &settings_disp, "Welcome",
                  LABEL_OTHER_TYPE, "NULL", 0, debug);
    labelInit(welcomeLable, ppublic->welcomeLable, LABEL_TYPE_IMAGE,
              welcomePath, WELCOME, "NULL", false);
    qDebug() << "------Update------";
    readLabelAttr(&ppublic->updateLable, &settings_disp, "Update",
                  LABEL_OTHER_TYPE, "NULL", 0, debug);
    labelInit(updateLable, ppublic->updateLable, LABEL_TYPE_TEXT,
              "NULL", WELCOME, "Oswald", false);
    updateLable.setAlignment(Qt::AlignCenter);
    qDebug() << "------Background------";
    readLabelAttr(&ppublic->background, &settings_disp, "Background",
                  LABEL_OTHER_TYPE, "NULL", 0, debug);
    labelInit(allBgLabel, ppublic->background, LABEL_TYPE_IMAGE,
              imageDir + "ui_all/BG/bg.png", ALL, "NULL", false);
    labelInit(preBgLabel, ppublic->background, LABEL_TYPE_IMAGE,
              imageDir + "ui_pre/BG/bg.png", PRE, "NULL", false);
    labelInit(arrBgLabel, ppublic->background, LABEL_TYPE_IMAGE,
              imageDir + "ui_arr/BG/bg.png", ARR, "NULL", false);
    qDebug() << "------SNColor------";
    readLabelAttr(&ppublic->SNOutRangeColor, &settings_disp, "SNOutRangeColor",
                  LABEL_OTHER_TYPE, "Ch", 0, debug);
    readLabelAttr(&ppublic->SNPassColor, &settings_disp, "SNPassColor",
                  LABEL_OTHER_TYPE, "Ch", 0, debug);
    readLabelAttr(&ppublic->SNCurrentColor, &settings_disp, "SNCurrentColor",
                  LABEL_OTHER_TYPE, "Ch", 0, debug);
    readLabelAttr(&ppublic->SNNextColor, &settings_disp, "SNNextColor",
                  LABEL_OTHER_TYPE, "Ch", 0, debug);
    readLabelAttr(&ppublic->SNNextNColor, &settings_disp, "SNNextNColor",
                  LABEL_OTHER_TYPE, "Ch", 0, debug);
    //"******菜单栏信息******"
    qDebug() << "------DateTime------";
    debug = false;
    readLabelAttr(&ppublic->dateLabel, &settings_disp, "Date", LABEL_OTHER_TYPE, "Ch", 0, debug);
    readLabelAttr(&ppublic->weekChLabel, &settings_disp, "WeekCh", LABEL_OTHER_TYPE, "Ch", 0, debug);
    readLabelAttr(&ppublic->weekEnLabel, &settings_disp, "WeekEn", LABEL_OTHER_TYPE, "Ch", 0, debug);
    readLabelAttr(&ppublic->timeLabel, &settings_disp, "Time", LABEL_OTHER_TYPE, "Ch", 0, debug);
    labelInit(dateLabel, ppublic->dateLabel, LABEL_TYPE_TEXT, "NULL", ALL, "Oswald", false);
    dateLabel.setAlignment(Qt::AlignLeft);
    labelInit(weekChLabel, ppublic->weekChLabel, LABEL_TYPE_TEXT,
              "NULL", ALL, "Source Han Sans CN", false);
    weekChLabel.setAlignment(Qt::AlignLeft);
    labelInit(weekEnLabel, ppublic->weekEnLabel, LABEL_TYPE_TEXT,
              "NULL", ALL, "Oswald", false);
    weekEnLabel.setAlignment(Qt::AlignLeft);
    labelInit(timeLabel, ppublic->timeLabel, LABEL_TYPE_TEXT,
              "NULL", ALL, "Oswald", false);
    timeLabel.setAlignment(Qt::AlignLeft);
    labelInit(preDateLabel, ppublic->dateLabel, LABEL_TYPE_TEXT,
              "NULL", PRE, "Oswald", false);
    preDateLabel.setAlignment(Qt::AlignLeft);
    labelInit(preWeekChLabel, ppublic->weekChLabel, LABEL_TYPE_TEXT,
              "NULL", PRE, "Source Han Sans CN", false);
    preWeekChLabel.setAlignment(Qt::AlignLeft);
    labelInit(preWeekEnLabel, ppublic->weekEnLabel, LABEL_TYPE_TEXT,
              "NULL", PRE, "Oswald", false);
    preWeekEnLabel.setAlignment(Qt::AlignLeft);
    labelInit(preTimeLabel, ppublic->timeLabel, LABEL_TYPE_TEXT,
              "NULL", PRE, "Oswald", false);
    preTimeLabel.setAlignment(Qt::AlignLeft);
    labelInit(arrDateLabel, ppublic->dateLabel, LABEL_TYPE_TEXT,
              "NULL", ARR, "Oswald", false);
    arrDateLabel.setAlignment(Qt::AlignLeft);
    labelInit(arrWeekChLabel, ppublic->weekChLabel, LABEL_TYPE_TEXT,
              "NULL", ARR, "Source Han Sans CN", false);
    arrWeekChLabel.setAlignment(Qt::AlignLeft);
    labelInit(arrWeekEnLabel, ppublic->weekEnLabel, LABEL_TYPE_TEXT,
              "NULL", ARR, "Oswald", false);
    arrWeekEnLabel.setAlignment(Qt::AlignLeft);
    labelInit(arrTimeLabel, ppublic->timeLabel, LABEL_TYPE_TEXT,
              "NULL", ARR, "Oswald", false);
    arrTimeLabel.setAlignment(Qt::AlignLeft);
    qDebug() << "------MDstSN------";
    readLabelAttr(&ppublic->MDstSNCh, &settings_disp, "MDstSN", LABEL_CHEN_TYPE, "Ch", 0, debug);
    readLabelAttr(&ppublic->MDstSNEn, &settings_disp, "MDstSN", LABEL_CHEN_TYPE, "En", 0, debug);
    labelInit(allMDstSNCh, ppublic->MDstSNCh, LABEL_TYPE_TEXT, "NULL", ALL, "Source Han Sans CN", false);
    labelInit(allMDstSNEn, ppublic->MDstSNEn, LABEL_TYPE_TEXT, "NULL", ALL, "Oswald", false);
    labelInit(preMDstSNCh, ppublic->MDstSNCh, LABEL_TYPE_TEXT, "NULL", PRE, "Source Han Sans CN", false);
    labelInit(preMDstSNEn, ppublic->MDstSNEn, LABEL_TYPE_TEXT, "NULL", PRE, "Oswald", false);
    labelInit(arrMDstSNCh, ppublic->MDstSNCh, LABEL_TYPE_TEXT, "NULL", ARR, "Source Han Sans CN", false);
    labelInit(arrMDstSNEn, ppublic->MDstSNEn, LABEL_TYPE_TEXT, "NULL", ARR, "Oswald", false);
    //    allMDstSNCh.setAlignment(Qt::AlignCenter);
    //    allMDstSNEn.setAlignment(Qt::AlignCenter);
    //    preMDstSNCh.setAlignment(Qt::AlignCenter);
    //    preMDstSNEn.setAlignment(Qt::AlignCenter);
    //    arrMDstSNCh.setAlignment(Qt::AlignCenter);
    //    arrMDstSNEn.setAlignment(Qt::AlignCenter);
    qDebug() << "------MDstText------";
    labelInit(allMDstTextCh, ppublic->MDstSNCh, LABEL_TYPE_TEXT, "NULL", ALL, "Source Han Sans CN", false);
    labelInit(allMDstTextEn, ppublic->MDstSNEn, LABEL_TYPE_TEXT, "NULL", ALL, "Oswald", false);
    labelInit(preMDstTextCh, ppublic->MDstSNCh, LABEL_TYPE_TEXT, "NULL", PRE, "Source Han Sans CN", false);
    labelInit(preMDstTextEn, ppublic->MDstSNEn, LABEL_TYPE_TEXT, "NULL", PRE, "Oswald", false);
    labelInit(arrMDstTextCh, ppublic->MDstSNCh, LABEL_TYPE_TEXT, "NULL", ARR, "Source Han Sans CN", false);
    labelInit(arrMDstTextEn, ppublic->MDstSNEn, LABEL_TYPE_TEXT, "NULL", ARR, "Oswald", false);
    //    allMDstTextCh.setText("终点站");
    //    allMDstTextEn.setText("Terminus");
    //    preMDstTextCh.setText("终点站");
    //    preMDstTextEn.setText("Terminus");
    //    arrMDstTextCh.setText("终点站");
    //    arrMDstTextEn.setText("Terminus");
    allMDstTextCh.adjustSize();
    allMDstTextEn.adjustSize();
    preMDstTextCh.adjustSize();
    preMDstTextEn.adjustSize();
    arrMDstTextCh.adjustSize();
    arrMDstTextEn.adjustSize();
    //    readLabelAttr(&ppublic->MDstText,&settings_disp,"MDstText",LABEL_OTHER_TYPE,"Ch",0,debug);
    //    labelInit(allMDstText,ppublic->MDstText,LABEL_TYPE_IMAGE,imageDir+"menu/dstText.png",ALL,"NULL",false);
    //    labelInit(arrMDstText,ppublic->MDstText,LABEL_TYPE_IMAGE,imageDir+"menu/dstText.png",ARR,"NULL",false);
    qDebug() << "------MNextSN------";
    readLabelAttr(&ppublic->MNextSNCh, &settings_disp, "MNextSN",
                  LABEL_CHEN_TYPE, "Ch", 0, debug);
    readLabelAttr(&ppublic->MNextSNEn, &settings_disp, "MNextSN",
                  LABEL_CHEN_TYPE, "En", 0, debug);
    labelInit(allMNextSNCh, ppublic->MNextSNCh, LABEL_TYPE_TEXT,
              "NULL", ALL, "Source Han Sans CN", true);
    labelInit(allMNextSNEn, ppublic->MNextSNEn, LABEL_TYPE_TEXT,
              "NULL", ALL, "Oswald", false);
    labelInit(preMNextSNCh, ppublic->MNextSNCh, LABEL_TYPE_TEXT,
              "NULL", PRE, "Source Han Sans CN", true);
    labelInit(preMNextSNEn, ppublic->MNextSNEn, LABEL_TYPE_TEXT,
              "NULL", PRE, "Oswald", false);
    labelInit(arrMNextSNCh, ppublic->MNextSNCh, LABEL_TYPE_TEXT,
              "NULL", ARR, "Source Han Sans CN", true);
    labelInit(arrMNextSNEn, ppublic->MNextSNEn, LABEL_TYPE_TEXT,
              "NULL", ARR, "Oswald", false);
    //    label_info_t MNextSNInfo;
    //    MNextSNInfo = ppublic->MNextSNEn;
    //    MNextSNInfo.rectUp.y += 12;
    //    labelInit(allMNextSNEn2, MNextSNInfo, LABEL_TYPE_TEXT, "NULL", ALL, "Oswald", false);
    //    labelInit(preMNextSNEn2, MNextSNInfo, LABEL_TYPE_TEXT, "NULL", PRE, "Oswald", false);
    //    labelInit(arrMNextSNEn2, MNextSNInfo, LABEL_TYPE_TEXT, "NULL", ARR, "Oswald", false);
    //    allMNextSNCh.setAlignment(Qt::AlignCenter);
    //    allMNextSNEn.setAlignment(Qt::AlignCenter);
    //    preMNextSNCh.setAlignment(Qt::AlignCenter);
    //    preMNextSNEn.setAlignment(Qt::AlignCenter);
    //    arrMNextSNCh.setAlignment(Qt::AlignCenter);
    //    arrMNextSNEn.setAlignment(Qt::AlignCenter);
    //    allMNextSNEn2.setAlignment(Qt::AlignCenter);
    //    preMNextSNEn2.setAlignment(Qt::AlignCenter);
    //    arrMNextSNEn2.setAlignment(Qt::AlignCenter);
    qDebug() << "------MNextText------";
    readLabelAttr(&ppublic->MNextText, &settings_disp, "MNextText",
                  LABEL_OTHER_TYPE, "Ch", 0, debug);
    labelInit(allMNextText, ppublic->MNextText, LABEL_TYPE_IMAGE,
              imageDir + "menu/nextText.png", ALL, "NULL", false);
    labelInit(preMNextText, ppublic->MNextText, LABEL_TYPE_IMAGE,
              imageDir + "menu/nextText.png", PRE, "NULL", false);
    qDebug() << "------MCurrentText------";
    readLabelAttr(&ppublic->MCurrentText, &settings_disp, "MNextText",
                  LABEL_OTHER_TYPE, "Ch", 0, debug);
    labelInit(arrMCurrentText, ppublic->MCurrentText, LABEL_TYPE_IMAGE,
              imageDir + "menu/curText.png", ARR, "NULL", false);
    qDebug() << "------DoorErr------";
    readLabelAttr(&ppublic->DoorErr, &settings_disp, "DoorErr",
                  LABEL_OTHER_TYPE, "Ch", 0, debug);
    readLabelAttr(&ppublic->Imetion, &settings_disp, "Imetion",
                  LABEL_OTHER_TYPE, "Ch", 0, debug);
}

void MainWindow::mirrorInfo(label_info_t *info)
{
    if (info == NULL)
    {
        qDebug() << "mirrorInfo null";
        return;
    }
    
    info->rectUp.x = gPublicInfo.widget.rectUp.w - info->rectUp.x - info->rectUp.w;
}

//void MainWindow::allmirroInfo(all_info_t *info)
//{
//    if(info == NULL)
//    {
//        qDebug()<<"mirrorInfo null";
//        return;
//    }

//    info->rectUp.x = gPublicInfo.widget.rectUp.w - info->rectUp.x - info->rectUp.w;

//}


void MainWindow::allReInit()
{
    QString str = "NULL";
    all_info_t *pallInfo = &gAllInfo;
    label_info_t labelInfo;
    qDebug() << "drmd==>---allReInit---";
    
    for (int i = 0; i < stationNum; i++)
    {
        labelInfo = pallInfo->track[i];
        
        if (gMainReceive.mirror)
        {
            mirrorInfo(&labelInfo);
        }
        
        labelInit(*allTrack[i], labelInfo, LABEL_TYPE_IMAGE, str, ALL, "NULL", false);
    }
    
//    labelInit(allFlowLed1, pallInfo->track[0], LABEL_TYPE_IMAGE, "NULL", ALL, "NULL", false);
//    labelInit(allFlowLed2, pallInfo->track[0], LABEL_TYPE_IMAGE, "NULL", ALL, "NULL", false);
    labelInit(allTrackFlash, pallInfo->track[0], LABEL_TYPE_IMAGE, str, ALL, "NULL", false);
    
    //qDebug() << "---Allchg---";
    for (int i = 0; i < 100; i++)
    {
        if (pallInfo->chg[i].stationId != 0)
        {
            labelInfo.rectUp.x = pallInfo->chg[i].rect.x;
            labelInfo.rectUp.y = pallInfo->chg[i].rect.y;
            labelInfo.rectUp.w = pallInfo->chg[i].rect.w;
            labelInfo.rectUp.h = pallInfo->chg[i].rect.h;
            str = QString("%1%2%3").arg(imageDir + "ui_all/CHANGE/line/")
                  .arg(pallInfo->chg[i].stationId, 2, 10, QLatin1Char('0'))
                  .arg("station.png");
                  
            if (gMainReceive.mirror)
            {
                mirrorInfo(&labelInfo);
            }
            
            labelInit(*allChg[i], labelInfo, LABEL_TYPE_IMAGE, str, ALL, "NULL", false);
        }
        else
        {
            break;
        }
    }
    
    //qDebug() << "---AllSite---";
    
    for (int i = 0; i < stationNum; i++)
    {
        labelInfo = pallInfo->site[i];
        
        if (gMainReceive.mirror)
        {
            mirrorInfo(&labelInfo);
        }
        
        labelInit(*allSite[i], labelInfo, LABEL_TYPE_IMAGE, "NULL", ALL, "NULL", false);
    }
    
    labelInit(allSiteFlash, pallInfo->site[0], LABEL_TYPE_IMAGE,
              "NULL", ALL, "NULL", false);
              
    for (int i = 0; i < stationNum; i++)
    {
        labelInfo = pallInfo->site[i];
        labelInfo.rectUp.y = pallInfo->site[i].rectUp.y + 30;
        labelInfo.rectUp.w = pallInfo->site[i].rectUp.w + 30;
        labelInfo.rectUp.x = pallInfo->site[i].rectUp.x - 18;
        
        if (gMainReceive.mirror)
        {
            mirrorInfo(&labelInfo);
        }
        
        labelInit(*allSiteTime[i], labelInfo, LABEL_TYPE_IMAGE, "NULL", ALL, "NULL", false);
        setFont(*allSiteTime[i], "Oswald", false, pallInfo->siteTime.fontSize,
                pallInfo->siteTime.rgba.r, pallInfo->siteTime.rgba.g,
                pallInfo->siteTime.rgba.b);
        allSiteTime[i]->setAlignment(Qt::AlignCenter);
    }
    
    //qDebug() << "---AllSN---";
    for (int i = 0; i < stationNum; i++)
#if 1
    {
        // 英文
        labelInfo = pallInfo->enSN[i];
        
        if (gMainReceive.mirror)
        {
            labelInfo.rectUp.x = pallInfo->enSN[i].rectUp.x - pallInfo->site[i].rectUp.w * 3 - 18;
            labelInfo.rectUp.y = pallInfo->enSN[i].rectUp.y;
            mirrorInfo(&labelInfo);
        }
        
        allSNPassEn[i]->setParent(&widget[ALL]);
        allSNPassEn[i]->setGeometry(labelInfo.rectUp.x,
                                    labelInfo.rectUp.y,
                                    labelInfo.rectUp.w,
                                    labelInfo.rectUp.h);
        allSNPassEn[i]->hide();
        allSNPassEn2[i]->setParent(&widget[ALL]);
        allSNPassEn2[i]->setGeometry(labelInfo.rectUp.x,
                                     labelInfo.rectUp.y,
                                     labelInfo.rectUp.w,
                                     labelInfo.rectUp.h);
        allSNPassEn2[i]->hide();
        allSNNextEn[i]->setParent(&widget[ALL]);
        allSNNextEn[i]->setGeometry(labelInfo.rectUp.x,
                                    labelInfo.rectUp.y,
                                    labelInfo.rectUp.w,
                                    labelInfo.rectUp.h);
        allSNNextEn[i]->hide();
        allSNNextEn2[i]->setParent(&widget[ALL]);
        allSNNextEn2[i]->setGeometry(labelInfo.rectUp.x,
                                     labelInfo.rectUp.y,
                                     labelInfo.rectUp.w,
                                     labelInfo.rectUp.h);
        allSNNextEn2[i]->hide();
        // 中文
        labelInfo = pallInfo->chSN[i];
        
        if (gMainReceive.mirror)
        {
            labelInfo.rectUp.x = pallInfo->chSN[i].rectUp.x - pallInfo->site[i].rectUp.w * 2 - 18;
            labelInfo.rectUp.y = pallInfo->enSN[i].rectUp.y - 6;
            mirrorInfo(&labelInfo);
        }
        
        allSNPassCh[i]->setParent(&widget[ALL]);
        allSNPassCh[i]->setGeometry(labelInfo.rectUp.x,
                                    labelInfo.rectUp.y,
                                    labelInfo.rectUp.w,
                                    labelInfo.rectUp.h);
        allSNPassCh[i]->hide();
        allSNNextCh[i]->setParent(&widget[ALL]);
        allSNNextCh[i]->setGeometry(labelInfo.rectUp.x,
                                    labelInfo.rectUp.y,
                                    labelInfo.rectUp.w,
                                    labelInfo.rectUp.h);
        allSNNextCh[i]->hide();
        allSNPassCh[i]->setText(stationChName[i]);
        allSNNextCh[i]->setText(stationChName[i]);
        allSNPassEn[i]->setText(stationEnName[i]);
        allSNNextEn[i]->setText(stationEnName[i]);
        allSNPassEn2[i]->setText(stationEnName2[i]);
        allSNNextEn2[i]->setText(stationEnName2[i]);
    }
    
#endif
    
    //qDebug() << "---AllJump---";                        //越站图标
    for (int i = 0; i < stationNum; i++)
    {
        allJump[i]->setParent(&widget[ALL]);
        allJump[i]->setGeometry(allSite[i]->x() - (allJump[i]->width() - allSite[i]->width()) / 2,
                                allSite[i]->y() - (allJump[i]->height() - allSite[i]->height()) / 2,
                                allJump[i]->width(), allJump[i]->height());
        allJump[i]->hide();
    }
    
    
    //qDebug() << "---AllStartEnd---";                    //起始终点站图标
    for (int i = 0; i < stationNum; i++)
    {
        allStart[i]->setParent(&widget[ALL]);
        allStart[i]->setGeometry(allSite[i]->x() - (allStart[i]->width() - allSite[i]->width()) / 2,
                                 allSite[i]->y() - (allStart[i]->height() - allSite[i]->height()) / 2,
                                 allStart[i]->width(), allStart[i]->height());
        allStart[i]->hide();
        allEnd[i]->setParent(&widget[ALL]);
        allEnd[i]->setGeometry(allSite[i]->x() - (allEnd[i]->width() - allSite[i]->width()) / 2,
                               allSite[i]->y() - (allEnd[i]->height() - allSite[i]->height()) / 2,
                               allEnd[i]->width(), allEnd[i]->height());
        allEnd[i]->hide();
    }


}

void MainWindow::allInit()
{
    QImage image;
    QString str;
    bool debug = false;
    QString displayPath = filePath + "display.ini";
    QSettings settings_disp(displayPath, QSettings::IniFormat);
    settings_disp.setIniCodec("UTF8");
    all_info_t *pallInfo = &gAllInfo;
    qDebug() << "------------------------------allInit------------------------------";
    qDebug() << "---AllTrack---";
    
    for (int i = 0; i < stationNum; i++)
    {
        str = QString("%1%2").arg("AllTrack_").arg(i + 1, 2, 10, QLatin1Char('0'));
        readLabelAttr(&pallInfo->track[i], &settings_disp, str, LABEL_OTHER_TYPE, "Ch", i, debug);
        allTrack[i] = new QLabel();
        str = imageDir + "ui_all/TRACK/nextnL.png";
        labelInit(*allTrack[i], pallInfo->track[i], LABEL_TYPE_IMAGE, str, ALL, "NULL", false);
    }
    
    labelInit(allFlowLed1, pallInfo->track[0], LABEL_TYPE_IMAGE, "NULL", ALL, "NULL", false);
    labelInit(allFlowLed2, pallInfo->track[0], LABEL_TYPE_IMAGE, "NULL", ALL, "NULL", false);

    str = "NULL";
    labelInit(allTrackFlash, pallInfo->track[0], LABEL_TYPE_IMAGE, str, ALL, "NULL", false);
    qDebug() << "---Allchg---";
    int chgnum = 0;
    int chgIndex = 0;
    
    for (int i = 0; i < stationNum; i++) // 换乘线路信息
    {
        str = QString("%1%2%3").arg("AllStationChg_")
              .arg(i + 1, 2, 10, QLatin1Char('0')).arg("/chgnum");
        chgnum = settings_disp.value(str).toInt();
        pallInfo->stationChgNum[i] = chgnum;
        
        if (chgnum > 0)
        {
            for (int j = 0; j < chgnum; j++)
            {
                str = QString("%1%2%3").arg("AllStationChg_")
                      .arg(i + 1, 2, 10, QLatin1Char('0')).arg("/x");
                pallInfo->chg[chgIndex].rect.x = settings_disp.value(str).toInt();
                str = QString("%1%2%3").arg("AllStationChg_")
                      .arg(i + 1, 2, 10, QLatin1Char('0')).arg("/y");
                pallInfo->chg[chgIndex].rect.y = settings_disp.value(str).toInt();
                str = QString("%1%2%3").arg("AllStationChg_")
                      .arg(i + 1, 2, 10, QLatin1Char('0')).arg("/w");
                pallInfo->chg[chgIndex].rect.w = settings_disp.value(str).toInt();
                str = QString("%1%2%3").arg("AllStationChg_")
                      .arg(i + 1, 2, 10, QLatin1Char('0')).arg("/h");
                pallInfo->chg[chgIndex].rect.h = settings_disp.value(str).toInt();
                str = QString("%1%2%3").arg("AllStationChg_")
                      .arg(i + 1, 2, 10, QLatin1Char('0')).arg("/x_down");
                pallInfo->chg[chgIndex].rectDown.x = settings_disp.value(str).toInt();
                str = QString("%1%2%3").arg("AllStationChg_")
                      .arg(i + 1, 2, 10, QLatin1Char('0')).arg("/y_down");
                pallInfo->chg[chgIndex].rectDown.y = settings_disp.value(str).toInt();
                str = QString("%1%2%3").arg("AllStationChg_")
                      .arg(i + 1, 2, 10, QLatin1Char('0')).arg("/w_down");
                pallInfo->chg[chgIndex].rectDown.w = settings_disp.value(str).toInt();
                str = QString("%1%2%3").arg("AllStationChg_")
                      .arg(i + 1, 2, 10, QLatin1Char('0')).arg("/h_down");
                pallInfo->chg[chgIndex].rectDown.h = settings_disp.value(str).toInt();
                pallInfo->chg[chgIndex].stationId = i + 1;
                chgIndex++;
            }
        }
    }
    
    pallInfo->chg[chgIndex].stationId = 0;
    label_info_t chgLabelInfo;
    
    for (int i = 0; i < 100; i++)
    {
        if (pallInfo->chg[i].stationId != 0)
        {
            allChg[i] = new QLabel();
            chgLabelInfo.rectUp.x = pallInfo->chg[i].rect.x;
            chgLabelInfo.rectUp.y = pallInfo->chg[i].rect.y;
            chgLabelInfo.rectUp.w = pallInfo->chg[i].rect.w;
            chgLabelInfo.rectUp.h = pallInfo->chg[i].rect.h;
            labelInit(*allChg[i], chgLabelInfo, LABEL_TYPE_IMAGE, "NULL", ALL, "NULL", false);
        }
        else
        {
            break;
        }
    }
    
    qDebug() << "---Allsp---";
    int spnum = 0;
    int spIndex = 0;
    
    for (int i = 0; i < stationNum; i++) // 特殊图标信息
    {
        str = QString("%1%2%3").arg("AllStationSp_")
              .arg(i + 1, 2, 10, QLatin1Char('0')).arg("/spnum");
        spnum = settings_disp.value(str).toInt();
        pallInfo->stationspNum[i] = spnum;
        
        if (spnum > 0)
        {
            for (int j = 0; j < spnum; j++)
            {
                str = QString("%1%2%3").arg("AllStationSp_")
                      .arg(i + 1, 2, 10, QLatin1Char('0')).arg("/x");
                pallInfo->sp[spIndex].rect.x = settings_disp.value(str).toInt();
                str = QString("%1%2%3").arg("AllStationSp_")
                      .arg(i + 1, 2, 10, QLatin1Char('0')).arg("/y");
                pallInfo->sp[spIndex].rect.y = settings_disp.value(str).toInt();
                str = QString("%1%2%3").arg("AllStationSp_")
                      .arg(i + 1, 2, 10, QLatin1Char('0')).arg("/w");
                pallInfo->sp[spIndex].rect.w = settings_disp.value(str).toInt();
                str = QString("%1%2%3").arg("AllStationSp_")
                      .arg(i + 1, 2, 10, QLatin1Char('0')).arg("/h");
                pallInfo->sp[spIndex].rect.h = settings_disp.value(str).toInt();
                pallInfo->sp[spIndex].stationId = i + 1;
                qDebug() << "spid:" << pallInfo->sp[spIndex].stationId;
                spIndex++;
            }
        }
    }
    
    pallInfo->sp[spIndex].stationId = 0;
    label_info_t spLabelInfo;
    
    for (int i = 0; i < 100; i++)
    {
        if (pallInfo->sp[i].stationId != 0)
        {
            allSp[i] = new QLabel();
            spLabelInfo.rectUp.x = pallInfo->sp[i].rect.x;
            spLabelInfo.rectUp.y = pallInfo->sp[i].rect.y;
            spLabelInfo.rectUp.w = pallInfo->sp[i].rect.w;
            spLabelInfo.rectUp.h = pallInfo->sp[i].rect.h;
            labelInit(*allSp[i], spLabelInfo, LABEL_TYPE_IMAGE, "NULL", ALL, "NULL", false);
        }
        else
        {
            break;
        }
    }
    
    qDebug() << "---AllSite---";
    
    for (int i = 0; i < stationNum; i++)
    {
        str = QString("%1%2").arg("AllSite_").arg(i + 1, 2, 10, QLatin1Char('0'));
        readLabelAttr(&pallInfo->site[i], &settings_disp, str, LABEL_OTHER_TYPE, "Ch", i, debug);
        allSite[i] = new QLabel();
        labelInit(*allSite[i], pallInfo->site[i], LABEL_TYPE_IMAGE, "NULL", ALL, "NULL", false);
    }
    
    labelInit(allSiteFlash, pallInfo->site[0], LABEL_TYPE_IMAGE,
              "NULL", ALL, "NULL", false);
    readLabelAttr(&pallInfo->siteTime, &settings_disp, "AllSiteTime",
                  LABEL_CHEN_TYPE, "Ch", 0, debug);
                  
    for (int i = 0; i < stationNum; i++)
    {
        allSiteTime[i] = new QLabel();
        labelInit(*allSiteTime[i], pallInfo->site[i], LABEL_TYPE_IMAGE, "NULL", ALL, "NULL", false);
        setFont(*allSiteTime[i], "Oswald", false, pallInfo->siteTime.fontSize,
                pallInfo->siteTime.rgba.r, pallInfo->siteTime.rgba.g,
                pallInfo->siteTime.rgba.b);
        allSiteTime[i]->setAlignment(Qt::AlignCenter);
    }
    
    qDebug() << "---AllSN---";
    QFont font;
    QColor color;
    
    for (int i = 0; i < stationNum; i++)
#if 1
    {
        str = QString("%1%2").arg("AllSN_").arg(i + 1, 2, 10, QLatin1Char('0'));
        readLabelAttr(&pallInfo->chSN[i], &settings_disp, str, LABEL_CHEN_TYPE, "Ch", i, debug);
        readLabelAttr(&pallInfo->enSN[i], &settings_disp, str, LABEL_CHEN_TYPE, "En", i, debug);
        pallInfo->enSN[i].rectUp.x = pallInfo->site[i].rectUp.x - pallInfo->site[i].rectUp.w / 2 + 26;
        pallInfo->chSN[i].rectUp.x = pallInfo->enSN[i].rectUp.x - 15;
        pallInfo->enSN[i].rectUp.y = pallInfo->enSN[i].rectUp.y - 4;
        pallInfo->chSN[i].rectUp.y = pallInfo->enSN[i].rectUp.y - 7;
        // 英文
        allSNPassEn[i] = new VERTICALEN(315);
        allSNPassEn[i]->setParent(&widget[ALL]);
        allSNPassEn[i]->setGeometry(pallInfo->enSN[i].rectUp.x,
                                    pallInfo->enSN[i].rectUp.y,
                                    pallInfo->enSN[i].rectUp.w,
                                    pallInfo->enSN[i].rectUp.h);
        font.setPixelSize(pallInfo->enSN[i].fontSize);
        font.setBold(false);
        font.setFamily("Arial-BoldMT");
        color.setRgb(gPublicInfo.SNPassColor.rgba.r,
                     gPublicInfo.SNPassColor.rgba.g,
                     gPublicInfo.SNPassColor.rgba.b);
        allSNPassEn[i]->setTextColor(color);
        allSNPassEn[i]->setFont(font);
        allSNPassEn[i]->setText(stationEnName[i]);
        allSNPassEn[i]->hide();
        allSNPassEn[i]->setHeight(pallInfo->enSN[i].rectUp.h);
        allSNPassEn2[i] = new VERTICALEN(315);
        allSNPassEn2[i]->setParent(&widget[ALL]);
        allSNPassEn2[i]->setGeometry(pallInfo->enSN[i].rectUp.x,
                                     pallInfo->enSN[i].rectUp.y,
                                     pallInfo->enSN[i].rectUp.w,
                                     pallInfo->enSN[i].rectUp.h);
        allSNPassEn2[i]->setTextColor(color);
        allSNPassEn2[i]->setFont(font);
        allSNPassEn2[i]->setText(stationEnName2[i]);
        allSNPassEn2[i]->hide();
        allSNPassEn2[i]->setHeight(pallInfo->enSN[i].rectUp.h);
        allSNNextEn[i] = new VERTICALEN(315);
        allSNNextEn[i]->setParent(&widget[ALL]);
        allSNNextEn[i]->setGeometry(pallInfo->enSN[i].rectUp.x,
                                    pallInfo->enSN[i].rectUp.y,
                                    pallInfo->enSN[i].rectUp.w,
                                    pallInfo->enSN[i].rectUp.h);
        color.setRgb(gPublicInfo.SNNextColor.rgba.r,
                     gPublicInfo.SNNextColor.rgba.g,
                     gPublicInfo.SNNextColor.rgba.b);
        allSNNextEn[i]->setTextColor(color);
        allSNNextEn[i]->setFont(font);
        allSNNextEn[i]->setText(stationEnName[i]);
        allSNNextEn[i]->hide();
        allSNNextEn[i]->setHeight(pallInfo->enSN[i].rectUp.h);
        allSNNextEn2[i] = new VERTICALEN(315);
        allSNNextEn2[i]->setParent(&widget[ALL]);
        allSNNextEn2[i]->setGeometry(pallInfo->enSN[i].rectUp.x,
                                     pallInfo->enSN[i].rectUp.y,
                                     pallInfo->enSN[i].rectUp.w,
                                     pallInfo->enSN[i].rectUp.h);
        allSNNextEn2[i]->setTextColor(color);
        allSNNextEn2[i]->setFont(font);
        allSNNextEn2[i]->setText(stationEnName2[i]);
        allSNNextEn2[i]->hide();
        allSNNextEn2[i]->setHeight(pallInfo->enSN[i].rectUp.h);
        // 中文
        allSNPassCh[i] = new VERTICALCH(315);
        allSNPassCh[i]->setParent(&widget[ALL]);
        allSNPassCh[i]->setGeometry(pallInfo->chSN[i].rectUp.x,
                                    pallInfo->chSN[i].rectUp.y,
                                    pallInfo->chSN[i].rectUp.w,
                                    pallInfo->chSN[i].rectUp.h);
        font.setPixelSize(pallInfo->chSN[i].fontSize);
        font.setBold(false);
        font.setFamily("SourceHanSansCN-Medium");
        color.setRgb(gPublicInfo.SNPassColor.rgba.r,
                     gPublicInfo.SNPassColor.rgba.g,
                     gPublicInfo.SNPassColor.rgba.b);
        allSNPassCh[i]->setTextColor(color);
        allSNPassCh[i]->setFont(font);
        allSNPassCh[i]->hide();
        allSNPassCh[i]->setHeight(pallInfo->chSN[i].rectUp.h);
        allSNNextCh[i] = new VERTICALCH(315);
        allSNNextCh[i]->setParent(&widget[ALL]);
        allSNNextCh[i]->setGeometry(pallInfo->chSN[i].rectUp.x,
                                    pallInfo->chSN[i].rectUp.y,
                                    pallInfo->chSN[i].rectUp.w,
                                    pallInfo->chSN[i].rectUp.h);
        color.setRgb(gPublicInfo.SNNextColor.rgba.r,
                     gPublicInfo.SNNextColor.rgba.g,
                     gPublicInfo.SNNextColor.rgba.b);
        allSNNextCh[i]->setTextColor(color);
        allSNNextCh[i]->setFont(font);
        allSNNextCh[i]->setText(stationChName[i]);
        allSNNextCh[i]->setHeight(pallInfo->chSN[i].rectUp.h);
        allSNNextCh[i]->hide();
    }
    
#endif
    qDebug() << "---AllJump---";                        //越站图标
    image.load(allSitePath + "jump.png");
    str = allSitePath + "jump.png";
    
    for (int i = 0; i < stationNum; i++)
    {
        allJump[i] = new QLabel();
        allJump[i]->setParent(&widget[ALL]);
        allJump[i]->setGeometry(pallInfo->site[i].rectUp.x -
                                (image.width() - pallInfo->site[i].rectUp.w) / 2,
                                pallInfo->site[i].rectUp.y -
                                (image.height() - pallInfo->site[i].rectUp.h) / 2,
                                image.width(), image.height());
        allJump[i]->setPixmap(str);
        allJump[i]->hide();
    }
    
    //    allNotRun.setParent(&widget[ALL]);
    //    if(!gMainReceive.mirror)
    //    {
    //        if(gMainReceive.start < gMainReceive.end)
    //        {
    //            allNotRun.setGeometry(pallInfo->site[NOT_IN_SERVICE - 1].rectUp.x+23,
    //                    pallInfo->site[NOT_IN_SERVICE - 1].rectUp.y+37,
    //                    pallInfo->site[NOT_IN_SERVICE - 1].rectUp.w+100,
    //                    pallInfo->site[NOT_IN_SERVICE - 1].rectUp.h);
    //        }
    //        else
    //        {
    //            allNotRun.setGeometry(pallInfo->site[stationNum - NOT_IN_SERVICE-2].rectUp.x,
    //                    pallInfo->site[stationNum - NOT_IN_SERVICE].rectUp.y,
    //                    pallInfo->site[stationNum - NOT_IN_SERVICE].rectUp.w,
    //                    pallInfo->site[stationNum - NOT_IN_SERVICE].rectUp.h);
    //        }
    //    }
    //    allNotRun.setPixmap(allSitePath + "notRun.png");
    qDebug() << "---AllStartEnd---";                    //起始终点站图标
    image.load(allSitePath + "end.png");
    str = allSitePath + "end.png";
    
    for (int i = 0; i < stationNum; i++)
    {
        str = allSitePath + "start.png";
        allStart[i] = new QLabel();
        allStart[i]->setParent(&widget[ALL]);
        allStart[i]->setGeometry(pallInfo->site[i].rectUp.x -
                                 (image.width() - pallInfo->site[i].rectUp.w) / 2,
                                 pallInfo->site[i].rectUp.y -
                                 (image.height() - pallInfo->site[i].rectUp.h) / 2,
                                 image.width(), image.height());
        allStart[i]->setPixmap(str);
        allStart[i]->hide();
        str = allSitePath + "end.png";
        allEnd[i] = new QLabel();
        allEnd[i]->setParent(&widget[ALL]);
        allEnd[i]->setGeometry(pallInfo->site[i].rectUp.x -
                               (image.width() - pallInfo->site[i].rectUp.w) / 2,
                               pallInfo->site[i].rectUp.y -
                               (image.height() - pallInfo->site[i].rectUp.h) / 2,
                               image.width(), image.height());
        allEnd[i]->setPixmap(str);
        allEnd[i]->hide();
    }

    labelInit(allDoorErr, gPublicInfo.DoorErr, LABEL_TYPE_IMAGE,
              imageDir + "doorError.png", ALL, "NULL", false);
    labelInit(allImetion, gPublicInfo.Imetion, LABEL_TYPE_IMAGE,
              /*imageDir + "imetion/01.png"*/"NULL", ALL, "NULL", false);
    

}

void MainWindow::preReInit()
{
    QString str = "NULL";
    part_info_t *ppartInfo = &gPartInfo;
    label_info_t labelInfo;
    qDebug() << "drmd==>---preReInit---";
    
    //qDebug() << "---PreTrack---";
    for (int i = 0; i < ppartInfo->leaveStationNum; i++)
    {
        labelInfo = ppartInfo->track[i];
        
        if (gMainReceive.mirror)
        {
            mirrorInfo(&labelInfo);
        }
        
        labelInit(*preTrack[i], labelInfo, LABEL_TYPE_IMAGE,
                  str, PRE, "NULL", false);
    }
    
    //qDebug() << "------Pre Chg------";
    qDebug() << "---PreSite---";
    
    for (int i = 0; i < ppartInfo->leaveStationNum; i++)
    {
        labelInfo = ppartInfo->site[i];
        
        if (gMainReceive.mirror)
        {
            mirrorInfo(&labelInfo);
        }
        
        labelInit(*preSite[i], labelInfo, LABEL_TYPE_IMAGE,
                  "NULL", PRE, "NULL", false);
    }
    
    labelInit(preSiteFlash, ppartInfo->site[2], LABEL_TYPE_IMAGE,
              imageDir + "ui_pre/SITE/next.png", PRE, "NULL", false);
    qDebug() << "---PreSiteTime---";
#if 0
    
    for (int i = 0; i < ppartInfo->leaveStationNum; i++)
    {
        labelInfo = ppartInfo->site[i];
        
        if (gMainReceive.mirror)
        {
            mirrorInfo(&labelInfo);
        }
        
        labelInit(*preSiteTime[i], labelInfo, LABEL_TYPE_IMAGE,
                  "NULL", PRE, "NULL", false);
        setFont(*preSiteTime[i], "Oswald", false, ppartInfo->siteTime.fontSize,
                ppartInfo->siteTime.rgba.r, ppartInfo->siteTime.rgba.g,
                ppartInfo->siteTime.rgba.b);
        preSiteTime[i]->setAlignment(Qt::AlignCenter);
    }
    
#endif
    //qDebug() << "---PreSN---";
    QFont font;
    QColor color;
    
    for (int i = 0; i < ppartInfo->leaveStationNum; i++)
    {
        labelInfo = ppartInfo->enSN[i];
        
        if (gMainReceive.mirror)
        {
            mirrorInfo(&labelInfo);
        }
        
        labelInit(*preSNEn[i], labelInfo, LABEL_TYPE_TEXT,
                  "NULL", PRE, "Oswald", false);
        preSNEn[i]->setAlignment(Qt::AlignCenter);
        labelInfo = ppartInfo->chSN[i];
        
        if (gMainReceive.mirror)
        {
            mirrorInfo(&labelInfo);
        }
        
        labelInit(*preSNCh[i], labelInfo, LABEL_TYPE_TEXT,
                  "NULL", PRE, "Source Han Sans CN", false);
        preSNCh[i]->setAlignment(Qt::AlignCenter);
    }
    
    //qDebug() << "---PreJump---";                        //越站图标
    for (int i = 0; i < ppartInfo->leaveStationNum; i++)
    {
        preJump[i]->setGeometry(preSite[i]->x() - (preJump[i]->width() - preSite[i]->width()) / 2,
                                preSite[i]->y() - (preJump[i]->height() - preSite[i]->height()) / 2,
                                preJump[i]->width(), preJump[i]->height());
        preJump[i]->hide();
    }
    
    preNotRun.setParent(&widget[PRE]);
    preNotRun.setGeometry(ppartInfo->site[0].rectUp.x,
                          ppartInfo->site[0].rectUp.y,
                          ppartInfo->site[0].rectUp.w,
                          ppartInfo->site[0].rectUp.h);
    preNotRun.setPixmap(preSitePath + "notRun.png");
    preNotRun.hide();
    //二维码区域
    labelInfo = ppartInfo->mationImg;
    labelInit(premationImg, labelInfo, LABEL_TYPE_IMAGE,
              imageDir + "ui_pre/DOOR/promote.png", PRE, "NULL", false);
    //qDebug() << "------Pre Door------";
    labelInfo = ppartInfo->doorOpenImg;
    //    if(gMainReceive.mirror)
    //    {
    //        mirrorInfo(&labelInfo);
    //    }
    labelInit(preOpenImg, labelInfo, LABEL_TYPE_IMAGE,
              imageDir + "ui_pre/DOOR/open.png", PRE, "NULL", false);
    labelInfo = ppartInfo->doorOpenText;
    //    if(gMainReceive.mirror)
    //    {
    //        mirrorInfo(&labelInfo);
    //    }
    labelInit(preOpenText, labelInfo, LABEL_TYPE_IMAGE,
              imageDir + "ui_pre/DOOR/openText.png", PRE, "NULL", false);
    labelInfo = ppartInfo->doorCloseImg;
    //    if(gMainReceive.mirror)
    //    {
    //        mirrorInfo(&labelInfo);
    //    }
    labelInit(preCloseImg, labelInfo, LABEL_TYPE_IMAGE,
              imageDir + "ui_pre/DOOR/close.png", PRE, "NULL", false);
    labelInfo = ppartInfo->doorCloseText;
    //    if(gMainReceive.mirror)
    //    {
    //        mirrorInfo(&labelInfo);
    //    }
    labelInit(preCloseText, labelInfo, LABEL_TYPE_IMAGE,
              imageDir + "ui_pre/DOOR/closeText.png", PRE, "NULL", false);


    qDebug() << "------Pre DoorErr------";
    labelInit(preDoorErr, gPublicInfo.DoorErr, LABEL_TYPE_IMAGE,
              imageDir + "doorError.png", PRE, "NULL", false);
    labelInit(preImetion, gPublicInfo.Imetion, LABEL_TYPE_IMAGE,
              /*imageDir + "imetion/01.png"*/"NULL", PRE, "NULL", false);

}
void MainWindow::preInit()
{
    QString str;
    QImage image;
    bool debug = false;
    QString displayPath = filePath + "display.ini";
    QSettings settings_disp(displayPath, QSettings::IniFormat);
    settings_disp.setIniCodec("UTF8");
    part_info_t *ppartInfo = &gPartInfo;
    qDebug() << "------------------------------preInit------------------------------";
    qDebug() << "---PreTrack---";
    
    for (int i = 0; i < ppartInfo->leaveStationNum; i++)
    {
        str = QString("%1%2").arg("PartTrack_").arg(i + 1, 2, 10, QLatin1Char('0'));
        readLabelAttr(&ppartInfo->track[i], &settings_disp, str,
                      LABEL_OTHER_TYPE, "Ch", i, debug);
        preTrack[i] = new QLabel();
        str = "NULL";//imageDir+"ui_pre/TRACK/nextn.png";
        labelInit(*preTrack[i], ppartInfo->track[i], LABEL_TYPE_IMAGE,
                  str, PRE, "NULL", false);
    }
    
    qDebug() << "------Pre Chg------";
    readLabelAttr(&ppartInfo->chg[0], &settings_disp, "PartChg",
                  LABEL_OTHER_TYPE, "NULL", 0, debug);
                  
    for (int i = 0; i < ppartInfo->leaveStationNum; i++)
    {
        labelInit(preChg[i], ppartInfo->chg[0], LABEL_TYPE_IMAGE,
                  "NULL", PRE, "NULL", false);
    }
    
    qDebug() << "------Pre Sp------";
    readLabelAttr(&ppartInfo->sp[0], &settings_disp, "PartSp",
                  LABEL_OTHER_TYPE, "NULL", 0, debug);
                  
    for (int i = 0; i < ppartInfo->leaveStationNum; i++)
    {
        labelInit(preSp[i], ppartInfo->sp[0], LABEL_TYPE_IMAGE,
                  "NULL", PRE, "NULL", false);
    }
    
    qDebug() << "---PreSite---";
    
    for (int i = 0; i < ppartInfo->leaveStationNum; i++)
    {
        str = QString("%1%2").arg("PartSite_").arg(i + 1, 2, 10, QLatin1Char('0'));
        readLabelAttr(&ppartInfo->site[i], &settings_disp, str,
                      LABEL_OTHER_TYPE, "Ch", i, debug);
        preSite[i] = new QLabel();
        labelInit(*preSite[i], ppartInfo->site[i], LABEL_TYPE_IMAGE,
                  "NULL", PRE, "NULL", false);
    }
    
    labelInit(preSiteFlash, ppartInfo->site[2], LABEL_TYPE_IMAGE,
              imageDir + "ui_pre/SITE/next.png", PRE, "NULL", false);
#if 0
    qDebug() << "---PreSiteTime---";
    readLabelAttr(&ppartInfo->siteTime, &settings_disp, "PartSiteTime",
                  LABEL_CHEN_TYPE, "Ch", 0, debug);
                  
    for (int i = 0; i < ppartInfo->leaveStationNum; i++)
    {
        preSiteTime[i] = new QLabel();
        labelInit(*preSiteTime[i], ppartInfo->site[i], LABEL_TYPE_IMAGE, "NULL", PRE, "NULL", false);
        setFont(*preSiteTime[i], "Oswald", false, ppartInfo->siteTime.fontSize,
                ppartInfo->siteTime.rgba.r, ppartInfo->siteTime.rgba.g,
                ppartInfo->siteTime.rgba.b);
        preSiteTime[i]->setAlignment(Qt::AlignCenter);
    }
    
#endif
    qDebug() << "---PreSN---";
    
    for (int i = 0; i < ppartInfo->leaveStationNum; i++)
    {
        str = QString("%1%2").arg("PartSN_").arg(i + 1, 2, 10, QLatin1Char('0'));
        readLabelAttr(&ppartInfo->chSN[i], &settings_disp, str, LABEL_CHEN_TYPE, "Ch", i, debug);
        readLabelAttr(&ppartInfo->enSN[i], &settings_disp, str, LABEL_CHEN_TYPE, "En", i, debug);
        preSNEn[i] = new QLabel();
        ppartInfo->enSN[i].rectUp.x = ppartInfo->site[i].rectUp.x + ppartInfo->site[i].rectUp.w / 2 - ppartInfo->enSN[i].rectUp.w  / 2;
        labelInit(*preSNEn[i], ppartInfo->enSN[i], LABEL_TYPE_TEXT,
                  "NULL", PRE, "Oswald", false);
        preSNEn[i]->setAlignment(Qt::AlignCenter);
        preSNCh[i] = new QLabel();
        ppartInfo->chSN[i].rectUp.x = ppartInfo->site[i].rectUp.x + ppartInfo->site[i].rectUp.w / 2 - ppartInfo->chSN[i].rectUp.w  / 2;
        labelInit(*preSNCh[i], ppartInfo->chSN[i], LABEL_TYPE_TEXT,
                  "NULL", PRE, "Source Han Sans CN", false);
        preSNCh[i]->setAlignment(Qt::AlignCenter);
    }
    
    qDebug() << "---PreJump---";                        //越站图标
    image.load(preSitePath + "jump.png");
    str = preSitePath + "jump.png";
    
    for (int i = 0; i < ppartInfo->leaveStationNum; i++)
    {
        preJump[i] = new QLabel();
        preJump[i]->setParent(&widget[PRE]);
        preJump[i]->setGeometry(ppartInfo->site[i].rectUp.x -
                                (image.width() - ppartInfo->site[i].rectUp.w) / 2,
                                ppartInfo->site[i].rectUp.y -
                                (image.height() - ppartInfo->site[i].rectUp.h) / 2,
                                image.width(), image.height());
        preJump[i]->setPixmap(str);
        preJump[i]->hide();
    }
    
    preNotRun.setParent(&widget[PRE]);
    preNotRun.setGeometry(ppartInfo->site[0].rectUp.x,
                          ppartInfo->site[0].rectUp.y,
                          ppartInfo->site[0].rectUp.w,
                          ppartInfo->site[0].rectUp.h);
    preNotRun.setPixmap(preSitePath + "notRun.png");
    qDebug() << "------Pre Door------";
    readLabelAttr(&ppartInfo->mationImg, &settings_disp, "Promation",
                  LABEL_OTHER_TYPE, "NULL", 0, debug);
    labelInit(premationImg, ppartInfo->mationImg, LABEL_TYPE_IMAGE,
              imageDir + "ui_pre/DOOR/promote.png", PRE, "NULL", false);
    readLabelAttr(&ppartInfo->doorOpenImg, &settings_disp, "PartOpenDoor",
                  LABEL_OTHER_TYPE, "NULL", 0, debug);
    labelInit(preOpenImg, ppartInfo->doorOpenImg, LABEL_TYPE_IMAGE,
              imageDir + "ui_pre/DOOR/open.png", PRE, "NULL", false);
    readLabelAttr(&ppartInfo->doorOpenText, &settings_disp, "PartOpenText",
                  LABEL_OTHER_TYPE, "NULL", 0, debug);
    labelInit(preOpenText, ppartInfo->doorOpenText, LABEL_TYPE_IMAGE,
              imageDir + "ui_pre/DOOR/openText.png", PRE, "NULL", false);
    readLabelAttr(&ppartInfo->doorCloseImg, &settings_disp, "PartCloseDoor",
                  LABEL_OTHER_TYPE, "NULL", 0, debug);
    labelInit(preCloseImg, ppartInfo->doorCloseImg, LABEL_TYPE_IMAGE,
              imageDir + "ui_pre/DOOR/close.png", PRE, "NULL", false);
    readLabelAttr(&ppartInfo->doorCloseText, &settings_disp, "PartCloseText",
                  LABEL_OTHER_TYPE, "NULL", 0, debug);
    labelInit(preCloseText, ppartInfo->doorCloseText, LABEL_TYPE_IMAGE,
              imageDir + "ui_pre/DOOR/closeText.png", PRE, "NULL", false);


}

void MainWindow::arrReInit()
{
    QString str = "NULL";
    part_info_t *ppartInfo = &gPartInfo;
    label_info_t labelInfo;
    qDebug() << "drmd==>---arrReInit---";
    //qDebug() << "---Arr Open DOOR && Img---";
    labelInfo = ppartInfo->arrOpenDoor;
    
    if (gMainReceive.mirror)
    {
        mirrorInfo(&labelInfo);
    }
    
    labelInit(arrOpenDoor, labelInfo, LABEL_TYPE_IMAGE,
              imageDir + "ui_arr/DOOR/open.gif", ARR, "NULL", false);
    labelInfo = ppartInfo->arrOpenText;
    
    if (gMainReceive.mirror)
    {
        mirrorInfo(&labelInfo);
    }
    
    labelInit(arrOpenText, labelInfo, LABEL_TYPE_IMAGE,
              imageDir + "ui_arr/DOOR/openText.png", ARR, "NULL", false);
    //qDebug() << "---Arr Close DOOR---";
    labelInfo = ppartInfo->serviceImg;
    
    if (gMainReceive.mirror)
    {
        mirrorInfo(&labelInfo);
    }
    
    labelInit(serviceImg, labelInfo, LABEL_TYPE_IMAGE,
              imageDir + "ui_arr/DOOR/service.png", ARR, "NULL", false);
    labelInfo = ppartInfo->TipsImg;
    
    if (gMainReceive.mirror)
    {
        mirrorInfo(&labelInfo);
    }
    
    labelInit(TipsImg, labelInfo, LABEL_TYPE_IMAGE,
              imageDir + "ui_arr/DOOR/Tips.png", ARR, "NULL", false);
//    labelInfo = ppartInfo->exportImg;
    
//    if (gMainReceive.mirror)
//    {
//        mirrorInfo(&labelInfo);
//    }
    
//    labelInit(exportImg, labelInfo, LABEL_TYPE_IMAGE,
//              imageDir + "ui_arr/DOOR/Escalator.png", ARR, "NULL", false);
    labelInfo = ppartInfo->arrCloseDoor;
    
    if (gMainReceive.mirror)
    {
        mirrorInfo(&labelInfo);
    }
    
    labelInit(arrCloseDoor, labelInfo, LABEL_TYPE_IMAGE,
              imageDir + "ui_arr/DOOR/close.png", ARR, "NULL", false);
    labelInfo = ppartInfo->arrCloseText;
    
    if (gMainReceive.mirror)
    {
        mirrorInfo(&labelInfo);
    }
    
    labelInit(arrCloseText, labelInfo, LABEL_TYPE_IMAGE,
              imageDir + "ui_arr/DOOR/closeText.png", ARR, "NULL", false);
    //qDebug() << "---Arr SN---";
    labelInfo = ppartInfo->arrOpenChSN;
    
    if (gMainReceive.mirror)
    {
        mirrorInfo(&labelInfo);
    }
    
    labelInit(arrChSN, labelInfo, LABEL_TYPE_TEXT,
              "NULL", ARR, "Source Han Sans CN", false);
    labelInfo = ppartInfo->arrOpenEnSN;
    
    if (gMainReceive.mirror)
    {
        mirrorInfo(&labelInfo);
    }
    
    labelInit(arrEnSN, labelInfo, LABEL_TYPE_TEXT,
              "NULL", ARR, "Oswald", false);
    arrChSN.setAlignment(Qt::AlignCenter);
    arrEnSN.setAlignment(Qt::AlignCenter);
    //qDebug() << "------Arr Chg------";
#if 0
    labelInfo = ppartInfo->arrChgText;
    
    if (gMainReceive.mirror)
    {
        mirrorInfo(&labelInfo);
    }
    
    labelInit(arrChgText, labelInfo, LABEL_TYPE_IMAGE,
              imageDir + "ui_arr/DOOR/chgText.png", ARR, "NULL", false);
#endif
    labelInfo = ppartInfo->arrChgLine;
    
    if (gMainReceive.mirror)
    {
        mirrorInfo(&labelInfo);
    }
    
    labelInit(arrChgLine, labelInfo, LABEL_TYPE_IMAGE, "NULL", ARR, "NULL", false);
    //qDebug() << "---Arr Img---";
    labelInfo = ppartInfo->arrExit;
    if(gMainReceive.mirror)
    {
        mirrorInfo(&labelInfo);
    }
    labelInit(arrExit, ppartInfo->arrExit, LABEL_TYPE_IMAGE, "NULL", ARR, "NULL", false);
    //qDebug() << "---Arr Train Car---";
    labelInfo = ppartInfo->arrTrain;
    
    if (gMainReceive.mirror)
    {
        mirrorInfo(&labelInfo);
    }
    
    labelInit(arrTrain, labelInfo, LABEL_TYPE_IMAGE,
              "NULL", ARR, "NULL", false);
              
    for (int i = 0; i < 8; i++)
    {
        labelInfo = ppartInfo->arrCar[i];
        
        if (gMainReceive.mirror)
        {
            mirrorInfo(&labelInfo);
        }
        
        labelInit(arrCar[i], labelInfo, LABEL_TYPE_IMAGE,
                  "NULL", ARR, "NULL", false);
    }
    
    for (int i = 0; i < 8; i++)
    {
        labelInfo = ppartInfo->curentCar[i];
        
        if (gMainReceive.mirror)
        {
            mirrorInfo(&labelInfo);
        }
        
        labelInit(curentCar[i], labelInfo, LABEL_TYPE_IMAGE,
                  "NULL", ARR, "NULL", false);
    }
}

void MainWindow::arrInit()
{
    bool debug = false;
    QString str;
    QString displayPath = filePath + "display.ini";
    QSettings settings_disp(displayPath, QSettings::IniFormat);
    settings_disp.setIniCodec("UTF8");
    part_info_t *ppartInfo = &gPartInfo;
    qDebug() << "------------------------------arrInit------------------------------";
    qDebug() << "---Arr Open DOOR && Img---";
    readLabelAttr(&ppartInfo->serviceImg, &settings_disp, "Service",
                  LABEL_OTHER_TYPE, "NULL", 0, debug);
    labelInit(serviceImg, ppartInfo->serviceImg, LABEL_TYPE_IMAGE,
              imageDir + "ui_arr/DOOR/service.png", ARR, "NULL", false);
    readLabelAttr(&ppartInfo->TipsImg, &settings_disp, "Tips",
                  LABEL_OTHER_TYPE, "NULL", 0, debug);
    labelInit(TipsImg, ppartInfo->TipsImg, LABEL_TYPE_IMAGE,
              imageDir + "ui_arr/DOOR/Tips.png", ARR, "NULL", false);
//    readLabelAttr(&ppartInfo->exportImg, &settings_disp, "export",
//                  LABEL_OTHER_TYPE, "NULL", 0, debug);
//    labelInit(exportImg, ppartInfo->exportImg, LABEL_TYPE_IMAGE,
//              imageDir + "ui_arr/DOOR/Escalator.png", ARR, "NULL", false);
    readLabelAttr(&ppartInfo->arrOpenDoor, &settings_disp, "ArrOpenDoor",
                  LABEL_OTHER_TYPE, "NULL", 0, debug);
    labelInit(arrOpenDoor, ppartInfo->arrOpenDoor, LABEL_TYPE_IMAGE,
              imageDir + "ui_arr/DOOR/open.gif", ARR, "NULL", false);
    readLabelAttr(&ppartInfo->arrOpenText, &settings_disp, "ArrOpenText",
                  LABEL_OTHER_TYPE, "NULL", 0, debug);
    labelInit(arrOpenText, ppartInfo->arrOpenText, LABEL_TYPE_IMAGE,
              imageDir + "ui_arr/DOOR/openText.png", ARR, "NULL", false);
    qDebug() << "---Arr Close DOOR---";
    readLabelAttr(&ppartInfo->arrCloseDoor, &settings_disp, "ArrCloseDoor",
                  LABEL_OTHER_TYPE, "NULL", 0, debug);
    labelInit(arrCloseDoor, ppartInfo->arrCloseDoor, LABEL_TYPE_IMAGE,
              imageDir + "ui_arr/DOOR/close.png", ARR, "NULL", false);
    readLabelAttr(&ppartInfo->arrCloseText, &settings_disp, "ArrCloseText",
                  LABEL_OTHER_TYPE, "NULL", 0, debug);
    labelInit(arrCloseText, ppartInfo->arrCloseText, LABEL_TYPE_IMAGE,
              imageDir + "ui_arr/DOOR/closeText.png", ARR, "NULL", false);
    qDebug() << "---Arr SN---";
    readLabelAttr(&ppartInfo->arrOpenChSN, &settings_disp, "ArrSN",
                  LABEL_CHEN_TYPE, "Ch", 0, debug);
    readLabelAttr(&ppartInfo->arrOpenEnSN, &settings_disp, "ArrSN",
                  LABEL_CHEN_TYPE, "En", 0, debug);
    labelInit(arrChSN, ppartInfo->arrOpenChSN, LABEL_TYPE_TEXT,
              "NULL", ARR, "Source Han Sans CN", false);
    labelInit(arrEnSN, ppartInfo->arrOpenEnSN, LABEL_TYPE_TEXT,
              "NULL", ARR, "Oswald", false);
    //    arrChSN.setAlignment(Qt::AlignCenter);
    //    arrEnSN.setAlignment(Qt::AlignCenter);
    arrChSN.hide();
    arrEnSN.hide();
#if 0
    qDebug() << "------Arr Chg------";
    readLabelAttr(&ppartInfo->arrChgText, &settings_disp, "ArrChgText",
                  LABEL_OTHER_TYPE, "NULL", 0, debug);
    labelInit(arrChgText, ppartInfo->arrChgText, LABEL_TYPE_IMAGE,
              imageDir + "ui_arr/DOOR/chgText.png", ARR, "NULL", false);
#endif
    readLabelAttr(&ppartInfo->arrChgLine, &settings_disp, "ArrChgLine",
                  LABEL_OTHER_TYPE, "NULL", 0, debug);
    labelInit(arrChgLine, ppartInfo->arrChgLine, LABEL_TYPE_IMAGE,
              "NULL", ARR, "NULL", false);
    readLabelAttr(&ppartInfo->arrSp, &settings_disp, "ArrSp",
                  LABEL_OTHER_TYPE, "NULL", 0, debug);
    labelInit(arrSp, ppartInfo->arrSp, LABEL_TYPE_IMAGE,
              "NULL", ARR, "NULL", false);
    qDebug() << "---Arr Img---";
    readLabelAttr(&ppartInfo->arrExit, &settings_disp, "ArrExit", LABEL_OTHER_TYPE, "NULL", 0, debug);
    labelInit(arrExit, ppartInfo->arrExit, LABEL_TYPE_IMAGE, "NULL", ARR, "NULL", false);
    qDebug() << "---Arr Train Car---";
    readLabelAttr(&ppartInfo->arrTrain, &settings_disp, "ArrTrain",
                  LABEL_OTHER_TYPE, "NULL", 0, debug);
    //labelInit(arrTrain, ppartInfo->arrTrain, LABEL_TYPE_IMAGE, "NULL", ARR, "NULL", false);
    arrTrainWidget.setParent(&widget[ARR]);
    //    arrTrainWidget.setGeometry(ppartInfo->arrTrain.rectUp.x,
    //                               ppartInfo->arrTrain.rectUp.y,
    //                               ppartInfo->arrTrain.rectUp.w,
    //                               ppartInfo->arrTrain.rectUp.h);
    arrTrain.setParent(&arrTrainWidget);
    arrTrain.setGeometry(ppartInfo->arrTrain.rectUp.x,
                         ppartInfo->arrTrain.rectUp.y,
                         ppartInfo->arrTrain.rectUp.w,
                         ppartInfo->arrTrain.rectUp.h);
                         
    for (int i = 0; i < 8; i++)
    {
        str = QString("%1%2").arg("ArrCar_").arg(i + 1, 2, 10, QLatin1Char('0'));
        readLabelAttr(&ppartInfo->arrCar[i], &settings_disp, str,
                      LABEL_OTHER_TYPE, "Ch", i, debug);
        labelInit(arrCar[i], ppartInfo->arrCar[i], LABEL_TYPE_IMAGE,
                  "NULL", ARR, "NULL", false);
    }
    
    for (int i = 0; i < 8; i++)
    {
        str = QString("%1%2").arg("CurentCar_").arg(i + 1, 2, 10, QLatin1Char('0'));
        readLabelAttr(&ppartInfo->curentCar[i], &settings_disp, str,
                      LABEL_OTHER_TYPE, "Ch", i, debug);
        labelInit(curentCar[i], ppartInfo->curentCar[i], LABEL_TYPE_IMAGE,
                  "NULL", ARR, "NULL", false);
    }
    
    qDebug() << "---Arr DOOR ERR---";
    labelInit(arrDoorErr, gPublicInfo.DoorErr, LABEL_TYPE_IMAGE,
              imageDir + "doorError.png", ARR, "NULL", false);

    labelInit(arrImetion, gPublicInfo.Imetion, LABEL_TYPE_IMAGE,
             /* "imageDir + "imetion/01.png"*/"NULL", ARR, "NULL", false);


}


/*init display*/
void MainWindow::initDipaly()
{
    QString str;
    QString displayPath = filePath + "display.ini";
    QSettings settings_disp(displayPath, QSettings::IniFormat);
    settings_disp.setIniCodec("UTF8");
    /*###### 1.导航栏显示初始化 ######*/
    publicInit();
    /*###### 2.全局界面显示初始化 ######*/
    allInit();
    /*###### 3.局部界面显示初始化 ######*/
    preInit();
    /*###### 4.到站界面显示初始化 ######*/
    arrInit();
    /*###### 5.启动界面显示参数初始化 ######*/
    versionLabel.setParent(&widget[INFODISP]);
    versionLabel.setGeometry(200, 40, 500, 50);
    carIdLabel.setParent(&widget[INFODISP]);
    carIdLabel.setGeometry(200, 100, 500, 50);
    devIdLabel.setParent(&widget[INFODISP]);
    devIdLabel.setGeometry(500, 100, 500, 50);
    ipLabel.setParent(&widget[INFODISP]);
    ipLabel.setGeometry(200, 160, 500, 50);
    ipConflictLabel.setParent(&widget[INFODISP]);
    ipConflictLabel.setGeometry(300, 160, 500, 50);
    macLabel.setParent(&widget[INFODISP]);
    macLabel.setGeometry(500, 160, 500, 50);
    setFont(versionLabel, "Oswald", false, 34, 200, 200, 200);
    versionLabel.setAlignment(Qt::AlignLeft);
    setFont(carIdLabel, "Oswald", false, 34, 200, 200, 200);
    carIdLabel.setAlignment(Qt::AlignLeft);
    setFont(devIdLabel, "Oswald", false, 34, 200, 200, 200);
    devIdLabel.setAlignment(Qt::AlignLeft);
    setFont(ipLabel, "Oswald", false, 34, 200, 200, 200);
    ipLabel.setAlignment(Qt::AlignLeft);
    setFont(ipConflictLabel, "Oswald", false, 34, 200, 200, 200);
    ipConflictLabel.setAlignment(Qt::AlignLeft);
    setFont(macLabel, "Oswald", false, 34, 200, 200, 200);
    macLabel.setAlignment(Qt::AlignLeft);
    //    QString str;
    str = "Ip: " + mytimer.ipStr;
    ipLabel.setText(str);
    str = QString("%1%2%3%4").arg("version: ").arg(VERSION_MAX).arg(".").arg(VERSION_MIN);
    versionLabel.setText(str);
    str = "Mac: " + macStr;
    macLabel.setText(str);
    int carid = (ip4 - 1) / 10 + 1;
    int carno = (ip4 - 1) % 10 + 1;
    QString caridStr = QString("%1%2%3%4").arg("Car: ").arg(carid).arg("        Num: ").arg(carno);
    carIdLabel.setText(caridStr);
    int dev = (carid - 1) * 8 + carno;
    QString devidStr = QString("%1%2").arg("Dev: ").arg(dev);
    devIdLabel.setText(devidStr);
    //setInfo();
    setWelcome();
    /*###### 6.颜色显示参数初始化 ######*/
    holeDisp.setParent(&widget[COLORDISP]);
    holeDisp.setGeometry(0, 0, 1920, 360);
    holeDisp.setAutoFillBackground(true);
    
    for (int i = 0; i < 16; i++)
    {
        grayScale16[i].setParent(&widget[COLORDISP]);
        grayScale16[i].setGeometry((1920 / 16)*i, 0, 1920 / 16, 360);
        grayScale16[i].setAutoFillBackground(true);
    }
}

void MainWindow::InitEnv()
{
    qDebug() << "InitEnv";
    run_st = SET_IDLE;
    leaveSt = LEAVE_ST_OFF;
    arrToLeaveFlag = 0;
    imageInitFlag = 0;
    colorDispUpDown = 0;
    /*(1)init Socket*/
    connect(&mycontroludp, SIGNAL(running_trigger_signal()), this, SLOT(running_trigger()));
    connect(&mycontroludp, SIGNAL(setDisplayType(BYTE)), this, SLOT(MainDisplayType(BYTE)));
    connect(&mycontroludp, SIGNAL(setColorTest(BYTE , BYTE , BYTE )), this,
            SLOT(setColorTest(BYTE , BYTE , BYTE )));

//    connect(&mycontroludp, SIGNAL(syn_signal()), this, SLOT(syndistance()));

#if 1
    psysTimer = new QTimer(this);
    connect(psysTimer, SIGNAL(timeout()), this, SLOT(sysproc()));
    psysTimer->start(1000);
#endif
    connect(&mytimer, SIGNAL(timeOut20ms()), this, SLOT(TimeoutProc()));
    connect(&mytimer, SIGNAL(timeOut1000ms()), &mycontroludp, SLOT(mainSendProc()));
    mytimer.stopFlag = 0;
    mytimer.startFlag = 0;
    connect(&mytimer2, SIGNAL(timeOut20ms()), this, SLOT(TimeoutProc()));
    connect(&mytimer2, SIGNAL(timeOut1000ms()), &mycontroludp, SLOT(mainSendProc()));
    mytimer2.stopFlag = 0;
    mytimer2.startFlag = 0;
    connect(&mytimer3, SIGNAL(timeOut20ms()), this, SLOT(TimeoutProc()));
    connect(&mytimer3, SIGNAL(timeOut1000ms()), &mycontroludp, SLOT(mainSendProc()));
    mytimer3.stopFlag = 0;
    mytimer3.startFlag = 0;
    /*gif movie初始化*/
    movieAllTrack = new QMovie(this);
    movieAllTrack->setCacheMode(QMovie::CacheAll);
    moviePartDoor = new QMovie(this);
    moviePartDoor->setCacheMode(QMovie::CacheAll);
    moviePreTrack = new QMovie(this);
    moviePreTrack->setCacheMode(QMovie::CacheAll);
    movieArrDoor = new QMovie(this);
    movieArrDoor->setCacheMode(QMovie::CacheAll);
    movieArrFlash = new QMovie(this);
    movieArrFlash->setCacheMode(QMovie::CacheAll);
#if 0
    arrTrainMovie = new QMovie(this);
    arrTrainMovie->setCacheMode(QMovie::CacheAll);
    connect(this->arrTrainMovie, SIGNAL(finished()), this, SLOT(dispArrCar()));
    arrCarMovie = new QMovie(this);
    arrCarMovie->setCacheMode(QMovie::CacheAll);
#endif
    weeklist << "Monday" << "Tuesday" << "Wednesday" << "Thursday" << "Friday" << "Saturday" << "Sunday";
    weekchlist << "星期一" << "星期二" << "星期三" << "星期四" << "星期五" << "星期六" << "星期日";
}

void MainWindow::dispArrCar()
{
    QString filename;
    int carid = (ip4 - 1) / 10 + 1;
    QString strCar = QString("%1").arg(carid, 02, 10, QLatin1Char('0'));
    qDebug() << "drmd==>dispArrCar()";
    arrCarMovie->stop();
    
    if (gMainReceive.keySide == 1)
    {
        if (gMainReceive.mirror == 0)
        {
            filename = imageDir + "ui_arr/TRAIN/" + strCar + "R.gif";
            arrCar[6 - carid].setMovie(arrCarMovie);
            arrCar[6 - carid].show();
        }
        else
        {
            filename = imageDir + "ui_arr/TRAIN/" + strCar + "L.gif";
            arrCar[carid - 1].setMovie(arrCarMovie);
            arrCar[carid - 1].show();
        }
    }
    
    if (gMainReceive.keySide == 2)
    {
        if (gMainReceive.mirror == 0)
        {
            filename = imageDir + "ui_arr/TRAIN/" + strCar + "L.gif";
            arrCar[carid - 1].setMovie(arrCarMovie);
            arrCar[carid - 1].show();
        }
        else
        {
            filename = imageDir + "ui_arr/TRAIN/" + strCar + "R.gif";
            arrCar[6 - carid].setMovie(arrCarMovie);
            arrCar[6 - carid].show();
        }
    }
    
    arrCarMovie->setFileName(filename);
    arrCarMovie->setSpeed(100);
    arrCarMovie->start();
}

void MainWindow::imageInit()
{
    imageInitFlag = 1;
    qDebug() << "==>image init finish";
}

void MainWindow::chgToWidget(int index)
{
    for (int i = 0; i < WIDGET_NUM; i++)
    {
        if (i == index)
        {
            widgetIndex = i;
            widget[i].show();
            qDebug() << "chg widget:" << index;
        }
        else
            widget[i].hide();
    }
}


void MainWindow::setFont(QLabel &label, QString name, bool bold, int size, int r, int g, int b)
{
    QFont font;
    QColor color;
    QPalette palette;
    font.setPixelSize(size);
    font.setBold(bold);
    font.setFamily(name);
    //    font.setWeight(weight);
    color.setRgb(r, g, b);
    palette.setColor(QPalette::WindowText, color);
    label.setPalette(palette);
    //    label.setAlignment(Qt::AlignCenter);
    //    label.setWordWrap(true);
    //    label.setLineWidth(1);
    label.setFont(font);
}

bool MainWindow::checkStation(int start, int current, int next, int end, int station1, int station2)
{
    if (start == end)
    {
        qDebug() << "------------station false------------";
        return false;
    }
    else if (start >= station1 && start <= current && current <= next && next <= end && end <= station2)
    {
        return true;
    }
    else if (start <= station2 && start >= current && current >= next && next >= end && end >= station1)
    {
        return true;
    }
    else
    {
        qDebug() << "------------station false------------";
        return false;
    }
}


void MainWindow::setUpdate(int update)
{
    chgToWidget(WELCOME);
    updateLable.show();
    welcomeLable.hide();
    QColor color;
    QPalette palette;
    color.setRgb(0, 0, 0);
    palette = QPalette(updateLable.palette());
    updateLable.setAutoFillBackground(true);
    palette.setColor(QPalette::Background, color);
    updateLable.setPalette(palette);
    
    if (update)
        updateLable.setText("正在升级程序，请等待！");
    else
        updateLable.setText("升级成功");
}

void MainWindow::setInfo()
{
    chgToWidget(INFODISP);
}

void MainWindow::setColorTest(BYTE mode, BYTE time, BYTE index)
{
    cycleMode = mode;
    
    if (time > 0)
        cycleTime = time;
        
    if ((index == 0) || (index == 0xff))
        cycleIndex = 0xff;
    else if (index == 0xFA)
    {
        cycleIndex = 0xff;
        colorDispUpDown = 1;
    }
    else
        cycleIndex = index - 1;
        
    if (mode == SET_CYCLE_TEST_ON)
    {
        chgToWidget(COLORDISP);
    }
    else
        setWelcome();
        
    qDebug() << "color cycle test";
    qDebug() << "mode:" << cycleMode;
    qDebug() << "time:" << cycleTime;
    qDebug() << "index:" << cycleIndex;
}


void MainWindow::setWelcome()
{
    chgToWidget(WELCOME);
    welcomeLable.show();
    updateLable.hide();
}

void MainWindow::screenshot(void)
{
    QString format = "jpg";
    //    QString initialPath = QDir::currentPath() + tr("/untitled.") + format;
    //    QString fileName = QFileDialog::getSaveFileName(this, tr("Save As"),
    //                               initialPath,
    //                               tr("%1 Files (*.%2);;All Files (*)")
    //                               .arg(format.toUpper())
    //                               .arg(format));
    QDir myDir(filePath + "images/");
    QFile rmFile;
    
    for (unsigned int i = 0; i < myDir.count(); i++)
    {
        QString sext = myDir[i].toLower();
        //QTextStream out(stdout);
        //qDebug()<<sext;
        
        if (sext.contains("screenshot", Qt::CaseSensitive))
        {
            rmFile.setFileName(filePath + "images/" + sext);
            
            if (!rmFile.remove())
                qDebug() << "==>remove err:" << rmFile.errorString();
                
            //QFile::remove(filePath+"images/"+sext);//刪除文件
            qDebug() << "==>remove file:" << sext;
        }
    }
    
    QString fileName;
    fileName = imageDir + "screenshot" + QDateTime::currentDateTime().toString("yyyyMMdd_hhmmss") + ".jpg";
    QFile file(fileName);
    originalPixmap = QPixmap();
    originalPixmap = QPixmap::grabWindow(widget[widgetIndex].winId());
    originalPixmap.save(fileName, format.toAscii());
    qDebug() << "==>create file:" << fileName;
    
    if (file.exists())
        file.flush();
}

void MainWindow::ipConflictProc(int flag)
{
    if (flag)
    {
        chgToWidget(WELCOME);
        updateLable.show();
        welcomeLable.hide();
        QString str = "IP: " + mytimer.ipStr + " Conflict";
        updateLable.clear();
        updateLable.setText(str);
    }
    else
    {
        setWelcome();
    }
}

void MainWindow::flashProc()
{
    QString str1, str2, str_r;
    leave_chg_time++;
    change_time++;
    
    //    if((leave_chg_time%6 !=0)||(imageInitFlag==0))   // 20ms 中断一次，n?ms更新动画
    //        return;
    if (leaveSt == LEAVE_ST_ARR) // 到站
    {
        return;
    }
    setSiteTime(ALL);
    if ((leaveSt == LEAVE_ST_ALL) && (widgetIndex == ALL))
    {
        if (leave_chg_time % 100 == 0)
        {
            if (gMainReceive.start < gMainReceive.end)
            {
                if (allEnd[gMainReceive.end - 1]->isVisible())
                {
                    allEnd[gMainReceive.end - 1]->hide();
                }
                else
                {
                    allEnd[gMainReceive.end - 1]->show();
                }
            }
            else
            {
                if (allEnd[StationMaxNumber - gMainReceive.end]->isVisible())
                {
                    allEnd[StationMaxNumber - gMainReceive.end]->hide();
                }
                else
                {
                    allEnd[StationMaxNumber - gMainReceive.end]->show();
                }
            }
        }
    }
    
    if ((leaveSt == LEAVE_ST_PRE) && (widgetIndex == PRE))
    {

    }
    
#ifdef ALL_TEST
    return;

#endif
#ifdef PRE_TEST
    return;
#endif
    
#if 1
    if (leaveSt && (leave_chg_time > 500) && (widgetIndex >= ALL))
    {
        if (leaveSt == LEAVE_ST_PRE)
        {
            leaveSt = LEAVE_ST_ALL;
            leave_chg_time = 0;
            allProc();
        }
        else
        {
            leaveSt = LEAVE_ST_PRE;
            leave_chg_time = 0;
            preProc();
        }
    }
#endif


    
    if (gMainReceive.start < gMainReceive.end)
    {
        allSiteFlash.setGeometry(allSite[gMainReceive.next - 1]->x(),
                                 allSite[gMainReceive.next - 1]->y(),
                                 allSite[gMainReceive.next - 1]->width(),
                                 allSite[gMainReceive.next - 1]->height());
        preSiteFlash.setGeometry(preSite[2]->x(),
                                 preSite[2]->y(),
                                 preSite[2]->width(),
                                 preSite[2]->height());
    }
    
    if (gMainReceive.start > gMainReceive.end)
    {
        allSiteFlash.setGeometry(allSite[stationNum - gMainReceive.next]->x(),
                                 allSite[stationNum - gMainReceive.next]->y(),
                                 allSite[stationNum - gMainReceive.next]->width(),
                                 allSite[stationNum - gMainReceive.next]->height());
        preSiteFlash.setGeometry(preSite[2]->x(),
                                 preSite[2]->y(),
                                 preSite[2]->width(),
                                 preSite[2]->height());
    }
    
    if (gMainReceive.next >= NOT_IN_SERVICE)
    {
        allSiteFlash.hide();
    }
    
#ifdef PREDO
    
    if (gMainReceive.start < gMainReceive.end)
    {
        if (gMainReceive.next == 2 || gMainReceive.next == gMainReceive.end - 1 )
        {
            if (gMainReceive.mirror)
            {
                if (gMainReceive.next == 2)
                {
                    preSiteFlash.setGeometry(preSite[2]->x() + 253,
                                             preSite[2]->y() - 18,
                                             preSite[2]->width() + 36,
                                             preSite[2]->height() + 36);
                }
                
                if (gMainReceive.next == gMainReceive.end - 1)
                {
                    preSiteFlash.setGeometry(preSite[2]->x() - 217,
                                             preSite[2]->y() - 18,
                                             preSite[2]->width() + 36,
                                             preSite[2]->height() + 36);
                }
            }
            else
            {
                if (gMainReceive.next == 2)
                {
                    preSiteFlash.setGeometry(preSite[2]->x() - 217,
                                             preSite[2]->y() - 18,
                                             preSite[2]->width() + 36,
                                             preSite[2]->height() + 36);
                }
                
                if (gMainReceive.next == gMainReceive.end - 1)
                {
                    preSiteFlash.setGeometry(preSite[2]->x() + 253,
                                             preSite[2]->y() - 18,
                                             preSite[2]->width() + 36,
                                             preSite[2]->height() + 36);
                }
            }
        }
        
        if (gMainReceive.next == gMainReceive.end)
        {
            if (gMainReceive.mirror)
            {
                preSiteFlash.setGeometry(preSite[2]->x() - 452,
                                         preSite[2]->y() - 18,
                                         preSite[2]->width() + 36,
                                         preSite[2]->height() + 36);
            }
            else
            {
                preSiteFlash.setGeometry(preSite[2]->x() + 488,
                                         preSite[2]->y() - 18,
                                         preSite[2]->width() + 36,
                                         preSite[2]->height() + 36);
            }
        }
    }
    else
    {
        if (gMainReceive.next == 2 || gMainReceive.next == gMainReceive.start - 1 )
        {
            if (gMainReceive.mirror)
            {
                if (gMainReceive.next == 2)
                {
                    preSiteFlash.setGeometry(preSite[2]->x() - 217,
                                             preSite[2]->y() - 18,
                                             preSite[2]->width() + 36,
                                             preSite[2]->height() + 36);
                }
                
                if (gMainReceive.next == gMainReceive.start - 1)
                {
                    preSiteFlash.setGeometry(preSite[2]->x() + 253,
                                             preSite[2]->y() - 18,
                                             preSite[2]->width() + 36,
                                             preSite[2]->height() + 36);
                }
            }
            else
            {
                if (gMainReceive.next == 2)
                {
                    preSiteFlash.setGeometry(preSite[2]->x() + 253,
                                             preSite[2]->y() - 18,
                                             preSite[2]->width() + 36,
                                             preSite[2]->height() + 36);
                }
                
                if (gMainReceive.next == gMainReceive.start - 1)
                {
                    preSiteFlash.setGeometry(preSite[2]->x() - 217,
                                             preSite[2]->y() - 18,
                                             preSite[2]->width() + 36,
                                             preSite[2]->height() + 36);
                }
            }
        }
        
        if (gMainReceive.next == gMainReceive.end)
        {
            if (gMainReceive.mirror)
            {
                preSiteFlash.setGeometry(preSite[2]->x() - 452,
                                         preSite[2]->y() - 18,
                                         preSite[2]->width() + 36,
                                         preSite[2]->height() + 36);
            }
            else
            {
                preSiteFlash.setGeometry(preSite[2]->x() + 488,
                                         preSite[2]->y() - 18,
                                         preSite[2]->width() + 36,
                                         preSite[2]->height() + 36);
            }
        }
    }
    
    if (change_time < 50)
    {
        allSiteFlash.setPixmap(imageDir + "ui_all/SITE/next.png");
        preSiteFlash.setPixmap(imageDir + "ui_pre/SITE/next.png");
    }
    
    if (change_time >= 50)
    {
        allSiteFlash.setPixmap(imageDir + "ui_all/SITE/no.png");
        preSiteFlash.setPixmap(imageDir + "ui_pre/SITE/pass.png");
    }
    
    if (change_time >= 100)
        change_time = 0;
        
    static  int flowUpIndex = gMainReceive.current + 1;
    static  int flowDownIndex = gMainReceive.current - 1;
    int tmpEnd = 0;
    PrechageTime++;
    
    if (PrechageTime < 20)
    {
        if (gMainReceive.start < gMainReceive.end)
        {
            if (gMainReceive.mirror)
            {
                if (flowUpIndex >= (gMainReceive.end - 1))
                {
                    flowUpIndex = gMainReceive.next;
                }

                if(gMainReceive.end >= NOT_IN_SERVICE)
                    tmpEnd = NOT_IN_SERVICE - 1;
                else
                    tmpEnd = gMainReceive.end;

                if (tmpEnd - gMainReceive.next > 2)
                {
                    if(gMainReceive.next < NOT_IN_SERVICE)
                    {
                        if(flowUpIndex >= 30)
                            flowUpIndex = gMainReceive.next;
                        allFlowLed1.show();
                        allFlowLed2.show();
                        allFlowLed1.setGeometry(gAllInfo.track[stationNum - flowUpIndex - 1].rectUp.x - 1,
                                                gAllInfo.track[stationNum - flowUpIndex].rectUp.y,
                                                gAllInfo.track[stationNum - flowUpIndex].rectUp.w,
                                                gAllInfo.track[stationNum - flowUpIndex].rectUp.h);
                        allFlowLed2.setGeometry(gAllInfo.track[stationNum - flowUpIndex - 2].rectUp.x - 1,
                                                gAllInfo.track[stationNum - flowUpIndex].rectUp.y,
                                                gAllInfo.track[stationNum - flowUpIndex].rectUp.w,
                                                gAllInfo.track[stationNum - flowUpIndex].rectUp.h);

                    }
                    else
                    {
                        allFlowLed1.hide();
                        allFlowLed2.hide();
                    }

                }
                else
                {
                    allFlowLed1.hide();
                    allFlowLed2.hide();

                }

            }
            else
            {
                if (flowUpIndex + 1 >= (gMainReceive.end - 1))
                {
                    flowUpIndex = gMainReceive.current;
                }

                if(gMainReceive.end >= NOT_IN_SERVICE)
                    tmpEnd = NOT_IN_SERVICE - 1;
                else
                    tmpEnd = gMainReceive.end;

                if (tmpEnd - gMainReceive.next > 2)
                {

                    allFlowLed1.show();
                    allFlowLed2.show();
                    allFlowLed1.setGeometry(gAllInfo.track[flowUpIndex].rectUp.x,
                                            gAllInfo.track[flowUpIndex].rectUp.y,
                                            gAllInfo.track[flowUpIndex].rectUp.w,
                                            gAllInfo.track[flowUpIndex].rectUp.h);
                    allFlowLed2.setGeometry(gAllInfo.track[flowUpIndex + 1].rectUp.x,
                                            gAllInfo.track[flowUpIndex].rectUp.y,
                                            gAllInfo.track[flowUpIndex].rectUp.w,
                                            gAllInfo.track[flowUpIndex].rectUp.h);
                }
                else
                {
                    allFlowLed1.hide();
                    allFlowLed2.hide();

                }

            }
        }
        else
        {
            if (gMainReceive.start > gMainReceive.end)
            {
                if (gMainReceive.mirror)
                {
                    if (flowDownIndex <= (gMainReceive.end))
                    {
                        flowDownIndex = gMainReceive.next - 1;
                    }

                    if(gMainReceive.next > 3)
                    {
                        allFlowLed1.show();
                        allFlowLed2.show();
                        if (gMainReceive.next - gMainReceive.end > 2)
                        {
                            allFlowLed1.setGeometry(gAllInfo.track[flowDownIndex - 1].rectUp.x - 1,
                                                    gAllInfo.track[flowDownIndex].rectUp.y,
                                                    gAllInfo.track[flowDownIndex].rectUp.w,
                                                    gAllInfo.track[flowDownIndex].rectUp.h);
                            allFlowLed2.setGeometry(gAllInfo.track[flowDownIndex - 2].rectUp.x - 1,
                                                    gAllInfo.track[flowDownIndex].rectUp.y,
                                                    gAllInfo.track[flowDownIndex].rectUp.w,
                                                    gAllInfo.track[flowDownIndex].rectUp.h);
                        }
                        else
                        {
                            allFlowLed1.hide();
                            allFlowLed2.hide();

                        }
                    }
                    else
                    {
                        allFlowLed1.hide();
                        allFlowLed2.hide();
                    }
                }
                else
                {
                    if (flowDownIndex - 1 <= (gMainReceive.end))
                    {
                        flowDownIndex = gMainReceive.current - 1;
                    }
                    

                    if (gMainReceive.next - gMainReceive.end > 2)
                    {
                        allFlowLed1.show();
                        allFlowLed2.show();

                        allFlowLed1.setGeometry(gAllInfo.track[stationNum - flowDownIndex].rectUp.x,
                                                gAllInfo.track[stationNum - flowDownIndex].rectUp.y,
                                                gAllInfo.track[stationNum - flowDownIndex].rectUp.w,
                                                gAllInfo.track[stationNum - flowDownIndex].rectUp.h);
                        allFlowLed2.setGeometry(gAllInfo.track[stationNum - flowDownIndex + 1].rectUp.x,
                                                gAllInfo.track[stationNum - flowDownIndex].rectUp.y,
                                                gAllInfo.track[stationNum - flowDownIndex].rectUp.w,
                                                gAllInfo.track[stationNum - flowDownIndex].rectUp.h);
                    }
                    else
                    {
                        allFlowLed1.hide();
                        allFlowLed2.hide();

                    }
                }
            }
        }
    }
    
    if (PrechageTime >= 20)
    {
        PrechageTime = 0;
        
        if (gMainReceive.start < gMainReceive.end)
        {
            flowUpIndex++;
        }
        
        if (gMainReceive.start > gMainReceive.end)
        {
            flowDownIndex--;
        }
    }
    
#endif
}

void MainWindow::TimeoutProc()
{
    flashProc();
}



void MainWindow::sysproc()
{
    //static unsigned int info_time = 0;
    static unsigned int colorDispTime = 0;
    QString wstr, dstr, temp;
    wstr = QDateTime::currentDateTime().toString("dddd");
    static int life1 = 0, life2 = 0, life3 = 0, life4 = 0;
    static int cnt1 = 0, cnt2 = 0, cnt3 = 0, cnt4 = 0;
    int colorMask = 7;
#if 1//调试
    
    if ((mytimer.startFlag == 0) && (mytimer.stopFlag == 0))
    {
        imageInit();
        cnt1 = 0;
        mytimer.startFlag = 1;
        mytimer.start();
    }
    
#endif
    
    if (widgetIndex == COLORDISP)
    {
        if ((colorDispTime % cycleTime == 0) || (cycleIndex != 0xff))
        {
            QColor color;
            QPalette palette;
            holeDisp.hide();
            
            for (int i = 0; i < 16; i++)
                grayScale16[i].hide();
                
            for (int i = 0; i < 32; i++)
                grayScale32[i].hide();
                
            for (int i = 0; i < 64; i++)
                grayScale64[i].hide();
                
            if (cycleIndex != 0xff)
                colorDispIndex = cycleIndex;
                
            if (colorDispUpDown)
                colorMask = 3;
            else
                colorMask = 7;
                
            switch (colorDispIndex % colorMask)
            {
                case 0:
                    color.setRgb(255, 0, 0);
                    palette.setColor(QPalette::Background, color);
                    holeDisp.setPalette(palette);
                    holeDisp.show();
                    //qDebug()<<"---red disp---";
                    break;
                    
                case 1:
                    color.setRgb(0, 255, 0);
                    palette.setColor(QPalette::Background, color);
                    holeDisp.setPalette(palette);
                    holeDisp.show();
                    //qDebug()<<"---green disp---";
                    break;
                    
                case 2:
                    color.setRgb(0, 0, 255);
                    palette.setColor(QPalette::Background, color);
                    holeDisp.setPalette(palette);
                    holeDisp.show();
                    //qDebug()<<"---blue disp---";
                    break;
                    
                case 3:
                    color.setRgb(0, 0, 0);
                    palette.setColor(QPalette::Background, color);
                    holeDisp.setPalette(palette);
                    holeDisp.show();
                    //qDebug()<<"---black disp---";
                    break;
                    
                case 4:
                    color.setRgb(128, 128, 128);
                    palette.setColor(QPalette::Background, color);
                    holeDisp.setPalette(palette);
                    holeDisp.show();
                    //qDebug()<<"---gray m disp---";
                    break;
                    
                case 5:
                    color.setRgb(255, 255, 255);
                    palette.setColor(QPalette::Background, color);
                    holeDisp.setPalette(palette);
                    holeDisp.show();
                    //qDebug()<<"---white disp---";
                    break;
                    
                case 6:
                    for (int i = 0; i < 16; i++)
                    {
                        color.setRgb(i * 256 / 16, i * 256 / 16, i * 256 / 16);
                        palette.setColor(QPalette::Background, color);
                        grayScale16[i].setPalette(palette);
                        grayScale16[i].show();
                    }
                    
                    //qDebug()<<"---gray scale16 disp---";
                    break;
                    
                case 7:
                    for (int i = 0; i < 32; i++)
                    {
                        color.setRgb(i * 256 / 32, i * 256 / 32, i * 256 / 32);
                        palette.setColor(QPalette::Background, color);
                        grayScale32[i].setPalette(palette);
                        grayScale32[i].show();
                    }
                    
                    qDebug() << "---gray scale32 disp---";
                    break;
                    
                case 8:
                    for (int i = 0; i < 64; i++)
                    {
                        color.setRgb(i * 256 / 64, i * 256 / 64, i * 256 / 64);
                        palette.setColor(QPalette::Background, color);
                        grayScale64[i].setPalette(palette);
                        grayScale64[i].show();
                    }
                    
                    qDebug() << "---gray scale64 disp---";
                    break;
                    
                default:
                    break;
            }
            
            colorDispIndex++;
        }
        
        colorDispTime++;
    }
    
#if 0
    
    for (int i = 0; i < weeklist.size(); i++)
    {
        temp = weeklist.at(i);
        
        if (QString::compare(wstr, temp) == 0)
        {
            dstr = weekchlist.at(i);
            break;
        }
    }
    
    QString h = QDateTime::currentDateTime().toString("hh");
    QString m = QDateTime::currentDateTime().toString("mm");
    //    QString stime = h.mid(0,1)+h.mid(1,1)+" : "+m.mid(0,1)+" "+m.mid(1,1);
    wstr = QDateTime::currentDateTime().toString("ddd");
    dateLabel.setText(QDateTime::currentDateTime().toString("yyyy-MM-dd"));
    timeLabel.setText(h + ":" + m);
    weekChLabel.setText(dstr);
    weekEnLabel.setText("/" + wstr);
    preDateLabel.setText(QDateTime::currentDateTime().toString("yyyy-MM-dd"));
    preTimeLabel.setText(h + ":" + m);
    preWeekChLabel.setText(dstr);
    preWeekEnLabel.setText("/" + wstr);
    arrDateLabel.setText(QDateTime::currentDateTime().toString("yyyy-MM-dd"));
    arrTimeLabel.setText(h + ":" + m);
    arrWeekChLabel.setText(dstr);
    arrWeekEnLabel.setText("/" + wstr);
#endif
    
    //定时器生命信号处理
    if (life1 != mytimer.life)
        cnt1 = 0;
        
    if (life2 != mytimer2.life)
        cnt2 = 0;
        
    if (life3 != mytimer3.life)
        cnt3 = 0;
        
    life1 = mytimer.life;
    life2 = mytimer2.life;
    life3 = mytimer3.life;
    
    if ((++cnt1 > 5) && (mytimer.startFlag > 0))
    {
        mytimer.startFlag = 0;
        mytimer.stopFlag = 1;
        cnt2 = 0;
        mytimer2.startFlag = 2;
        mytimer2.start();
        qDebug() << "drmd==>mytimer stop && mytimer2 start";
    }
    
    if ((++cnt2 > 5) && (mytimer2.startFlag > 0))
    {
        mytimer2.startFlag = 0;
        mytimer2.stopFlag = 1;
        cnt3 = 0;
        mytimer3.startFlag = 3;
        mytimer3.start();
        qDebug() << "drmd==>mytimer2 stop && mytimer3 start";
    }
    
    if ((++cnt3 > 5) && (mytimer3.startFlag > 0))
    {
        mytimer3.startFlag = 0;
        mytimer3.stopFlag = 1;
        qDebug() << "drmd==>mytimer3 stop";
        this->close();
    }
}

void MainWindow::MainDisplayType(BYTE type)
{
    run_st = type;
    
    if ((widgetIndex == COLORDISP) && (colorDispUpDown == 0))
        return;
        
    colorDispUpDown = 0;
    
    switch (type)
    {
        case SET_OPEN:
            break;
            
        case SET_CLOSE:
            break;
            
        case SET_EME:
            break;
            
        case SET_NOSIGNAL:
            setWelcome();
            break;
            
        case SET_CLOSE_LCD:
            break;
            
        case SET_UPDATE_ING:
            updateFlag = 1;
            setUpdate(1);
            break;
            
        case SET_UPDATE_OK:
            updateFlag = 0;
            setUpdate(0);
            break;
            
        case SET_UPDATE_ERROR:
            break;
            
        case SET_IP_CONFLICT_ON:
            ipConflictProc(1);
            break;
            
        case SET_IP_CONFLICT_OFF:
            ipConflictProc(0);
            break;
            
        case SET_UP_DOWN_CHG:
            setColorTest(SET_CYCLE_TEST_ON, 2, 0xFA);
            break;
            
        default:
            break;
    }
}

void MainWindow::setSiteTime(int uiIndex)
{
    QColor color;
    QPalette palette;
    QString str;
    int time = 0;
    int pretime = 0;
    timeflag++;
    static int changeflag = 1;
    int tmp_distance = gMainReceive.next_distance;
    int tmp_id = gMainReceive.current;
    int tmp_time = 0;

    if((tmp_distance != 0)&&(tmpSiteTime[tmp_id] != 0)&&(distance[tmp_id - 1] != 0))
        tmp_time = (tmp_distance * tmpSiteTime[tmp_id]) / distance[tmp_id - 1];


    if (tmp_time < 1)
    {
        tmp_time += 1;
    }

    //qDebug()<<"1.tmp_time 2.tmp_distance 3.tmpSiteTime[tmp_id]  4.distance[tmp_id - 1]"<<tmp_time<<tmp_distance<<tmpSiteTime[tmp_id]<<distance[tmp_id - 1]<<distance[gMainReceive.next - 1];
    // All
    time = 0;
    
    if (gMainReceive.start < gMainReceive.end)
    {
        for (int i = gMainReceive.current; i < StationMaxNumber; i++)
        {
            siteTime[gMainReceive.current] = tmp_time;
            time += siteTime[i];
            
            if (timeflag <= 300)
            {
                str = QString("%1%2").arg(time).arg("min");
            }
            
            if (timeflag > 300)
            {
                str = QString("%1%2").arg(time).arg("分钟");
            }
            
            if (timeflag >= 600)
            {
                timeflag = 0;
            }
            
            if ((i + 1) == gMainReceive.next)
            {
                color.setRgb(0, 0, 0);
            }
            
            palette.setColor(QPalette::WindowText, color);
            allSiteTime[i]->setPalette(palette);
            allSiteTime[i]->setText(str);
        }
    }
    else
    {
        for (int i = gMainReceive.current - 1; i > 0; i--)
        {
            siteTime[gMainReceive.current - 1] = tmp_time;
            time += siteTime[i];
            
            if (timeflag <= 300)
            {
                str = QString("%1%2").arg(time).arg("min");
            }
            
            if (timeflag > 300)
            {
                str = QString("%1%2").arg(time).arg("分钟");
            }
            
            if (timeflag >= 600)
            {
                timeflag = 0;
            }
            
            //qDebug()<< "set down time" << time <<"str:"<<str;
            
            if (i == gMainReceive.next)
            {
                color.setRgb(0, 0, 0);
            }
            
            palette.setColor(QPalette::WindowText, color);
            allSiteTime[StationMaxNumber - i]->setPalette(palette);
            allSiteTime[StationMaxNumber - i]->setText(str);
        }
    }
    
    if (uiIndex != PRE)
        return;
        
#if 0
    //Pre
    pretime = 0;
    
    if (gMainReceive.start < gMainReceive.end)
    {
        if (preSnIndex > gMainReceive.current) //jump
        {
            for (int j = 0; j < (preSnIndex - gMainReceive.current - 1); j++)
            {
                pretime += siteTime[gMainReceive.current + j];
                //qDebug()<< "up jump:" << pretime;
            }
        }
        
        //qDebug()<< "up preSnIndex:" << preSnIndex << "current:" << gMainReceive.current;
        //qDebug()<< "up pretime:" << pretime;
        
        for (int i = 0; i < gPartInfo.leaveStationNum; i++)
        {
            if ((preSnIndex + i) > gMainReceive.current)
            {
                pretime += siteTime[preSnIndex + i - 1];
                str = QString("%1%2").arg(pretime).arg("mins");
                //qDebug()<< "set pre uptime" << pretime <<"str:"<<str
                //        << "siteTime[preSnIndex + i - 1]:"<< siteTime[preSnIndex + i - 1];
                
                if ((preSnIndex + i) == gMainReceive.next)
                {
                    color.setRgb(0, 0, 0);
                }
                else
                {
                    color.setRgb(255, 255, 255);
                }
                
                palette.setColor(QPalette::WindowText, color);
                preSiteTime[i]->setPalette(palette);
                preSiteTime[i]->setText(str);
                preSiteTime[i]->show();
            }
            else
            {
                preSiteTime[i]->hide();
            }
        }
    }
    else
    {
        if (preSnIndex < gMainReceive.current) //jump
        {
            for (int j = 0; j < (gMainReceive.current - preSnIndex - 1); j++)
            {
                pretime += siteTime[preSnIndex + j - 1];
                //qDebug()<< "down jump:" << pretime <<"str:"<<siteTime[preSnIndex + j - 1];
            }
        }
        
        for (int i = 0; i < gPartInfo.leaveStationNum; i++)
        {
            if ((preSnIndex - i) < gMainReceive.current)
            {
                pretime += siteTime[preSnIndex - i];
                str = QString("%1%2").arg(pretime).arg("mins");
                //qDebug()<< "set downtime" << pretime <<"str:"<<str;
                
                if ((preSnIndex - i) == gMainReceive.next)
                {
                    color.setRgb(0, 0, 0);
                }
                else
                {
                    color.setRgb(255, 255, 255);
                }
                
                palette.setColor(QPalette::WindowText, color);
                preSiteTime[i]->setPalette(palette);
                preSiteTime[i]->setText(str);
                preSiteTime[i]->show();
            }
            else
            {
                preSiteTime[i]->hide();
            }
        }
    }
    
#endif
}

void MainWindow::running_trigger()
{
    if ((false == checkStation(gMainReceive.start, gMainReceive.current, gMainReceive.next, gMainReceive.end, 1, StationMaxNumber)) || (updateFlag))
    {
        return;
    }
    
    if ((widgetIndex == COLORDISP) && (colorDispUpDown == 0))
        return;
        
    qDebug() << "running_trigger station:" << gMainReceive.start << gMainReceive.current << gMainReceive.next << gMainReceive.end << "cmd type" << gMainReceive.cmd;
    qDebug() << "gMainReceive.mirror:" << gMainReceive.mirror;
#ifdef ALL_TEST
    
    if (gMainReceive.cmd == SET_LEAVE)
    {
        gMainReceive.cmd = SET_LEAVE;
        leaveSt = LEAVE_ST_ALL;
    }
    
#endif
#ifdef PRE_TEST
    
    if (gMainReceive.cmd == SET_LEAVE)
    {
        gMainReceive.cmd = SET_PRE;
        leaveSt = LEAVE_ST_PRE;
    }
    
#endif
    colorDispUpDown = 0;
    
    switch (gMainReceive.cmd)
    {
        case SET_LEAVE:
            allProc();
            leaveSt = LEAVE_ST_ALL;
            leave_chg_time = 0;
            break;
            
        case SET_PRE:
            leaveSt = LEAVE_ST_PRE;
            preProc();
            break;
            
        case SET_ARRIVED:
            leaveSt = LEAVE_ST_ARR;
            arrProc();
            break;
            
        case SET_DOOR_ERROR:
            break;
            
        case SET_DOOR_OK:
            break;
            
        default:
            break;
    }
}

void MainWindow::allUp()
{
    QString site_str;
    int times = 0;
    QString str, track_str, str_r;
    int i; // 站名索引
    int ST_StationName[StationMaxNumber]; // 各个站状态
    int ST_Site[StationMaxNumber];
    int ST_Track[StationMaxNumber];
    qDebug() << "---allUp---";
    
    // 1. 状态处理
    for (i = 0; i < StationMaxNumber; i++) // 站名
    {
        allSiteTime[i]->hide();
        
        if ((i + 1) < gMainReceive.start)
        {
            ST_StationName[i] = ST_OUT;
            ST_Site[i] = ST_OUT;
            
            if (i > 0)
                ST_Track[i - 1] = ST_OUT;
        }
        else if ((i + 1) < gMainReceive.current)
        {
            ST_StationName[i] = ST_PASS;
            ST_Site[i] = ST_PASS;
            
            if (i > 0)
                ST_Track[i - 1] = ST_PASS;

            if ((i + 1) == gMainReceive.start)
            {
                if (i > 0)
                    ST_Track[i - 1] = ST_OUT;
            }
        }
        else if ((i + 1) == gMainReceive.current)
        {
            ST_StationName[i] = ST_CURRENT;
            ST_Site[i] = ST_CURRENT;
            
            if (i > 0)
                ST_Track[i - 1] = ST_CURRENT;
        }
        else if (((i + 1) > gMainReceive.current) && ((i + 1) < gMainReceive.next))
        {
            ST_StationName[i] = ST_SKIP;
            ST_Site[i] = ST_SKIP;
            
            if (i > 0)
                ST_Track[i - 1] = ST_SKIP;
        }
        else if ((i + 1) == gMainReceive.next)
        {
            ST_StationName[i] = ST_NEXT;
            ST_Site[i] = ST_NEXT;
            //if((i + 1) != gMainReceive.end)
            {
                allSiteTime[i]->show();
            }
            
            if (i > 0)
                ST_Track[i - 1] = ST_NEXT;
        }
        else if (((i + 1) > gMainReceive.next) && ((i + 1) <= gMainReceive.end))
        {
            ST_StationName[i] = ST_NEXTN;
            ST_Site[i] = ST_NEXTN;
            
            if (i > 0)
                ST_Track[i - 1] = ST_NEXTN;
                
            //if((i + 1) != gMainReceive.end)
            {
                allSiteTime[i]->show();
            }
        }
        else
        {
            ST_StationName[i] = ST_OUT;
            ST_Site[i] = ST_OUT;
            
            if (i > 0)
                ST_Track[i - 1] = ST_OUT;
        }
        
        allSNPassCh[i]->setText(stationChName[i]);
        allSNNextCh[i]->setText(stationChName[i]);
        allSNNextEn[i]->setText(stationEnName[i]);
        allSNPassEn[i]->setText(stationEnName[i]);
        
        if ((stationEnDispLines[i] == 2) || (stationEnDispLines[i] == 20))
        {
            allSNPassEn2[i]->setText(stationEnName2[i]);
            allSNNextEn2[i]->setText(stationEnName2[i]);
        }
        else
        {
            allSNPassEn2[i]->hide();
            allSNNextEn2[i]->hide();
        }
    }
    
    // 越站判断
    for (i = 0; i < StationMaxNumber; i++)
    {
        if ((gMainReceive.skipId[i / 8] & (1 << (i % 8))) > 0)
        {
            ST_StationName[i] = ST_SKIP;
            ST_Site[i] = ST_SKIP;
            
            if (i > 0)
                ST_Track[i - 1] = ST_SKIP;
        }
    }
    
    for (i = 0; i < StationMaxNumber; i++) // 当前站和下一站
    {
        if ((i + 1) == gMainReceive.next)
        {
            ST_StationName[i] = ST_NEXT;
            ST_Site[i] = ST_NEXT;
            
            if (i > 0)
                ST_Track[i - 1] = ST_NEXT;
        }
        else if ((i + 1) == gMainReceive.current)
        {
            ST_StationName[i] = ST_CURRENT;
            ST_Site[i] = ST_CURRENT;
            
            if (i > 0)
                ST_Track[i - 1] = ST_PASS;
        }
        else {}
        
        if((i + 1) >= NOT_IN_SERVICE)  //未开通
//        if (((i + 1) == NOT_OPEN_ID_32) || ((i + 1) == NOT_OPEN_ID_33) || ((i + 1) == NOT_OPEN_ID_34))
        {
            allSiteTime[i]->hide();
            ST_StationName[i] = ST_OUT;
            ST_Site[i] = ST_OUT;
            
            if (i > 0)
                ST_Track[i - 1] = ST_OUT;
        }
    }
    
    // 2. 显示处理
    for (i = 0; i < StationMaxNumber; i++)
    {
        allStart[i]->hide();
        allEnd[i]->hide();
        allSNPassEn[i]->show();
        allSNNextEn[i]->show();
        allSNPassEn2[i]->show();
        allSNNextEn2[i]->show();
        allSNPassCh[i]->show();
        allSNNextCh[i]->show();

        qDebug()<<"ST_Track["<<i<<"] = "<<ST_Track[i];
        
        switch (ST_StationName[i])
        {
            case ST_OUT:
                allSNPassEn[i]->show();
                allSNNextEn[i]->hide();
                
                if ((stationEnDispLines[i] == 2) || (stationEnDispLines[i] == 20))
                {
                    allSNPassEn2[i]->show();
                    allSNNextEn2[i]->hide();
                }
                
                allSNPassCh[i]->show();
                allSNNextCh[i]->hide();
                break;
#if 0
                
            case ST_PASS:
                allSNPassEn[i]->show();
                allSNNextEn[i]->hide();
                
                if ((stationEnDispLines[i] == 2) || (stationEnDispLines[i] == 20))
                {
                    allSNPassEn2[i]->show();
                    allSNNextEn2[i]->hide();
                }
                
                allSNPassCh[i]->show();
                allSNNextCh[i]->hide();
                break;
                
            case ST_CURRENT:
                allSNPassEn[i]->show();
                allSNNextEn[i]->hide();
                
                if ((stationEnDispLines[i] == 2) || (stationEnDispLines[i] == 20))
                {
                    allSNPassEn2[i]->show();
                    allSNNextEn2[i]->hide();
                }
                
                allSNPassCh[i]->show();
                allSNNextCh[i]->hide();
                break;
                
            case ST_NEXT:
                allSNPassEn[i]->hide();
                allSNNextEn[i]->show();
                
                if ((stationEnDispLines[i] == 2) || (stationEnDispLines[i] == 20))
                {
                    allSNPassEn2[i]->hide();
                    allSNNextEn2[i]->show();
                }
                
                allSNPassCh[i]->hide();
                allSNNextCh[i]->show();
                break;
                
            case ST_NEXTN:
                allSNPassEn[i]->hide();
                allSNNextEn[i]->show();
                
                if ((stationEnDispLines[i] == 2) || (stationEnDispLines[i] == 20))
                {
                    allSNPassEn2[i]->hide();
                    allSNNextEn2[i]->show();
                }
                
                allSNPassCh[i]->hide();
                allSNNextCh[i]->show();
                break;
                
            case ST_SKIP:
                break;
#endif
                
            default:
                break;
        }
        
        allTrack[i]->show();
        
        if (gMainReceive.mirror)
        {
            str_r = "L.png";
        }
        else
        {
            str_r = "R.png";
        }
        
        switch (ST_Track[i]) // 轨道图标
        {
            case ST_OUT:
                track_str = allTrackPath + "opened" + str_r;
                break;
                
            case ST_PASS:
                track_str = allTrackPath + "pass" + str_r;
                break;
                
            case ST_CURRENT:
                track_str = allTrackPath + "pass" + str_r;
                break;
                
            case ST_NEXT:
                if ((gMainReceive.next == gMainReceive.start) && (gMainReceive.next > 1))
                    track_str = allTrackPath + "pass" + str_r;
                else
                    track_str = allTrackPath + "nextn" + str_r;
                    
                break;
                
            case ST_NEXTN:
                track_str = allTrackPath + "nextned" + str_r;
                break;
                
            case ST_SKIP:
                track_str = allTrackPath + "nextned" + str_r;
                break;
                
            default:
                break;
        }

        allTrack[i]->setPixmap(track_str);
        allSite[i]->show();
        allJump[i]->hide();
        
        switch (ST_Site[i]) // 站点图标
        {
            case ST_OUT:
                site_str = allSitePath + "no.png";
                break;
                
            case ST_PASS:
                site_str = allSitePath + "pass.png";
                break;
                
            case ST_CURRENT:
                site_str = allSitePath + "pass.png";
                break;
                
            case ST_NEXT:
                allSite[i]->hide();
                break;
                
            case ST_NEXTN:
                site_str = allSitePath + "nextn.png";
                break;
                
            case ST_SKIP:
                site_str = allSitePath + "nextn.png";
                break;
                
            default:
                break;
        }
        
        allSite[i]->setPixmap(site_str);
    }
    
    for (i = 0; i < StationMaxNumber; i++)
    {
        switch (ST_StationName[i])
        {
            case ST_SKIP:
                //                if(((i+1)>gMainReceive.current)&&((i+1)<=gMainReceive.end))
                //                {
                //                    allSNPassEn[i]->hide();
                //                    allSNNextEn[i]->show();
                //                    if((stationEnDispLines[i]==2)||(stationEnDispLines[i]==20))
                //                    {
                //                        allSNPassEn2[i]->hide();
                //                        allSNNextEn2[i]->show();
                //                    }
                //                    allSNPassCh[i]->hide();
                //                    allSNNextCh[i]->show();
                //                }
                //                else
            {
                allSNPassEn[i]->show();
                allSNNextEn[i]->hide();
                
                if ((stationEnDispLines[i] == 2) || (stationEnDispLines[i] == 20))
                {
                    allSNPassEn2[i]->show();
                    allSNNextEn2[i]->hide();
                }
                
                allSNPassCh[i]->show();
                allSNNextCh[i]->hide();
            }
            break;
            
            default:
                break;
        }
        
        if (gMainReceive.mirror)
        {
            str_r = "L.png";
        }
        else
        {
            str_r = "R.png";
        }
        
        switch (ST_Track[i])
        {
            case ST_SKIP:
                if (((i + 1) > gMainReceive.current) && ((i + 1) <= gMainReceive.end))
                    track_str = allTrackPath + "nextned" + str_r;
                else if ((i + 1) == gMainReceive.current)
                    track_str = allTrackPath + "pass" + str_r;
                else
                    track_str = allTrackPath + "pass" + str_r;
                    
                allTrack[i]->setPixmap(track_str);
                break;
                
            default:
                break;
        }
        
        switch (ST_Site[i])
        {
            case ST_SKIP:
                if (((i + 1) > gMainReceive.current) && ((i + 1) <= gMainReceive.end))
                {
                    if (((i + 1) != gMainReceive.start && (i + 1) != gMainReceive.end))
                    {
                        allJump[i]->show();
                    }
                    
                    site_str = allSitePath + "nextn.png";
                }
                else if ((i + 1) == gMainReceive.current)
                {
                    site_str = allSitePath + "current.png";
                    allJump[i]->show();
                }
                else
                    site_str = allSitePath + "pass.png";
                    
                allSite[i]->setPixmap(site_str);
                break;
                
            default:
                break;
        }
    }
    
    // 换乘显示
    nextChgFlag = 0;
    
    for (int i = 0; i < 100; i++)
    {
        if ((gAllInfo.chg[i].stationId != 0))
        {
            allChg[i]->setGeometry(allSite[gAllInfo.chg[i].stationId - 1]->x() + 6 ,
                                   gAllInfo.chg[i].rect.y,
                                   gAllInfo.chg[i].rect.w,
                                   gAllInfo.chg[i].rect.h);
            str = QString("%1%2%3").arg(imageDir + "ui_all/CHANGE/line/")
                  .arg(gAllInfo.chg[i].stationId, 2, 10, QLatin1Char('0'))
                  .arg("station.png");
            allChg[i]->setPixmap(str);
            allChg[i]->show();
        }
        else
        {
            break;
        }
    }
    
    // Sp image
    int chgFlag = 0, k = 0;
    
    for (int i = 0; i < 100; i++)
    {
        if ((gAllInfo.sp[i].stationId != 0))
        {
            chgFlag = 0;
            
            for (k = 0; k < 100; k++)
            {
                if (gAllInfo.chg[k].stationId == gAllInfo.sp[i].stationId)
                {
                    chgFlag = 1;
                    break;
                }
                else if (gAllInfo.chg[k].stationId == 0)
                {
                    break;
                }
            }
            
            if (chgFlag)
            {
                allSp[i]->setGeometry(allSite[gAllInfo.sp[i].stationId - 1]->x() +
                                      gAllInfo.site[0].rectUp.w / 2 - gAllInfo.chg[k].rect.w / 2
                                      - 4 - gAllInfo.sp[i].rect.w / 2 + 18,
                                      gAllInfo.sp[i].rect.y,
                                      gAllInfo.sp[i].rect.w,
                                      gAllInfo.sp[i].rect.h);
                allChg[k]->setGeometry(allSp[i]->x() + 6,
                                       gAllInfo.chg[k].rect.y,
                                       gAllInfo.chg[k].rect.w,
                                       gAllInfo.chg[k].rect.h);
            }
            else
            {
                allSp[i]->setGeometry(allSite[gAllInfo.sp[i].stationId - 1]->x() +
                                      gAllInfo.site[0].rectUp.w / 2 -
                                      gAllInfo.sp[i].rect.w / 2 + 18,
                                      gAllInfo.sp[i].rect.y,
                                      gAllInfo.sp[i].rect.w,
                                      gAllInfo.sp[i].rect.h);
            }
            
            str = QString("%1%2%3").arg(imageDir + "ui_all/SP/sp")
                  .arg(gAllInfo.sp[i].stationId, 2, 10, QLatin1Char('0'))
                  .arg(".png");
            allSp[i]->setPixmap(str);
            allSp[i]->show();
        }
        else
        {
            break;
        }
    }
    
    //site flash
    //    allSiteFlash.setGeometry(allSite[gMainReceive.next - 1]->x(),
    //                             allSite[gMainReceive.next - 1]->y(),
    //                             allSite[gMainReceive.next - 1]->width(),
    //                             allSite[gMainReceive.next - 1]->height());
    //    allSiteFlash.setPixmap(imageDir + "ui_all/SITE/next.png");
    
    if (gMainReceive.mirror)
    {
        str_r = "L.png";
    }
    else
    {
        str_r = "R.png";
    }
    
    allFlowLed1.setGeometry(gAllInfo.track[gMainReceive.current].rectUp.x,
                            gAllInfo.track[gMainReceive.current].rectUp.y,
                            gAllInfo.track[gMainReceive.current].rectUp.w,
                            gAllInfo.track[gMainReceive.current].rectUp.h);
    allFlowLed2.setGeometry(gAllInfo.track[gMainReceive.current + 1].rectUp.x,
                            gAllInfo.track[gMainReceive.current + 1].rectUp.y,
                            gAllInfo.track[gMainReceive.current + 1].rectUp.w,
                            gAllInfo.track[gMainReceive.current + 1].rectUp.h);
    allFlowLed1.setPixmap(allTrackPath + "opened" + str_r);
    allFlowLed2.setPixmap(allTrackPath + "opened" + str_r);
    //PrechageTime = 0;
#if 0
    
    //track flash
    if (gMainReceive.mirror)
    {
        str_r = "L.gif";
    }
    else
    {
        str_r = "R.gif";
    }
    
    allTrackFlash.hide();
    
    if (gMainReceive.next != gMainReceive.start)
    {
        allTrackFlash.show();
        movieAllTrackFile = filePath + "images/ui_all/TRACK/next" + str_r;
        movieAllTrack->stop();
        allTrackFlash.setGeometry(allTrack[gMainReceive.next - 2]->x(),
                                  allTrack[gMainReceive.next - 2]->y(),
                                  allTrack[gMainReceive.next - 2]->width(),
                                  allTrack[gMainReceive.next - 2]->height());
        allTrackFlash.setMovie(movieAllTrack);
        movieAllTrack->setFileName(movieAllTrackFile);
        movieAllTrack->setSpeed(300);
        movieAllTrack->start();
    }
    
#endif
    str = allSitePath + "start.png";
    
    if (gMainReceive.current != gMainReceive.start)
    {
        str = allSitePath + "startPass.png";
    }
    
    allStart[gMainReceive.start - 1]->setPixmap(str);
    allStart[gMainReceive.start - 1]->show();
    str = allSitePath + "end.png";
    
    if (gMainReceive.next == gMainReceive.end)
    {
        str = allSitePath + "endNext.png";
    }
    
    allEnd[gMainReceive.end - 1]->setPixmap(str);
    allEnd[gMainReceive.end - 1]->show();
    allDoorErr.hide();
    
    if (gMainReceive.doorErr == 1)
    {
        allDoorErr.show();
    }
    
    allImetion.hide();
    str = QString("%1%2%3").arg(imageDir + "imetion/")
          .arg(gMainReceive.emeId, 2, 10, QLatin1Char('0'))
          .arg(".png");
          
    if (gMainReceive.emeId)
    {
        allImetion.setPixmap(str);
        allImetion.show();
    }
}

void MainWindow::allDown()
{
    QString site_str, str_r;
    QString track_str, str;
    int i, snIndex; // 站名索引
    int ST_StationName[StationMaxNumber]; // 各个站状态
    int ST_Site[StationMaxNumber];
    int ST_Track[StationMaxNumber];
    qDebug() << "---allDown---";
    snIndex = StationMaxNumber;
    
    for (i = 0; i < StationMaxNumber; i++)
    {
        allSiteTime[i]->hide();
        
        if ((snIndex - i) > gMainReceive.start)
        {
            ST_StationName[i] = ST_OUT;
            ST_Site[i] = ST_OUT;
            
            if (i > 0)
                ST_Track[i - 1] = ST_OUT;
        }
        else if ((snIndex - i) > gMainReceive.current)
        {
            ST_StationName[i] = ST_PASS;
            ST_Site[i] = ST_PASS;
            
            if (i > 0)
                ST_Track[i - 1] = ST_PASS;

            if( (snIndex - i) == gMainReceive.start)
            {
                ST_Track[i - 1] = ST_OUT;
            }
        }
        else if ((snIndex - i) == gMainReceive.current)
        {
            ST_StationName[i] = ST_CURRENT;
            ST_Site[i] = ST_CURRENT;
            
            if (i > 0)
                ST_Track[i - 1] = ST_CURRENT;
        }
        else if (((snIndex - i) < gMainReceive.current)
                 && ((snIndex - i) > gMainReceive.next))
        {
            ST_StationName[i] = ST_SKIP;
            ST_Site[i] = ST_SKIP;
            
            if (i > 0)
                ST_Track[i - 1] = ST_SKIP;
        }
        else if ((snIndex - i) == gMainReceive.next)
        {
            ST_StationName[i] = ST_NEXT;
            ST_Site[i] = ST_NEXT;
            
            if (i > 0)
                ST_Track[i - 1] = ST_NEXT;
                
            allSiteTime[i]->show();
        }
        else if (((snIndex - i) < gMainReceive.next) && ((snIndex - i) >= gMainReceive.end))
        {
            ST_StationName[i] = ST_NEXTN;
            ST_Site[i] = ST_NEXTN;
            
            if (i > 0)
                ST_Track[i - 1] = ST_NEXTN;
                
            //if((snIndex - i) != gMainReceive.end)
            {
                allSiteTime[i]->show();
            }
        }
        else
        {
            ST_StationName[i] = ST_OUT;
            ST_Site[i] = ST_OUT;
            
            if (i > 0)
                ST_Track[i - 1] = ST_OUT;
        }
        
        allSNPassCh[i]->setText(stationChName[StationMaxNumber - i - 1]);
        allSNNextCh[i]->setText(stationChName[StationMaxNumber - i - 1]);
        allSNNextEn[i]->setText(stationEnName[StationMaxNumber - i - 1]);
        allSNPassEn[i]->setText(stationEnName[StationMaxNumber - i - 1]);
        
        if ((stationEnDispLines[StationMaxNumber - i - 1] == 2) || (stationEnDispLines[StationMaxNumber - i - 1] == 20))
        {
            allSNPassEn2[i]->setText(stationEnName2[StationMaxNumber - i - 1]);
            allSNNextEn2[i]->setText(stationEnName2[StationMaxNumber - i - 1]);
        }
        else
        {
            allSNPassEn2[i]->hide();
            allSNNextEn2[i]->hide();
        }
    }
    
    // 越站判断
    int skipbit = 0;
    
    for (i = 0; i < StationMaxNumber; i++)
    {
        skipbit = StationMaxNumber - i - 1;
        
        if ((gMainReceive.skipId[skipbit / 8] & (1 << (skipbit % 8))) > 0)
        {
            ST_StationName[i] = ST_SKIP;
            ST_Site[i] = ST_SKIP;
            ST_Track[i] = ST_SKIP;
        }
    }
    
    for (i = 0; i < StationMaxNumber; i++) // 当前站和下一站
    {
        if ((StationMaxNumber - i) == gMainReceive.next)
        {
            ST_StationName[i] = ST_NEXT;
            ST_Site[i] = ST_NEXT;
            ST_Track[i - 1] = ST_NEXT;
        }
        else if ((StationMaxNumber - i) == gMainReceive.current)
        {
            ST_StationName[i] = ST_CURRENT;
            ST_Site[i] = ST_CURRENT;
            if(i > 0)
                ST_Track[i] = ST_CURRENT;
        }
        else
        {
        }
        
        //
        if((StationMaxNumber - i) >= NOT_IN_SERVICE)
//        if (((StationMaxNumber - i) == NOT_OPEN_ID_32) || ((StationMaxNumber - i) == NOT_OPEN_ID_33) || ((StationMaxNumber - i) == NOT_OPEN_ID_34))
        {
            allSiteTime[i]->hide();
            ST_StationName[i] = ST_OUT;
            ST_Site[i] = ST_OUT;
            ST_Track[i] = ST_OUT;
        }
    }
    
    // 2. 显示处理
    for (i = 0; i < StationMaxNumber; i++)
    {
        allStart[i]->hide();
        allEnd[i]->hide();
        //qDebug()<<"ST_Track["<<i<<"]"<<ST_Track[i];
        allSNPassEn[i]->show();
        allSNNextEn[i]->show();
        allSNPassEn2[i]->show();
        allSNNextEn2[i]->show();
        allSNPassCh[i]->show();
        allSNNextCh[i]->show();
        
        switch (ST_StationName[i])
        {
            case ST_OUT:
                allSNPassEn[i]->show();
                allSNNextEn[i]->hide();
                
                if ((stationEnDispLines[StationMaxNumber - i - 1] == 2)
                    || (stationEnDispLines[StationMaxNumber - i - 1] == 20))
                {
                    allSNPassEn2[i]->show();
                    allSNNextEn2[i]->hide();
                }
                
                allSNPassCh[i]->show();
                allSNNextCh[i]->hide();
                break;
#if 0
                
            case ST_PASS:
                allSNPassEn[i]->show();
                allSNNextEn[i]->hide();
                
                if ((stationEnDispLines[StationMaxNumber - i - 1] == 2)
                    || (stationEnDispLines[StationMaxNumber - i - 1] == 20))
                {
                    allSNPassEn2[i]->show();
                    allSNNextEn2[i]->hide();
                }
                
                allSNPassCh[i]->show();
                allSNNextCh[i]->hide();
                break;
                
            case ST_CURRENT:
                allSNPassEn[i]->hide();
                allSNNextEn[i]->show();
                
                if ((stationEnDispLines[StationMaxNumber - i - 1] == 2)
                    || (stationEnDispLines[StationMaxNumber - i - 1] == 20))
                {
                    allSNPassEn2[i]->hide();
                    allSNNextEn2[i]->show();
                }
                
                allSNPassCh[i]->hide();
                allSNNextCh[i]->show();
                break;
                
            case ST_NEXT:
                allSNPassEn[i]->hide();
                allSNNextEn[i]->show();
                
                if ((stationEnDispLines[StationMaxNumber - i - 1] == 2)
                    || (stationEnDispLines[StationMaxNumber - i - 1] == 20))
                {
                    allSNPassEn2[i]->hide();
                    allSNNextEn2[i]->show();
                }
                
                allSNPassCh[i]->hide();
                allSNNextCh[i]->show();
                break;
                
            case ST_NEXTN:
                allSNPassEn[i]->hide();
                allSNNextEn[i]->show();
                
                if ((stationEnDispLines[StationMaxNumber - i - 1] == 2)
                    || (stationEnDispLines[StationMaxNumber - i - 1] == 20))
                {
                    allSNPassEn2[i]->hide();
                    allSNNextEn2[i]->show();
                }
                
                allSNPassCh[i]->hide();
                allSNNextCh[i]->show();
                break;
                
            case ST_SKIP:
                break;
#endif
                
            default:
                break;
        }
        
        allTrack[i]->show();
        
        if (gMainReceive.mirror)
        {
            str_r = "L.png";
        }
        else
        {
            str_r = "R.png";
        }
        
        switch (ST_Track[i]) // 轨道图标
        {
            case ST_OUT:
                track_str = allTrackPath + "opened" + str_r;
                break;
                
            case ST_PASS:
                track_str = allTrackPath + "pass" + str_r;
                break;
                
            case ST_CURRENT:
                track_str = allTrackPath + "pass" + str_r;
                break;
                
            case ST_NEXT:
                if ((gMainReceive.next == gMainReceive.start) && (gMainReceive.next < StationMaxNumber))
                    track_str = allTrackPath + "pass" + str_r;
                else
                    track_str = allTrackPath + "nextn" + str_r;
                    
                break;
                
            case ST_NEXTN:
                track_str = allTrackPath + "nextned" + str_r;
                break;
                
            case ST_SKIP:
                track_str = allTrackPath + "nextn" + str_r;
                break;
                
            default:
                break;
        }
        
        allTrack[i]->setPixmap(track_str);
        //        if (i < StationMaxNumber - 1)
        //            allTrack[StationMaxNumber - i - 2]->setPixmap(track_str);
        allSite[i]->show();
        allJump[i]->hide();
        
        switch (ST_Site[i]) // 站点图标
        {
            case ST_OUT:
                site_str = allSitePath + "no.png";
                break;
                
            case ST_PASS:
                site_str = allSitePath + "pass.png";
                break;
                
            case ST_CURRENT:
                site_str = allSitePath + "pass.png";
                break;
                
            case ST_NEXT:
                allSite[i]->hide();
                break;
                
            case ST_NEXTN:
                site_str = allSitePath + "nextn.png";
                break;
                
            case ST_SKIP:
                site_str = allSitePath + "nextn.png";
                break;
                
            default:
                break;
        }
        
        allSite[i]->setPixmap(site_str);
    }
    
    for (i = 0; i < StationMaxNumber; i++)
    {
        switch (ST_StationName[i])
        {
            case ST_SKIP:
                allSNPassEn[i]->show();
                allSNNextEn[i]->hide();
                
                if ((stationEnDispLines[StationMaxNumber - i - 1] == 2)
                    || (stationEnDispLines[StationMaxNumber - i - 1] == 20))
                {
                    allSNPassEn2[i]->show();
                    allSNNextEn2[i]->hide();
                }
                
                allSNPassCh[i]->show();
                allSNNextCh[i]->hide();
                break;
                
            default:
                break;
        }
        
        if (gMainReceive.mirror)
        {
            str_r = "L.png";
        }
        else
        {
            str_r = "R.png";
        }
        
        switch (ST_Track[i])
        {
            case ST_SKIP:
                if (((StationMaxNumber - i) < gMainReceive.current)
                    && ((StationMaxNumber - i) >= gMainReceive.end))
                    track_str = allTrackPath + "nextn" + str_r;
                else if ((StationMaxNumber - i) == gMainReceive.current)
                    track_str = allTrackPath + "nextn" + str_r;
                else
                    track_str = allTrackPath + "pass" + str_r;
                    
                allTrack[i]->setPixmap(track_str);
                break;
                
            default:
                break;
        }
        
        switch (ST_Site[i])
        {
            case ST_SKIP:
                if (((StationMaxNumber - i) < gMainReceive.current)
                    && ((StationMaxNumber - i) >= gMainReceive.end))
                {
                    if (((StationMaxNumber - i) != gMainReceive.start
                         && (StationMaxNumber - i) != gMainReceive.end))
                    {
                        allJump[i]->show();
                    }
                    
                    site_str = allSitePath + "nextn.png";
                }
                else if ((StationMaxNumber - i) == gMainReceive.current)
                {
                    site_str = allSitePath + "current.png";
                    allJump[i]->show();
                }
                else
                    site_str = allSitePath + "pass.png";
                    
                allSite[i]->setPixmap(site_str);
                break;
                
            default:
                break;
        }
    }
    
    // 换乘显示
    nextChgFlag = 0;
    
    for (int i = 0; i < 100; i++)
    {
        if ((gAllInfo.chg[i].stationId != 0))
        {
            str = QString("%1%2%3").arg(imageDir + "ui_all/CHANGE/line/")
                  .arg(gAllInfo.chg[i].stationId, 2, 10, QLatin1Char('0'))
                  .arg("station.png");
            allChg[i]->setGeometry(allSite[gAllInfo.chg[i].stationId - 1]->x() + 6 ,
                                   gAllInfo.chg[i].rect.y,
                                   gAllInfo.chg[i].rect.w,
                                   gAllInfo.chg[i].rect.h);
            allChg[i]->setPixmap(str);
            allChg[i]->show();
        }
        else
        {
            break;
        }
    }
    
    // Sp image
    int chgFlag = 0, k = 0;
    
    for (int i = 0; i < 100; i++)
    {
        if ((gAllInfo.sp[i].stationId != 0))
        {
            chgFlag = 0;
            
            for (k = 0; k < 100; k++)
            {
                if (gAllInfo.chg[k].stationId == gAllInfo.sp[i].stationId)
                {
                    chgFlag = 1;
                    break;
                }
                else if (gAllInfo.chg[k].stationId == 0)
                {
                    break;
                }
            }
            
            if (chgFlag)
            {
                allSp[i]->setGeometry(allSite[StationMaxNumber - gAllInfo.sp[i].stationId]->x() +
                                      gAllInfo.site[0].rectUp.w / 2 - gAllInfo.chg[k].rect.w / 2
                                      - 4 - gAllInfo.sp[i].rect.w / 2 + 18,
                                      gAllInfo.sp[i].rect.y,
                                      gAllInfo.sp[i].rect.w,
                                      gAllInfo.sp[i].rect.h);
                allChg[k]->setGeometry(allSp[i]->x() + 6,
                                       gAllInfo.chg[k].rect.y,
                                       gAllInfo.chg[k].rect.w,
                                       gAllInfo.chg[k].rect.h);
            }
            else
            {
                allSp[i]->setGeometry(allSite[StationMaxNumber - gAllInfo.sp[i].stationId]->x() +
                                      gAllInfo.site[0].rectUp.w / 2 -
                                      gAllInfo.sp[i].rect.w / 2 + 18,
                                      gAllInfo.sp[i].rect.y,
                                      gAllInfo.sp[i].rect.w,
                                      gAllInfo.sp[i].rect.h);
            }
            
            str = QString("%1%2%3").arg(imageDir + "ui_all/SP/sp")
                  .arg(gAllInfo.sp[i].stationId, 2, 10, QLatin1Char('0'))
                  .arg(".png");
            allSp[i]->setPixmap(str);
            allSp[i]->show();
        }
        else
        {
            break;
        }
    }
    
    //site flash
    //    allSiteFlash.setGeometry(allSite[StationMaxNumber - gMainReceive.next]->x(),
    //                             allSite[StationMaxNumber - gMainReceive.next]->y(),
    //                             allSite[StationMaxNumber - gMainReceive.next]->width(),
    //                             allSite[StationMaxNumber - gMainReceive.next]->height());
    //    allSiteFlash.setPixmap(imageDir + "ui_all/SITE/next.png");
    
    if (gMainReceive.mirror)
    {
        str_r = "L.png";
    }
    else
    {
        str_r = "R.png";
    }
    
    allFlowLed1.setPixmap(allTrackPath + "opened" + str_r);
    allFlowLed2.setPixmap(allTrackPath + "opened" + str_r);
    allFlowLed1.setGeometry(gAllInfo.track[gMainReceive.current].rectUp.x,
                            gAllInfo.track[gMainReceive.current].rectUp.y,
                            gAllInfo.track[gMainReceive.current].rectUp.w,
                            gAllInfo.track[gMainReceive.current].rectUp.h);
    allFlowLed2.setGeometry(gAllInfo.track[gMainReceive.current - 1].rectUp.x,
                            gAllInfo.track[gMainReceive.current - 1].rectUp.y,
                            gAllInfo.track[gMainReceive.current - 1].rectUp.w,
                            gAllInfo.track[gMainReceive.current - 1].rectUp.h);
    //track flash
#if 0
    
    if (gMainReceive.mirror)
    {
        str_r = "L.gif";
    }
    else
    {
        str_r = "R.gif";
    }
    
    allTrackFlash.hide();
    
    if (gMainReceive.next != gMainReceive.start)
    {
        allTrackFlash.show();
        movieAllTrackFile = filePath + "images/ui_all/TRACK/next" + str_r;
        movieAllTrack->stop();
        allTrackFlash.setGeometry(allTrack[StationMaxNumber - gMainReceive.next - 1]->x(),
                                  allTrack[StationMaxNumber - gMainReceive.next - 1]->y(),
                                  allTrack[StationMaxNumber - gMainReceive.next - 1]->width(),
                                  allTrack[StationMaxNumber - gMainReceive.next - 1]->height());
        allTrackFlash.setMovie(movieAllTrack);
        movieAllTrack->setFileName(movieAllTrackFile);
        movieAllTrack->setSpeed(300);
        movieAllTrack->start();
    }
    
#endif
    str = allSitePath + "start.png";
    
    if (gMainReceive.current != gMainReceive.start)
    {
        str = allSitePath + "startPass.png";
    }
    
    allStart[StationMaxNumber - gMainReceive.start]->setPixmap(str);
    allStart[StationMaxNumber - gMainReceive.start]->show();
    str = allSitePath + "end.png";
    
    if (gMainReceive.next == gMainReceive.end)
    {
        str = allSitePath + "endNext.png";
    }
    
    allEnd[StationMaxNumber - gMainReceive.end]->setPixmap(str);
    allEnd[StationMaxNumber - gMainReceive.end]->show();
    allDoorErr.hide();
    
    if (gMainReceive.doorErr == 1)
    {
        allDoorErr.show();
    }
    
    allImetion.hide();
    str = QString("%1%2%3").arg(imageDir + "imetion/")
          .arg(gMainReceive.emeId, 2, 10, QLatin1Char('0'))
          .arg(".png");
          
    if (gMainReceive.emeId)
    {
        allImetion.setPixmap(str);
        allImetion.show();
    }
}

void MainWindow::allProc()
{
    allReInit();
    
    if (gMainReceive.start < gMainReceive.end)
        allUp();
    else
        allDown();
        
    //menu disp
    allMDstSNEn.setText(stationEnName[gMainReceive.end - 1]); // dst
    allMDstSNCh.setText(stationChName[gMainReceive.end - 1]);
    allMDstSNEn.adjustSize();
    allMDstSNCh.adjustSize();
    allMDstSNEn.setGeometry(allMDstSNCh.x() + allMDstSNCh.width() + 8,
                            gPublicInfo.MDstSNEn.rectUp.y,
                            allMDstSNEn.width(),
                            gPublicInfo.MDstSNEn.rectUp.h);
    allMDstSNEn.setAlignment(Qt::AlignLeft);
    int dstText_x = 0;
    dstText_x = gPublicInfo.MDstSNCh.rectUp.x + gPublicInfo.MDstSNCh.rectUp.w / 2
                - (allMDstTextCh.width() + 10 + allMDstTextEn.width() +
                   allMDstSNCh.width() + 10 + allMDstSNEn.width()) / 2;
    allMDstTextCh.setGeometry(gPublicInfo.MDstSNCh.rectUp.x,
                              gPublicInfo.MDstSNCh.rectUp.y,
                              allMDstTextCh.width(),
                              gPublicInfo.MDstSNCh.rectUp.h);
    allMDstTextEn.setGeometry(gPublicInfo.MDstSNCh.rectUp.x,
                              gPublicInfo.MDstSNEn.rectUp.y,
                              allMDstTextEn.width(),
                              gPublicInfo.MDstSNEn.rectUp.h);
    allMDstSNCh.setGeometry(gPublicInfo.MDstSNCh.rectUp.x,
                            gPublicInfo.MDstSNCh.rectUp.y - 2,
                            allMDstSNCh.width(),
                            gPublicInfo.MDstSNCh.rectUp.h + 2);
    allMDstSNEn.setGeometry(gPublicInfo.MDstSNCh.rectUp.x + 2,
                            gPublicInfo.MDstSNEn.rectUp.y - 6,
                            allMDstSNEn.width(),
                            gPublicInfo.MDstSNEn.rectUp.h);
    //MNext
    int psize = 0;
    QFont font = allMNextSNCh.font();
    font.setPixelSize(gPublicInfo.MNextSNCh.fontSize);
    allMNextSNCh.setFont(font);
    allMNextSNCh.setText(stationChName[gMainReceive.next - 1]);
    allMNextSNCh.adjustSize();
    
    while (allMNextSNCh.width() > gPublicInfo.MNextSNCh.rectUp.w)
    {
        psize++;
        font.setPixelSize(gPublicInfo.MNextSNCh.fontSize - psize);
        allMNextSNCh.setFont(font);
        allMNextSNCh.setText(stationChName[gMainReceive.next - 1]);
        allMNextSNCh.adjustSize();
    }
    
    allMNextSNCh.setGeometry(gPublicInfo.MNextSNCh.rectUp.x,
                             gPublicInfo.MNextSNCh.rectUp.y - 12,
                             gPublicInfo.MNextSNCh.rectUp.w,
                             gPublicInfo.MNextSNCh.rectUp.h);
    //    allMNextSNCh.setAlignment(Qt::AlignCenter);
    allMNextSNCh.setText(stationChName[gMainReceive.next - 1]);
    allMNextSNEn.setGeometry(gPublicInfo.MNextSNEn.rectUp.x,
                             gPublicInfo.MNextSNEn.rectUp.y - 10,
                             gPublicInfo.MNextSNEn.rectUp.w,
                             gPublicInfo.MNextSNEn.rectUp.h);
    //    allMNextSNEn.setAlignment(Qt::AlignCenter);
    allMNextSNEn.setText(stationEnName[gMainReceive.next - 1]);

    allNotRun.setParent(&widget[ALL]);

    if (gMainReceive.mirror)
    {
        if (gMainReceive.start < gMainReceive.end)
        {
            allNotRun.setGeometry(allSite[NOT_IN_SERVICE + 1]->x() + 23,
                                  allSite[NOT_IN_SERVICE - 1]->y() + 37,
                                  allSite[NOT_IN_SERVICE - 1]->width() + 100,
                                  allSite[NOT_IN_SERVICE - 1]->height() + 5);
        }
        else
        {
            allNotRun.setGeometry(allSite[stationNum - NOT_IN_SERVICE]->x() + 23,
                                  allSite[stationNum - NOT_IN_SERVICE]->y() + 37,
                                  allSite[stationNum - NOT_IN_SERVICE]->width() + 100,
                                  allSite[stationNum - NOT_IN_SERVICE]->height() + 5);
        }
    }
    else
    {
        if (gMainReceive.start < gMainReceive.end)
        {
            allNotRun.setGeometry(allSite[NOT_IN_SERVICE - 1]->x() + 23,
                                  allSite[NOT_IN_SERVICE - 1]->y() + 37,
                                  allSite[NOT_IN_SERVICE - 1]->width() + 100,
                                  allSite[NOT_IN_SERVICE - 1]->height() + 5);
        }
        else
        {
            allNotRun.setGeometry(allSite[stationNum - NOT_IN_SERVICE - 2]->x() + 23,
                                  allSite[stationNum - NOT_IN_SERVICE]->y() + 37,
                                  allSite[stationNum - NOT_IN_SERVICE]->width() + 100,
                                  allSite[stationNum - NOT_IN_SERVICE]->height() + 5);
        }
    }

    allNotRun.setPixmap(allSitePath + "notRun.png");
    allNotRun.show();
    if (gMainReceive.emeId != 0 || gMainReceive.doorErr == 1)
        allNotRun.hide();


//    setSiteTime(ALL);
    chgToWidget(ALL);
}

#define CROSS_ROAD 1
void MainWindow::preUp()
{
    QString str, strLine, filename, str_r;
    QColor color;
    QPalette palette;
    int i; // 站名索引
    int snIndex; // 站名索引
    int nextIndex; // 下一站放大索引
    int ST_StationName[StationMaxNumber]; // 各个站状态
    int preStationName[StationMaxNumber]; // 各个站id
    int ST_Site[StationMaxNumber];
    int ST_Track[StationMaxNumber];
    qDebug() << "---preUp---";
    // 1. 状态处理
#if CROSS_ROAD
    
    if ((gMainReceive.next <= gMainReceive.start + 2) &&
        (gMainReceive.end - gMainReceive.start + 1 > gPartInfo.leaveStationNum))
    {
        nextIndex = gMainReceive.next - gMainReceive.start;
        snIndex = gMainReceive.start;
    }
    else if ((gMainReceive.next >= gMainReceive.start + 2) &&
             (gMainReceive.next + 2 <= gMainReceive.end))
    {
        nextIndex = 2;
        snIndex = gMainReceive.next - 2;
    }
    else
    {
        nextIndex = gPartInfo.leaveStationNum - 1 - (gMainReceive.end - gMainReceive.next);
        snIndex = gMainReceive.end - gPartInfo.leaveStationNum + 1;
    }
    
#else
    
    if (gMainReceive.next <= 2)
    {
        nextIndex = gMainReceive.next - 1;
        snIndex = 1;
    }
    else if ((gMainReceive.next >= 3) && (gMainReceive.next + 2 <= StationMaxNumber))
    {
        nextIndex = 2;
        snIndex = gMainReceive.next - 2;
    }
    else
    {
        nextIndex = gPartInfo.leaveStationNum - 1 - (StationMaxNumber - gMainReceive.next);
        snIndex = StationMaxNumber - gPartInfo.leaveStationNum + 1;
    }
    
#endif
    qDebug() << "nextIndex:" << nextIndex;
    qDebug() << "snIndex:" << snIndex;
    preSnIndex = snIndex ;
    
    for (i = 0; i < gPartInfo.leaveStationNum; i++) // 站名
    {
        if ((i + snIndex) < gMainReceive.start)
        {
            ST_StationName[i] = ST_OUT;
            ST_Site[i] = ST_OUT;
            
            if (i > 0)
                ST_Track[i - 1] = ST_OUT;
        }
        else if ((i + snIndex) == gMainReceive.start)
        {
            ST_StationName[i] = ST_PASS;
            ST_Site[i] = ST_PASS;
            
            if (i > 0)
                ST_Track[i - 1] = ST_OUT;
        }
        else if ((i + snIndex) < gMainReceive.current)
        {
            ST_StationName[i] = ST_PASS;
            ST_Site[i] = ST_PASS;
            
            if (i > 0)
                ST_Track[i - 1] = ST_PASS;
        }
        else if ((i + snIndex) == gMainReceive.current)
        {
            ST_StationName[i] = ST_CURRENT;
            ST_Site[i] = ST_CURRENT;
            
            if (i > 0)
                ST_Track[i - 1] = ST_CURRENT;
        }
        else if (((i + snIndex) > gMainReceive.current) && ((i + snIndex) < gMainReceive.next))
        {
            ST_StationName[i] = ST_SKIP;
            ST_Site[i] = ST_SKIP;
            
            if (i > 0)
                ST_Track[i - 1] = ST_NEXTN;
        }
        else if ((i + snIndex) == gMainReceive.next)
        {
            ST_StationName[i] = ST_NEXT;
            ST_Site[i] = ST_NEXT;
            
            if (i > 0)
                ST_Track[i - 1] = ST_NEXT;
        }
        else if (((i + snIndex) > gMainReceive.next) && ((i + snIndex) <= gMainReceive.end))
        {
            ST_StationName[i] = ST_NEXTN;
            ST_Site[i] = ST_NEXTN;
            
            if (i > 0)
                ST_Track[i - 1] = ST_NEXTN;
                
            if ((gMainReceive.skipId[(snIndex + i - 1) / 8] & (1 << ((snIndex + i - 1) % 8))) > 0)
            {
                ST_StationName[i] = ST_SKIP;
                ST_Site[i] = ST_SKIP;
                
                if (i > 0)
                    ST_Track[i - 1] = ST_NEXTN;
            }
        }
        else
        {
            ST_StationName[i] = ST_OUT;
            ST_Site[i] = ST_OUT;
            
            if (i > 0)
                ST_Track[i - 1] = ST_OUT;
        }
        
        preStationName[i] = snIndex + i;
    }
    
    preNotRun.hide();
    
    for (i = 0; i < gPartInfo.leaveStationNum; i++)
    {
        if ((i + snIndex) == gMainReceive.current)
        {
            ST_StationName[i] = ST_CURRENT;
            ST_Site[i] = ST_CURRENT;
            
            if (i > 0)
                ST_Track[i - 1] = ST_CURRENT;
        }
        
        if ((i + snIndex) == gMainReceive.next)
        {
            ST_StationName[i] = ST_NEXT;
            ST_Site[i] = ST_NEXT;
            
            if (i > 0)
                ST_Track[i - 1] = ST_NEXT;
        }
        
        if((i + snIndex) >= NOT_IN_SERVICE)
//        if (((i + snIndex) == NOT_OPEN_ID_32) || ((i + snIndex) == NOT_OPEN_ID_33) || ((i + snIndex) == NOT_OPEN_ID_34))
        {
            ST_StationName[i] = ST_OUT;
//            ST_Site[i] = ST_OUT;
            preNotRun.show();
            preNotRun.setGeometry(preSite[i]->x(), preSite[i]->y(),
                                  preSite[i]->width(), preSite[i]->height());
        }
    }
    
    // 2. 显示处理
    preTrack[gPartInfo.leaveStationNum - 1]->hide();
    
    for (i = 0; i < gPartInfo.leaveStationNum; i++)
    {
        preSNEn[i]->hide();
        preSNCh[i]->hide();
        
        switch (ST_StationName[i])
        {
            case ST_OUT:
                preSNEn[i]->show();
                preSNCh[i]->show();
                color.setRgb(gPublicInfo.SNNextColor.rgba.r, gPublicInfo.SNNextColor.rgba.g,
                             gPublicInfo.SNNextColor.rgba.b);
                palette.setColor(QPalette::WindowText, color);
                break;
                
            case ST_PASS:
                preSNEn[i]->show();
                preSNCh[i]->show();
                color.setRgb(gPublicInfo.SNNextColor.rgba.r, gPublicInfo.SNNextColor.rgba.g,
                             gPublicInfo.SNNextColor.rgba.b);
                palette.setColor(QPalette::WindowText, color);
                break;
                
            case ST_CURRENT:
                preSNEn[i]->show();
                preSNCh[i]->show();
                color.setRgb(gPublicInfo.SNNextColor.rgba.r, gPublicInfo.SNNextColor.rgba.g,
                             gPublicInfo.SNNextColor.rgba.b);
                palette.setColor(QPalette::WindowText, color);
                break;
                
            case ST_NEXT:
                color.setRgb(gPublicInfo.SNNextColor.rgba.r, gPublicInfo.SNNextColor.rgba.g,
                             gPublicInfo.SNNextColor.rgba.b);
                palette.setColor(QPalette::WindowText, color);
                preSNEn[i]->show();
                preSNCh[i]->show();
                break;
                
            case ST_NEXTN:
                color.setRgb(gPublicInfo.SNNextNColor.rgba.r, gPublicInfo.SNNextNColor.rgba.g,
                             gPublicInfo.SNNextNColor.rgba.b);
                palette.setColor(QPalette::WindowText, color);
                preSNEn[i]->show();
                preSNCh[i]->show();
                break;
                
            case ST_SKIP:
                color.setRgb(gPublicInfo.SNPassColor.rgba.r, gPublicInfo.SNPassColor.rgba.g,
                             gPublicInfo.SNPassColor.rgba.b);
                palette.setColor(QPalette::WindowText, color);
                preSNEn[i]->show();
                preSNCh[i]->show();
                break;
                
            default:
                break;
        }
        
        preSNEn[i]->setPalette(palette);
        preSNCh[i]->setPalette(palette);
        
        if (preStationName[i] > 0)
        {
            preSNEn[i]->setText(stationEnName[preStationName[i] - 1]);
            preSNCh[i]->setText(stationChName[preStationName[i] - 1]);
            
            if (stationEnDispLines[preStationName[i] - 1] == 2)
                preSNEn[i]->setText(stationEnName[preStationName[i] - 1]
                                    + stationEnName2[preStationName[i] - 1]);
                                    
            if (stationEnDispLines[preStationName[i] - 1] == 20)
                preSNEn[i]->setText(stationEnName[preStationName[i] - 1] + " "
                                    + stationEnName2[preStationName[i] - 1]);
        }
        
        if (gMainReceive.mirror)
        {
            str_r = "L.png";
        }
        else
        {
            str_r = "R.png";
        }
        
        switch (ST_Track[i])
        {
            case ST_OUT:
                str = imageDir + "ui_pre/TRACK/pass" + str_r;
                preTrack[i]->show();
                break;
                
            case ST_PASS:
                str = imageDir + "ui_pre/TRACK/pass" + str_r;
                preTrack[i]->show();
                break;
                
            case ST_CURRENT:
                str = imageDir + "ui_pre/TRACK/pass" + str_r;
                preTrack[i]->show();
                break;
                
            case ST_NEXT:
                str = imageDir + "ui_pre/TRACK/next" + str_r;
                preTrack[i]->show();
                break;
                
            case ST_NEXTN:
                str = imageDir + "ui_pre/TRACK/nextn" + str_r;
                preTrack[i]->show();
                break;
                
            case ST_SKIP:
                str = imageDir + "ui_pre/TRACK/nextn" + str_r;
                preTrack[i]->show();
                break;
                
            default:
                break;
        }
        
        preTrack[i]->setPixmap(str);
        preJump[i]->hide();
        
        switch (ST_Site[i])
        {
            case ST_OUT:
                preSite[i]->show();
                str = imageDir + "ui_pre/SITE/current.png";
                break;
                
            case ST_PASS:
                preSite[i]->show();
                str = imageDir + "ui_pre/SITE/current.png";
                break;
                
            case ST_CURRENT:
                preSite[i]->show();
                str = imageDir + "ui_pre/SITE/current.png";
                break;
                
            case ST_NEXT:
                preSiteSt = 1;
                preSite[i]->show();
                break;
                
            case ST_NEXTN:
                preSite[i]->show();
                str = imageDir + "ui_pre/SITE/nextn.png";
                break;
                
            case ST_SKIP:
                preSite[i]->show();
                str = imageDir + "ui_pre/SITE/current.png";
                preJump[i]->show();
                break;
                
            default:
                break;
        }
        
        preSite[i]->setPixmap(str);
    }
    
#ifdef PREDO
    
    if (gMainReceive.next == 2)
    {
        preSite[1]->hide();
    }
    
    if (gMainReceive.next == gMainReceive.end - 1)
    {
        preSite[3]->hide();
    }
    
#endif
    
    if (gMainReceive.next == gMainReceive.end)
    {
        preSite[4]->hide();
    }
    
    // 换乘显示
    // 下一站换乘
    bool chgDispIndex[10];
    
    for (i = 0; i < gPartInfo.leaveStationNum; i++)
    {
        preChg[i].hide();
        chgDispIndex[i] = false;
    }
    
    int chgSiteIndex = 0;
    
    for (int i = 0; i < 100; i++)
    {
        if ((gAllInfo.chg[i].stationId >= snIndex)
            && (gAllInfo.chg[i].stationId <= snIndex + gPartInfo.leaveStationNum - 1))
        {
            chgSiteIndex = gAllInfo.chg[i].stationId - snIndex;
            preChg[chgSiteIndex].setGeometry(preSite[chgSiteIndex]->x() +
                                             (gPartInfo.site[chgSiteIndex].rectUp.w -
                                              gPartInfo.chg[0].rectUp.w) / 2,
                                             gPartInfo.chg[0].rectUp.y,
                                             gPartInfo.chg[0].rectUp.w,
                                             gPartInfo.chg[0].rectUp.h);
            strLine = QString("%1%2%3").arg(imageDir + "ui_pre/CHANGE/line/")
                      .arg(gAllInfo.chg[i].stationId, 2, 10, QLatin1Char('0'))
                      .arg("station.png");
            preChg[chgSiteIndex].setPixmap(strLine);
            preChg[chgSiteIndex].show();
            chgDispIndex[chgSiteIndex] = true;
        }
        
        if (gAllInfo.chg[i].stationId == 0)
            break;
    }
    
    // 特殊图标
    for (i = 0; i < gPartInfo.leaveStationNum; i++)
    {
        preSp[i].hide();
    }
    
    int spSiteIndex = 0;
    
    for (int i = 0; i < 100; i++)
    {
        if ((gAllInfo.sp[i].stationId >= snIndex)
            && (gAllInfo.sp[i].stationId <= snIndex + gPartInfo.leaveStationNum - 1))
        {
            spSiteIndex = gAllInfo.sp[i].stationId - snIndex;
            
            if (chgDispIndex[spSiteIndex] == true)
            {
                preSp[spSiteIndex].setGeometry(preSite[spSiteIndex]->x() +
                                               gPartInfo.site[spSiteIndex].rectUp.w / 2 -
                                               gPartInfo.chg[0].rectUp.w / 2
                                               - 4 - gPartInfo.sp[0].rectUp.w / 2,
                                               gPartInfo.sp[0].rectUp.y,
                                               gPartInfo.sp[0].rectUp.w,
                                               gPartInfo.sp[0].rectUp.h);
                preChg[spSiteIndex].setGeometry(preSp[spSiteIndex].x() + preSp[spSiteIndex].width() + 4,
                                                gPartInfo.chg[0].rectUp.y,
                                                gPartInfo.chg[0].rectUp.w,
                                                gPartInfo.chg[0].rectUp.h);
            }
            else
            {
                preSp[spSiteIndex].setGeometry(preSite[spSiteIndex]->x() +
                                               (gPartInfo.site[spSiteIndex].rectUp.w -
                                                gPartInfo.sp[0].rectUp.w) / 2,
                                               gPartInfo.sp[0].rectUp.y,
                                               gPartInfo.sp[0].rectUp.w,
                                               gPartInfo.sp[0].rectUp.h);
            }
            
            strLine = QString("%1%2%3").arg(imageDir + "ui_pre/SP/sp")
                      .arg(gAllInfo.sp[i].stationId, 2, 10, QLatin1Char('0'))
                      .arg(".png");
            preSp[spSiteIndex].setPixmap(strLine);
            preSp[spSiteIndex].show();
        }
        
        if (gAllInfo.sp[i].stationId == 0)
            break;
    }
    
    //5. 开门侧显示
    premationImg.hide();
    preOpenImg.hide();
    preOpenText.hide();
    preCloseImg.hide();
    preCloseText.hide();
    
    if (gMainReceive.door)
    {
        premationImg.show();
        preOpenImg.show();
        preOpenText.show();
    }
    else
    {
        premationImg.show();
        preCloseImg.show();
        preCloseText.show();
    }
    
    preDoorErr.hide();
    
    if (gMainReceive.doorErr == 1)
        preDoorErr.show();


    preImetion.hide();
    str = QString("%1%2%3").arg(imageDir + "imetion/")
          .arg(gMainReceive.emeId, 2, 10, QLatin1Char('0'))
          .arg(".png");

    if (gMainReceive.emeId)
    {
        preImetion.setPixmap(str);
        preImetion.show();
    }

}

void MainWindow::preDown()
{
    QString str, str_r, strLine;
    QString filename;
    QColor color;
    QPalette palette;
    int i; // 站名索引
    int snIndex; // 站名索引
    int nextIndex; // 下一站放大索引
    int ST_StationName[StationMaxNumber]; // 各个站状态
    int preStationName[StationMaxNumber]; // 各个站id
    int ST_Site[StationMaxNumber];
    int ST_Track[StationMaxNumber];
    qDebug() << "---preDown---";
    // 1. 状态处理
#if CROSS_ROAD
    
    if ((gMainReceive.start <= gMainReceive.next + 2)
        && (gMainReceive.start - gMainReceive.end + 1 > gPartInfo.leaveStationNum))
    {
        nextIndex = gMainReceive.start - gMainReceive.next;
        snIndex = gMainReceive.start;
    }
    else if ((gMainReceive.start >= gMainReceive.next + 2)
             && (gMainReceive.end + 2 <= gMainReceive.next))
    {
        nextIndex = 2;
        snIndex = gMainReceive.next + 2;
    }
    else
    {
        nextIndex = gPartInfo.leaveStationNum - 1 - (gMainReceive.next - gMainReceive.end);
        snIndex = gMainReceive.end + gPartInfo.leaveStationNum - 1;
    }
    
#else
    
    if (gMainReceive.next >= (StationMaxNumber - 1))
    {
        nextIndex = StationMaxNumber - gMainReceive.next;
        snIndex = StationMaxNumber;
    }
    else if (((gMainReceive.next + 2) <= StationMaxNumber) && (gMainReceive.next >= 3))
    {
        nextIndex = 2;
        snIndex = gMainReceive.next + 2;
    }
    else
    {
        nextIndex = gPartInfo.leaveStationNum - 1 - (gMainReceive.next - 1);
        snIndex = gPartInfo.leaveStationNum;
    }
    
#endif
    qDebug() << "nextIndex:" << nextIndex;
    qDebug() << "snIndex:" << snIndex;
    preSnIndex = snIndex;
    
    for (i = 0; i < gPartInfo.leaveStationNum; i++) // 站名
    {
        if ((snIndex - i) > gMainReceive.start)
        {
            ST_StationName[i] = ST_OUT;
            ST_Site[i] = ST_OUT;
            
            if (i > 0)
                ST_Track[i - 1] = ST_OUT;
        }
        else if ((snIndex - i) == gMainReceive.start)
        {
            ST_StationName[i] = ST_PASS;
            ST_Site[i] = ST_PASS;
            
            if (i > 0)
                ST_Track[i - 1] = ST_OUT;
        }
        else if ((snIndex - i) > gMainReceive.current)
        {
            ST_StationName[i] = ST_PASS;
            ST_Site[i] = ST_PASS;
            
            if (i > 0)
                ST_Track[i - 1] = ST_PASS;
        }
        else if ((snIndex - i) == gMainReceive.current)
        {
            ST_StationName[i] = ST_CURRENT;
            ST_Site[i] = ST_CURRENT;
            
            if (i > 0)
                ST_Track[i - 1] = ST_CURRENT;
        }
        else if (((snIndex - i) < gMainReceive.current)
                 && ((snIndex - i) > gMainReceive.next))
        {
            ST_StationName[i] = ST_SKIP;
            ST_Site[i] = ST_SKIP;
            
            if (i > 0)
                ST_Track[i - 1] = ST_NEXTN;
        }
        else if ((snIndex - i) == gMainReceive.next)
        {
            ST_StationName[i] = ST_NEXT;
            ST_Site[i] = ST_NEXT;
            
            if (i > 0)
                ST_Track[i - 1] = ST_NEXT;
        }
        else if (((snIndex - i) < gMainReceive.next)
                 && ((snIndex - i) >= gMainReceive.end))
        {
            ST_StationName[i] = ST_NEXTN;
            ST_Site[i] = ST_NEXTN;
            
            if (i > 0)
                ST_Track[i - 1] = ST_NEXTN;
                
            //pre jump
            if ((gMainReceive.skipId[(snIndex - i - 1) / 8] & (1 << ((snIndex - i - 1) % 8))) > 0)
            {
                ST_StationName[i] = ST_SKIP;
                ST_Site[i] = ST_SKIP;
                
                if (i > 0)
                    ST_Track[i - 1] = ST_NEXTN;
            }
        }
        else
        {
            ST_StationName[i] = ST_OUT;
            ST_Site[i] = ST_OUT;
            
            if (i > 0)
                ST_Track[i - 1] = ST_OUT;
        }
        
        preStationName[i] = snIndex - i;
    }
    
    preNotRun.hide();
    
    for (i = 0; i < gPartInfo.leaveStationNum; i++)
    {
        if ((snIndex - i) == gMainReceive.current)
        {
            ST_StationName[i] = ST_CURRENT;
            ST_Site[i] = ST_CURRENT;
            
            if (i > 0)
                ST_Track[i - 1] = ST_CURRENT;
        }
        
        if ((snIndex - i) == gMainReceive.next)
        {
            ST_StationName[i] = ST_NEXT;
            ST_Site[i] = ST_NEXT;
            
            if (i > 0)
                ST_Track[i - 1] = ST_NEXT;
        }
        
        if((snIndex - i) >= NOT_IN_SERVICE)
//        if (((snIndex - i) == NOT_OPEN_ID_32) || ((snIndex - i) == NOT_OPEN_ID_33) || ((snIndex - i) == NOT_OPEN_ID_34))
        {
            ST_StationName[i] = ST_OUT;
//            ST_Site[i] = ST_OUT;
            preNotRun.show();
            preNotRun.setGeometry(preSite[i]->x(), preSite[i]->y(),
                                  preSite[i]->width(), preSite[i]->height());
        }
    }
    
    // 2. 显示处理
    preTrack[gPartInfo.leaveStationNum - 1]->hide();
    
    for (i = 0; i < gPartInfo.leaveStationNum; i++)
    {
        preSNEn[i]->hide();
        preSNCh[i]->hide();
        
        switch (ST_StationName[i])
        {
            case ST_OUT:
                preSNEn[i]->show();
                preSNCh[i]->show();
                color.setRgb(gPublicInfo.SNNextColor.rgba.r, gPublicInfo.SNNextColor.rgba.g,
                             gPublicInfo.SNNextColor.rgba.b);
                palette.setColor(QPalette::WindowText, color);
                break;
                
            case ST_PASS:
                preSNEn[i]->show();
                preSNCh[i]->show();
                color.setRgb(gPublicInfo.SNNextColor.rgba.r, gPublicInfo.SNNextColor.rgba.g,
                             gPublicInfo.SNNextColor.rgba.b);
                palette.setColor(QPalette::WindowText, color);
                break;
                
            case ST_CURRENT:
                preSNEn[i]->show();
                preSNCh[i]->show();
                color.setRgb(gPublicInfo.SNNextColor.rgba.r, gPublicInfo.SNNextColor.rgba.g,
                             gPublicInfo.SNNextColor.rgba.b);
                palette.setColor(QPalette::WindowText, color);
                break;
                
            case ST_NEXT:
                color.setRgb(gPublicInfo.SNNextColor.rgba.r, gPublicInfo.SNNextColor.rgba.g,
                             gPublicInfo.SNNextColor.rgba.b);
                palette.setColor(QPalette::WindowText, color);
                preSNEn[i]->show();
                preSNCh[i]->show();
                break;
                
            case ST_NEXTN:
                color.setRgb(gPublicInfo.SNNextNColor.rgba.r, gPublicInfo.SNNextNColor.rgba.g,
                             gPublicInfo.SNNextNColor.rgba.b);
                palette.setColor(QPalette::WindowText, color);
                preSNEn[i]->show();
                preSNCh[i]->show();
                break;
                
            case ST_SKIP:
                color.setRgb(gPublicInfo.SNPassColor.rgba.r, gPublicInfo.SNPassColor.rgba.g,
                             gPublicInfo.SNPassColor.rgba.b);
                palette.setColor(QPalette::WindowText, color);
                preSNEn[i]->show();
                preSNCh[i]->show();
                break;
                
            default:
                break;
        }
        
        preSNEn[i]->setPalette(palette);
        preSNCh[i]->setPalette(palette);
        
        if (preStationName[i] > 0)
        {
            preSNEn[i]->setText(stationEnName[preStationName[i] - 1]);
            preSNCh[i]->setText(stationChName[preStationName[i] - 1]);
            
            if (stationEnDispLines[preStationName[i] - 1] == 2)
                preSNEn[i]->setText(stationEnName[preStationName[i] - 1]
                                    + stationEnName2[preStationName[i] - 1]);
                                    
            if (stationEnDispLines[preStationName[i] - 1] == 20)
                preSNEn[i]->setText(stationEnName[preStationName[i] - 1] + " "
                                    + stationEnName2[preStationName[i] - 1]);
        }
        
        if (gMainReceive.mirror)
        {
            str_r = "L.png";
        }
        else
        {
            str_r = "R.png";
        }
        
        switch (ST_Track[i])
        {
            case ST_OUT:
                str = imageDir + "ui_pre/TRACK/pass" + str_r;
                preTrack[i]->show();
                break;
                
            case ST_PASS:
                str = imageDir + "ui_pre/TRACK/pass" + str_r;
                preTrack[i]->show();
                break;
                
            case ST_CURRENT:
                str = imageDir + "ui_pre/TRACK/pass" + str_r;
                preTrack[i]->show();
                break;
                
            case ST_NEXT:
                str = imageDir + "ui_pre/TRACK/next" + str_r;
                preTrack[i]->show();
                break;
                
            case ST_NEXTN:
                str = imageDir + "ui_pre/TRACK/nextn" + str_r;
                preTrack[i]->show();
                break;
                
            case ST_SKIP:
                str = imageDir + "ui_pre/TRACK/nextn" + str_r;
                preTrack[i]->show();
                break;
                
            default:
                break;
        }
        
        preTrack[i]->setPixmap(str);
        preJump[i]->hide();
        
        switch (ST_Site[i])
        {
            case ST_OUT:
                preSite[i]->show();
                str = imageDir + "ui_pre/SITE/current.png";
                break;
                
            case ST_PASS:
                preSite[i]->show();
                str = imageDir + "ui_pre/SITE/current.png";
                break;
                
            case ST_CURRENT:
                preSite[i]->show();
                str = imageDir + "ui_pre/SITE/current.png";
                break;
                
            case ST_NEXT:
                preSiteSt = 1;
                preSite[i]->show();
                break;
                
            case ST_NEXTN:
                preSite[i]->show();
                str = imageDir + "ui_pre/SITE/nextn.png";
                break;
                
            case ST_SKIP:
                preSite[i]->show();
                str = imageDir + "ui_pre/SITE/current.png";
                preJump[i]->show();
                break;
                
            default:
                break;
        }
        
        preSite[i]->setPixmap(str);
    }
    
#ifdef PREDO
    
    if (gMainReceive.next == 2)
        preSite[3]->hide();
        
    if (gMainReceive.next == gMainReceive.start - 1)
        preSite[1]->hide();
        
#endif
        
    if (gMainReceive.next == gMainReceive.end)
        preSite[4]->hide();
        
    // 换乘显示
    // 下一站换乘
    bool chgDispIndex[10];
    
    for (i = 0; i < gPartInfo.leaveStationNum; i++)
    {
        preChg[i].hide();
        chgDispIndex[i] = false;
    }
    
    int chgSiteIndex = 0;
    
    for (int i = 0; i < 100; i++)
    {
        if ((gAllInfo.chg[i].stationId <= snIndex)
            && (gAllInfo.chg[i].stationId >= snIndex - gPartInfo.leaveStationNum + 1))
        {
            chgSiteIndex = snIndex - gAllInfo.chg[i].stationId;
            preChg[chgSiteIndex].setGeometry(preSite[chgSiteIndex]->x() +
                                             (gPartInfo.site[chgSiteIndex].rectUp.w -
                                              gPartInfo.chg[0].rectUp.w) / 2,
                                             gPartInfo.chg[0].rectUp.y,
                                             gPartInfo.chg[0].rectUp.w,
                                             gPartInfo.chg[0].rectUp.h);
            strLine = QString("%1%2%3").arg(imageDir + "ui_pre/CHANGE/line/")
                      .arg(gAllInfo.chg[i].stationId, 2, 10, QLatin1Char('0'))
                      .arg("station.png");
            preChg[chgSiteIndex].setPixmap(strLine);
            preChg[chgSiteIndex].show();
            chgDispIndex[chgSiteIndex] = true;
        }
        
        if (gAllInfo.chg[i].stationId == 0)
            break;
    }
    
    // 特殊图标
    for (i = 0; i < gPartInfo.leaveStationNum; i++)
    {
        preSp[i].hide();
    }
    
    int spSiteIndex = 0;
    
    for (int i = 0; i < 100; i++)
    {
        if ((gAllInfo.sp[i].stationId <= snIndex)
            && (gAllInfo.sp[i].stationId >= snIndex - gPartInfo.leaveStationNum + 1))
        {
            spSiteIndex = snIndex - gAllInfo.sp[i].stationId;
            
            if (chgDispIndex[spSiteIndex] == true)
            {
                preSp[spSiteIndex].setGeometry(preSite[spSiteIndex]->x() +
                                               gPartInfo.site[spSiteIndex].rectUp.w / 2 -
                                               gPartInfo.chg[0].rectUp.w / 2
                                               - 4 - gPartInfo.sp[0].rectUp.w / 2,
                                               gPartInfo.sp[0].rectUp.y,
                                               gPartInfo.sp[0].rectUp.w,
                                               gPartInfo.sp[0].rectUp.h);
                preChg[spSiteIndex].setGeometry(preSp[spSiteIndex].x() + preSp[spSiteIndex].width() + 4,
                                                gPartInfo.chg[0].rectUp.y,
                                                gPartInfo.chg[0].rectUp.w,
                                                gPartInfo.chg[0].rectUp.h);
            }
            else
            {
                preSp[spSiteIndex].setGeometry(preSite[spSiteIndex]->x() +
                                               (gPartInfo.site[spSiteIndex].rectUp.w -
                                                gPartInfo.sp[0].rectUp.w) / 2,
                                               gPartInfo.sp[0].rectUp.y,
                                               gPartInfo.sp[0].rectUp.w,
                                               gPartInfo.sp[0].rectUp.h);
            }
            
            //qDebug()<<"gAllInfo.sp[i].stationId:"<< gAllInfo.sp[i].stationId;
            strLine = QString("%1%2%3").arg(imageDir + "ui_pre/SP/sp")
                      .arg(gAllInfo.sp[i].stationId, 2, 10, QLatin1Char('0'))
                      .arg(".png");
            preSp[spSiteIndex].setPixmap(strLine);
            preSp[spSiteIndex].show();
        }
        
        if (gAllInfo.sp[i].stationId == 0)
            break;
    }
    
    //5. 开门侧显示
    premationImg.hide();
    preOpenImg.hide();
    preOpenText.hide();
    preCloseImg.hide();
    preCloseText.hide();
    
    if (gMainReceive.door)
    {
        preOpenImg.show();
        premationImg.show();
        preOpenText.show();
    }
    else
    {
        premationImg.show();
        preCloseImg.show();
        preCloseText.show();
    }
    
    preDoorErr.hide();
    
    if (gMainReceive.doorErr == 1)
        preDoorErr.show();

    preImetion.hide();
    str = QString("%1%2%3").arg(imageDir + "imetion/")
          .arg(gMainReceive.emeId, 2, 10, QLatin1Char('0'))
          .arg(".png");

    if (gMainReceive.emeId)
    {
        preImetion.setPixmap(str);
        preImetion.show();
    }

}

void MainWindow::preProc()
{
    preReInit();
    
    if (gMainReceive.start < gMainReceive.end)
        preUp();
    else
        preDown();
        
    //menu disp
    preMDstSNEn.setText(stationEnName[gMainReceive.end - 1]); // dst
    preMDstSNCh.setText(stationChName[gMainReceive.end - 1]);
    preMDstSNEn.adjustSize();
    preMDstSNCh.adjustSize();
    preMDstSNEn.setGeometry(preMDstSNCh.x() + preMDstSNCh.width() + 8,
                            gPublicInfo.MDstSNEn.rectUp.y,
                            preMDstSNEn.width(),
                            gPublicInfo.MDstSNEn.rectUp.h);
    preMDstSNEn.setAlignment(Qt::AlignLeft);
    int dstText_x = 0;
    dstText_x = gPublicInfo.MDstSNCh.rectUp.x + gPublicInfo.MDstSNCh.rectUp.w / 2
                - (preMDstTextCh.width() + 10 + preMDstTextEn.width() +
                   preMDstSNCh.width() + 10 + preMDstSNEn.width()) / 2;
    preMDstTextCh.setGeometry( gPublicInfo.MDstSNCh.rectUp.x,
                               gPublicInfo.MDstSNCh.rectUp.y,
                               preMDstTextCh.width(),
                               gPublicInfo.MDstSNCh.rectUp.h);
    preMDstTextEn.setGeometry( gPublicInfo.MDstSNCh.rectUp.x,
                               gPublicInfo.MDstSNEn.rectUp.y,
                               preMDstTextEn.width(),
                               gPublicInfo.MDstSNEn.rectUp.h);
    preMDstSNCh.setGeometry( gPublicInfo.MDstSNCh.rectUp.x,
                             gPublicInfo.MDstSNCh.rectUp.y - 2,
                             preMDstSNCh.width(),
                             gPublicInfo.MDstSNCh.rectUp.h + 2);
    preMDstSNEn.setGeometry( gPublicInfo.MDstSNCh.rectUp.x + 2,
                             gPublicInfo.MDstSNEn.rectUp.y - 6,
                             preMDstSNEn.width(),
                             gPublicInfo.MDstSNEn.rectUp.h);
    //MNext
    int psize = 0;
    QFont font = preMNextSNCh.font();
    font.setPixelSize(gPublicInfo.MNextSNCh.fontSize);
    preMNextSNCh.setFont(font);
    preMNextSNCh.setText(stationChName[gMainReceive.next - 1]);
    preMNextSNCh.adjustSize();
    
    while (preMNextSNCh.width() > gPublicInfo.MNextSNCh.rectUp.w)
    {
        psize++;
        font.setPixelSize(gPublicInfo.MNextSNCh.fontSize - psize);
        preMNextSNCh.setFont(font);
        preMNextSNCh.setText(stationChName[gMainReceive.next - 1]);
        preMNextSNCh.adjustSize();
    }
    
    preMNextSNCh.setGeometry(gPublicInfo.MNextSNCh.rectUp.x,
                             gPublicInfo.MNextSNCh.rectUp.y - 12,
                             gPublicInfo.MNextSNCh.rectUp.w,
                             gPublicInfo.MNextSNCh.rectUp.h);
    //    preMNextSNCh.setAlignment(Qt::AlignCenter);
    preMNextSNCh.setText(stationChName[gMainReceive.next - 1]);
    preMNextSNEn.setGeometry(gPublicInfo.MNextSNEn.rectUp.x,
                             gPublicInfo.MNextSNEn.rectUp.y - 10,
                             gPublicInfo.MNextSNEn.rectUp.w,
                             gPublicInfo.MNextSNEn.rectUp.h);
    //    preMNextSNEn.setAlignment(Qt::AlignCenter);
    preMNextSNEn.setText(stationEnName[gMainReceive.next - 1]);
//    setSiteTime(PRE);
    chgToWidget(PRE);
}

void MainWindow::arrProc()
{
    QString str;
    QString filename;
    int i; // 站名索引
    qDebug() << "---arrProc---";
    //arrReInit();
    //menu disp
    arrMDstSNEn.setText(stationEnName[gMainReceive.end - 1]); // dst
    arrMDstSNCh.setText(stationChName[gMainReceive.end - 1]);
    arrMDstSNEn.adjustSize();
    arrMDstSNCh.adjustSize();
    arrMDstSNEn.setGeometry(arrMDstSNCh.x() + arrMDstSNCh.width() + 8,
                            gPublicInfo.MDstSNEn.rectUp.y,
                            arrMDstSNEn.width(),
                            gPublicInfo.MDstSNEn.rectUp.h);
    arrMDstSNEn.setAlignment(Qt::AlignLeft);
    int dstText_x = 0;
    dstText_x = gPublicInfo.MDstSNCh.rectUp.x + gPublicInfo.MDstSNCh.rectUp.w / 2
                - (arrMDstTextCh.width() + 10 + arrMDstTextEn.width() +
                   arrMDstSNCh.width() + 10 + arrMDstSNEn.width()) / 2;
    arrMDstTextCh.setGeometry(gPublicInfo.MDstSNCh.rectUp.x,
                              gPublicInfo.MDstSNCh.rectUp.y,
                              arrMDstTextCh.width(),
                              gPublicInfo.MDstSNCh.rectUp.h);
    arrMDstTextEn.setGeometry(gPublicInfo.MDstSNCh.rectUp.x,
                              gPublicInfo.MDstSNEn.rectUp.y,
                              arrMDstTextEn.width(),
                              gPublicInfo.MDstSNEn.rectUp.h);
    arrMDstSNCh.setGeometry(gPublicInfo.MDstSNCh.rectUp.x,
                            gPublicInfo.MDstSNCh.rectUp.y - 2,
                            arrMDstSNCh.width(),
                            gPublicInfo.MDstSNCh.rectUp.h + 2);
    arrMDstSNEn.setGeometry(gPublicInfo.MDstSNCh.rectUp.x + 2,
                            gPublicInfo.MDstSNEn.rectUp.y - 6,
                            arrMDstSNEn.width(),
                            gPublicInfo.MDstSNEn.rectUp.h);
    //MNext
    int psize = 0;
    QFont font = arrMNextSNCh.font();
    font.setPixelSize(gPublicInfo.MNextSNCh.fontSize);
    arrMNextSNCh.setFont(font);
    arrMNextSNCh.setText(stationChName[gMainReceive.next - 1]);
    arrMNextSNCh.adjustSize();
    
    while (arrMNextSNCh.width() > gPublicInfo.MNextSNCh.rectUp.w)
    {
        psize++;
        font.setPixelSize(gPublicInfo.MNextSNCh.fontSize - psize);
        arrMNextSNCh.setFont(font);
        arrMNextSNCh.setText(stationChName[gMainReceive.next - 1]);
        arrMNextSNCh.adjustSize();
    }
    
    arrMNextSNCh.setGeometry(gPublicInfo.MNextSNCh.rectUp.x,
                             gPublicInfo.MNextSNCh.rectUp.y - 12,
                             gPublicInfo.MNextSNCh.rectUp.w,
                             gPublicInfo.MNextSNCh.rectUp.h);
    //    arrMNextSNCh.setAlignment(Qt::AlignCenter);
    arrMNextSNCh.setText(stationChName[gMainReceive.next - 1]);
    arrMNextSNEn.setGeometry(gPublicInfo.MNextSNEn.rectUp.x,
                             gPublicInfo.MNextSNEn.rectUp.y - 10,
                             gPublicInfo.MNextSNEn.rectUp.w,
                             gPublicInfo.MNextSNEn.rectUp.h);
    //    arrMNextSNEn.setAlignment(Qt::AlignCenter);
    arrMNextSNEn.setText(stationEnName[gMainReceive.next - 1]);
    
    // arr sn
    //arrChSN.setText(stationChName[gMainReceive.next - 1]);
    
    if (stationEnDispLines[gMainReceive.next - 1] == 2)
        arrEnSN.setText(stationEnName[gMainReceive.next - 1] + stationEnName2[gMainReceive.next - 1]);
    else if (stationEnDispLines[gMainReceive.next - 1] == 20)
        arrEnSN.setText(stationEnName[gMainReceive.next - 1] + " " + stationEnName2[gMainReceive.next - 1]);
    else
        arrEnSN.setText(stationEnName[gMainReceive.next - 1]);
        
    // arr Train
    QGraphicsOpacityEffect *opacityEffect2 = new QGraphicsOpacityEffect;
    opacityEffect2->setOpacity(0.8);
    arrTrainWidget.setGraphicsEffect(opacityEffect2);
    arrTrain.setParent(&arrTrainWidget);
    arrTrain.setGeometry(gPartInfo.arrTrain.rectUp.x,
                         gPartInfo.arrTrain.rectUp.y,
                         gPartInfo.arrTrain.rectUp.w,
                         gPartInfo.arrTrain.rectUp.h);
    int carid = (ip4 - 1) / 10 + 1;
    int maxid = 7;
    QString strCar;
    strCar = QString("%1%2").arg(carid, 02, 10, QLatin1Char('0')).arg(".png");
    
    if (gMainReceive.keySide == 1)
    {
        if (gMainReceive.mirror == 0)
        {
            arrCar[6 - carid].setPixmap(filename = imageDir + "ui_arr/TRAIN/" + strCar);
            curentCar[6 - carid].setPixmap(filename = imageDir + "ui_arr/TRAIN/" + "current.png");
        }
        else
        {
            arrCar[carid - 1].setPixmap(filename = imageDir + "ui_arr/TRAIN/" + strCar);
            curentCar[carid - 1].setPixmap(filename = imageDir + "ui_arr/TRAIN/" + "current.png");
        }
    }
    
    if (gMainReceive.keySide == 2)
    {
        if (gMainReceive.mirror == 0)
        {
            arrCar[carid - 1].setPixmap(filename = imageDir + "ui_arr/TRAIN/" + strCar);
            curentCar[carid - 1].setPixmap(filename = imageDir + "ui_arr/TRAIN/" + "current.png");
        }
        else
        {
            arrCar[6 - carid].setPixmap(filename = imageDir + "ui_arr/TRAIN/" + strCar);
            curentCar[6 - carid].setPixmap(filename = imageDir + "ui_arr/TRAIN/" + "current.png");
        }
    }
    

    
  //  for (i = 0; i < 8; i++)
  //      arrCar[i].hide();
        
    //arrCar[carid-1].show();
    str = QString("%1%2").arg(gMainReceive.next, 02, 10, QLatin1Char('0')).arg(".png");
    if(gMainReceive.next >= NOT_IN_SERVICE)
        str = "32.png";
    
    if (gMainReceive.mirror == 0)
    {
        if (gMainReceive.start < gMainReceive.end)
        {
            arrExit.setPixmap(imageDir + "ui_arr/EXIT/up/leftSide/" + str);
        }
        else
        {
            arrExit.setPixmap(imageDir + "ui_arr/EXIT/down/leftSide/" + str);
        }
    }
    else
    {
        if (gMainReceive.start < gMainReceive.end)
        {
            arrExit.setPixmap(imageDir + "ui_arr/EXIT/up/rightSide/" + str);
        }
        else
        {
            arrExit.setPixmap(imageDir + "ui_arr/EXIT/down/rightSide/" + str);
        }
    }
    

    
    if (gMainReceive.keySide == 1)
    {
        if (gMainReceive.mirror == 0)
        {
            filename = imageDir + "ui_arr/TRAIN/tc1.png";
        }
        else
        {
            filename = imageDir + "ui_arr/TRAIN/tc1M.png";
        }
    }
    
    if (gMainReceive.keySide == 2)
    {
        if (gMainReceive.mirror == 0)
        {
            filename = imageDir + "ui_arr/TRAIN/tc2.png";
        }
        else
        {
            filename = imageDir + "ui_arr/TRAIN/tc2M.png";
        }
    }
    
    arrTrain.setPixmap(filename);
#if 0
    arrTrainMovie->stop();
    arrTrain.setMovie(arrTrainMovie);
    arrTrainMovie->setFileName(filename);
    arrTrainMovie->setSpeed(100);
    arrTrainMovie->start();
    qDebug() << "keySide=" << gMainReceive.keySide << " mirror=" << gMainReceive.mirror;
#endif
#if 0
    // 换乘显示
    arrChgLine.hide();
    arrChgText.hide();
    int spChgFlag = 0;
    QImage imageChg;
    
    for (int i = 0; i < 100; i++)
    {
        if ((gAllInfo.chg[i].stationId != 0))
        {
            if (gAllInfo.chg[i].stationId == gMainReceive.next)
            {
                str = QString("%1%2%3").arg(imageDir + "ui_arr/CHANGE/line/")
                      .arg(gMainReceive.next, 2, 10, QLatin1Char('0')).arg("station.png");
                imageChg.load(str);
                arrChgLine.setGeometry(gPartInfo.arrChgLine.rectUp.x,
                                       gPartInfo.arrChgLine.rectUp.y,
                                       imageChg.width(),
                                       imageChg.height());
                arrChgLine.setPixmap(str);
                arrChgLine.show();
                arrChgText.show();
                spChgFlag = 1;
            }
        }
        else
        {
            break;
        }
    }
    
#endif
    // 特殊图标显示
#if 0
    arrSp.hide();
    QImage imageSp;
    
    for (int i = 0; i < 100; i++)
    {
        if ((gAllInfo.sp[i].stationId != 0))
        {
            if (gAllInfo.sp[i].stationId == gMainReceive.next)
            {
                str = QString("%1%2%3").arg(imageDir + "ui_arr/SP/sp").arg(gMainReceive.next, 2, 10, QLatin1Char('0')).arg(".png");
                imageSp.load(str);
                
                if (spChgFlag)
                {
                    arrSp.setGeometry(arrChgLine.x() + arrChgLine.width() + 8,
                                      gPartInfo.arrSp.rectUp.y,
                                      imageSp.width(),
                                      imageSp.height());
                }
                else
                {
                    arrSp.setGeometry(gPartInfo.arrSp.rectUp.x,
                                      gPartInfo.arrSp.rectUp.y,
                                      imageSp.width(),
                                      imageSp.height());
                }
                
                arrSp.setPixmap(str);
                arrSp.show();
                arrChgText.show();
            }
        }
        else
        {
            break;
        }
    }
    
#endif
    //door
    //exportImg.hide();
    TipsImg.hide();
    serviceImg.hide();
    //    arrOpenDoor.hide();
    arrOpenText.hide();
    arrCloseDoor.hide();
    arrCloseText.hide();
    movieArrDoor->stop();
    
    if (gMainReceive.door)
    {
        arrOpenDoor.show();
        filename = filePath + "images/ui_arr/DOOR/open.gif";
        movieArrDoor->stop();
        arrOpenDoor.setMovie(movieArrDoor);
        movieArrDoor->setFileName(filename);
        movieArrDoor->setSpeed(100);
        movieArrDoor->start();
        //exportImg.show();
        TipsImg.show();
        serviceImg.show();
        arrOpenText.show();
    }
    else
    {
        arrOpenDoor.hide();
        //exportImg.show();
        TipsImg.show();
        serviceImg.show();
        arrCloseDoor.show();
        arrCloseText.show();
    }
    
    arrDoorErr.hide();
    
    if (gMainReceive.doorErr == 1)
    {
        arrDoorErr.show();
    }
    

    arrImetion.hide();
    str = QString("%1%2%3").arg(imageDir + "imetion/")
          .arg(gMainReceive.emeId, 2, 10, QLatin1Char('0'))
          .arg(".png");

    if (gMainReceive.emeId)
    {
        arrImetion.setPixmap(str);
        arrImetion.show();
    }

    chgToWidget(ARR);

}

