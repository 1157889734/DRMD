#ifndef VERTICALCH_H
#define VERTICALCH_H

#include <QObject>
#include <QLabel>
#include <QPaintEvent>
#include <QPainter>
#include <QColor>

class VERTICALCH : public QLabel
{
    Q_OBJECT
public:
    explicit VERTICALCH(QLabel *parent = 0);
    VERTICALCH(int angle);
    void setPaintOffset(int offset);
    void setTextColor(QColor color);
    void setHeight(int high);

    int height;
    int angle;
    int paintOffset;
    QColor paintColor;

signals:

public slots:

protected:
    void paintEvent(QPaintEvent *);

};

#endif // MYCLASS_H
