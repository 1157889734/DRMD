

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <pthread.h>


#include "SzbhDefine.h"
#include "SzbhApi.h"
#include "XVehiclePlayerDefine.h"
#include "XSectionFile.h"
#include "XTsFfmpegDemux.h"


#ifdef dXUseFfmpegTsDemux
#include "libavformat/avformat.h"
#include "libavcodec/avcodec.h"


#define dXTsStreamSource		"udp://225.1.1.40:4000"

static AVFormatContext *pXFormatContext=NULL;

//static AVFormatContext *pXFormatCheckContext=NULL;


typedef struct __tagVideoType{
	int mStreams;
	int pCodecId[4];
}XVideoType;

static XVideoType tXVideoCheckType;

static pthread_mutex_t tXStreamMutex;

extern int XCheckScreenSave(void);

static int XTsFfmpegInit(void)
{
	if(pXFormatContext==NULL)
	{
		pXFormatContext=avformat_alloc_context();
		if(pXFormatContext==NULL)
		{
			printf("Damon ==> avformat alloc context failed  !\n");
			return -1;
		}
	}

	if(0!=avformat_open_input(&pXFormatContext, dXTsStreamSource, NULL, NULL))
	{
		printf("Damon ==> avformat open input failed  !\n");
		return -2;
	}

	if(avformat_find_stream_info(pXFormatContext, NULL)<0)
	{
		printf("Damon ==> avformat find stream failed !\n");
		return -3;
	}

	return 0;
}

static void XTsFfmpegClose()
{
	if(pXFormatContext)
	{
		avformat_close_input(&pXFormatContext);
	}
}

