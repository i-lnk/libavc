#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <sys/time.h>
#include <math.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <pthread.h>
#include <sys/syscall.h>
#include <unistd.h>  
#include <sys/stat.h> 
#include <sys/types.h> 

#include "utility.h"
#include "PPPPChannel.h"  

#include "PPPP_API.h"
#include "iLnkDefine.h"
#include "iLnkRingBuf.h"
#include "PPPP_Error.h"

#include "gain_control.h"
#include "object_jni.h"

#include "appreq.h"
#include "apprsp.h"

//#define ENABLE_VIDEO_RECORD_FIX
#define ENABLE_AUDIO_RECORD
#define ENABLE_AEC
#define ENABLE_AGC
#define ENABLE_NSX_I
#define ENABLE_NSX_O
//#define ENABLE_VAD

#ifdef PLATFORM_ANDROID
#endif
//#define ENABLE_DEBUG

#ifdef PLATFORM_ANDROID

#include <jni.h>
extern JavaVM * g_JavaVM;

#else

static inline int gettid(){ return 0;}

#endif

extern jobject   g_CallBack_Handle;
extern jmethodID g_CallBack_ConnectionNotify;
extern jmethodID g_CallBack_MessageNotify;
extern jmethodID g_CallBack_VideoDataProcess;
extern jmethodID g_CallBack_AlarmNotifyDoorBell;
extern jmethodID g_CallBack_UILayerNotify;
extern jmethodID g_CallBack_CmdRecv;



extern COMMO_LOCK g_CallbackContextLock;

COMMO_LOCK OpenSLLock = PTHREAD_MUTEX_INITIALIZER;

unsigned long GetAudioTime(){
#ifdef PLATFORM_ANDROID
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (ts.tv_sec * 1000 + ts.tv_nsec / 1000000);
#else
    struct timeval  ts;
    gettimeofday(&ts,NULL);
    return (ts.tv_sec * 1000 + ts.tv_usec / 1000000);
#endif
}

#ifdef PLATFORM_ANDROID

// this callback handler is called every time a buffer finishes recording
static void recordCallback(
	SLAndroidSimpleBufferQueueItf bq, 
	void *context
){
	OPENXL_STREAM * p = (OPENXL_STREAM *)context;
	CPPPPChannel * hPC = (CPPPPChannel *)p->context;

    short * hFrame = p->recordBuffer+(p->iBufferIndex * hPC->Audio10msLength  / sizeof(short));
	
	hPC->hAudioGetList->Put((char*)hFrame,hPC->Audio10msLength);

	(*p->recorderBufferQueue)->Enqueue(p->recorderBufferQueue,(char*)hFrame,hPC->Audio10msLength);

    p->iBufferIndex = (p->iBufferIndex+1)%CBC_CACHE_NUM;
}

// this callback handler is called every time a buffer finishes playing
static void playerCallback(
	SLAndroidSimpleBufferQueueItf bq, 
	void *context
){
	OPENXL_STREAM * p = (OPENXL_STREAM *)context;
	CPPPPChannel * hPC = (CPPPPChannel *)p->context;

    short *hFrame = p->outputBuffer+(p->oBufferIndex * hPC->Audio10msLength / sizeof(short));

	hPC->hAudioPutList->Put((char*)hFrame,hPC->Audio10msLength);
	
	int stocksize = hPC->hSoundBuffer->Used();

	if(stocksize >= hPC->Audio10msLength){
		hPC->hSoundBuffer->Get((char*)hFrame,hPC->Audio10msLength);
	}else{
        memset((char*)hFrame,0,hPC->Audio10msLength);
	}

	(*p->bqPlayerBufferQueue)->Enqueue(p->bqPlayerBufferQueue,(char*)hFrame,hPC->Audio10msLength);

    p->oBufferIndex = (p->oBufferIndex+1)%CBC_CACHE_NUM;
}

#else

static void recordCallback(char * data, int lens, void *context){
    OPENXL_STREAM * p = (OPENXL_STREAM *)context;
    CPPPPChannel * hPC = (CPPPPChannel *)p->context;

	if(lens > hPC->Audio10msLength){
        Log3("audio record sample is too large:[%d].",lens);
        return;
    }
    
    char * pr = (char*)p->recordBuffer;
    memcpy(pr + p->recordSize,data,lens);
    p->recordSize += lens;
    
    if(p->recordSize >= hPC->Audio10msLength){
        hPC->hAudioGetList->Put(pr,hPC->Audio10msLength);
        p->recordSize -= hPC->Audio10msLength;
        memcpy(pr,pr + hPC->Audio10msLength,p->recordSize);
    }
}

static void playerCallback(char * data, int lens, void *context){
    OPENXL_STREAM * p = (OPENXL_STREAM *)context;
    CPPPPChannel * hPC = (CPPPPChannel *)p->context;

	 if(lens > hPC->Audio10msLength){
        Log3("audio output sample is too large:[%d].",lens);
        return;
    }
    
    int stocksize = hPC->hSoundBuffer->Used();
    
    if(stocksize >= lens){
        hPC->hSoundBuffer->Get((char*)data,lens);
    }else{
        memset((char*)data,0,lens);
    }
    
    char * po = (char*)p->outputBuffer;
    memcpy(po + p->outputSize,data,lens);
    p->outputSize += lens;
    
    if(p->outputSize >= hPC->Audio10msLength){
        hPC->hAudioPutList->Put(po,hPC->Audio10msLength);
        p->outputSize -= hPC->Audio10msLength;
        memcpy(po,
               po + hPC->Audio10msLength,
               p->outputSize);
    }

}

#endif

void FileParam_free(st_fileParam *fileParam){
	if (fileParam!=NULL){
		free(fileParam);
	}
}

st_fileParam *FileParam_get(
    unsigned char	type,		//  0->264 idr frame 1->264 p frame	
    unsigned int 	gettime,	//  diff time
	unsigned int size,
	unsigned int offset,
	char	*filename,
	char   *magic,
	FILE *fd
	){
	st_fileParam *fileParam=(st_fileParam *)calloc(1,sizeof(st_fileParam));

	if(NULL != fileParam){
		fileParam->fd=fd;
		fileParam->next=NULL;
		fileParam->pres=NULL;
		fileParam->gettime=gettime;
		fileParam->size=size;
		fileParam->offset=offset;
		strncpy(fileParam->filename,filename,128);
		Log3("Strncpy fileParam->filename:%s -------filename:%s",fileParam->filename,filename);
		memcpy(fileParam->magic,magic,4);
		Log3("Strncpy fileParam->magic:%s ----------magic:%s",fileParam->magic,magic);
		fileParam->type=type;
	}
	return fileParam;
}

int FilePush(st_fileParam *data,void * hVoid){
	
	CPPPPChannel * hPC = (CPPPPChannel *)hVoid;
	Log3("Function FilePush  ===================>1");
	Log3("Function FilePush  ===================>2");
	if (hPC->m_rear!=NULL){
		Log3("Function FilePush hPC->m_rear!=NULL ===================>1");
		hPC->m_rear->next=data;
		Log3("Function FilePush hPC->m_rear!=NULL ===================>2");
		hPC->m_rear=data;
		Log3("Function FilePush hPC->m_rear!=NULL ===================>3");
		hPC->m_nNmb++;
		Log3("Function FilePush hPC->m_rear!=NULL ===================>4");
	}else{
	
		Log3("Function FilePush  ===================>3");
		hPC->m_rear=data;
		Log3("Function FilePush  ===================>4");
		hPC->m_front=data;
		Log3("Function FilePush  ===================>5");
		hPC->m_nNmb=1;
		Log3("Function FilePush  ===================>6");
	}
	return hPC->m_nNmb;
}

int FileWrite(JNIEnv * env,char *magic,void *data,unsigned int len,void * hVoid){
	char MsgStr[128] = {0};
	CPPPPChannel * hPC = (CPPPPChannel *)hVoid;
	st_fileParam *cur=hPC->m_front;
	st_fileParam *mNext=NULL;

	if (magic==NULL){
		Log3("magic == NULL");
		return -1;
	}
	
	if (cur==NULL){
		Log3("can not find out the node which magic=%s",magic);
		return -1;
	}

	while(cur!=NULL){
		mNext=cur->next;
		if (memcmp(cur->magic,magic,4)==0)break;
		cur=mNext;
	}

	fwrite((const void*)data,len,1,cur->fd);
	cur->offset+=len;
	hPC->m_n=cur->offset;
	
	//�ص����ؽ���
	sprintf(MsgStr,"%d",(hPC->m_n * 100) / cur->size);
	Log3("Current has DownLoad progress : ===================>%s",MsgStr);

		GET_LOCK(&g_CallbackContextLock);
		    if(g_CallBack_Handle != NULL)
		    {   

				jstring	jstring_did = env->NewStringUTF(hPC->szDID);
		        jstring jstring_msg = env->NewStringUTF(MsgStr);
					
				env->CallVoidMethod(g_CallBack_Handle,g_CallBack_UILayerNotify,jstring_did,CMD_FILE_DOWNLOAD_PROGRESS,jstring_msg);

				env->DeleteLocalRef(jstring_did); 
		        env->DeleteLocalRef(jstring_msg);
	    	}
		PUT_LOCK(&g_CallbackContextLock);

	if (hPC->m_n == cur->size){
		if (cur->pres!=NULL){
			cur->pres->next=cur->next;
		}
		else{
			hPC->m_front=cur->next;
			if (hPC->m_front!=NULL)hPC->m_front->pres=NULL;
		}
		
		if (cur->next!=NULL){
			cur->next->pres=cur->pres;
		}
		else{
			hPC->m_rear=cur->pres;
			if (hPC->m_rear!=NULL)hPC->m_rear->next=NULL;
		}

		hPC->m_n=0;

		Log3("close the fileMagic[%s]",cur->magic);
		fclose(cur->fd);		
		FileParam_free(cur);
		Log3("FileParam_free(cur)");
	}

	return hPC->m_n;
}

