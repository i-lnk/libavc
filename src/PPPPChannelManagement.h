#ifndef _PPPP_CHANNEL_MANAGEMENT_H_
#define _PPPP_CHANNEL_MANAGEMENT_H_

#include "PPPPChannel.h"

#define MAX_DID_LENGTH 64
#define MAX_PPPP_CHANNEL_NUM 64

typedef struct _PPPP_CHANNEL
{
    char szDID[MAX_DID_LENGTH] ;
    CPPPPChannel *pPPPPChannel;
    int bValid;    
}PPPP_CHANNEL, *PPPPPCHANNEL;

class CPPPPChannelManagement
{
public:
    CPPPPChannelManagement();
    ~CPPPPChannelManagement();
    int Start(char *szDID, char *user, char *pwd,char *server);
    int Stop(char *szDID);
    void StopAll();

    int StartPPPPLivestream(
		char * szDID, 
		char * szURL,
		int audio_recv_codec,
		int audio_send_codec,
		int video_recv_codec
		);
	
    int ClosePPPPLivestream(char *szDID);
	
	int GetAudioStatus(char * szDID);
	int SetAudioStatus(char * szDID,int AudioStatus);

    int PPPPSetSystemParams(char *szDID, int type, char *msg, int len);
	int CmdExcute(char * szDID,int gwChannel,int cmdType,char * cmdContent,int cmdLen);
	int StartRecorderByDID(char * szDID,char * filepath);
	int CloseRecorderByDID(char * szDID);

	//////////尝试还是将音频处理放公共的地方/////////////
	OPENXL_STREAM * 	hOSL;
	CAudioDataList *	hAudioGetList;//手机采集录制的声音
	CAudioDataList *	hAudioPutList;//播放过缓存的参考的声音
	
	CCircleBuf *		hSoundBuffer;	//音频录制缓冲区

private:
    PPPP_CHANNEL m_PPPPChannel[MAX_PPPP_CHANNEL_NUM];
	COMMO_LOCK PPPPChannelLock;
	COMMO_LOCK PPPPCommandLock;
	
	COMMO_LOCK AudioLock;
		

};

#endif
