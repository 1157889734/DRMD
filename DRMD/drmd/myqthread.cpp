#include "myqthread.h"
#include <QDebug>
#include <sys/select.h>
//#include "ipCheck.h"
#include <QImage>
#include "param.h"

#if 0
struct timeval
{
    long tv_sec;   /*√Î */
    long tv_usec;  /*Œ¢√Î */
}
int select(int maxfdp1, fd_set *readset, fd_set *writeset, fd_set *exceptset, struct timeval *timeout);
#include <sys/select.h>
int FD_ZERO(int fd, fd_set *fdset);
int FD_CLR(int fd, fd_set *fdset);
int FD_SET(int fd, fd_set *fd_set);
int FD_ISSET(int fd, fd_set *fdset);
#endif

extern public_info_t gPublicInfo;
extern all_info_t gAllInfo;
extern part_info_t gPartInfo;

myqthread::myqthread(QThread *parent) :
    QThread(parent)
{
}
void myqthread::run()
{
    struct timeval timeout;
    static int cnt = 0;
    int ret;
    int times = 0;
    
    while (stopFlag == 0)
    {
        timeout.tv_sec = 0;
        timeout.tv_usec = 20000;
        ret = select(0, NULL, NULL, NULL, &timeout);
        
        if (0 == ret)
        {
            times++;
            this->timeOut20ms();
            
            if (times % 50 == 0)
            {
                this->timeOut1000ms();
                this->life++;
            }
        }
        
        //usleep(100);
        //if(++cnt % 3000 == 1)
        //{
        //printf("drmd==>myqthread_%d running\n", startFlag);
        //}
    }
}


