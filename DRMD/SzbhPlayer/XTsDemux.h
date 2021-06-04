

#ifndef __X_TS_DEMUX_H__
#define __X_TS_DEMUX_H__

#define dXMaxTableLen	512

enum{
	dXStream_PmtIdx=0,
	dXStream_VideoIdx,
	dXStream_AudioIdx,
	dXStream_Max
};

enum
{
	content_type_unknown 			= 0,
	content_type_audio				= 1,
	content_type_video				= 2
};

enum
{
	codec_type_data				= 0,
	codec_type_mpeg2_video 		= 1,
	codec_type_mpeg4_video		= 2,
	codec_type_h264_video			= 3,
	codec_type_vc1_video			= 4,
	codec_type_ac3_audio			= 5,
	codec_type_aac_audio			= 6,
	codec_type_mpeg2_audio 		= 7,
	codec_type_lpcm_audio			= 8,
	codec_type_mp3_audio			=9
};


typedef struct __tagXTsTable
{
	unsigned char pData[dXMaxTableLen];
	int mLen;
	int mOffset;
}XTsTable;

typedef struct __tagStream
{
	int mFile;

	unsigned short mPid;
	unsigned short mProgram;
	unsigned char mId;
	unsigned char mType;

	unsigned char mStreamId;
	int mContentType;		// 1:audio  ,  2:video
	unsigned long long mDts;
	unsigned long long mFirstPts;
	unsigned long long mCurPts;
	unsigned long long mLastPts;
	unsigned long long mFrameRate;	
	unsigned long long mFrameNum;

	unsigned long long mStartTimeCode;
	unsigned int mNalCtx;
	unsigned long long mNalFrameNum;

	unsigned char *pData;
	int mDataLen;
	int mOffset;
}XTsStream;


int XTsDemuxInit(void);

#endif