int PPPP_IndeedWrite(
	int sessionHandle,
	unsigned channel, 
	char * buf, 
	int len
	){
	int iRet = 0;
	#define MAX_P2P_SEND_SIZE   4 * 1024
	
	unsigned int nFreeBytes = len;
	char* pBuffer = buf;
	int nWriteBytes = 0;
	unsigned int nSendBytes = 0;
	while (nFreeBytes>0){
		nSendBytes = (nFreeBytes < MAX_P2P_SEND_SIZE) ? nFreeBytes : MAX_P2P_SEND_SIZE;
		iRet = PPPP_Write(sessionHandle, channel,pBuffer,nSendBytes);
		
		switch(iRet){
			case  ERROR_PPPP_SESSION_CLOSED_REMOTE:
			case  ERROR_PPPP_SESSION_CLOSED_TIMEOUT:
				 Log3("PPPP_Read ERROR_PPPP_SESSION_CLOSED_REMOTE : %d", iRet);
				 PPPP_Close(sessionHandle);
				 break;		
		}
		if ( iRet < 0 ){
			Log3("PPPP_Write Error ! %d \n",iRet);
			return iRet;
		}
	
		nFreeBytes -= nSendBytes;
		pBuffer += nSendBytes;
		nWriteBytes += iRet;
		usleep(50000);
		}
	
	iRet = nWriteBytes;

	return iRet;
	}

int PPPP_IndeedRead(
	int sessionHandle,
	unsigned char channel, 
	char * buf, 
	int len
	){
	char *p = buf;	  
	int readSize ;
	int remainSize = len;
	int res; 
	#define MAX_P2P_RECV_SIZE	32*1024
	do
	{
		readSize = (remainSize>MAX_P2P_RECV_SIZE)?MAX_P2P_RECV_SIZE:remainSize;

		
		res = PPPP_Read(sessionHandle, channel, p, &readSize, 100);  
		
		if(res == ERROR_PPPP_TIME_OUT){
			remainSize -= readSize;
			p += readSize;
			usleep(100000);
			continue;
		}
		
		if(res < 0){
			Log3("PPPP_Read error : %d", res);
			return res;
		}

		remainSize -= readSize;
		p += readSize;
	}while(remainSize != 0);

	return 0;
}

//��ȡSD���ļ������߳�,ע:������ʵ�֣����Ǵ�������!!
static void *FileRecvProcess(
	void * hVoid
){
    CPPPPChannel * hPC= (CPPPPChannel *)hVoid;
	int status; 
    JNIEnv * hEnv = NULL; 	
#ifdef PLATFORM_ANDROID

	bool isAttached = false; 
    status = g_JavaVM->GetEnv((void **) &(hEnv), JNI_VERSION_1_4); 

    if(status < 0) 
    { 
        status = g_JavaVM->AttachCurrentThread(&(hEnv), NULL); 
        if(status < 0)
        {
            Log3("CommandThread AttachCurrentThread Failed!!");
            return NULL;
        }
        isAttached = true; 
    }    
#endif
    
  	int nRet=0;
	FILE_DATA_HEAD dataHead;
	PFILE_ATTR pFileAttr=NULL;
	char pbuf[64*1024] = {0,}; 
	unsigned int dHeadLen=sizeof(FILE_DATA_HEAD);
	unsigned int readSize=0;
	unsigned int counter=0;
	int tag=0;
	
	jstring    jstring_did = hEnv->NewStringUTF(hPC->szDID);
	jbyteArray jbyteArray_yuv = hEnv->NewByteArray(hPC->YUVSize);
	jbyte *	   jbyte_yuv = (jbyte *)(hEnv->GetByteArrayElements(jbyteArray_yuv,0));
	
    while(hPC->fileRecving){
		usleep(10000);
        //read head
        memset(&dataHead,0,sizeof(FILE_DATA_HEAD));
		
		nRet=PPPP_Check_Buffer(hPC->sessionID,P2P_CHANNEL_FILE,NULL,&readSize);
		if(nRet < 0){
			Log3("File_RecvProc  PPPP_Check_Buffer=%d", nRet);
			break ;
		}
		
		if (readSize<dHeadLen){
			//Log3("File_RecvProc  can read %d", readSize);
			continue;
		}
		
		nRet = PPPP_IndeedRead(hPC->sessionID,P2P_CHANNEL_FILE, (char *)&dataHead, dHeadLen);
		if(nRet < 0){
			Log3("FileRecv PPPP_IndeedRead failed  return: %d", nRet);
			status=nRet;
			break ;
		 }
		
		if (dataHead.startcode!=0xff12ff34){
			
			if (tag==0){
				tag=1;
				Log3("error!! counter=%u,dataHead.startcode=%04x", counter,dataHead.startcode);
			}
			
			continue ;
		}
		counter=ntohs(dataHead.blockseq);
		tag=0;
        if(dataHead.len > MAX_FRAME_LENGTH){
			Log3("FileRecv  PPPP_IndeedRead failed dataHead.len :%d",dataHead.len );
			continue;
        }

		if(dataHead.len == 0){
			Log3("FileRecv  PPPP_IndeedRead failed dataHead.len :%d",dataHead.len );
			continue;
		}

		memset(pbuf,0x00,sizeof(pbuf));
		if (dataHead.type==0){//�ļ�����
			nRet = PPPP_IndeedRead(hPC->sessionID,P2P_CHANNEL_FILE, (char*)pbuf, dataHead.len);
			if(nRet < 0){
				Log3("FileRecv-attr PPPP_IndeedRead error: %d", nRet);
				status=nRet;
				break ;
			}else{
				pFileAttr = (PFILE_ATTR)pbuf;
				st_fileParam * fileParam = NULL;
				char fullname[256] = {0};
				
				if(access(hPC->FileDataPath,0) != 0 ){
					Log3("Mkdir FileDir ======>%s",hPC->FileDataPath);
					mkdir(hPC->FileDataPath,777);
				}
				
				sprintf(fullname,"%s/%s",hPC->FileDataPath,hPC->FileDataName);

				FILE *fd=fopen(fullname, "w+b");
				if (fd==NULL){
					Log3("p2pChannel_File_RecvProc file cannot be created!!!\n\r,fullname=%s\n\r",fullname);
					continue;
				}
				
				fileParam =(st_fileParam *)FileParam_get(0,time(0),pFileAttr->filesize,0,(char *)pFileAttr->filename,(char *)dataHead.magic,fd);

				Log3("FileParam_get fileParam ----------------------->TEST");

				FilePush(fileParam,hVoid);

				Log3("p2pChannel_File_RecvProc PPPP_IndeedRead getfileattr: %s size:%d", fullname,pFileAttr->filesize);
			}
		}
		else{
			nRet = PPPP_IndeedRead(hPC->sessionID,P2P_CHANNEL_FILE, (char *)pbuf,dataHead.len);
			if(nRet < 0){
				Log3("FileRecv-content PPPP_IndeedRead error: %d", nRet);
				status=nRet;
				break ;
			}
			nRet = FileWrite(hEnv,dataHead.magic,(void *)pbuf,dataHead.len,hVoid);
			if (nRet < 0){
				Log3("Error!!! p2pChannel_File_RecvProc FileWrite data:%d", nRet);
			}
		}
		
    }
	hEnv->ReleaseByteArrayElements(jbyteArray_yuv,jbyte_yuv,0);
	hEnv->DeleteLocalRef(jbyteArray_yuv);
	hEnv->DeleteLocalRef(jstring_did);

	Log3("p2pChannel_File_RecvProc quit ...");
#ifdef PLATFORM_ANDROID
    if(isAttached) 
        g_JavaVM->DetachCurrentThread(); 
#endif
	
    return NULL;
}

void * MeidaCoreProcess(
	void * hVoid
){
	
	SET_THREAD_NAME("MeidaCoreProcess");
	
	Log3("current thread id is:[%d].",gettid());

	CPPPPChannel * hPC = (CPPPPChannel*)hVoid;

	JNIEnv * hEnv = NULL;
#ifdef PLATFORM_ANDROID
    char isAttached = 0;

	int status = g_JavaVM->GetEnv((void **) &(hEnv), JNI_VERSION_1_4); 
	if(status < 0){ 
		status = g_JavaVM->AttachCurrentThread(&(hEnv), NULL); 
		if(status < 0){
			Log3("iocmd send process AttachCurrentThread Failed!!");
			return NULL;
		}
		isAttached = 1;  
	}
#endif

	int resend = 0;
	int wakeup_times = 20;
	int retry = 5;

	int ret = 0;
	int connection_status = PPPP_STATUS_DISCONNECTED;
connect:
    hPC->MsgNotify(hEnv, MSG_NOTIFY_TYPE_PPPP_STATUS, PPPP_STATUS_CONNECTING);
    
	hPC->speakerChannel = -1;
	hPC->avstremChannel =  0;
	
	Log3("[1:%s]=====>start get free session id for client connection.",
         hPC->szDID);
	//yunni p2p
	Log3("p2p_ConnectProc Begin----------------\n");
	ret = hPC->p2p_ConnectProc(hEnv);
    switch(ret){
		case 0:
			break;
		case -1:
			connection_status = PPPP_STATUS_DISCONNECTED;
			goto jumperr;
		case -2:
			connection_status = PPPP_STATUS_USER_NOT_LOGIN;
			goto jumperr;
		default:
			goto jumperr;
	}

	Log3("[2:%s]=====>channel init command proc here.",hPC->szDID);
	hPC->StartIOCmdChannel();
	
	Log3("[3:%s]=====>channel init fileRecv proc done.",hPC->szDID);
	hPC->StartFileRecvThread();
	
	GET_LOCK(&hPC->SessionStatusLock);
	hPC->SessionStatus = STATUS_SESSION_IDLE;
	Log3("SESSION STATUS:IDLE");
	PUT_LOCK(&hPC->SessionStatusLock);
	
	while(hPC->mediaLinking){
		//Check User Status
		st_PPPP_Session1 SInfo;
		memset(&SInfo,0x00,sizeof(st_PPPP_Session1));
		ret = PPPP_Check(hPC->sessionID,&SInfo);
		
		if(ret < 0){
		//	Log3("P2P Connect Status ------------------------>%d ",ret);
			hPC->MsgNotify(hEnv,MSG_NOTIFY_TYPE_PPPP_STATUS, PPPP_STATUS_DEVICE_OFFLINE);
			Log3("[7:%s]=====>stop old media process start.\n",hPC->szDID);
			
    	   	if(!hPC->CloseWholeThreads()){
				Log3("MediaCoreProcess hPC->PPPPClose().........1");
				hPC->PPPPClose();
			}
			Log3("[7:%s]=====>stop old media process close.\n",hPC->szDID);
			
			if(ret = ERROR_PPPP_INVALID_SESSION_HANDLE){
				if(retry > 0){
					retry--;
					usleep(100000);
					goto connect;
				}
			}
			break;
		}
		sleep(1);
	}

jumperr:

    if(!hPC->CloseWholeThreads()){   // make sure other service thread all exit.
    	Log3("MediaCoreProcess hPC->PPPPClose()........2");
		hPC->PPPPClose();
	} 	

	Log3("Get Session status lock");

	GET_LOCK(&hPC->SessionStatusLock);
	hPC->SessionStatus = STATUS_SESSION_DIED;
	Log3("SESSION STATUS:DIED");
	PUT_LOCK(&hPC->SessionStatusLock);

	Log3("UILayer status set start");
	hPC->MsgNotify(hEnv,MSG_NOTIFY_TYPE_PPPP_STATUS, connection_status);
	Log3("UILayer status set done");

#ifdef PLATFORM_ANDROID
	if(isAttached) g_JavaVM->DetachCurrentThread();
#endif

	Log3("MeidaCoreProcess Stop ..............");

	return NULL;	
}

