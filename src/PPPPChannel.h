

#ifndef _PPPP_CHANNEL_H_
#define _PPPP_CHANNEL_H_

#include "CircleBuf.h"

#ifdef PLATFORM_ANDROID
#include <jni.h>
#include <sys/system_properties.h>
#include <sys/prctl.h>
#endif

#include <sys/resource.h>

#include <unistd.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <signal.h>

#include "H264Decoder.h"

#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/avutil.h>
#include <libswscale/swscale.h>
#include <libavutil/time.h>

#include "ffmpeg_mp4.h"

#include "openxl_io.h"
#include "audio_codec.h"
#include "audiodatalist.h"

#include "object_jni.h"
#ifndef TUTK_PPPP
#include "iLnkRingBuf.h"
#include "iLnkDefine.h"
#endif

#ifdef PLATFORM_ANDROID
#define SET_THREAD_NAME(name) \
{\
    prctl(PR_SET_NAME, (unsigned long)name, 0,0,0); \
    pid_t tid;\
    tid = gettid();\
}
#else
#define SET_THREAD_NAME(name)
#endif

#define MAX_PATHNAME_LEN 	256

//msgtype
#define MSG_NOTIFY_TYPE_PPPP_STATUS 0   	/* ¡¨Ω”◊¥Ã¨ */
#define MSG_NOTIFY_TYPE_PPPP_MODE 1   		/* ¡¨Ω”ƒ£ Ω */
#define MSG_NOTIFY_TYPE_STREAM_TYPE 2

#define P2P_CHANNEL_CMMND   0
#define P2P_CHANNEL_VIDEO    1
#define P2P_CHANNEL_AUDIO    2
#define P2P_CHANNEL_FILE     3

//pppp status
#define PPPP_STATUS_CONNECTING 0 			/* ’˝‘⁄¡¨Ω” */
#define PPPP_STATUS_INITIALING 1 			/* ’˝‘⁄≥ı ºªØ */
#define PPPP_STATUS_ON_LINE 2 				/* …Ë±∏‘⁄œﬂ */
#define PPPP_STATUS_CONNECT_FAILED 3		/* ¡¨Ω” ß∞‹ */
#define PPPP_STATUS_DISCONNECT 4 			/* ¡¨Ω”∂œø™ */
#define PPPP_STATUS_INVALID_ID 5 			/* Œﬁ–ß…Ë±∏ */
#define PPPP_STATUS_DEVICE_NOT_ON_LINE 6	/* …Ë±∏¿Îœﬂ */
#define PPPP_STATUS_CONNECT_TIMEOUT 7 		/* ¡¨Ω”≥¨ ± */
#define PPPP_STATUS_INVALID_USER_PWD 8 		/* Œﬁ–ß’Àªß√‹¬Î */
#define PPPP_STATUS_DEVICE_SLEEP 9

#define PPPP_STATUS_NOT_LOGIN 11			/* Œ¥µ«¬º */
#define PPPP_STATUS_EXCEED_SESSION 13		/* √ª”–ø…”√µƒª·ª∞ */

#define COMMAND_BUFFER_SIZE 32*1024
//TUTK VIDEO INFO
typedef struct tag_AV_HEAD
{
    unsigned int   		startcode;	//  0xa815aa55
    unsigned char		type;		//  0->264 idr frame 1->264 p frame
    unsigned char  	    streamid;	
    unsigned short  	militime;	//  diff time
    unsigned int 		sectime;	//  diff time
    unsigned int    	frameno;	//  frameno
    unsigned int 		len;		//  data len
    unsigned char		version;	// version
    unsigned char		sessid;		//ssid
    unsigned char		other[2];
    unsigned char		other1[8];
	char 				d[0];
}AV_HEAD,*PAV_HEAD;

