#ifndef MYQTHREAD_H
#define MYQTHREAD_H

#include <QObject>
#include <QThread>
//#include "ptu.h"
class myqthread : public QThread
{
    Q_OBJECT
private:

public:
    explicit myqthread(QThread *parent = 0);
    void run();
    
    QString ipStr;
    QString macStr;
    int stopFlag = 0;
    int startFlag = 0;
    int life = 0;
    
signals:
    void timeOut20ms();
    void timeOut1000ms();
    void ipConflictMsg(int flag);
public slots:
};

#endif // MYQTHREAD_H