void * IOCmdSendProcess(
	void * hVoid
){
    SET_THREAD_NAME("IOCmdSendProcess");
	CPPPPChannel * hPC = (CPPPPChannel*)hVoid;
	int ret = 0;
	char Cmds[8192] = {0};
	APP_CMD_HEAD * hCmds = (APP_CMD_HEAD*)Cmds;

	int nBytesRead = 0;
	
	hPC->hIOCmdBuffer->Clear();
	
    while(hPC->iocmdSending){
		if(hPC->hIOCmdBuffer == NULL){
			Log3("[X:%s]=====>Invalid hIOCmdBuffer.",hPC->szDID);
			sleep(2);
			continue;
		}

		if(hPC->hIOCmdBuffer->Used() >= (int)sizeof(APP_CMD_HEAD)){
			
			nBytesRead = hPC->hIOCmdBuffer->Get((char*)hCmds,sizeof(APP_CMD_HEAD));
			if(nBytesRead == sizeof(APP_CMD_HEAD)){
                if(hCmds->Magic != 0x78787878
                || hCmds->CgiLens > sizeof(Cmds)
                || hCmds->CgiLens < 0
                ){
                    Log3("[X:%s]=====>invalid IOCTRL cmds from application. M:[%08X] L:[%d].\n",
                         hPC->szDID,
                         hCmds->Magic,
                         hCmds->CgiLens
                         );
                    
                    hPC->hIOCmdBuffer->Clear();
                }

				nBytesRead = 0;
				while(nBytesRead != hCmds->CgiLens){
					
					nBytesRead = hPC->hIOCmdBuffer->Get(hCmds->CgiData,hCmds->CgiLens);

					Log3("[X:%s]=====>data not ready.\n",hPC->szDID);
					usleep(1000);
				}
				
				while(hPC->iocmdSending){
					ret = SendCmds(hPC->avIdx,hCmds->AppType,hCmds->CgiData,hPC);
	
					if(ret == 0){
						break;
					}else if( ret == -1){
						break;
					}else{
						break;
					}			
				}
			}
		}
		usleep(1000);
	}

	Log3("[X:%s]=====>IOCmdSendProcess Exit.",hPC->szDID);

	return NULL;
}

void * IOCmdRecvProcess(
	void * hVoid
){
    SET_THREAD_NAME("IOCmdRecvProcess");

	Log3("current thread id is:[%d].",gettid());

	int ret = 0;
	CPPPPChannel * hPC = (CPPPPChannel*)hVoid;
    JNIEnv * hEnv = NULL;
    
#ifdef PLATFORM_ANDROID
	char isAttached = 0;

	int status = g_JavaVM->GetEnv((void **) &(hEnv), JNI_VERSION_1_4); 
	if(status < 0){ 
		status = g_JavaVM->AttachCurrentThread(&(hEnv), NULL); 
		if(status < 0){
			Log("iocmd recv process AttachCurrentThread Failed!!");
			return NULL;
		}
		isAttached = 1; 
	}
#endif

	hPC->m_JNIMainEnv = hEnv;
	unsigned int ReadSize=0;
	char Params[128*1024] = {0};

	unsigned int IOCtrlType = 0;
	CMD_CHANNEL_HEAD * hCCH = (CMD_CHANNEL_HEAD*)Params;
	int avIdx = hPC->avIdx;

	jbyteArray jbyteArray_cmd = hEnv->NewByteArray(sizeof(Params));
	jstring jstring_did = hEnv->NewStringUTF(hPC->szDID);
	
	while(hPC->iocmdRecving){
//yunni p2p 
		
		ret = PPPP_Check_Buffer(hPC->sessionID,P2P_CHANNEL_CMMND,NULL,(unsigned int *)&ReadSize);
		if(ret<0){
			Log3("error, PPPP_Check_Buffer=%d",ret);
			break;
		}
		
		if(ReadSize<sizeof(CMD_CHANNEL_HEAD)){
			usleep(100000);
			continue;
		}
		ret = PPPP_IndeedRead(hPC->sessionID,P2P_CHANNEL_CMMND,Params,sizeof(CMD_CHANNEL_HEAD));
		if(ret < 0){
			Log3("error!!!! PPPP_IndeedRead=%d",ret);
            break ;
        }

        //check head
        if(hCCH->startcode != CMD_START_CODE 
			|| hCCH->len < 0){
            Log3("error!!! cmdhead.startcode != CMD_START_CODE or len < 0,res[%d]\n",ret);
            break;
        }

        if(hCCH->len == 0){
            continue;
        }
		
        //read data
        ret = PPPP_IndeedRead(hPC->sessionID,P2P_CHANNEL_CMMND,hCCH->d,hCCH->len);
        if(ret < 0){
			Log3("error, PPPP_IndeedRead=%d",ret);
            break;
        }
		
		//
		// here we process command for yunni p2p
		// ...
		Log3("ProcessCommand: hand instructions from Camera [%04x] ! \n",hCCH->cmd);

		if(hCCH->cmd == CMD_SYSTEM_USER_CHK){
			hPC->ConnectUserCheckAcK(hEnv,hCCH->d,hCCH->len);
			Log3("ConnectUserCheckAcK,len=%d",hCCH->len);
			continue;
		}

		GET_LOCK(&g_CallbackContextLock);
		
	    if(g_CallBack_Handle != NULL){     
			hEnv->SetByteArrayRegion(jbyteArray_cmd,0,hCCH->len,(jbyte*)hCCH->d);
			
			hEnv->CallVoidMethod(
				g_CallBack_Handle, 
				g_CallBack_CmdRecv, 
				jstring_did, 
				hPC->sessionID, 
				hCCH->cmd, 
				jbyteArray_cmd, 
				hCCH->len
				);
	    }
		
		PUT_LOCK(&g_CallbackContextLock);
    }

	hEnv->DeleteLocalRef(jbyteArray_cmd); 
    hEnv->DeleteLocalRef(jstring_did);
	
#ifdef PLATFORM_ANDROID
	if(isAttached) g_JavaVM->DetachCurrentThread();
#endif
    
	Log3("[X:%s]=====>iocmd recv proc exit.",hPC->szDID);

	return NULL;
}

static void * VideoPlayProcess(
	void * hVoid
){
	SET_THREAD_NAME("VideoPlayProcess");

	Log3("current thread id is:[%d].",gettid());

	CPPPPChannel * hPC = (CPPPPChannel*)hVoid;
	JNIEnv * hEnv = NULL;
    
#ifdef PLATFORM_ANDROID
	char isAttached = 0;

	int status = g_JavaVM->GetEnv((void **) &(hEnv), JNI_VERSION_1_4); 
    if(status < 0){ 
        status = g_JavaVM->AttachCurrentThread(&(hEnv), NULL); 
        if(status < 0){
            return NULL;
        }
        isAttached = 1; 
    }
#endif

	jstring    jstring_did = hEnv->NewStringUTF(hPC->szDID);
	jbyteArray jbyteArray_yuv = hEnv->NewByteArray(hPC->YUVSize);

	GET_LOCK(&hPC->DisplayLock);
	hPC->hVideoFrame = NULL;
	PUT_LOCK(&hPC->DisplayLock);

	while(hPC->videoPlaying){

		if(g_CallBack_Handle == NULL || g_CallBack_VideoDataProcess == NULL){
			usleep(1000); continue;
		}

		if(hPC->mediaEnabled != 1){
			usleep(1000); continue;
		}

		GET_LOCK(&hPC->DisplayLock);

		if(hPC->hVideoFrame == NULL){
			PUT_LOCK(&hPC->DisplayLock);
			usleep(1000); continue;
		}

		hEnv->SetByteArrayRegion(jbyteArray_yuv, 0, hPC->YUVSize, (const jbyte *)hPC->hVideoFrame);
//		memcpy(jbyte_yuv,hPC->hVideoFrame,hPC->YUVSize);

		hPC->hVideoFrame = NULL;

		int NW = hPC->MW;
		int NH = hPC->MH;
		int NS = NW * NH + NW * NH * 3 / 2;
		unsigned int TS = hPC->PlayBackTime;

		PUT_LOCK(&hPC->DisplayLock);

		// put h264 yuv data to java layer
		GET_LOCK(&g_CallbackContextLock);	
	   
		// for yuv draw process
		hEnv->CallVoidMethod(
			g_CallBack_Handle,
			g_CallBack_VideoDataProcess, 
			jstring_did,
			jbyteArray_yuv,
			1, 
			NS,
			NW,
			NH,
			TS);

		PUT_LOCK( &g_CallbackContextLock );
	}

	hEnv->DeleteLocalRef(jbyteArray_yuv);
	hEnv->DeleteLocalRef(jstring_did);

#ifdef PLATFORM_ANDROID
	if(isAttached) g_JavaVM->DetachCurrentThread();
#else
    jbyteArray_yuv = NULL;
    jstring_did = NULL;
#endif
	Log3("video play proc exit.");

	return NULL;
}

