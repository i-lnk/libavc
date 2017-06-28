
#ifdef PLATFORM_ANDROID
#include <jni.h>
#endif

#include "utility.h"
#include "PPPP_API.h"
#include "PPPPChannelManagement.h"

static COMMO_LOCK PPPPChannelLock = PTHREAD_MUTEX_INITIALIZER;
static PPPP_CHANNEL sessions[MAX_PPPP_CHANNEL_NUM] = {0};

CPPPPChannelManagement::CPPPPChannelManagement()
{
	unsigned int iotcVer;
	iotcVer=PPPP_GetAPIVersion();
	unsigned char *p1 = (unsigned char *)&iotcVer;
	char szIOVer[4];
	sprintf(szIOVer, "%d.%d.%d.%d", p1[3], p1[2], p1[1], p1[0]);
	Log3("PPPP_GetAPIVersion  [%s] ", szIOVer);

	GET_LOCK( &PPPPChannelLock );
	
	int i = 0;
	for(i = 0;i < MAX_PPPP_CHANNEL_NUM;i++){
		memset(&sessions[i], 0 ,sizeof(PPPP_CHANNEL));
	}

	PUT_LOCK( &PPPPChannelLock );
	
    InitOpenXL();
}

CPPPPChannelManagement::~CPPPPChannelManagement()
{    
    StopAll();
	DEL_LOCK( &PPPPChannelLock );
}

int CPPPPChannelManagement::Start(char * szDID, char *user, char *pwd,char *server)
{
	if(szDID == NULL) return 0;

	Log3("GET CHANNEL LOCK");
	
	GET_LOCK( &PPPPChannelLock );

	int r = 1;
    int i = 0;

	Log3("channel management start device id:[%s]",szDID);

    for(i = 0; i < MAX_PPPP_CHANNEL_NUM; i++)
    {
        if(strcmp(sessions[i].szDID, szDID) == 0){
           	r = sessions[i].session->Start(user, pwd, server);
            goto jumpout;
        }
    }

    for(i = 0; i < MAX_PPPP_CHANNEL_NUM; i++)
    {
		Log3("Looking for [%s][%d]",sessions[i].szDID,sessions[i].bValid);
	
        if(sessions[i].bValid == 0)
        {
            sessions[i].bValid = 1;

            strcpy(sessions[i].szDID, szDID); 
			
            sessions[i].session = new CPPPPChannel(szDID, user, pwd, server);
            sessions[i].session->Start(user, pwd, server);
			
			goto jumpout;
        }
    }

	r = 0;

jumpout:

	Log3("PUT CHANNEL LOCK");

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
        if(sessions[i].bValid == 1 && strcmp(sessions[i].szDID, szDID) == 0)
        {            
            memset(sessions[i].szDID, 0, sizeof(sessions[i].szDID));
            SAFE_DELETE(sessions[i].session);       
            sessions[i].bValid = 0;

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
        if(sessions[i].bValid == 1)
        {
			Log3("close channel with sid:[%s] done.",sessions[i].szDID);
		
            memset(sessions[i].szDID, 0, sizeof(sessions[i].szDID));
            SAFE_DELETE(sessions[i].session);           
            sessions[i].bValid = 0;

			Log3("close channel done.",sessions[i].szDID);
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
        if(sessions[i].bValid == 1 && strcmp(sessions[i].szDID, szDID) == 0)
        {
        	Log3("start connection with did:[%s].",szDID);
           ret = sessions[i].session->StartMediaStreams(
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
        if(sessions[i].bValid == 1 && strcmp(sessions[i].szDID, szDID) == 0)
        {
        	Log3("channel manager close live stream by %s.",szDID);
           	ret = sessions[i].session->CloseMediaStreams();
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
    	if(sessions[i].bValid == 1 && strcmp(sessions[i].szDID, szDID) == 0)
        {
			Log3("GET ----> AUDIO PLAY:[%d] AUDIO TALK:[%d].",
				sessions[i].session->audioEnabled,
				sessions[i].session->voiceEnabled
				);

			int val = (sessions[i].session->audioEnabled | sessions[i].session->voiceEnabled << 1) & 0x3;

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
        if(sessions[i].bValid == 1 && strcmp(sessions[i].szDID, szDID) == 0)
        {
            sessions[i].session->audioEnabled = (AudioStatus & 0x1);
			sessions[i].session->voiceEnabled = (AudioStatus & 0x2) >> 1;

			Log3("SET ----> AUDIO PLAY:[%d] AUDIO TALK:[%d].",
				sessions[i].session->audioEnabled,
				sessions[i].session->voiceEnabled
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
        if(sessions[i].bValid == 1 && strcmp(sessions[i].szDID, szDID) == 0)
        {
            int ret = sessions[i].session->StartRecorder(
				sessions[i].session->MW,
				sessions[i].session->MH,
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
        if(sessions[i].bValid == 1 && strcmp(sessions[i].szDID, szDID) == 0)
        {
            sessions[i].session->CloseRecorder();
			
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
		if(NULL == sessions[i].session){
			Log3("Invalid PPPP Channel Object");
			continue;
		}
		
        if(sessions[i].bValid == 1 && strcmp(sessions[i].szDID, szDID) == 0){

			int ret = sessions[i].session->SetSystemParams(type, msg, len);    	   

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
		if(NULL == sessions[i].session){
			Log3("Invalid PPPP Channel Object");
			continue;
		}
		
        if(sessions[i].bValid == 1 && strcmp(sessions[i].szDID, szDID) == 0){

			int ret = sessions[i].session->IOCmdSend(gwChannel,cmdType,cmdContent,cmdLen);

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


