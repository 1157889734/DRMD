#ifndef VERTICALEN_H
#define VERTICALEN_H

#include <QObject>
#include <QLabel>
#include <QPaintEvent>
#include <QPainter>

class VERTICALEN : public QLabel
{
    Q_OBJECT
public:
    explicit VERTICALEN(QLabel *parent = 0);
    VERTICALEN(int angle);
    void setPaintOffset(int offset);
    void setTextColor(QColor color);
    void setTextMirror(int value);
    void setHeight(int high);

    int angle;
    int height;
    int paintOffset;
    int mirror;
    QColor paintColor;

signals:

public slots:

protected:
    void paintEvent(QPaintEvent *);

};

#endif // MYCLASS_H