static void * VideoRecvProcess(
	void * hVoid
){    
	SET_THREAD_NAME("VideoRecvProcess");
	
	CPPPPChannel * hPC = (CPPPPChannel*)hVoid;
	int FrmSize = hPC->YUVSize/3;
	AV_HEAD_YUNNI * hFrm  = (AV_HEAD_YUNNI*)malloc(FrmSize);
	char   * hYUV = (char*)malloc(hPC->YUVSize);;
	
	int firstKeyFrameComming = 0;
	int	isKeyFrame = 0;
	int nRet =0;
#ifdef PLATFORM_ANDROID
	JNIEnv * hEnv = NULL;
	bool isAttached = false;
	int status;
	status = g_JavaVM->GetEnv((void **)&(hEnv), JNI_VERSION_1_4);
	if(status < 0)
		{ 
        status = g_JavaVM->AttachCurrentThread(&(hEnv), NULL); 
        if(status < 0)
        {
            Log3("VideoRecvProcess AttachCurrentThread Failed!!");
            return NULL;
        }
        isAttached = true; 
	}	
#endif
	memset(hFrm,0,FrmSize);
	memset(hYUV,0,hPC->YUVSize);
	
	while(hPC->videoPlaying)
	{
			
		nRet = PPPP_IndeedRead(hPC->sessionID,P2P_CHANNEL_VIDEO, (char *)hFrm, sizeof(AV_HEAD_YUNNI));

		if(nRet < 0){
			Log3("p2pChannel_Video_RecvProc  PPPP_IndeedRead failed  return: %d, %s", nRet,hPC->szDID);
			firstKeyFrameComming = 0;
			break;
		}

		if(hFrm->len == 0){
			continue;
		}

		nRet = PPPP_IndeedRead(hPC->sessionID,P2P_CHANNEL_VIDEO, (char *)hFrm->d, hFrm->len);
		
		if(nRet < 0){
			Log3("p2pChannel_Video_RecvProc  PPPP_IndeedRead failed  return: %d, %s", nRet,hPC->szDID);
			firstKeyFrameComming = 0;
			break;
		}
		
		if(hFrm->startcode!=AVF_STARTCODE
			||	hFrm->len > MAX_FRAME_LENGTH){
			Log3("avhead invalid!!%s,startcode=%04x,len=%d\n",
				 hPC->szDID,hFrm->startcode,hFrm->len);
			break;
			}
		
		if(hFrm->len == 0){
			continue;
		}
		
		if(0==hPC->m_bPlayStreamOK){
			if (hFrm->type==0 || hFrm->type==1){
				hPC->m_EnumVideoMode = ENUM_VIDEO_MODE_H264;
			}
			else if(hFrm->type == 3){
				hPC->m_EnumVideoMode = ENUM_VIDEO_MODE_MJPEG;
			}
			else{
				continue;
			}
			hPC->m_bPlayStreamOK=1;
		}
		
		if(hFrm->type  == ENUM_FRAME_TYPE_I){
			firstKeyFrameComming = 1;
			isKeyFrame = 1;
		}else{
			isKeyFrame = 0;
		}
		
		if(firstKeyFrameComming != 1){
			Log3("waiting for first video key frame coming.\n");
			continue;
		}

		int W = 0;
		int H = 0;
	
		// decode h264 frame
		if(hPC->hDec->DecoderFrame((uint8_t *)hFrm->d,hFrm->len,W,H,isKeyFrame) <= 0){
			Log3("decode h.264 frame failed.");
			firstKeyFrameComming = 0;
			continue;
		}

		if(W <= 0 || H <= 0){
			Log3("invalid decode resolution W:%d H:%d.",W,H);
			continue;
		}

		int nBytesHave = hPC->hVideoBuffer->Available();

		if(hPC->recordingExit){
			if(nBytesHave >= hFrm->len + sizeof(AV_HEAD)){
				hPC->hVideoBuffer->Put((char*)hFrm,hFrm->len + sizeof(AV_HEAD));
			}
		}

		if(TRY_LOCK(&hPC->DisplayLock) != 0){
			continue;
		}

		hPC->MW = W - hPC->MWCropSize;
		hPC->MH = H - hPC->MHCropSize;
		
		// get h264 yuv data
		hPC->hDec->GetYUVBuffer((uint8_t*)hYUV,hPC->YUVSize,hPC->MW,hPC->MH);
		hPC->hVideoFrame = hYUV;
		hPC->PlayBackTime = hFrm->sectime;	//��ȡ��ǰ���ݷ���ʱ���

		PUT_LOCK(&hPC->DisplayLock);
	}
	
	
	GET_LOCK(&hPC->DisplayLock);
	if(hFrm)   free(hFrm); hFrm = NULL;
	if(hYUV)   free(hYUV); hYUV = NULL;
    hPC->hVideoFrame = NULL;
	
#ifdef PLATFORM_ANDROID
	if(isAttached) 
		g_JavaVM->DetachCurrentThread(); 
#endif

	firstKeyFrameComming = 0;
	isKeyFrame = 0;
	
	PUT_LOCK(&hPC->DisplayLock);

	Log3("video recv proc exit.");
	
	return NULL;
}

static void * AudioRecvProcess(
	void * hVoid
){    
	SET_THREAD_NAME("AudioRecvProcess");
	Log3("current AudioRecvProcess thread id is:[%d].",gettid());

	int ret = 0;
	CPPPPChannel * hPC = (CPPPPChannel*)hVoid;

	void * hCodec = audio_dec_init(
		hPC->AudioRecvFormat,
		hPC->AudioSampleRate,
		hPC->AudioChannel
		);
	
	if(hCodec == NULL){
		Log3("initialize audio decodec handle failed.\n");
		return NULL;
	}
	
	char Cache[2048] = {0};	
	char Codec[4096] = {0};	

	int  CodecLength = 0;
    int  CodecLengthNeed = 960;
	
	AV_HEAD_YUNNI * avhead =(AV_HEAD_YUNNI *)Cache;

	hPC->hAudioBuffer->Clear();
	hPC->hSoundBuffer->Clear();
	
	//��ʼ��
	void * hAgc = NULL;
	void * hNsx = NULL;
	
#ifdef ENABLE_AGC
	hAgc = audio_agc_init(
		20,
		2,
		0,
		255,
		hPC->AudioSampleRate);

	if(hAgc == NULL){
		Log3("initialize audio agc failed.\n");
		goto jumperr;
	}
#endif

#ifdef ENABLE_NSX_I
	hNsx = audio_nsx_init(2,hPC->AudioSampleRate);

	if(hNsx == NULL){
		Log3("initialize audio nsx failed.\n");
		goto jumperr;
	}
#endif
	
	Log3("AudioRecvProcess  come to read buff ----------> ");

	while(hPC->audioPlaying){

		ret = PPPP_IndeedRead(hPC->sessionID,P2P_CHANNEL_AUDIO, (char *)avhead, sizeof(AV_HEAD_YUNNI));
		
		if( ret < 0 ){
			Log3("Error!!!!PPPP_IndeedRead=%d\n",ret);
			break;
		}
		
		if (avhead->startcode != AVF_STARTCODE){
			Log3( "recv audio data is invalid!!,avhead.startcode=%04x\n",avhead->startcode );
			break;
			
		}
		
		if( avhead->len > MAX_AUDIO_DATA_LENGTH){
			Log3( "recv audio data is invalid!!,avhead.startcode=%04x,avhead.len=%d\n",avhead->startcode,avhead->len );
			break;
		}

		if( avhead->len == 0 ){
			Log3( "recv audio data is invalid!!\n" );
			continue;
		}
		
		ret = PPPP_IndeedRead(hPC->sessionID,P2P_CHANNEL_AUDIO, (char *)avhead->d, avhead->len);
		
		if( ret < 0 ){
			Log3("Error!!!!PPPP_IndeedRead=%d\n",ret);
			break;
		}
		
		if( (hPC->audioEnabled != 1)){
			usleep(10000);
			continue;
		}
		
		if(avhead->audiocodec != hPC->AudioRecvFormat){
			Log3("invalid packet format for audio decoder:[%02X].",avhead->audiocodec);
			audio_dec_free(hCodec);
			
			Log3("initialize new audio decoder here.\n")
				
			hCodec = audio_dec_init(
				avhead->audiocodec,
				hPC->AudioSampleRate,
				hPC->AudioChannel
				);
			
			if(hCodec == NULL){
				Log3("initialize audio decodec handle for codec:[%02X] failed.",avhead->audiocodec);
				break;
			}
			Log3("initialize new audio decoder done.\n")
			hPC->AudioRecvFormat = avhead->audiocodec;
			continue;
		}
		
		if((ret = audio_dec_process(
                hCodec,
                avhead->d,
                avhead->len,
                &Codec[CodecLength],
                sizeof(Codec) - CodecLength)) < 0){
			
			Log3("audio decodec process run error:%d with codec:[%02X] lens:[%d].\n",
				ret,
				hPC->AudioRecvFormat,
				avhead->len
				);
			
			continue;
		}

		CodecLength += ret;

		if(CodecLength < CodecLengthNeed){
            continue;
        }

		CodecLengthNeed = CodecLength - (CodecLength % hPC->Audio10msLength);
		int Round = CodecLengthNeed/hPC->Audio10msLength;
        
		for(int i = 0; i < Round; i++){
#ifdef ENABLE_NSX_I
			audio_nsx_proc(hNsx,&Codec[hPC->Audio10msLength*i],hPC->Audio10msLength);
#endif
#ifdef ENABLE_AGC
			audio_agc_proc(hAgc,&Codec[hPC->Audio10msLength*i],hPC->Audio10msLength);
#endif
			if(hPC->audioEnabled){
				hPC->hSoundBuffer->Put((char*)&Codec[hPC->Audio10msLength*i],hPC->Audio10msLength); // for audio player callback
			}
			
#ifdef ENABLE_AUDIO_RECORD
			hPC->hAudioBuffer->Put((char*)&Codec[hPC->Audio10msLength*i],hPC->Audio10msLength); // for audio avi record
#endif
		}
        
        CodecLength -= CodecLengthNeed;
        memcpy(Codec,&Codec[CodecLengthNeed],CodecLength);

	}
	
jumperr:

	audio_dec_free(hCodec);

	Log3("audio recv proc exit.");

	return NULL;
}

