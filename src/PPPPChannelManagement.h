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

	//////////���Ի��ǽ���Ƶ����Ź����ĵط�/////////////
	OPENXL_STREAM * 	hOSL;

private:
    PPPP_CHANNEL m_PPPPChannel[MAX_PPPP_CHANNEL_NUM];
	COMMO_LOCK PPPPChannelLock;
	COMMO_LOCK PPPPCommandLock;
};

#endif
