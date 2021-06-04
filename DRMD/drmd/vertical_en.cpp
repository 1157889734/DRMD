#include "vertical_en.h"
#include <QDebug>

VERTICALEN::VERTICALEN(QLabel *parent) :
    QLabel(parent)
{
}

VERTICALEN::VERTICALEN(int a)
{
    angle = a;
    QPalette palette;
    palette.setColor(QPalette::Background, QColor(0,0,0,0));
    this->setAutoFillBackground(true);
    this->setPalette(palette);
}

void VERTICALEN::paintEvent(QPaintEvent *)
{
    QFont font;

    font = this->font();
    QPainter painter(this);
    painter.setPen(paintColor.rgb());
    painter.setFont(font);
    painter.translate(5, height);
    painter.rotate(angle);
    painter.drawText(0,0, this->text());
    painter.end();
}

void VERTICALEN::setHeight(int high)
{
    height = high;

}
void VERTICALEN::setPaintOffset(int offset)
{
    paintOffset = offset;
}

void VERTICALEN::setTextColor(QColor color)
{
    paintColor = color;
}

void VERTICALEN::setTextMirror(int value)
{
    mirror = value;
}
