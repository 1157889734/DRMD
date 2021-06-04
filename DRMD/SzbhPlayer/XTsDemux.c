

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>


#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <net/if.h>
#include <arpa/inet.h>
#include <sys/ioctl.h>


#include "SzbhApi.h"

#include "XVehiclePlayerDefine.h"
#include "XSectionFile.h"
#include "XConfigFile.h"
#include "XTsDemux.h"


#define dXFrameBufLen		(1024*512)

static unsigned char pXTsDataBuf[188*10+1]={0};

static int tXTsDemuxInit=0;

static XTsStream pXTsStreams[dXStream_Max];
static XTsTable tXTsTable;

extern int XCheckScreenSave(void);
extern int XGetTick(unsigned int *pOutputMs);

static pthread_t tXEsVideoCheckThread=0;
static volatile int tXEsVideoCheckFlag=0;
static unsigned int tXEsPlayerCheckPrevTick=0;


static unsigned short __ntohs(unsigned short tInputData)
{
	return ((tInputData<<8) & 0xff00) | ((tInputData>>8) & 0x00ff);
}

static void XTsTableReset(XTsTable *pInputTable, int tInputLen)
{
	if(pInputTable==NULL)
		return ;

	pInputTable->mLen=tInputLen;
	pInputTable->mOffset=0;
}

static int XTSGetStreamCodecType(unsigned char tInputTypeId)
{
    switch(tInputTypeId)
    {
    case 0x01:
    case 0x02:
        return codec_type_mpeg2_video;
    case 0x80:
        return codec_type_mpeg2_video;
    case 0x10:
	return codec_type_mpeg4_video;
    case 0x1b:
        return codec_type_h264_video;
    case 0xea:
        return codec_type_vc1_video;
    case 0x81:
    case 0x06:
        return codec_type_ac3_audio;
    case 0x0f:
	return codec_type_aac_audio;
    case 0x03:
    case 0x04:
        return codec_type_mp3_audio;
    }

    return codec_type_data;
}

int XTsGetStreamType(unsigned char tInputTypeId)
{
	int tTempIdx=XTSGetStreamCodecType(tInputTypeId);

	static const int pTempCodecType[10]={content_type_unknown, content_type_video, content_type_video, content_type_video, 
		content_type_video, content_type_audio, content_type_audio, content_type_audio, content_type_audio, content_type_audio};

	return pTempCodecType[tTempIdx];
}

u_int64_t XTsDecodePts(const unsigned char* p)
{
    u_int64_t pts=((p[0]&0xe)<<29);
    pts|=((p[1]&0xff)<<22);
    pts|=((p[2]&0xfe)<<14);
    pts|=((p[3]&0xff)<<7);
    pts|=((p[4]&0xfe)>>1);

    return pts;
}