static void * AudioSendProcess(
	void * hVoid
){    
	SET_THREAD_NAME("AudioSendProcess");
	
	CPPPPChannel * hPC = (CPPPPChannel*)hVoid;

	int ret = 0;
	unsigned int frameno = 0;

	void * hCodec = audio_enc_init(hPC->AudioSendFormat,hPC->AudioSampleRate,hPC->AudioChannel);
	if(hCodec == NULL){
		Log3("initialize audio encodec handle failed.\n");
		return NULL;
	}

#ifdef ENABLE_AEC
	void * hAEC = audio_echo_cancellation_init(3,hPC->AudioSampleRate);
#endif

#ifdef ENABLE_NSX_O
	void * hNsx = audio_nsx_init(2,hPC->AudioSampleRate);

	if(hNsx == NULL){
		Log3("initialize audio nsx failed.\n");
	}
#endif

#ifdef ENABLE_VAD
	void * hVad = audio_vad_init();
	if(hVad == NULL){
		Log3("initialize audio vad failed.\n");
	}
#endif

	hPC->hAudioPutList = new CCircleBuffer(32,hPC->Audio10msLength,0);
	hPC->hAudioGetList = new CCircleBuffer(32,hPC->Audio10msLength,0);
	
	if(hPC->hAudioPutList == NULL || hPC->hAudioGetList == NULL){
		Log3("audio data list init failed.");
		hPC->audioPlaying = 0;
	}
	
	OPENXL_STREAM * hOSL = NULL;
	hOSL = InitOpenXLStream(
		hPC->AudioSampleRate,
		hPC->AudioChannel,
		hPC->AudioChannel,
		hVoid,
		recordCallback,
		playerCallback
		);
	
	if(!hOSL){
		Log3("opensl init failed.");
		hPC->audioPlaying = 0;
	}

	char hFrame[4*960] = {0};
	char hCodecFrame[2*960] = {0};

	AV_HEAD_YUNNI* hAV = (AV_HEAD_YUNNI*)hFrame;
	char * WritePtr = hAV->d;

	int nBytesNeed = 1920;
	int nBytesPost = 0;
	
#ifdef ENABLE_VAD
	int nVadFrames = 0;
#endif

	char speakerData[320] = {0};
	char captureData[320] = {0};
	
	while(hPC->audioPlaying){

		if((hPC->voiceEnabled!=1)){
			usleep(1000);
			continue;
		}
		
		int captureLens = hPC->hAudioGetList->Used();
		int speakerLens = hPC->hAudioPutList->Used();

		if(captureLens < hPC->Audio10msLength || speakerLens < hPC->Audio10msLength){
			usleep(10);
			continue;
		}

		hPC->hAudioGetList->Get(captureData,hPC->Audio10msLength);
		hPC->hAudioPutList->Get(speakerData,hPC->Audio10msLength);

		if(hPC->voiceEnabled != 1){
			usleep(10); 
			continue;
		}
		
		
#ifdef ENABLE_AEC
		if (audio_echo_cancellation_farend(hAEC,(char*)speakerData,hPC->Audio10msLength/sizeof(short)) != 0){
				Log3("WebRtcAecm_BufferFarend() failed.");
		}
		
		if (audio_echo_cancellation_proc(hAEC,(char*)captureData,(char*)WritePtr,hPC->Audio10msLength/sizeof(short)) != 0){
				Log3("WebRtcAecm_Process() failed.");
		}
#else
		memcpy(WritePtr,captureData,hPC->Audio10msLength);
#endif

#ifdef ENABLE_NSX_O
		audio_nsx_proc(hNsx,WritePtr,hPC->Audio10msLength);
#endif

#ifdef ENABLE_VAD
		int logration = audio_vad_proc(hVad,WritePtr,hPC->Audio10msLength);

		if(logration < 1024){
//			Log3("audio detect vad actived:[%d].\n",logration);
			nVadFrames ++;
		}else{
			nVadFrames = 0;
		}
#endif

		hAV->len += hPC->Audio10msLength;
		WritePtr += hPC->Audio10msLength;

		if(hAV->len < nBytesNeed){
			continue;
		}

		ret = audio_enc_process(
            hCodec,
            hAV->d,
            hAV->len,
            hCodecFrame,
            sizeof(hCodecFrame));
        
		if(ret < 2){
			Log3("audio encode failed with error:%d.\n",ret);
			hAV->len = 0;
			WritePtr = hAV->d;
			continue;
		}
		
		struct timeval tv = {0,0};
		gettimeofday(&tv,NULL);	
		int pts = (tv.tv_sec * 1000) + (tv.tv_usec / 1000);

		hAV->startcode = 0xa815aa55;
		hAV->streamid = 1;
		hAV->audiocodec = hPC->AudioSendFormat;
		hAV->sectime= pts/1000000;
		hAV->militime= pts/1000;
		hAV->len = ret;
		hAV->frameno = frameno++;
		
		memcpy(hAV->d,hCodecFrame,hAV->len);

		nBytesPost = sizeof(AV_HEAD_YUNNI) + hAV->len;
		
		ret = PPPP_IndeedWrite(hPC->sessionID,P2P_CHANNEL_AUDIO,(char*)hAV,nBytesPost);
		
		if(ret < 0){
			Log3("PPPP_IndeedWrite ----------->Failed\n");
			break;
		}else if (ret != nBytesPost){
			Log3("PPPP_IndeedWrite ----------->unfinished[ret=%d,want=%d]\n",ret,nBytesPost);
		}

//		Log3("audio send data with codec:[%02X] length:[%04d] bytes.",hAV->audiocodec,nBytesPost);

		hAV->len = 0;
		WritePtr = hAV->d;
	}
	
	FreeOpenXLStream(hOSL);
	hOSL = NULL;

	audio_enc_free(hCodec);

#ifdef ENABLE_AEC
	audio_echo_cancellation_free(hAEC);
#endif

#ifdef ENABLE_NSX_O
	audio_nsx_free(hNsx);
#endif

#ifdef ENABLE_VAD
	audio_vad_free(hVad);
#endif

	if(hPC->hAudioPutList) delete hPC->hAudioPutList;
	if(hPC->hAudioGetList) delete hPC->hAudioGetList;
	
	hPC->hAudioPutList = NULL;
	hPC->hAudioGetList = NULL;
		
	Log3("AudioSendProcess exit.\n");

	return NULL;
	

}

//
// avi recording process
//
void * RecordingProcess(void * Ptr){
	SET_THREAD_NAME("RecordingProcess");

	Log2("current thread id is:[%d].",gettid());

	CPPPPChannel * hPC = (CPPPPChannel*)Ptr;
	if(hPC == NULL){
		Log2("Invalid channel class object.");
		return NULL;
	}

	long long   ts = 0;

	int nFrame = 0;
	int nBytesRead = 0;

	int firstKeyFrameComming = 0;
	
#ifdef ENABLE_VIDEO_RECORD_FIX 
	int sts = time(NULL);
	int pts = 0;
	int fps = 0;
	int fix = 0;
#endif
//	int	isKeyFrame = 0;
	
	hPC->hAudioBuffer->Clear();
	hPC->hVideoBuffer->Clear();

	AV_HEAD * pVFrm = (AV_HEAD*)malloc(hPC->YUVSize/3);
	AV_HEAD * pAFrm = (AV_HEAD*)malloc(hPC->AudioSaveLength + sizeof(AV_HEAD));

	pAFrm->len = hPC->AudioSaveLength;

	while(hPC->recordingExit){

		int Type = WriteFrameType();

		if(Type < 0){
			usleep(10); continue;
		}

		if(Type){
			int vBytesHave = hPC->hVideoBuffer->Used();
			
			if(vBytesHave > (int)(sizeof(AV_HEAD))){
				nBytesRead = hPC->hVideoBuffer->Get((char*)pVFrm,sizeof(AV_HEAD));

				if(pVFrm->startcode != 0xa815aa55){
					Log3("invalid video frame lens:[%d].",pVFrm->len);
					hPC->hVideoBuffer->Clear();
					usleep(10); continue;
				}

				if(pVFrm->type == 0){
					firstKeyFrameComming = 1;
				}

				if(firstKeyFrameComming != 1){
					hPC->hVideoBuffer->Mov(pVFrm->len);
					continue;
				}else{
					nBytesRead = hPC->hVideoBuffer->Get(pVFrm->d,pVFrm->len);
				}

				hPC->WriteRecorder(
					pVFrm->d,pVFrm->len,
					1,
					pVFrm->type,
					ts);

				nFrame++;
			}else{
#ifdef ENABLE_VIDEO_RECORD_FIX                
                if(pts <= 5) continue;
                if(fix == 0 || firstKeyFrameComming != 1){
                    continue;
                }
			
				Log3("recording fps:[%d] lost frame count:[%d] auto fix.\n",fps,fix);

				for(int i = 0;i < fix;i++){
					hPC->WriteRecorder(
						pVFrm->d,128,
						1,
						0,
						ts
						);
				}

				nFrame += fix;
#endif
			}
#ifdef ENABLE_VIDEO_RECORD_FIX 
			pts = time(NULL) - sts;
			pts = pts > 0 ? pts : 1;
			
			fps = nFrame / pts;
			
			fix = hPC->FPS - fps;
			fix = fix > 0 ? fix : 0;
#endif	
		}else{
            if(firstKeyFrameComming != 1){
                continue;
            }

#ifdef ENABLE_AUDIO_RECORD
			int aBytesHave = hPC->hAudioBuffer->Used();
			
			if(aBytesHave > pAFrm->len){
				nBytesRead = hPC->hAudioBuffer->Get(pAFrm->d,pAFrm->len);
				hPC->WriteRecorder(pAFrm->d,nBytesRead,0,0,ts);
			}else{
				memset(pAFrm->d,0,pAFrm->len);
				hPC->WriteRecorder(pAFrm->d,pAFrm->len,0,0,ts);
			}
#endif
		}
	}

	free(pAFrm); pAFrm = NULL;
	free(pVFrm); pVFrm = NULL;

	Log3("stop recording process done.");
	
	return NULL;
}