//YUNNI VIDEO INFO
#define AVF_STARTCODE 0xa815aa55
#define MAX_FRAME_LENGTH 209715200 //200 * 1024 * 1024 ; 200KB
#define MAX_AUDIO_DATA_LENGTH 2048
typedef struct tag_AV_HEAD_YUNNI
{
    unsigned int   		startcode;	//  0xa815aa55
    
    unsigned char		type;		//  0->264 idr frame 1->264 p frame
    unsigned char  	    streamid;	//ͨ�� 0->
    unsigned short  	militime;	//  diff time
    
    unsigned int 		sectime;	//  diff time
    unsigned int    	frameno;	//  frameno
    int 		len;		//  data len
    
    unsigned char		version;	// version
    unsigned char		sessid;		//ssid 
    unsigned char		audiocodec; //��Ƶ�����ʽ ��audio_codec.h _AUDIO_CODEC
    unsigned char		audioSamplerate;//������ 0->8000 1->16000 2->32000 
    
    unsigned char		audioBitrate;	//λ��    0->8000 1->16000 2->320000
    unsigned char		audioTrack;//ͨ�� 0->��, 1->˫
    unsigned char		other[2];
	
    unsigned char		other1[4];
	char 				d[0];
}AV_HEAD_YUNNI,*PAV_HEAD_YUNNI;

typedef struct fileParam{
    unsigned char	type;		//  0->264 idr frame 1->264 p frame
    unsigned char  	reserved[3];	
    unsigned int 	gettime;	//  diff time
	unsigned int size;
	unsigned int offset;
	char	filename[128];
	char	magic[4];
	FILE *fd;
	struct fileParam *next;
	struct fileParam *pres;
}st_fileParam;

//Command Channel head
#define CMD_START_CODE 0x0a01
typedef struct _CMD_CHANNEL_HEAD
{
    unsigned short startcode;
    unsigned short cmd;
    unsigned short len;
    unsigned short version;
	char 		   d[0];
}CMD_CHANNEL_HEAD, *PCMD_CHANNEL_HEAD;

typedef enum{
	STATUS_SESSION_START = 1,		
	STATUS_SESSION_CLOSE,		
	STATUS_SESSION_IDLE,
	STATUS_SESSION_DIED,
	STATUS_SESSION_START_PLAY,
	STATUS_SESSION_PLAYING,
	STATUS_SESSION_CLOSE_PLAY,
}SESSION_STATUS;

typedef double UINT64;

class CPPPPChannel
{
public:
    CPPPPChannel(
		char *DID,
		char *user,
		char *pwd,
		char *server
		);
    virtual ~CPPPPChannel();

    int  Start();
	void Close();
	int StartMediaChannel();
    int StartVideoChannel();
	int StartAudioChannel();
    int StartAlarmChannel();
    int StartIOCmdChannel();
	int StartIOSesChannel();
	
	int StartMediaStreams(
		const char * url,
		int audio_recv_codec,
		int audio_send_codec,
		int video_recv_codec
		);
	
	int CloseMediaStreams();

	int CloseWholeThreads();
	
	int SendAVAPIStartIOCtrl();
	int SendAVAPICloseIOCtrl();
	int SetSystemParams(int type, char *msg, int len);
	
    void MsgNotify(JNIEnv * hEnv,int MsgType, int Param);
	
#ifndef TUTK_PPPP
	int StartFileRecvThread();
	int SetSystemParams_yunni(int gwChannel,int cmdType,char *cmdContent,int cmdLen);
	void p2p_ConnectProc(JNIEnv *env);
	int ExtAckCmdHeaderBuild(exSendCmd_t *extCmdHead,unsigned char sessionHandle,short cmd,short len);
	int ExtCmdHeaderBuild(exSendCmd_t *extCmdHead,unsigned char gwChannel,short cmd,short len); 
	void ConnectUserCheckAcK(JNIEnv * env,char *pbuf,unsigned short len);
	void ProcessCommand_EX(JNIEnv * env,int gwChannel,int cmd, char *pbuf, int len);
	void ProcessCommand(JNIEnv * env,int gwChannel,int cmd, char *pbuf, int len);
#endif
	void AlarmNotifyDoorBell(JNIEnv * hEnv, char *did, char *type, char *time );