int XTsDemuxPacket(unsigned char *pInputData, int tInputDataLen)
{
	if(tInputDataLen!=188)
	{
		printf("Damon ==> ts data error !\n");
		return -1;
	}

	if(pInputData[0]!=0x47) // invalid packet sync byte
	{
		printf("Damon ==> ts packet head error !\n");
		return -2;
	}

	unsigned short tTempPid=__ntohs(*((unsigned short *)(pInputData+1)));
	if(tTempPid & 0x8000)		// transport error
	{
		printf("Damon ==> transport error !\n");
		return -3;
	}

	unsigned char tTempPayloadUnitStartIndicator=((tTempPid & 0x4000)>>14);
	unsigned char tTempFlags=pInputData[3];

	if(tTempFlags & 0xc0)	// scrambled
	{
		printf("Damon ==> scrambled faield !\n");
		return -4;
	}


	unsigned char tTempAdaptationFieldExist=((tTempFlags & 0x20) >> 5);
	unsigned char tTempPayloadDataExist=((tTempFlags & 0x10) >> 4);

	tTempPid &= 0x1fff;

	unsigned char *pTempData=(unsigned char *)pInputData;
	int tTempDataLen=188;
//printf("\n\nDamon ==> pid : 0x%x  %d \n ", tTempPid, tTempPayloadDataExist);
	if(tTempPid!=0x1fff && tTempPayloadDataExist)
	{
		pTempData += 4;
		tTempDataLen -=4 ;

		if(tTempAdaptationFieldExist)	// skip adaption field
		{
			int tTempSkipLen= (*pTempData)+1;

			if(tTempSkipLen > tTempDataLen)
			{
				return -5;
			}

			pTempData += tTempSkipLen;
			tTempDataLen -= tTempSkipLen;
		}

		if(!tTempPid)
		{
	//	printf("Damon ==> pat table !\n");
			// PAT table
			if(tTempPayloadUnitStartIndicator)
			{
				// skip pointer field
				if(tTempDataLen<1)
				{
					return -6;
				}

				pTempData += 1;
				tTempDataLen -= 1;
			}

			if(*pTempData != 0x00)	// is not pat
			{
				return 0;
			}

			if(tTempDataLen<8)
			{
				return -7;
			}

			unsigned short tTempLen=__ntohs(*((unsigned short *)(pTempData+1)));
			if((tTempLen&0xb000) != 0xb000)
			{
				return -8;
			}

			tTempLen &= 0x0fff;
			tTempDataLen -= 3;
			if(tTempLen > tTempDataLen)
			{
				return -9;
			}

			tTempDataLen -= 5;
			pTempData += 8;
			tTempLen -= 5+4;

			if(tTempLen%4)
			{
				return -10;
			}

			int tTempNum=tTempLen/4;
			int i=0;
	//		printf("Damon ==> program num : %d \n", tTempNum);
			for(i=0; i<tTempNum; i++)
			{
				unsigned short tTempProgram=__ntohs(*((unsigned short *)pTempData));
				unsigned short tTempPid=__ntohs(*((unsigned short *)(pTempData+2)));

				if((tTempPid & 0xe000) != 0xe000)
					return -1;;

				tTempPid &= 0x1fff;
				pTempData += 4;

		//		printf("Damon ==> program id : %d, 0x%x \n", tTempProgram, tTempPid);
				XTsStream *pTempStream=pXTsStreams+dXStream_PmtIdx;
				if(pTempStream!=NULL)
				{
					pTempStream->mPid=tTempPid;
					pTempStream->mProgram=tTempProgram;
					pTempStream->mType=0xff;
				}
			}
		}else
		{
			XTsStream *pTempPmtStream=pXTsStreams+dXStream_PmtIdx;
			XTsStream *pTempVideoStream=pXTsStreams+dXStream_VideoIdx;
			XTsStream *pTempAudioStream=pXTsStreams+dXStream_AudioIdx;
			if(pTempPmtStream!=NULL && pTempPmtStream->mPid==tTempPid)
			{
				// PMT table
		//		printf("Damon ==> pmt table type : 0x%x  %d \n", pTempPmtStream->mType, tTempPayloadUnitStartIndicator);
				if(pTempPmtStream->mType==0xff)
				{
					if(tTempPayloadUnitStartIndicator)
					{
						if(tTempDataLen<1)
						{
							return -12;
						}

						pTempData += 1; 	// skip pointer field
						tTempDataLen -= 1;

						if(*pTempData != 0x02)	// is not pmt
						{printf("Damon ==> not pmt table !\n");
							return 0;
						}

						if(tTempDataLen<12)
						{
							return -13;
						}

						unsigned short tTempLen=__ntohs(*((unsigned short *)(pTempData+1)));

						if((tTempLen&0x3000) != 0x3000)
						{
							return -14;
						}

						tTempLen = (tTempLen&0x0fff)+3;
						if(tTempLen>dXMaxTableLen)
						{
							printf("Damon ==> pmt data error !\n ");
							return -141;
						}

		//			printf("Damon ==> set ts table data len : %d \n", tTempLen);
						XTsTableReset(&tXTsTable, tTempLen);
						unsigned short tTempLL=(tTempDataLen>tTempLen)?tTempLen:tTempDataLen;

						memcpy(tXTsTable.pData, pTempData, tTempLL);						
						tXTsTable.mOffset += tTempLL;
						if(tXTsTable.mOffset<tXTsTable.mLen)
							return 0;		// wait next part
					}else
					{
						if(!tXTsTable.mOffset)
							return -142;

						unsigned short tTempL=tXTsTable.mLen-tXTsTable.mOffset;
						unsigned short tTempLL=(tTempDataLen>tTempL)?tTempL:tTempDataLen;

						memcpy(tXTsTable.pData+tXTsTable.mOffset, pTempData, tTempLL);
						tXTsTable.mOffset += tTempLL;
						if(tXTsTable.mOffset<tXTsTable.mLen)
							return 0;			// wait next part
					}

					pTempData=tXTsTable.pData;
					unsigned short tTempL=tXTsTable.mLen;
					unsigned short tTempN=(__ntohs(*((unsigned short *)(pTempData+10))) & 0x0fff) + 12;

			//		printf("Damon ==> data le : %d %d \n", tTempN, tTempL);
					if(tTempN>tTempL)
					{
						return -15;
					}

					pTempData += tTempN;
					tTempDataLen -= tTempN;

					tTempL -= tTempN+4;
					while(tTempL)
					{
						if(tTempL<5)
						{
							return -16;
						}

						unsigned char tTempType=*pTempData;
						unsigned short tTempPid=__ntohs(*((unsigned short *)(pTempData+1)));

						if((tTempPid & 0xe000) != 0xe000)
						{
							return -17;
						}

						tTempPid &= 0x1fff;

						unsigned short tTempLL=(__ntohs(*((unsigned short *)(pTempData+3))) & 0x0fff)+5;
						if(tTempLL>tTempL)
						{
							return -18;
						}

						pTempData += tTempLL;
						tTempL -= tTempLL;

			//			printf("Damon ==> stream pid : 0x%x 0x%x \n", tTempPid, tTempType);

						XTsStream *pTempStream=NULL;
						int tTempStreamType=XTsGetStreamType(tTempType);
						if(tTempStreamType==content_type_video)
						{
							pTempStream=pTempVideoStream;
							pTempStream->mContentType=content_type_video;
						}else if(tTempStreamType==content_type_audio)
						{
							pTempStream=pTempAudioStream;
							pTempStream->mContentType=content_type_audio;
						}else
						{
							continue;
						}

						if(pTempStream && pTempStream->mType!=tTempType)
						{
							pTempStream->mPid=tTempPid;
							pTempStream->mType=tTempType;
							printf("Damon ==> stream change type = 0x%x \n", pTempStream->mType);

							// set es player type
							if(pTempStream->mContentType==content_type_audio)
							{
								int tTempAudioType=-1;
								int tTempCodecType=XTSGetStreamCodecType(pTempStream->mType);
								if(tTempCodecType==codec_type_aac_audio)
									tTempAudioType=EAudioCodec_AAC;
								else if(tTempCodecType==codec_type_ac3_audio)
									tTempAudioType=EAudioCodec_AC3;
								else if(tTempCodecType==codec_type_mpeg2_audio)
									tTempAudioType=EAudioCodec_MPEG2;
								else if(tTempCodecType==codec_type_mp3_audio)
									tTempAudioType=EAudioCodec_MP3;
								else
									tTempAudioType=ECodecType_Max;

							const char *pTempAudioType[4]={"aac", "ac3", "mp3", "mpeg2"};
							if(tTempAudioType!=ECodecType_Max)
								printf("Damon ==> audio codec type : 0x%x , %s \n", tTempAudioType, pTempAudioType[tTempAudioType-4]);
							else
								printf("Damon ==> Error not support this audio codec type !\n");

								if(Szbh_EsPlayerSetAcodecType(tTempAudioType)!=0)
									pTempStream->mType=0xff;
							}else if(pTempStream->mContentType==content_type_video)
							{
								int tTempVideoType=-1;
								int tTempCodecType=XTSGetStreamCodecType(pTempStream->mType);
								if(tTempCodecType==codec_type_mpeg2_video)
									tTempVideoType=EVideoCodec_MPEG2;
								else if(tTempCodecType==codec_type_mpeg4_video)
									tTempVideoType=EVideoCodec_MPEG4;
								else if(tTempCodecType==codec_type_h264_video)
									tTempVideoType=EVideoCodec_H264;
								else
									tTempVideoType=ECodecType_Max;

							const char *pTempVideoType[4]={"h263", "h264", "mpeg2", "mpeg4"};
							if(tTempVideoType!=ECodecType_Max)
								printf("Damon ==> video codec type : 0x%x , %s \n", tTempVideoType, pTempVideoType[tTempVideoType]);
							else
								printf("Damon ==> Error not support this video codec type !\n");
							
								if(Szbh_EsPlayerSetVcodecType(tTempVideoType)!=0)
									pTempStream->mType=0xff;
							}		

							printf("Damon ==> change codec type end !\n");
						}else
						{
							if(pTempStream && pTempStream->mType==tTempType)
							{
							//	printf("Damon ==> chamge stream id : %d \n", pTempStream->mPid);
								pTempStream->mPid=tTempPid;
							}
						//	printf("Damon ==> stream type : 0x%x 0x%x \n", pTempStream->mType, tTempType);
						}
					}

					if(tTempL>0)
					{
						return -19;
					}
				}else
				{
					printf("Damon ==> not check pmt table  !\n");
				}
			}else if((pTempVideoStream!=NULL && pTempVideoStream->mPid==tTempPid)
				||(pTempAudioStream!=NULL && pTempAudioStream->mPid==tTempPid))
			{

				XTsStream *pTempStream=NULL;
				if(pTempVideoStream->mPid==tTempPid)
					pTempStream=pTempVideoStream;
				else if(pTempAudioStream->mPid==tTempPid)
					pTempStream=pTempAudioStream;
				else
				{
					printf("Damon ==> not found stream id !\n");
					return -100;
				}

		//	printf("Damon ==> start indicator = %d \n", tTempPayloadUnitStartIndicator);
			
				// PES (Packetized Elementary Stream)
				if(tTempPayloadUnitStartIndicator)
				{
					// PES header
					if(tTempDataLen<6)
					{
						return -20;
					}

					static const unsigned char tTempStartCodePrefix[]={0x00,0x00,0x01};
					if(memcmp(pTempData, tTempStartCodePrefix,sizeof(tTempStartCodePrefix)))
					{
						return -21;
					}
					
					u_int8_t tTempStreamId=pTempData[3];
					u_int16_t tTempL=__ntohs(*((unsigned short *)(pTempData+4)));

					pTempData +=6;
					tTempDataLen -=6;

			//	printf("Damon ==> stream id = 0x%x \n", tTempStreamId);
					if((tTempStreamId>=0xbd && tTempStreamId<=0xbf) || (tTempStreamId>=0xc0 && tTempStreamId<=0xdf) || (tTempStreamId>=0xe0 && tTempStreamId<=0xef)  || (tTempStreamId>=0xfa && tTempStreamId<=0xfe))
					{
						// PES header extension
						if(tTempDataLen<3)
						{
							return -22;
						}

						unsigned char tTempBitmap=pTempData[1];
						unsigned char tTempHLen=pTempData[2]+3;

						if(tTempDataLen<tTempHLen)
						{
							return -23;
						}

						if(tTempL>0)
							tTempL -= tTempHLen;

				//	printf("Damon ==> bitmap : 0x%x \n", tTempBitmap);
						switch(tTempBitmap&0xc0)
						{
							case 0x80:			// PTS only
							{
								if(tTempHLen>=8)
								{
									u_int64_t tTempPts=XTsDecodePts(pTempData+3);

									if(pTempStream->mDts>0 && tTempPts>pTempStream->mDts)
										pTempStream->mFrameRate=tTempPts-pTempStream->mDts;

									pTempStream->mDts=tTempPts;

									if(tTempPts>pTempStream->mLastPts)
										pTempStream->mLastPts=tTempPts;

									if(!pTempStream->mFirstPts && pTempStream->mFrameNum==(pTempStream->mContentType==content_type_video?1:0))
										pTempStream->mFirstPts=tTempPts;

									pTempStream->mCurPts=tTempPts;
								}
							}
							break;
							case 0xc0:			// PTS,DTS
							{
								if(tTempHLen>=13)
								{
									u_int64_t tTempPts=XTsDecodePts(pTempData+3);
									u_int64_t tTempDts=XTsDecodePts(pTempData+8);

									if(pTempStream->mDts>0 && tTempDts>pTempStream->mDts)
										pTempStream->mFrameRate=tTempDts-pTempStream->mDts;

									pTempStream->mDts=tTempDts;

									if(tTempPts>pTempStream->mLastPts)
										pTempStream->mLastPts=tTempPts;

									if(!pTempStream->mFirstPts && pTempStream->mFrameNum==(pTempStream->mContentType==content_type_video?1:0))
										pTempStream->mFirstPts=tTempDts;

									pTempStream->mCurPts=tTempPts;

				//					printf("Damon ==> type = %d , %lld, %lld \n", pTempStream->mType, tTempDts, tTempPts);
								}
							}
							break;
						}
				//	printf("Damon ==> type : %d, cur pts : %lld  %d \n", pTempStream->mType, (int64_t)pTempVideoStream->mCurPts, (int)(pTempVideoStream->mCurPts/90));

						pTempData += tTempHLen;
						tTempDataLen -= tTempHLen;

						pTempStream->mStreamId=tTempStreamId;

						pTempStream->mFrameNum++;

					/*	if(fast_parse && s.content_type==stream_type::video && s.frame_num>max_fast_parse_frames)
						fast_parse_done=true;*/

					}else
					{
						pTempStream->mStreamId=0;
					}
				}

				if(pTempStream->mStreamId>0)
				{

					// play es data 
					if(tTempDataLen>0)
					{
			//		printf("Damon ==> data len : %d \n", tTempDataLen);
						if(tTempPayloadUnitStartIndicator || (pTempStream->mOffset+tTempDataLen)>dXFrameBufLen)
						{
					//	printf("Damon ==> put es data : %d %d \n", tTempPayloadUnitStartIndicator, pTempStream->mOffset);
							if(pTempStream->mOffset>0)
							{
								char *pTempTsData=NULL;
								int tTempTsDataLen=0;
								int tTempRet=0;
								if(pTempStream->mContentType==content_type_video)
								{
									tTempRet=Szbh_EsPlayerGetBuffer(true, &pTempTsData, &tTempTsDataLen, pTempStream->mOffset);
									if(tTempTsDataLen!=pTempStream->mOffset)
										printf("Damon ==> video stream get buf failed : %d %d \n", tTempTsDataLen, pTempStream->mOffset);
									if(tTempRet==0)
									{
										memcpy(pTempTsData, pTempStream->pData, tTempTsDataLen);
										unsigned int tTempVideoPts=(unsigned int)(pTempStream->mCurPts/90);
								//	printf("Damon ==>  video pts : %llu, %u \n", pTempStream->mCurPts, tTempVideoPts);
										tTempRet=Szbh_EsPlayerPutBuffer(true, tTempTsDataLen, tTempVideoPts);
										if(tTempRet!=0)
											printf("Damon ==> es player put video buf failed !\n");
									}else
									{
										printf("Damon ==> es player get video buf failed !\n");
										usleep(1000*30);
									}
							
								}else if(pTempStream->mContentType==content_type_audio)
								{
									tTempRet=Szbh_EsPlayerGetBuffer(false, &pTempTsData, &tTempTsDataLen, pTempStream->mOffset);
									if(tTempTsDataLen!=pTempStream->mOffset)
										printf("Damon ==> video stream get buf failed : %d %d \n", tTempTsDataLen, pTempStream->mOffset);
									if(tTempRet==0)
									{
										memcpy(pTempTsData, pTempStream->pData, tTempTsDataLen);
										unsigned int tTempAudioPts=(unsigned int)(pTempStream->mCurPts/90);
										tTempRet=Szbh_EsPlayerPutBuffer(false, tTempTsDataLen, tTempAudioPts);
										if(tTempRet!=0)
											printf("Damon ==> es player put audio buf failed !\n");
									}else
									{
										printf("Damon ==> es player get audio buf failed !\n");
										usleep(1000*30);
									}							
								}
							}

					//		printf("Damon ==> put end !\n");
						//	sleep(1);

							pTempStream->mOffset=0;
						}

						if(pTempStream->mOffset+tTempDataLen<dXFrameBufLen)
						{
							memcpy(pTempStream->pData+pTempStream->mOffset, pTempData, tTempDataLen);
							pTempStream->mOffset += tTempDataLen;
						}
					}
				
				/*	int tTempFd=pTempStream->mFile;
					if(tTempFd<0)
					{
						if(pTempStream->mContentType==audio)
						{
							tTempFd=open("./audio.es", O_CREAT | O_RDWR | O_TRUNC, 0777);						
						}else if(pTempStream->mContentType==video)
						{
							tTempFd=open("./video.es", O_CREAT | O_RDWR | O_TRUNC, 0777);
						}
						pTempStream->mFile=tTempFd;
					}
					
					if(tTempFd>0)
					{
						int tTempWriteLen=write(tTempFd, pTempData, tTempDataLen);
						printf("Damon ==> write len : %d  %d \n", tTempWriteLen, tTempDataLen);
					}*/
				}
			}else
			{
			//	printf("Damon ==> error data pid = 0x%x  !\n", tTempPid);
			}
		}
	}

	return 0;
}

