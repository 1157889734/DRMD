#include <stdio.h>
#include<pthread.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netinet/in.h>
#include<errno.h>
#include <unistd.h>
#include <strings.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <linux/un.h>

#include "drmd_ui.h"

#define RECV_SOCKET "/root/socket/iptv2drmd.socket"
#define SEND_SOCKET "/root/socket/drmd2iptv.socket"

static struct sockaddr_un recvSocketAddr;
static struct sockaddr_un sendSocketAddr;
static int flag = -1;
static int socketFd = -1;
static int socketSendFd = -1;

int DrmdInit()
{
    int ret;
    
    if (flag > 0)
    {
        return 1;
    }
    
    socketFd = socket(PF_LOCAL, SOCK_DGRAM, 0);
    
    if (socketFd < 0)
    {
        perror("socket");
        shutdown(socketFd, 2);
        return -1;
    }
    
    socketSendFd = socket(PF_LOCAL, SOCK_DGRAM, 0);
    
    if (socketSendFd < 0)
    {
        perror("socketSendFd");
        shutdown(socketSendFd, 2);
        return -1;
    }
    
    unlink(RECV_SOCKET);
    //unlink(SEND_SOCKET);
    bzero(&recvSocketAddr, sizeof(recvSocketAddr));
    recvSocketAddr.sun_family = AF_LOCAL;
    strcpy(recvSocketAddr.sun_path, RECV_SOCKET);
    ret = bind(socketFd , (struct sockaddr *)&recvSocketAddr , sizeof(recvSocketAddr));
    
    if (ret < 0)
    {
        perror("bind drmd");
        shutdown(socketFd, 2);
        shutdown(socketSendFd, 2);
        return -2;
    }
    
    bzero(&sendSocketAddr, sizeof(sendSocketAddr));
    sendSocketAddr.sun_family = AF_UNIX;
    strcpy(sendSocketAddr.sun_path, SEND_SOCKET);
    printf("DrmdInit is ok\n");
    //ret = pthread_create(&recvThreadFd, NULL, recvPthread, NULL);
    //ret = pthread_create(&sendThreadFd, NULL, sendPthread, NULL);
    flag = 1;
    return 1;
}

int DrmdReadData(unsigned char *buf, int len)
{
    int len1;
    
    if (flag < 0)
    {
        return -1;
    }
    
    if ((len1 = recvfrom(socketFd, buf, len, 0, NULL, NULL)) < 0)
    {
        perror("recvfrom");
        return -2;
    }
    
    return len1;
}

int DrmdWriteData(unsigned char *buf, int len)
{
    int len1;
    
    if (flag < 0)
    {
        return -1;
    }
    
    if ((len1 = sendto(socketSendFd, buf, len, 0, (struct sockaddr *)&sendSocketAddr, sizeof(sendSocketAddr))) < 0)
    {
        //perror("drmd sendto main");
        return -2;
    }
    
    return len1;
}

int target(unsigned char *dst, int type, int cmd, unsigned char *buf, int bufLen)
{
    int i;
    unsigned char sum;
    unsigned char xorSum;
    
    if (NULL == dst)
    {
        perror("target");
        return -1;
    }
    
    dst[0] = 0xFE;
    dst[1] = type;
    dst[2] = cmd;
    dst[3] = bufLen;
    
    for (i = 0; i < bufLen; i++)
    {
        dst[4 + i] = buf[i];
    }
    
    for (i = 1, sum = 0; i < bufLen + 4; i++)
    {
        sum += dst[i];
    }
    
    for (i = 1, xorSum = 0; i < bufLen + 4; i++)
    {
        xorSum ^= dst[i];
    }
    
    dst[bufLen + 4] = sum;
    dst[bufLen + 5] = xorSum;
    dst[bufLen + 6] = 0xFF;
    return bufLen + 7;
}

int getPacket(unsigned char *buf, int length)
{
    int ret = -1;
    
    if (NULL == buf)
    {
        perror("target");
        return -1;
    }
    
    if (length > 4)
    {
        if (buf[0] == 0xFE && buf[length - 1] == 0xFF && buf[3] == length - 4)
        {
            int i = 0;
            unsigned char l_sum = 0, l_xorSum = 0, l_len = buf[3];
            
            for (i = 1; i <= l_len; i++)
            {
                l_sum += buf[i];
                l_xorSum ^= buf[i];
            }
            
            if (l_sum == buf[length - 3] && l_xorSum == buf[length - 2])
            {
                ret = 1;
            }
        }
    }
    
    return ret;
}
