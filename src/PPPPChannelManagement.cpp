
#ifdef PLATFORM_ANDROID
#include <jni.h>
#endif

#include "utility.h"
#ifdef TUTK_PPPP
#include "IOTCAPIs.h"
#include "AVAPIs.h"
#include "AVFRAMEINFO.h"
#include "AVIOCTRLDEFs.h"
#else
#include "PPPP_API.h"
#endif

#include "PPPPChannelManagement.h"

static COMMO_LOCK PPPPChannelLock = PTHREAD_MUTEX_INITIALIZER;

CPPPPChannelManagement::CPPPPChannelManagement()
{
	unsigned int iotcVer;
	iotcVer=PPPP_GetAPIVersion();
	unsigned char *p1 = (unsigned char *)&iotcVer;
	char szIOVer[4];
	sprintf(szIOVer, "%d.%d.%d.%d", p1[3], p1[2], p1[1], p1[0]);
	Log3("PPPP_GetAPIVersion  [%s] ", szIOVer);
	
	Log3("CPPPPChannelManagement ------------------>1");
    memset(m_PPPPChannel, 0 ,sizeof(PPPP_CHANNEL)*MAX_PPPP_CHANNEL_NUM);
	//memset(&m_PPPPChannel, 0 ,sizeof(PPPP_CHANNEL));
	Log3("CPPPPChannelManagement ------------------>2");

	int err = 0;
	
	err = INT_LOCK( &PPPPChannelLock );
	if(err != 0){
		Log3("initialize lock error:[%d].",errno);
	}

	Log3("fucking thses lock cause start failed;[%p].",&PPPPChannelLock);
	Log3("fucking thses lock cause start failed;[%p].",&PPPPChannelLock);
	Log3("fucking thses lock cause start failed;[%p].",&PPPPChannelLock);

	GET_LOCK( &PPPPChannelLock );
	PUT_LOCK( &PPPPChannelLock );
	
    InitOpenXL();

	Log3("CPPPPChannelManagement ------------------>3");
}

CPPPPChannelManagement::~CPPPPChannelManagement()
{    
    StopAll();
	Log3("stopall done.");

	Log3("stop channel lock done.");
	DEL_LOCK( &PPPPChannelLock );
	Log3("CPPPPChannelManagement Done.");
	
}

int CPPPPChannelManagement::Start(char * szDID, char *user, char *pwd,char *server)
{
	if(szDID == NULL) return 0;
	
	GET_LOCK( &PPPPChannelLock );

	int r = 1;
    int i = 0;

    for(i = 0; i < MAX_PPPP_CHANNEL_NUM; i++)
    {
        if(strcmp(m_PPPPChannel[i].szDID, szDID) == 0)
        {
           	r = m_PPPPChannel[i].pPPPPChannel->Start(user, pwd, server);
            goto jumpout;
        }
    }

    for(i = 0; i < MAX_PPPP_CHANNEL_NUM; i++)
    {
        if(m_PPPPChannel[i].bValid == 0)
        {
            m_PPPPChannel[i].bValid = 1;

            strcpy(m_PPPPChannel[i].szDID, szDID);      
            m_PPPPChannel[i].pPPPPChannel = new CPPPPChannel(szDID, user, pwd, server);
            m_PPPPChannel[i].pPPPPChannel->Start(user, pwd, server);
			goto jumpout;
        }
    }

	r = 0;

jumpout:

	PUT_LOCK( &PPPPChannelLock );
    
    return r;
}

int CPPPPChannelManagement::Stop(char * szDID)
{
	if(szDID == NULL) return 0;

	Log3("stop get channel lock");
	GET_LOCK( &PPPPChannelLock );

    int i;
    for(i = 0; i < MAX_PPPP_CHANNEL_NUM; i++)
    {
        if(m_PPPPChannel[i].bValid == 1 && strcmp(m_PPPPChannel[i].szDID, szDID) == 0)
        {            
            memset(m_PPPPChannel[i].szDID, 0, sizeof(m_PPPPChannel[i].szDID));
            SAFE_DELETE(m_PPPPChannel[i].pPPPPChannel);       
            m_PPPPChannel[i].bValid = 0;

			Log3("stop put channel lock");
			PUT_LOCK( &PPPPChannelLock );

            return 1;
        }
    }

	Log3("stop get channel lock");
	PUT_LOCK( &PPPPChannelLock );
    
    return 0;
}