static void * MediaExitProcess(
	void * hVoid
){    
	SET_THREAD_NAME("CheckExitProcess");

	pthread_detach(pthread_self());
	
	CPPPPChannel * hPC = (CPPPPChannel*)hVoid;

	Log3("stop media process on device by avapi cmd ");
	if(hPC->SendAVAPICloseIOCtrl() < 0){
		Log3("SendAVAPICloseIOCtrl Error, PPPPClose !!!");
		hPC->PPPPClose();
	}
	
	Log3("stop video process.");
    if((long)hPC->videoRecvThread != -1) pthread_join(hPC->videoRecvThread,NULL);
	if((long)hPC->videoPlayThread != -1) pthread_join(hPC->videoPlayThread,NULL);
	
	Log3("stop audio process.");
    if((long)hPC->audioRecvThread != -1) pthread_join(hPC->audioRecvThread,NULL);
  	if((long)hPC->audioSendThread != -1) pthread_join(hPC->audioSendThread,NULL);

	Log3("stop recording process.");
	hPC->CloseRecorder();

	Log3("stop media process done.");
	hPC->videoRecvThread = (pthread_t)-1;
	hPC->videoPlayThread = (pthread_t)-1;
	hPC->audioRecvThread = (pthread_t)-1;
	hPC->audioSendThread = (pthread_t)-1;

	GET_LOCK(&hPC->SessionStatusLock);
	hPC->SessionStatus = STATUS_SESSION_IDLE;
	Log3("SESSION STATUS:IDLE");
	PUT_LOCK(&hPC->SessionStatusLock);
	
	Log3("CloseMediaStreams Process finished ... ");

	pthread_exit(0);
}

CPPPPChannel::CPPPPChannel(
	char *DID,
	char *user, 
	char *pwd,
	char *servser
	){ 
    memset(szDID, 0, sizeof(szDID));
    strcpy(szDID, DID);

    memset(szUsr, 0, sizeof(szUsr));
    strcpy(szUsr, user);

    memset(szPwd, 0, sizeof(szPwd));
    strcpy(szPwd, pwd);    

    memset(szServer, 0, sizeof(szServer));
    strcpy(szServer, servser);
	
	sessionType=0;   
	sessionID = -1;

	iocmdRecving = 0;
	
    videoPlaying = 0;

	audioPlaying = 0;
	voiceEnabled = 0;
	audioEnabled = 0;
	mediaEnabled = 0;
	speakEnabled = 1;
	
    fileRecving=0;
	fileRecvThread= (pthread_t)-1;
	mediaCoreThread = (pthread_t)-1;
	iocmdSendThread = (pthread_t)-1;
	iocmdRecvThread = (pthread_t)-1;
	videoPlayThread = (pthread_t)-1;
	videoRecvThread = (pthread_t)-1;
	audioSendThread = (pthread_t)-1;
	audioRecvThread = (pthread_t)-1;

	deviceType = -1;

	recordingExit = 0;
	avIdx = spIdx = sessionID = -1;

    SID = -1;

	AudioSaveLength = 0;
	Audio10msLength = 0;

	MW = 1920;
	MH = 1080;
	YUVSize = (MW * MH) + (MW * MH)/2;
	MWCropSize = 0;
	MHCropSize = 0;

	hAudioBuffer = new CCircleBuffer( 128 * 1024);
	hSoundBuffer = new CCircleBuffer( 128 * 1024);
	hVideoBuffer = new CCircleBuffer(4096 * 1024);
	hIOCmdBuffer = new CCircleBuffer(COMMAND_BUFFER_SIZE);
	
	INT_LOCK(&CaptureLock);
	INT_LOCK(&DisplayLock);
	INT_LOCK(&SndplayLock);
	
	INT_LOCK(&SessionStatusLock);
	
	GET_LOCK(&SessionStatusLock);
	SessionStatus = STATUS_SESSION_DIED;
	Log3("SESSION STATUS:DIED");
	PUT_LOCK(&SessionStatusLock);

	hDec = new CH264Decoder();

}

CPPPPChannel::~CPPPPChannel()
{
    Log3("start free class pppp channel:[0] start.");
	
    Log3("start free class pppp channel:[1] close p2p connection and threads."); 
	
	Close();

	Log3("start free class pppp channel:[2] free buffer.");

	delete(hAudioBuffer);
	delete(hVideoBuffer);
	delete(hSoundBuffer);
	delete(hIOCmdBuffer);
	
/*
	hIOCmdBuffer = 
	hAudioBuffer = 
	hSoundBuffer = 
	hVideoBuffer = NULL; 
*/
    Log3("start free class pppp channel:[3] free lock.");

	DEL_LOCK(&CaptureLock);
	DEL_LOCK(&DisplayLock);
	DEL_LOCK(&SndplayLock);
	DEL_LOCK(&SessionStatusLock);

	if(hDec) delete(hDec); hDec = NULL;
    
    Log3("start free class pppp channel:[4] close done.");
}

void CPPPPChannel::MsgNotify(
    JNIEnv * hEnv,
    int MsgType,
    int Param
){
	Log3("get callback lock from MsgNotify");
    GET_LOCK( &g_CallbackContextLock );
    if(g_CallBack_Handle != NULL && g_CallBack_ConnectionNotify!= NULL){
        jstring jsDID = ((JNIEnv *)hEnv)->NewStringUTF(szDID);
        ((JNIEnv *)hEnv)->CallVoidMethod(g_CallBack_Handle, g_CallBack_ConnectionNotify, jsDID, MsgType, Param);
        ((JNIEnv *)hEnv)->DeleteLocalRef(jsDID);
    }
	PUT_LOCK( &g_CallbackContextLock );
	Log3("put callback lock from MsgNotify");
}

int CPPPPChannel::StartFileRecvThread()
{
    if(fileRecvThread != ((pthread_t)-1)
		|| fileRecving!=0){
        return 0;
    }

    fileRecving = 1;

	Log3("FileRecvProcess ======================> Begin!");
	if (0!=pthread_create(&fileRecvThread, NULL, FileRecvProcess, (void*)this)){
		fileRecvThread = (pthread_t)-1;
		fileRecving=0;
		return -1;
	}
	
	return 0;
}

int CPPPPChannel::ExtAckCmdHeaderBuild(exSendCmd_t *extCmdHead,unsigned char sessionHandle,short cmd,short len){
	int ackValue=0;
	if (szTicket[0]==0x00 && cmd!=CMD_SYSTEM_USER_CHK){
		ackValue=APP_ERR_UNAUTH;
		}

    extCmdHead->cmdhead.startcode = CMD_START_CODE;
	extCmdHead->cmdhead.cmd=cmd;
	extCmdHead->cmdhead.len=len+4;//��4Ϊticket����
	memcpy(extCmdHead->ticket,&ackValue,4);
	
	char *gwChr=(char *)&extCmdHead->cmdhead.version;
	gwChr[0]=sessionHandle;

	return 0;
	}

int CPPPPChannel::ExtCmdHeaderBuild(exSendCmd_t *extCmdHead,unsigned char gwChannel,short cmd,short len){
	if (len>1000){
		return -1005;
	}
	Log3("Check szTicket[0] value --------------> %04x",szTicket[0]);
	if (szTicket[0]==0x00 && cmd!=CMD_SYSTEM_USER_CHK){
		return -1004;;
	}
    extCmdHead->cmdhead.startcode = CMD_START_CODE;
	extCmdHead->cmdhead.cmd=cmd;
	extCmdHead->cmdhead.len=len+4;//��4Ϊticket����
	memcpy(extCmdHead->ticket,szTicket,4);
	
	char *gwChr=(char *)&extCmdHead->cmdhead.version;
	gwChr[0]=gwChannel;

	return 0;
	}

int CPPPPChannel::SetSystemParams_yunni(
	int gwChannel,
	int cmdType, 
	char *cmdContent, 
	int cmdLen
	){
	//F_LOG;
    int nRet=0,extCmdLen=sizeof(exSendCmd_t);
	unsigned char cGwChannel=0x00;
    exSendCmd_t extCmdHead;
	char buf[1024]={0,};
	
	//������Ҫ������������
	switch(cmdType){
		case CMD_DATACTRL_AUDIO_START:
			audioEnabled=1;
			break;
		case CMD_DATACTRL_AUDIO_STOP:
			audioEnabled=0;
			break;
		case CMD_DATACTRL_TALK_START:
			voiceEnabled=1;
			break;
		case CMD_DATACTRL_TALK_STOP:
			voiceEnabled=0;
			break;
		case CMD_DATACTRL_PLAYLIVE_START:	
			videoPlayEnabled=1;		
			m_bPlayStreamOK = 0; 	
			break;
		case CMD_DATACTRL_PLAYLIVE_STOP:
			videoPlayEnabled=0; 
			break;
		case CMD_DATACTRL_PLAYBACK_START:
			if(sessionID < 0) return -1;
			GET_LOCK(&SessionStatusLock);
			if(SessionStatus != STATUS_SESSION_IDLE){
				Log3("session status is:[%d],can't start living stream.\n",SessionStatus);
				PUT_LOCK(&SessionStatusLock);
				return -1;
			}
			SessionStatus = STATUS_SESSION_PLAYING;
			Log3("SESSION STATUS:PLAY");
			PUT_LOCK(&SessionStatusLock);
			mediaEnabled = 1;
			StartVideoChannel();
		 	break;
		case CMD_DATACTRL_PLAYBACK_STOP:
			
			mediaEnabled = 0;
			videoPlaying = 0;   
			Log3("stop video process.");
			
		//	if(videoRecvThread != -1) pthread_join(videoRecvThread,NULL);
		//	if(videoPlayThread != -1) pthread_join(videoPlayThread,NULL);

			GET_LOCK(&SessionStatusLock);
				SessionStatus = STATUS_SESSION_IDLE;
			PUT_LOCK(&SessionStatusLock);

			break;
			
		}


	cGwChannel=gwChannel;
	memset(&extCmdHead,0x00,extCmdLen);
	
	nRet=ExtCmdHeaderBuild(&extCmdHead,cGwChannel,cmdType,cmdLen);
	if (nRet<0){
		Log3("ret[%d:%d]---[%04x]",sessionID,nRet,cmdType);
		return nRet;
	}
		
	memcpy(buf,&extCmdHead,extCmdLen);;
	if (NULL!=cmdContent){
		memcpy(buf+extCmdLen,cmdContent,cmdLen);
	}

	nRet=PPPP_IndeedWrite(sessionID,P2P_CHANNEL_CMMND,buf,cmdLen+extCmdLen);
	
	return 0;
}

