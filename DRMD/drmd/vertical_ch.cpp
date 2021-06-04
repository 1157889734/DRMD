#include "vertical_ch.h"
#include <QDebug>

VERTICALCH::VERTICALCH(QLabel *parent) :
    QLabel(parent)
{
}

VERTICALCH::VERTICALCH(int a)
{
    angle = a;
    QPalette palette;
    palette.setColor(QPalette::Background, QColor(0,0,0,0));
    this->setAutoFillBackground(true);
    this->setPalette(palette);
}

void VERTICALCH::paintEvent(QPaintEvent *)
{

    QFont font;

    font = this->font();
    QPainter painter(this);
    painter.setPen(paintColor.rgb());
    painter.setFont(font);
    painter.translate(10, height);
    painter.rotate(angle);
    painter.drawText(0,0, this->text());
    painter.end();

//    QFont font;

//    font = this->font();
//    //int chpix = font.pixelSize();
//    int enpix = font.pixelSize() - 1;
//    QPainter painter(this);

//    painter.setPen(paintColor.rgb());
//    //painter.setFont(font);

//    QChar qcPre, qc, qcNext;
//    ushort uniPre, uni, uniNext;
//    int chPre, ch, chNext;
//    QString str=this->text();

//    //qDebug()<<"str:"<<str;

//    for(int i = 0;i < str.length(); i++)
//    {
//        if(i > 0)
//        {
//            qcPre = str.at(i - 1);
//            uniPre = qc.unicode();
//            chPre = (uniPre >= 0x4E00 && uniPre <= 0x9FA5)? 1:0;
//        }
//        else
//        {
//            uniPre = 0;
//            chPre = 0;
//        }

//        qc = str.at(i);
//        uni = qc.unicode();
//        ch = (uni >= 0x4E00 && uni <= 0x9FA5)? 1:0;

//        if(uni == 0xff08)
//        {
//            font.setPixelSize(enpix);
//        }
//        painter.setFont(font);

//        if((i + 1) < str.length())
//        {
//            qcNext = str.at(i + 1);
//            uniNext = qcNext.unicode();
//        }
//        else
//        {
//            uniNext = 0;
//        }
//        chNext = (uniNext >= 0x4E00 && uniNext <= 0x9FA5)? 1:0;

//        if(ch)  // text = Ch
//        {
//            painter.translate(0, height);

//            if((chPre == 0) && (i > 0))
//                painter.translate(2, -4);

//            painter.save();
//            painter.drawText(0,0, qc);
//        }
//        else
//        {
//            painter.translate(0, font.pixelSize() / 2 + 2);

//            if((uniPre == 0x3001) || (uniPre == 0x2c) || (uniPre == 0xff0c)) // ‘、’ ','
//            {
//                painter.translate(0, -9);
//            }

//            painter.save();
//            painter.rotate(angle);

//            if(((i + 1) == str.length()) && (uni == 0xff09))
//            {
//                if(chPre)
//                    painter.drawText(-9,-4, qc);
//                else
//                    painter.drawText(-13,-4, qc);
//            }
//            else
//            {
//                if(uni == 0xff08)
//                    painter.drawText(-11,-4, qc);
//                else
//                    painter.drawText(-13,-4, qc);
//            }
//        }

//        painter.restore();
//    }

//    painter.end();
}

void VERTICALCH::setHeight(int high)
{
    height = high;

}

void VERTICALCH::setPaintOffset(int offset)
{
    paintOffset = offset;
}

void VERTICALCH::setTextColor(QColor color)
{
    paintColor = color;
}