static int XTsDemuxSocketInit(void)
{

#define dXTsStreamGroupIp	"225.1.1.40"
#define dXTsStreamGroupPort	4000

	int tTempSockFd;
	struct sockaddr_in tTempAddr;
	struct ip_mreq tTempIpmr;

	tTempSockFd=socket(AF_INET, SOCK_DGRAM, 0);
	if(tTempSockFd<0)
	{
		printf("Damon ==> [%s][%d] : Error create socket failed \n", __FUNCTION__, __LINE__);		
		close(tTempSockFd);
		return -1;
	}

	int tTempVal=1;
	if(setsockopt(tTempSockFd, SOL_SOCKET, SO_REUSEADDR, &tTempVal, sizeof(tTempVal))<0)
	{
	
		close(tTempSockFd);
		printf("Damon ==> [%s][%d] : Error setsocketopt failed \n", __FUNCTION__, __LINE__);
	}


	memset((void *)&tTempIpmr, 0, sizeof(tTempIpmr));
	tTempIpmr.imr_multiaddr.s_addr=inet_addr(dXTsStreamGroupIp);
	tTempIpmr.imr_interface.s_addr=htonl(INADDR_ANY);
	if(setsockopt(tTempSockFd, IPPROTO_IP, IP_ADD_MEMBERSHIP, (char *)&tTempIpmr, sizeof(tTempIpmr))<0)
	{
		printf("Damon ==> [%s][%d] : Error setsockopt failed errno=%d \n", __FUNCTION__, __LINE__, errno);
		close(tTempSockFd);
		return -2;
	}

	int tTempOptVal = 1*1024*1024;
	socklen_t tTempOptLen = sizeof(tTempOptVal);
	if(setsockopt(tTempSockFd, SOL_SOCKET, SO_RCVBUF, (char *)&tTempOptVal, tTempOptLen)<0)
	{
	
		close(tTempSockFd);
		printf("Damon ==>  [%s][%d] : Error setsockopt failed !", __FUNCTION__, __LINE__);
	}

	memset((void *)&tTempAddr, 0, sizeof(tTempAddr));
	tTempAddr.sin_family=AF_INET;
	tTempAddr.sin_addr.s_addr=inet_addr(dXTsStreamGroupIp); //htonl(INADDR_ANY);
	tTempAddr.sin_port=htons(dXTsStreamGroupPort);
//	inet_pton(AF_INET, dXTimsGroupIp, &tTempAddr.sin_addr);

	if(bind(tTempSockFd, (struct sockaddr *)&tTempAddr, sizeof(tTempAddr))<0)
	{
		printf("Damon ==> [%s][%d] : Error sock bind failed \n", __FUNCTION__, __LINE__);
		close(tTempSockFd);
		return -3;
	}

	return tTempSockFd;
}