void *XTsFfmpegThreadProc(void *pInputArg)
{
	int tTempFfmpegInifFlag=0;
	int tTempRecvFirstFrame=0;

	 int tTempAudioFrameCnt=0, tTempVideoFrameCnt=0;

	while(1)
	{
		if(tTempFfmpegInifFlag==0)
		{
			if(XTsFfmpegInit()==0)
			{
				tTempFfmpegInifFlag=1;

				tTempAudioFrameCnt=0;
				tTempVideoFrameCnt=0;
			}
		}else
		{
			int tTempRet=0;
			AVPacket tTempPacket;
			av_init_packet(&tTempPacket);
			tTempPacket.data = NULL;
			tTempPacket.size = 0;
			
			AVStream *pTempStream=NULL;
			
			int tTempPrevVidId=-1, tTempPrevAudId=-1;
			
			while((tTempRet=av_read_frame(pXFormatContext, &tTempPacket))>=0)
			{
		//	printf("Damon ==> read frame : %d %d %lld %lld \n", pXFormatContext->nb_streams, tTempPacket.stream_index, tTempPacket.pts, tTempPacket.dts);
			
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
					
					Szbh_GpioWriteData(/*6*8+3*/dXGpioNumMute, 0);
					
					tTempRecvFirstFrame=1;
				}
			
				// check screen save
				XCheckScreenSave();

				pTempStream=pXFormatContext->streams[tTempPacket.stream_index];
				if(pTempStream!=NULL)
				{
				#if 0
					pthread_mutex_lock(&tXStreamMutex);
					if(tXVideoCheckType.mStreams>0)
					{
						if(tXVideoCheckType.mStreams!=pXFormatContext->nb_streams
							|| tXVideoCheckType.pCodecId[tTempPacket.stream_index]!=pTempStream->codecpar->codec_id)
							break;
					}
					pthread_mutex_unlock(&tXStreamMutex);
				#endif

				
					if(pTempStream->codecpar->codec_type==AVMEDIA_TYPE_VIDEO)
					{
			/*	printf("Damon ==> video pts : %lld \n", tTempPacket.pts);
			//	int tTempSecond=tTempPacket.pts*av_q2d(pTempStream->time_base);
				printf("Damon ==> video time : %d %d - %lld \n", pTempStream->time_base.num, 
					pTempStream->time_base.den, tTempPacket.pts/90);*/

						tTempVideoFrameCnt++;
						tTempAudioFrameCnt=0;
						if(tTempVideoFrameCnt>=20 && pXFormatContext->nb_streams>=2)
							break;

					
						if(tTempPrevVidId != pTempStream->codecpar->codec_id)
						{
						printf("Damon ==> change video codec type : 0x%x 0x%x \n", tTempPrevVidId, pTempStream->codecpar->codec_id);
							tTempPrevVidId=pTempStream->codecpar->codec_id;
							if(tTempPrevVidId==AV_CODEC_ID_H264)
								Szbh_EsPlayerSetVcodecType(EVideoCodec_H264);
							else if(tTempPrevVidId==AV_CODEC_ID_H263)
								Szbh_EsPlayerSetVcodecType(EVideoCodec_H263);
							else if(tTempPrevVidId==AV_CODEC_ID_MPEG2VIDEO)
								Szbh_EsPlayerSetVcodecType(EVideoCodec_MPEG2);
							else if(tTempPrevVidId==AV_CODEC_ID_MPEG4)
								Szbh_EsPlayerSetVcodecType(EVideoCodec_MPEG4);
							else
							{
								printf("Damon ==> not support video codec type : 0x%x \n", tTempPrevVidId);
								av_packet_unref(&tTempPacket);
								continue;
							}
						}
						
						char *pTempData=NULL;
						int tTempDataLen=0;
						if(tTempPacket.size>0)
						{
						
							if(Szbh_EsPlayerGetBuffer(true, &pTempData, &tTempDataLen, tTempPacket.size)==0)
							{
								memcpy(pTempData, tTempPacket.data, tTempDataLen);
								if(Szbh_EsPlayerPutBuffer(true, tTempDataLen, tTempPacket.pts/90)!=0)
								{
									printf("Damon ==> put esplayer video buf failed !\n");
								}
							}else
							{
								printf("Damon ==> get esplay video buf failed : [%d] \n", tTempPacket.size);
							}
						}
						
					}else if(pTempStream->codecpar->codec_type==AVMEDIA_TYPE_AUDIO)
					{
				/*	printf("Damon ==> audio pts : %lld \n", tTempPacket.pts);
					int tTempSecond=tTempPacket.pts*av_q2d(pTempStream->time_base);
					printf("Damon ==> audio time : %d %d - %d \n", pTempStream->time_base.num,
							pTempStream->time_base.den, tTempSecond);*/

						tTempAudioFrameCnt++;
						tTempVideoFrameCnt=0;
						if(tTempAudioFrameCnt>=20 && pXFormatContext->nb_streams>=2)
							break;
						
					
						if(tTempPrevAudId != pTempStream->codecpar->codec_id)
						{
						printf("Damon ==> change audio codec type : 0x%x 0x%x \n", tTempPrevAudId, pTempStream->codecpar->codec_id);
							tTempPrevAudId=pTempStream->codecpar->codec_id;

							if(tTempPrevAudId==AV_CODEC_ID_MP3 || tTempPrevAudId==AV_CODEC_ID_MP2)
								Szbh_EsPlayerSetAcodecType(EAudioCodec_MP3);
							else if(tTempPrevAudId==AV_CODEC_ID_AAC)
								Szbh_EsPlayerSetAcodecType(EAudioCodec_AAC);
							else if(tTempPrevAudId==AV_CODEC_ID_AC3)
								Szbh_EsPlayerSetAcodecType(EAudioCodec_AC3);
							else
							{
								printf("Damon ==> not support audio codec type : 0x%x \n", tTempPrevVidId);
								av_packet_unref(&tTempPacket);
								continue;
							}						
						}

						char *pTempData=NULL;
						int tTempDataLen=0;
						if(tTempPacket.size>0)
						{
							if(Szbh_EsPlayerGetBuffer(false, &pTempData, &tTempDataLen, tTempPacket.size)==0)
							{
								memcpy(pTempData, tTempPacket.data, tTempDataLen);
								if(Szbh_EsPlayerPutBuffer(false, tTempDataLen, tTempPacket.pts/90)!=0)
								{
									printf("Damon ==> put esplayer audio buf failed !\n");
								}
							}else
							{
								printf("Damon ==> get esplay audio buf failed : [%d] \n", tTempPacket.size);
							}
						}

					}

				//	printf("Damon ==> codec id : [0x%x] [0x%x] - [0x%x] \n", tTempPrevVidId, tTempPrevAudId, pTempStream->codecpar->codec_id);
				}

				av_packet_unref(&tTempPacket);
			}

			printf("Damon ==> reset ffmpeg !\n");
			av_packet_unref(&tTempPacket);
			XTsFfmpegClose();
			tTempFfmpegInifFlag=0;
		}
	}

	return NULL;
}


int XTsFfmpegDemuxInit(void)
{
	avformat_network_init();

	memset(&tXVideoCheckType, 0, sizeof(tXVideoCheckType));

	pthread_mutex_init(&tXStreamMutex, NULL);

	pthread_t tTempHandle;
	pthread_create(&tTempHandle, NULL, XTsFfmpegThreadProc, NULL);


	return 0;
}
#endif