	int  PPPPClose();

public:
	JNIEnv *            m_JNIMainEnv;	// Java env
	
	// 
	CAudioDataList *	hAudioGetList;
	CAudioDataList *	hAudioPutList;
	
	
	COMMO_LOCK			SessionStatusLock;
	int					SessionStatus;

	char 				szURL[256];		//

	char 				szTicket[4];//�û���֤Ʊ��

	char 				szDID[64];		//
    char 				szUsr[64];		//
    char 				szPwd[64]; 		//
    char 				szServer[1024];	//

	int					avstremChannel; // 
	int					speakerChannel;	//
	int					sessionID;		//   YUNNI P2P SESSIONID
	int					avIdx;			//
	int					spIdx;			//
	unsigned int		deviceType;		//
    int                 deviceStandby;

	int					SID; 			// P2P session id
	
    int 				mediaEnabled;	// enable recv thread get media stream data.
	int					voiceEnabled;	// enable voice
	int					audioEnabled;	// enable audio
	int					speakEnabled;	// for vad detect we talk to device,set this to zero.

	int					mediaLinking;
	int 				videoPlaying;	
	int 				audioPlaying;	
	int					audioSending;
	int 				iocmdRecving;	
	int					iocmdSending;
	int 				fileRecving;	//��ȡSD�����ݿ�����ʶ
	
	int					AudioRecvFormat;
	int					VideoRecvFormat;
	int					AudioSendFormat;
	int					AudioEchoCancellationEnable;

	pthread_t			mediaCoreThread;

	pthread_t 			iocmdRecvThread;
	pthread_t			iocmdSendThread;

	pthread_t 			videoPlayThread;
	pthread_t			videoRecvThread;

	pthread_t 			audioRecvThread;
	pthread_t 			audioSendThread;

	int					MW;				
	int					MH;				// 
	int					YUVSize;		//

	char 			* 	hVideoFrame;	//
	
	COMMO_LOCK			DisplayLock;	//
	COMMO_LOCK			SndplayLock;	//

	AVFormatContext * 	hAVFmtContext;
	AVOutputFormat  * 	hAVFmtOutput;
	AVStream		* 	hAudioStream;
	AVStream		* 	hVideoStream;
	AVCodecContext  * 	hAVCodecContext;

	OutputStream 		sVOs;
	OutputStream 		sAOs;

	char *				hAudioRecCaches;
	int					aLen;			// 


	long long		  	vCTS;			// µ±«∞ ±º‰¥¡
	long long		  	vLTS;			// …œ¥Œ ±º‰¥¡
	long long		 	vPTS;			// ø™ º ±º‰¥¡ - µ±«∞÷° ±º‰¥¡

	long long			aCTS;			// µ±«∞ ±º‰¥¡
	long long			aLTS;			// …œ¥Œ ±º‰¥¡
	long long		 	aPTS;			// ø™ º ±º‰¥¡ - µ±«∞÷° ±º‰¥¡
	
	long long		  	sSTS;			// ø™ º ±º‰¥¡

	long long			vIdx;			//  ”∆µÀ˜“˝
	long long			aIdx;			// “Ù∆µÀ˜“˝

	char			*	hRecordFile;

	COMMO_LOCK			AviDataLock;
	PROCESS_HANDLE		avProc;			// ¬ºœÒœﬂ≥Ãæ‰±˙
	char 				avExit;			// ¬ºœÒœﬂ≥ÃÕÀ≥ˆ±Í÷æ

	// for avi proc
	CCircleBuf *		hVideoBuffer;	//  ”∆µª∫≥Â«¯
	CCircleBuf *		hAudioBuffer;	// “Ù∆µ≤•∑≈ª∫≥Â«¯
	CCircleBuf *		hSoundBuffer;	// “Ù∆µ¬º÷∆ª∫≥Â«¯
	