void *XTsDemuxProc(void *pInputArg)
{

printf("\n\nDamon ==> ts demux proc ..... \n\n");

	if(tXTsDemuxInit<=0)
	{
		printf("Damon ==> ts demux not init !\n");
		return NULL;
	}

	int tTempSockFd= -1;
	int tTempSockFdCnt = 0;

	if((tTempSockFd < 0) &&(tTempSockFdCnt < 5))
	{
		tTempSockFd = XTsDemuxSocketInit();
		tTempSockFdCnt++;

	}
	else
	{
			log_save(dXLogLevel_Error, "XTsDemuxSocketInit failed [%s][%d]\n", __FUNCTION__, __LINE__);
			exit(-1);
	}



	int tTempRet=0;
	struct sockaddr_in tTempSenderAddr;
	socklen_t tTempAddrLen=sizeof(tTempSenderAddr);
	int tTempReadLen=0;
	int tTempRecvFirstFrame=0;

	printf("\n\nDamon ==> start read ts stream data ...... \n\n");

//	int tTempFd=open("../test_data/10001.ts", O_RDONLY);

//	int tTempCount = 0;


	while(1)
	{
//	printf("Damon ==> start read data \n");
		tTempReadLen = recvfrom(tTempSockFd, pXTsDataBuf, 188*10, 0, (struct sockaddr *)&tTempSenderAddr, &tTempAddrLen);
//	printf("Damon ==> read ts len : %d \n", tTempReadLen);
//		tTempReadLen=read(tTempFd, pXTsDataBuf, 188);
//	printf("Damon ==> read len : %d \n", tTempReadLen);

/*		tTempCount++;
		if(tTempCount%50==0)
			printf("Damon ==> recv len : %d \n", tTempReadLen);*/

		if(tTempReadLen>0)
		{
			int tTempDataCount=tTempReadLen/188;
			if(tTempReadLen%188!=0)
			{
				printf("Damon ==> recv ts data is error : %d \n", tTempReadLen);
				continue;
			}


			if(tTempRecvFirstFrame==0)
			{
				// set player volume
				char pTempString[12]={0};
				if(XSectionGetValue(dXSetupConfigFile, dXSetupDispSectionName, dXSetupVolumeKeyName, pTempString, sizeof(pTempString)-1)==0)
				{
					int tTempVolume=atoi(pTempString);
					if(tTempVolume>=0 && tTempVolume<=100)
						Szbh_EsPlayerSetVolume(tTempVolume);
				}
				
				Szbh_GpioWriteData(dXGpioNumMute, 0);
				
				tTempRecvFirstFrame=1;
			}

		
			// check screen save
			XCheckScreenSave();


		#ifdef dXUseTsVideoCheck
			// add check video frame 
		//	if(tTempCheck==1)
			{
				static u32 tTempPrevTick=0;
				u32 tTempCurTick=0;
				if(XGetTick(&tTempCurTick)==0)
				{
			//	printf("Damon ==> recv ts data len : %d %d \n", tTempReadLen, tTempCurTick);
					static u32 tTempTestTick=0;
					if(tTempTestTick!=tTempCurTick)
					{
						if(tTempCurTick-tTempTestTick>500)
							printf("Ts recv tick timeout : %u - %d \n", tTempCurTick, tTempCurTick-tTempTestTick);
						tTempTestTick=tTempCurTick;
					}
			
					if(tTempCurTick-tTempPrevTick>15*1000)
					{
						tXEsVideoCheckFlag=1;
					
						tTempPrevTick=tTempCurTick;
					}
				}
			}
		#endif
			

			int i=0;
			for(i=0; i<tTempDataCount; i++)
			{
				tTempRet=XTsDemuxPacket(pXTsDataBuf+i*188, 188);
				if(tTempRet<0)
				{
					printf("Damon ==> ts packet demux failed !\n");
				}
			}
		}
	}

	return NULL;
}