int  CPPPPChannel::p2p_ConnectProc(JNIEnv *env)
{
   // F_LOG;
	Log3("PPPP_Connect begin...%s\n svrstr:%s\n", szDID,szServer);
	int msgType=MSG_NOTIFY_TYPE_CONNECT_PPPP_STATUS;
	int msgLen=4;
	int iMsgContent;
	char *svrStr=NULL;
	sessionID = PPPP_Connect(szDID,1,0,&svrStr);
	Log3("PPPP_Connect begin...szDID:%s sessionID:%d svrstr:%s\n", szDID,sessionID,szServer);
	if(sessionID < 0){
		Log3("PPPP_Connect failed.. %s return: %d", szDID, sessionID);  
		if (sessionID==ERROR_PPPP_SERVER_CHANGED){
			Log3("server changed, svr:%s\n",(svrStr==NULL)?"null":svrStr);
			if (svrStr!=NULL)free(svrStr);
			return -1;
			}
		
		switch(sessionID)
		{
			case ERROR_PPPP_TIME_OUT:
				iMsgContent=PPPP_STATUS_SESSIONSETUP_TIMEOUT;
				break;
			case ERROR_PPPP_INVALID_ID:
			case ERROR_PPPP_INVALID_PREFIX:
				iMsgContent=PPPP_STATUS_ID_INVALID;
				break;
			case ERROR_PPPP_ID_OUT_OF_DATE:
				iMsgContent=PPPP_STATUS_ID_OUTOFDATE;
				break;
			case ERROR_PPPP_DEVICE_MAX_SESSION:
				iMsgContent=PPPP_STATUS_LOCAL_SESSION_OVERLIMIT;
				break;
			case ERROR_PPPP_DEVICE_NOT_ONLINE:
				iMsgContent=PPPP_STATUS_DEVICE_OFFLINE;
				break;
			case ERROR_PPPP_NOT_INITIALIZED:
				iMsgContent=PPPP_STATUS_NOT_INITIALIZED;
				break;
			case ERROR_PPPP_NO_RELAY_SERVER_AVAILABLE:
				iMsgContent=PPPP_STATUS_UNKNOWN_ERROR;
				break;
			case ERROR_PPPP_MAX_SESSION:
				iMsgContent=PPPP_STATUS_REMOTE_SESSION_OVERLIMIT;
				break;
			case ERROR_PPPP_UDP_PORT_BIND_FAILED:
			case ERROR_PPPP_NO_NETCARD:
				iMsgContent=PPPP_STATUS_DEVICE_OFFLINE;
				break;
			case ERROR_PPPP_USER_CONNECT_BREAK: 
				iMsgContent=PPPP_STATUS_DEVICE_OFFLINE;
				break;

			default:
				iMsgContent=PPPP_STATUS_UNKNOWN_ERROR;
				break;
		}
		Log3("sessionID[%d]MsgNotify:msgType=%d,iMsgContent=%d\n",sessionID,msgType,iMsgContent);
		MsgNotify(env,msgType,iMsgContent);
		Log3("UILayer status set done");
		return -1;
	}
	

	/* ��ȡpppp session ��Ϣ */
	st_PPPP_Session1 SInfo;
	if(PPPP_Check(sessionID, &SInfo) != ERROR_PPPP_SUCCESSFUL){
	   return -1;
	}
	//������֤��Ϣ

	Log3("user:%s, pass:%s",szUsr,szPwd);
	
	if(strcmp(szUsr,"") == 0){
		//�״�����Ĭ��δ��½״̬
		iMsgContent=PPPP_STATUS_USER_NOT_LOGIN;
		MsgNotify(env,msgType,iMsgContent);
		return -2;
	}
	
	user_t userParam;
	strncpy(userParam.name,szUsr,32);
	strncpy(userParam.pass,szPwd,128);

	//SetSystemParams_yunni(0xff,CMD_SYSTEM_USER_CHK,(char *)&userParam,sizeof(user_t));
	char * Cgi = (char *)malloc(sizeof(userParam.name)+sizeof(userParam.pass));
	sprintf(Cgi,"name=%.32s&password=%.128s&",userParam.name,userParam.pass);
	Log3("sprintf Cgi result is : %s ",Cgi);
	SendCmds(sessionID,CMD_SYSTEM_USER_CHK,Cgi,(void *)this);

	return 0;
}

void CPPPPChannel::ConnectUserCheckAcK(
	JNIEnv * env,
	char *pbuf, 
	unsigned short len
	){
	int ret=0;
	int iMsgContent=0;
	int iMsgLen=4;
	int iMsgType=MSG_NOTIFY_TYPE_PPPP_STATUS;

	if (pbuf == NULL||len<4){
		iMsgContent=PPPP_STATUS_USER_INVALID;
		MsgNotify(env,iMsgType,iMsgContent);
		return;
	}
	
	//������Ӧ
	if (len<4+sizeof(st_usrChkRet)){
		memcpy(&ret,pbuf,4);
		
	Log3("Check User Connection status : [%d] \n",ret);
	switch(ret){
		case APP_ERR_OK:
			iMsgContent=PPPP_STATUS_USER_INVALID;
			break;
		case APP_ERR_UNAUTH:
		case APP_ERR_INVALID_PARAM:
		case APP_ERR_CMDEXCUTE_FAILED:
		case APP_ERR_NONE_RESULT:
		case APP_ERR_UNKNOWN:
		case APP_ERR_NO_PRIVILEGE:
			iMsgContent=PPPP_STATUS_USER_INVALID;
			break;
		}
		MsgNotify(env,iMsgType,iMsgContent);
		return;
	}
	
    st_usrChkRet *p = (st_usrChkRet *)(pbuf+4);

	Log3("privilege:%d,ticket:%s",p->privilege,p->ticket);
	if (p->privilege > 0){
		 strncpy(szTicket,p->ticket,4);
		 iMsgContent=PPPP_STATUS_CONNECTED ;
		 MsgNotify(env,iMsgType,iMsgContent);
		 Log3("Connect success--------%s\n",szDID);
		 return;
	}
	else{
		iMsgContent=PPPP_STATUS_USER_INVALID;
		MsgNotify(env,iMsgType,iMsgContent);
		return;
	}
}

int CPPPPChannel::PPPPClose()
{
	Log3("close connection by did:[%s] called.",szDID);

// yunni p2p 
	int Ret = 0;
	if(sessionID >= 0){ 	   
		Ret = PPPP_ForceClose(sessionID);
	}else{
	 	Ret = PPPP_Break(szDID);
	}
	Log3("PPPP Close status value : [%d]  \n",Ret);

	avIdx = spIdx = speakerChannel = SID = sessionID = -1;

	return 0;
}

int CPPPPChannel::Start(char * usr,char * pwd,char * server)
{   
    Log3("start pppp connection to device with uuid:[%s].\n",szDID);

	GET_LOCK(&SessionStatusLock);
	if(SessionStatus != STATUS_SESSION_DIED){
		Log3("session status is:[%d],can't start pppp connection.\n",SessionStatus);
		PUT_LOCK(&SessionStatusLock);
		return -1;
	}
	SessionStatus = STATUS_SESSION_START;
	Log3("SESSION STATUS:START");
	PUT_LOCK(&SessionStatusLock);

	StartMediaChannel();
	
    return 0;
}

void CPPPPChannel::Close()
{
	GET_LOCK(&SessionStatusLock);
	SessionStatus = STATUS_SESSION_CLOSE;
	Log3("SESSION STATUS:CLOSE");
	PUT_LOCK(&SessionStatusLock);
	
	mediaLinking = 0;
	
	if(mediaCoreThread != (pthread_t)-1){
		pthread_join(mediaCoreThread,NULL);
		mediaCoreThread = (pthread_t)-1;
	}else{
		GET_LOCK(&SessionStatusLock);
		SessionStatus = STATUS_SESSION_DIED;
		Log3("SESSION STATUS:DIED");
		PUT_LOCK(&SessionStatusLock);
	}

	Log3("stop media core thread.");
	
}

int CPPPPChannel::StartMediaChannel()
{

	mediaLinking = 1;
	pthread_create(&mediaCoreThread,NULL,MeidaCoreProcess,(void*)this);
	
	return 1;
}

int CPPPPChannel::StartIOCmdChannel()
{
	iocmdSending = 1;
    iocmdRecving = 1;
	
	pthread_create(&iocmdSendThread,NULL,IOCmdSendProcess,(void*)this);
    pthread_create(&iocmdRecvThread,NULL,IOCmdRecvProcess,(void*)this);
	
    return 1;
}

int CPPPPChannel::StartAudioChannel()
{
   	audioPlaying = 1;
	audioEnabled = 1;
	voiceEnabled = 1;
	
	pthread_create(&audioRecvThread,NULL,AudioRecvProcess,(void*)this);
	pthread_create(&audioSendThread,NULL,AudioSendProcess,(void*)this);
	
    return 1;
	
}