void CPPPPChannelManagement::StopAll(){

	Log3("stopall get channel lock");
    GET_LOCK( &PPPPChannelLock );

    int i;
    
    for(i = 0; i < MAX_PPPP_CHANNEL_NUM; i++)
    {
        if(m_PPPPChannel[i].bValid == 1)
        {
			Log3("close channel with sid:[%s] done.",m_PPPPChannel[i].szDID);
		
            memset(m_PPPPChannel[i].szDID, 0, sizeof(m_PPPPChannel[i].szDID));
            SAFE_DELETE(m_PPPPChannel[i].pPPPPChannel);           
            m_PPPPChannel[i].bValid = 0;

			Log3("close channel done.",m_PPPPChannel[i].szDID);
        }
    } 

	Log3("stopall put channel lock");
	PUT_LOCK( &PPPPChannelLock );
}

int CPPPPChannelManagement::StartPPPPLivestream(
	char * szDID, 
	char * szURL,
	int audio_recv_codec,
	int audio_send_codec,
	int video_recv_codec
){
  	if(szDID == NULL) return 0;

	int ret  = -1;

	Log3("StartPPPPLivestream get channel lock");
	GET_LOCK( &PPPPChannelLock );

    int i;
    for(i = 0; i < MAX_PPPP_CHANNEL_NUM; i++)
    {
        if(m_PPPPChannel[i].bValid == 1 && strcmp(m_PPPPChannel[i].szDID, szDID) == 0)
        {
        	Log3("start connection with did:[%s].",szDID);
           ret = m_PPPPChannel[i].pPPPPChannel->StartMediaStreams(
					szURL,
					8000,
					1,
					audio_recv_codec,
					audio_send_codec,
					video_recv_codec,
					0,
					0
					);
		   
			if(ret < 0){
				Log3("restart channel connection.");
			}
			
			break;
        }
    }

	Log3("StartPPPPLivestream put channel lock");
	PUT_LOCK( &PPPPChannelLock );
    
    return ret;
}

int CPPPPChannelManagement::ClosePPPPLivestream(char * szDID){

	if(szDID == NULL) return 0;

   	int ret  = -1;

//	Log3("close live stream get channel lock");

	GET_LOCK( &PPPPChannelLock );

    int i;
    for(i = 0; i < MAX_PPPP_CHANNEL_NUM; i++)
    {
        if(m_PPPPChannel[i].bValid == 1 && strcmp(m_PPPPChannel[i].szDID, szDID) == 0)
        {
        	Log3("channel manager close live stream by %s.",szDID);
           	ret = m_PPPPChannel[i].pPPPPChannel->CloseMediaStreams();
            break;
        }
    }  

//	Log3("close live stream put channel lock");

	PUT_LOCK( &PPPPChannelLock );
    
    return ret;
}

int CPPPPChannelManagement::GetAudioStatus(char * szDID){

	if(szDID == NULL) return 0;

	Log3("GetAudioStatus get channel lock");

	GET_LOCK( &PPPPChannelLock );

    int i;
    for(i = 0; i < MAX_PPPP_CHANNEL_NUM; i++)
    {
    	if(m_PPPPChannel[i].bValid == 1 && strcmp(m_PPPPChannel[i].szDID, szDID) == 0)
        {
			Log3("GET ----> AUDIO PLAY:[%d] AUDIO TALK:[%d].",
				m_PPPPChannel[i].pPPPPChannel->audioEnabled,
				m_PPPPChannel[i].pPPPPChannel->voiceEnabled
				);

			int val = (m_PPPPChannel[i].pPPPPChannel->audioEnabled | m_PPPPChannel[i].pPPPPChannel->voiceEnabled << 1) & 0x3;

			PUT_LOCK( &PPPPChannelLock );
		
        	return val;
    	}
    }

	Log3("GetAudioStatus put channel lock");

	PUT_LOCK( &PPPPChannelLock );

	return 0;
}

int CPPPPChannelManagement::SetAudioStatus(char * szDID,int AudioStatus){

	if(szDID == NULL) return 0;

	
	Log3("SetAudioStatus get channel lock");

	GET_LOCK( &PPPPChannelLock );

    int i;
    for(i = 0; i < MAX_PPPP_CHANNEL_NUM; i++)
    {
        if(m_PPPPChannel[i].bValid == 1 && strcmp(m_PPPPChannel[i].szDID, szDID) == 0)
        {
            m_PPPPChannel[i].pPPPPChannel->audioEnabled = (AudioStatus & 0x1);
			m_PPPPChannel[i].pPPPPChannel->voiceEnabled = (AudioStatus & 0x2) >> 1;

			Log3("SET ----> AUDIO PLAY:[%d] AUDIO TALK:[%d].",
				m_PPPPChannel[i].pPPPPChannel->audioEnabled,
				m_PPPPChannel[i].pPPPPChannel->voiceEnabled
				);
			
			Log3("SetAudioStatus put channel lock");

			PUT_LOCK( &PPPPChannelLock );
			
            return AudioStatus;
        }
    }  

	
	Log3("SetAudioStatus put channel lock");

	PUT_LOCK( &PPPPChannelLock );
   
    return 0;
}