//=============== check video play ===============
int XEsPlayerCheckVideoPlay(X2DRect tInputRect)
{
	static u8 *pTempCurData=NULL;
	static u8 *pTempPrevData=NULL;
	static int tTempPrevOutLen=0;
	static int tTempRgbDataLen=0;
	int tTempCurOutLen=0;
//	static u32 tTempPrevTick=0;
	u32 tTempCurTick;


	if(tInputRect.width<=0 || tInputRect.height<=0)
	{
		printf("Damon ==> input param valid ! \n");
		return -1;
	}


	if(XGetTick(&tTempCurTick)==0)
	{
		if(tTempCurTick>tXEsPlayerCheckPrevTick)
		{
			if(tTempCurTick-tXEsPlayerCheckPrevTick>=2*1000*60)
			{
				printf("Damon ==> system time have been adjuest = %u %u \n", tTempCurTick, tXEsPlayerCheckPrevTick);
				tXEsPlayerCheckPrevTick=tTempCurTick;
				return 0;
			}
		}else
		{
			tTempCurTick=tXEsPlayerCheckPrevTick;
			return 0;
		}
	
	
		if(pTempCurData==NULL)
		{
			tTempRgbDataLen=1920*1080*3+54+10;
			pTempCurData=(u8 *)malloc(tTempRgbDataLen);
			if(pTempCurData==NULL)
			{
				printf("Damon ==> not enought memory !\n");
				return -2;
			}
		}

		if(pTempPrevData==NULL)
		{
			pTempPrevData=(u8 *)malloc(tTempRgbDataLen);
			if(pTempPrevData==NULL)
			{
				printf("Damon ==> not enought memory !\n");
				return -3;
			}
		}


		if(Szbh_EsPlayerCapture(pTempCurData, tTempRgbDataLen, &tTempCurOutLen)==0)
		{
		//	printf("Damon ==> capture :  %d %d %d \n", tTempRgbDataLen, tTempPrevOutLen, tTempCurOutLen);

			if(tTempPrevOutLen==0)
			{
				memcpy(pTempPrevData, pTempCurData, tTempCurOutLen);
				tTempPrevOutLen=tTempCurOutLen;

			//	tTempPrevTick=tTempCurTick;
				if(tXEsPlayerCheckPrevTick<tTempCurTick)
					tXEsPlayerCheckPrevTick=tTempCurTick;
				return 0;
			}

			if(tTempCurOutLen==tTempPrevOutLen)
			{

				int i=0, tTempSameCount=0;
				for(i=0; i<tTempCurOutLen; i++)
				{
					if(pTempPrevData[i]==pTempCurData[i])
						tTempSameCount++;
				}

				if(tTempSameCount==tTempCurOutLen)
				{
				printf("Damon ==> play tick : %u %u \n", tTempCurTick, tXEsPlayerCheckPrevTick);
					if((tTempCurTick>tXEsPlayerCheckPrevTick) && ((tTempCurTick-tXEsPlayerCheckPrevTick)>1*60*1000))
					{
						printf("Damon ==> video frame not change !\n");
						return 1;
					}
				}else
				{
					memcpy(pTempPrevData, pTempCurData, tTempCurOutLen);
				//	tTempPrevTick=tTempCurTick;
					tXEsPlayerCheckPrevTick=tTempCurTick;
				}
			}else
			{
				memcpy(pTempPrevData, pTempCurData, tTempCurOutLen);
				tTempPrevOutLen=tTempCurOutLen;
				
			//	tTempPrevTick=tTempCurTick;
				tXEsPlayerCheckPrevTick=tTempCurTick;
			}
		}
	}

	return 0;
}