int CPPPPChannel::StartVideoChannel()
{
    videoPlaying = 1;
	
	int Err = 0;
	Err = pthread_create(&videoRecvThread,NULL,VideoRecvProcess,(void*)this);
	if(Err != 0){
		Log3("create video recv process failed.");
		goto jumperr;
	}

	Err = pthread_create(&videoPlayThread,NULL,VideoPlayProcess,(void*)this);
	if(Err != 0){
		Log3("create video play process failed.");
		goto jumperr;
	}

    return 1;

jumperr:

	videoPlaying = 0;

	return 0;
}

int CPPPPChannel::SendAVAPIStartIOCtrl(){
	return 0;
}

int CPPPPChannel::SendAVAPICloseIOCtrl(){
	int ret = 0;
	SendCmds(sessionID,CMD_DATACTRL_PLAYLIVE_STOP,NULL,(void *)this);
	if(ret < 0){
		Log3("SetSystemParams_yunni failed with err:[%d],sessionID:[%d].",ret,sessionID);
		return ret;
	}
	

	SendCmds(sessionID,CMD_DATACTRL_AUDIO_STOP,NULL,(void *
	if(ret < 0){
		Log3("SetSystemParams_yunni failed with err:[%d],sessionID:[%d].",ret,sessionID);
		return ret;
	}

	SendCmds(sessionID,CMD_DATACTRL_TALK_STOP,NULL,(void *)this);
	if(ret < 0){
		Log3("SetSystemParams_yunni failed with err:[%d],sessionID:[%d].",ret,sessionID);
		return ret;
	}

	return ret;
}

int CPPPPChannel::CloseWholeThreads()
{
    //F_LOG;
    if(!iocmdSending 
		|| !iocmdRecving 
		|| !mediaEnabled 
		|| !audioPlaying 
		|| !videoPlaying
		|| !fileRecving
	){
		iocmdSending = 0;
    	iocmdRecving = 0;
		mediaEnabled = 0;
    	audioPlaying = 0;
		videoPlaying = 0;
		fileRecving = 0;
	}

	recordingExit = 0;
	Log3("stop iocmd process.");
	
    if(iocmdSendThread != (pthread_t)-1) pthread_join(iocmdSendThread,NULL);
	iocmdSendThread = (pthread_t)-1;
	if(iocmdRecvThread != (pthread_t)-1) pthread_join(iocmdRecvThread,NULL);
	iocmdRecvThread = (pthread_t)-1;
	
	Log3("stop video process.");
    if(videoRecvThread != (pthread_t)-1) pthread_join(videoRecvThread,NULL);
	if(videoPlayThread != (pthread_t)-1) pthread_join(videoPlayThread,NULL);
	videoRecvThread = (pthread_t)-1;
	videoPlayThread = (pthread_t)-1;

	Log3("stop audio process.");
    if(audioRecvThread != (pthread_t)-1) pthread_join(audioRecvThread,NULL);
  	if(audioSendThread != (pthread_t)-1) pthread_join(audioSendThread,NULL);
	audioRecvThread = (pthread_t)-1;
	audioSendThread = (pthread_t)-1;
	
	Log3("stop FileRecv process.");
	if(fileRecvThread!= (pthread_t)-1) pthread_join(fileRecvThread,NULL);
	fileRecvThread = (pthread_t)-1;
		
	Log3("stop recording process.");
	CloseRecorder();
	Log3("stop media thread done.");

	return 0;

}

int CPPPPChannel::CloseMediaStreams(
){
	GET_LOCK(&SessionStatusLock);
	if(SessionStatus != STATUS_SESSION_PLAYING){
		Log3("session status is:[%d],can't close living stream.\n",SessionStatus);
		PUT_LOCK(&SessionStatusLock);
		return -1;
	}
	SessionStatus = STATUS_SESSION_IDLE;
	Log3("SESSION STATUS:IDLE");
	PUT_LOCK(&SessionStatusLock);
	
	mediaEnabled = 0;
	videoPlaying = 0;
	audioPlaying = 0;

	pthread_t tid;
	pthread_create(&tid,NULL,MediaExitProcess,(void*)this);
	
	return 0;
}
	
int CPPPPChannel::StartMediaStreams(
	const char * url,
	int audio_sample_rate,
	int audio_channel,
	int audio_recv_codec,
	int audio_send_codec,
	int video_recv_codec,
	int video_w_crop,
	int video_h_crop
){    
   // F_LOG;	
	int ret = 0;
	if(sessionID < 0) {
		Log3("invliad session...");
		return -1;
	}
	
	GET_LOCK(&SessionStatusLock);
	if(SessionStatus != STATUS_SESSION_IDLE){
		Log3("session status is:[%d],can't start living stream.\n",SessionStatus);
		ret = SessionStatus;
		PUT_LOCK(&SessionStatusLock);
		goto jumperr;
	}
	SessionStatus = STATUS_SESSION_PLAYING;
	Log3("SESSION STATUS:PLAY");
	PUT_LOCK(&SessionStatusLock);
	
	Log3("media stream start here.");

	mediaEnabled = 1;

	// pppp://usr:pwd@livingstream:[channel id]
	// pppp://usr:pwd@replay/mnt/sdcard/replay/file
	
	memset(szURL,0,sizeof(szURL));
	
	AudioSampleRate = audio_sample_rate;
	AudioChannel = audio_channel;

	// only support channel mono, 16bit, 8KHz or 16KHz
	//   2 is come from 16bits/8bits = 2bytes
	// 100 is come from 10ms/1000ms
	Audio10msLength = audio_sample_rate * audio_channel * 2  / 100;

	AudioRecvFormat = audio_recv_codec;
	AudioSendFormat = audio_send_codec;
	VideoRecvFormat = video_recv_codec;

	MHCropSize = video_h_crop;
	MWCropSize = video_w_crop;

	Log3(
		"audio format info:[\n"
		"samplerate = %d\n"
		"channel = %d\n"
		"length in 10 ms is %d\n"
		"]\n",
		AudioSampleRate,AudioChannel,Audio10msLength);

	if(url != NULL){
		memcpy(szURL,url,strlen(url));
	}
	
	//������Ƶ����ָ��
	Log3("Send instruction to Driver !");
	SendCmds(sessionID,CMD_DATACTRL_PLAYLIVE_START,NULL,(void *)this);
	//������Ƶ����ָ��
	SendCmds(sessionID,CMD_DATACTRL_AUDIO_START,NULL,(void *)this);
	//���ͶԽ�����ָ��
	SendCmds(sessionID,CMD_DATACTRL_TALK_START,NULL,(void *)this);
	
	Log3("channel init video proc.");
    StartVideoChannel();
	Log3("channel init audio proc.");
	StartAudioChannel();

	return 0;
	
jumperr:

	return ret;
}

int CPPPPChannel::SetSystemParams(int type,char * msg,int len)
{
	char AppCmds[2048] = {0};
	APP_CMD_HEAD * hCmds = (APP_CMD_HEAD*)AppCmds;
	
	hCmds->Magic = 0x78787878;
	hCmds->AppType = type;
	hCmds->CgiLens = len > 0 ? len : (int)strlen(msg);
	
	memcpy(hCmds->CgiData,msg,hCmds->CgiLens);
	
	return hIOCmdBuffer->Put(AppCmds,sizeof(APP_CMD_HEAD) + hCmds->CgiLens);
}

void CPPPPChannel::AlarmNotifyDoorBell(JNIEnv* hEnv,char *did, char *type, char *time )
{

	if( g_CallBack_Handle != NULL && g_CallBack_AlarmNotifyDoorBell != NULL )
	{
		jstring jdid	   = hEnv->NewStringUTF( szDID );
		jstring resultDid  = hEnv->NewStringUTF( did );
		jstring resultType = hEnv->NewStringUTF( type );
		jstring resultTime = hEnv->NewStringUTF( time );

		Log3("device msg push to %s with type:[%s] time:[%s].",did,type,time);

		hEnv->CallVoidMethod( g_CallBack_Handle, g_CallBack_AlarmNotifyDoorBell, jdid, resultDid, resultType, resultTime );

		hEnv->DeleteLocalRef( jdid );
		hEnv->DeleteLocalRef( resultDid );
		hEnv->DeleteLocalRef( resultType );
		hEnv->DeleteLocalRef( resultTime );
	}
}

int CPPPPChannel::StartRecorder(
	int 		W,			// \BF\ED\B6\C8
	int 		H,			// \B8ß¶\C8
	int 		FPS,		// Ö¡\C2\CA
	char *		SavePath	// 
){
	if(W == 0 || H == 0){
		W = this->MW;
		H = this->MH;
	}

	int Err = 0;

	if(FPS == 0){
		FPS = this->FPS;
    }else{
        this->FPS = FPS;
    }

	GET_LOCK(&CaptureLock);

	if(StartRecording(SavePath,FPS,W,H,this->AudioSampleRate,&AudioSaveLength) < 0){
		Log3("start recording with muxing failed.\n");
		goto jumperr;
	}

	recordingExit = 1;

	Err = pthread_create(
		&recordingThread,
		NULL,
		RecordingProcess,
		this);

	if(Err != 0){
		Log3("create av recording process failed.");
		CloseRecording();
		goto jumperr;
	}

	Log3("start recording process done.");

	PUT_LOCK(&CaptureLock);

	return  0;
	
jumperr:
	PUT_LOCK(&CaptureLock);

	return -1;
}

int CPPPPChannel::WriteRecorder(
	const char * 	FrameData,
	int				FrameSize,
	int				FrameCode, 	// audio or video codec [0|1]
	int				FrameType,	// keyframe or not [0|1]
	long long		Timestamp
){
//	Log3("frame write code and size:[%d][%d].\n",FrameCode,FrameSize);
	return WriteRecordingFrames((void*)FrameData,FrameSize,FrameCode,FrameType,Timestamp);
}

int CPPPPChannel::CloseRecorder(){

	GET_LOCK(&CaptureLock);
	
	Log3("wait avi record process exit.");
	recordingExit = 0;
	if((long)recordingThread != -1){
		pthread_join(recordingThread,NULL);
		recordingThread = (pthread_t)-1;
	}
	Log3("avi record process exit done.");

	CloseRecording();

	PUT_LOCK(&CaptureLock);

	return 0;
}