int CPPPPChannelManagement::StartRecorderByDID(char * szDID,char * filepath){
	
    if(szDID == NULL) return 0;

	
	Log3("StartRecorderByDID get channel lock");

	GET_LOCK( &PPPPChannelLock );

    int i;
    for(i = 0; i < MAX_PPPP_CHANNEL_NUM; i++)
    {
        if(m_PPPPChannel[i].bValid == 1 && strcmp(m_PPPPChannel[i].szDID, szDID) == 0)
        {
            int ret = m_PPPPChannel[i].pPPPPChannel->StartRecorder(
				m_PPPPChannel[i].pPPPPChannel->MW,
				m_PPPPChannel[i].pPPPPChannel->MH,
				25,
				filepath);

			Log3("StartRecorderByDID put channel lock");

			PUT_LOCK( &PPPPChannelLock );
            return ret;
        }
    }  

	
	Log3("StartRecorderByDID put channel lock");

	PUT_LOCK( &PPPPChannelLock );
   
    return 0;
}

int CPPPPChannelManagement::CloseRecorderByDID(char * szDID)
{
    if(szDID == NULL) return 0;

	Log3("CloseRecorderByDID get channel lock");

	GET_LOCK( &PPPPChannelLock );

    int i;
    for(i = 0; i < MAX_PPPP_CHANNEL_NUM; i++)
    {
        if(m_PPPPChannel[i].bValid == 1 && strcmp(m_PPPPChannel[i].szDID, szDID) == 0)
        {
            m_PPPPChannel[i].pPPPPChannel->CloseRecorder();
			
			Log3("CloseRecorderByDID put channel lock");
			
			PUT_LOCK( &PPPPChannelLock );
            return 1;
        }
    }  

	Log3("CloseRecorderByDID put channel lock");

	PUT_LOCK( &PPPPChannelLock );
   
    return 0;
}

int CPPPPChannelManagement::PPPPSetSystemParams(char * szDID,int type,char * msg,int len)
{   
	if(szDID == NULL){
		Log3("Invalid uuid for application layer caller");
		return 0;
	}

	GET_LOCK( &PPPPChannelLock );

	int r = 0;
    int i;
	
    for(i = 0; i < MAX_PPPP_CHANNEL_NUM; i++){
		if(NULL == m_PPPPChannel[i].pPPPPChannel){
			Log3("Invalid PPPP Channel Object");
			continue;
		}
		
        if(m_PPPPChannel[i].bValid == 1 && strcmp(m_PPPPChannel[i].szDID, szDID) == 0){

			int ret = m_PPPPChannel[i].pPPPPChannel->SetSystemParams(type, msg, len);    	   

         	if(1 == ret){
            	r = 1; goto jumpout;
            }else{
            	r = 0; goto jumpout;
           }
        }
    }

jumpout:

	PUT_LOCK( &PPPPChannelLock );
    
    return r;
}
int CPPPPChannelManagement::CmdExcute(
	char * szDID,
	int gwChannel,
	int cmdType,
	char * cmdContent,
	int cmdLen){  

	//F_LOG;
	
	if(szDID == NULL){
		Log3("Invalid szDID for application layer caller");
		return 0;
	}
	
	GET_LOCK( &PPPPChannelLock );

	int r = 0;
    int i;
	int tag=0;
    for(i = 0; i < MAX_PPPP_CHANNEL_NUM; i++){
		if(NULL == m_PPPPChannel[i].pPPPPChannel){
			Log3("Invalid PPPP Channel Object");
			continue;
		}
		
        if(m_PPPPChannel[i].bValid == 1 && strcmp(m_PPPPChannel[i].szDID, szDID) == 0){

			int ret = m_PPPPChannel[i].pPPPPChannel->IOCmdSend(gwChannel,cmdType,cmdContent,cmdLen);

         	if(1 == ret){
            	r = 1; goto jumpout;
            }else{
            	r = 0; goto jumpout;
           }
        }
    }
		
jumpout:

	PUT_LOCK( &PPPPChannelLock );
	
    return r;
}