void *XEsPlayerCheckThread(void *pInputArg)
{
	while(1)
	{
		if(tXEsVideoCheckFlag)
		{
			X2DRect tTempVideoRect={0, 0, 0, 0};
			XConfigSection *pTempVideoSection=XConfigGetSection("video");
			if(pTempVideoSection)
			{
				tTempVideoRect.x=pTempVideoSection->mXPos;
				tTempVideoRect.y=pTempVideoSection->mYPos;
				tTempVideoRect.width=pTempVideoSection->mWidth;
				tTempVideoRect.height=pTempVideoSection->mHeight;

				if(XEsPlayerCheckVideoPlay(tTempVideoRect)==1)
				{
					printf("Damon ==> reboot system !\n");
					sync();sleep(1);
					system("reboot");
				}
			}

			tXEsVideoCheckFlag=0;
		}

		sleep(1);
	}

	return NULL;
}
//=========================================


int XTsDemuxInit(void)
{
	int i=0;
	for(i=0; i<dXStream_Max; i++)
	{
		memset(pXTsStreams+i, 0, sizeof(XTsStream));
		pXTsStreams[i].mFile=-1;
		pXTsStreams[i].pData=malloc(dXFrameBufLen);
		pXTsStreams[i].mDataLen=0;
		pXTsStreams[i].mOffset=0;
	}

	memset(&tXTsTable, 0, sizeof(XTsTable));

	tXTsDemuxInit=1;

	pthread_t tTempHandle;
	pthread_create(&tTempHandle, NULL, XTsDemuxProc, NULL);


	if(tXEsVideoCheckThread==0)
	{
		pthread_create(&tXEsVideoCheckThread, NULL, XEsPlayerCheckThread, NULL);
		pthread_detach(tXEsVideoCheckThread);
	}

	return 0;
}