	CCircleBuf *		hIOCmdBuffer;	// ÷∏¡Ó∑¢ÀÕª∫≥Â«¯

	CCircleBuf *		hFileBuffer;
	
	int StartRecorder(
		int 			W,				// øÌ∂»
		int 			H,				// ∏ﬂ∂»
		int 			FPS,			// ÷°¬ 
		char *			SavePath	
	);

	int WriteRecorder(
		const char * 	FrameData,
		int				FrameSize,
		int				FrameCode, 		// audio or video codec [0|1]
		int				FrameType,		// keyframe or not [0|1]
		long long		Timestamp
	);
	
	int CloseRecorder();
#ifndef TUTK_PPPP
//YUNNI P2P DEFINE
	///////////////Xq Session////////////////////
	int					sessionType;
	int					channelBufSize;
	
	int m_bPlayStreamOK;
	int realTimeMode;

	char FileDataPath[128];
	char FileDataName[128];
	char FileDataDir[128];
	
	st_fileParam *m_front;
	st_fileParam *m_rear;
	int m_nSize;
	int m_nNmb;
	int m_n;
	//CVDataList *m_pVideo_RecvBuf;
	
	pthread_mutex_t 	audioPlayThreadLock;
	pthread_mutex_t 	audioSendThreadLock;
	pthread_mutex_t 	audioRecvThreadLock;
	pthread_mutex_t 	cmdRecvThreadLock;
	pthread_mutex_t 	fileRecvThreadLock;
	pthread_mutex_t 	DataWriteThreadLock;
	pthread_mutex_t 	m_PPPPCloseMutex;

	//��Ƶ����
typedef enum tag_ENUM_VIDEO_MODE
{
	ENUM_VIDEO_MODE_MJPEG,
	ENUM_VIDEO_MODE_H264,
	ENUM_VIDEO_MODE_AUDIO
}ENUM_VIDEO_MODE;

typedef enum tag_ENUM_FRAME_TYPE
{
	ENUM_FRAME_TYPE_I = 0,
	ENUM_FRAME_TYPE_P = 1,
	ENUM_FRAME_TYPE_MJPEG = 3,
	ENUM_FRAME_TYPE_AUDIO = 6
}ENUM_FRAME_TYPE;
	

	int 				m_bPlayback;
	ENUM_VIDEO_MODE_YUNNI_P2P m_EnumVideoMode;
	pthread_mutex_t 	videoRecvThreadLock;
//	pthread_t			videoRecvThread;
	int 				videoRecving;	//�߳�ѭ����ʶ


	pthread_t 			fileRecvThread;
	
	pthread_mutex_t 	videoPlayThreadLock;//�߳���
//	pthread_t 			videoPlayThread;//�߳̾��
//	int 				videoPlaying;	//�߳�ѭ����ʶ
	int					videoPlayEnabled;	//����

	// for avi proc
	//CCircleBuf *		hVideoBuffer;	// ��Ƶ������
	//CCircleBuf *		hAudioBuffer;	// ��Ƶ���Ż�����
	xqRingBuffer_t	*	avRecvRingBuffer; //����Ƶѭ������
	xqRingBuffer_t	*	hAvSendRingBuffer;//ʹ�ù�������Ƶ���ͻ�����
	CCircleBuf *		hSoundPlayCircleBuffer;	// ��Ƶ¼�ƻ�����
	
	typedef struct
	{
		int channel;				// living stream channel
		unsigned char url[256]; 	// for video playback
	} SMsgAVIoctrlAVStream_YUNNI;
	
	typedef struct {
    	int     Magic;
		int		AppType;
		int		CgiLens;
		char	CgiData[0];
	}APP_CMD_HEAD_YUNNI;
#endif
};

#endif
