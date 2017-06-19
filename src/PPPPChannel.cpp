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

    short *hFrame = p->recordBuffer+(p->iBufferIndex*AEC_CACHE_LEN/2);
	
	hPC->hAudioGetList->Write(hFrame,GetAudioTime());

	(*p->recorderBufferQueue)->Enqueue(p->recorderBufferQueue,(char*)hFrame,AEC_CACHE_LEN);

    p->iBufferIndex = (p->iBufferIndex+1)%CBC_CACHE_NUM;
}

// this callback handler is called every time a buffer finishes playing
static void playerCallback(
	SLAndroidSimpleBufferQueueItf bq, 
	void *context
){
	OPENXL_STREAM * p = (OPENXL_STREAM *)context;
	CPPPPChannel * hPC = (CPPPPChannel *)p->context;

    short *hFrame = p->outputBuffer+(p->oBufferIndex*AEC_CACHE_LEN/2);

	hPC->hAudioPutList->Write((short*)hFrame,GetAudioTime());
	
	int stocksize = hPC->hSoundBuffer->GetStock();

	if(stocksize >= AEC_CACHE_LEN){
//      Log3("read audio data from sound buffer with lens:[%d]",stocksize);
		hPC->hSoundBuffer->Read((char*)hFrame,AEC_CACHE_LEN);
	}else{
        memset((char*)hFrame,0,AEC_CACHE_LEN);
	}

	(*p->bqPlayerBufferQueue)->Enqueue(p->bqPlayerBufferQueue,(char*)hFrame,AEC_CACHE_LEN);

    p->oBufferIndex = (p->oBufferIndex+1)%CBC_CACHE_NUM;
}

#else

static void recordCallback(char * data, int lens, void *context){
    if(lens > AEC_CACHE_LEN){
        Log3("audio record sample is too large:[%d].",lens);
        return;
    }
    
    OPENXL_STREAM * p = (OPENXL_STREAM *)context;
    CPPPPChannel * hPC = (CPPPPChannel *)p->context;
    
    char * pr = (char*)p->recordBuffer;
    memcpy(pr + p->recordSize,data,lens);
    p->recordSize += lens;
    
    if(p->recordSize >= AEC_CACHE_LEN){
        hPC->hAudioGetList->Write((short*)pr,GetAudioTime());
        p->recordSize -= AEC_CACHE_LEN;
        memcpy(pr,pr + AEC_CACHE_LEN,p->recordSize);
    }
}

static void playerCallback(char * data, int lens, void *context){
    if(lens > AEC_CACHE_LEN){
        Log3("audio output sample is too large:[%d].",lens);
        return;
    }
    
    OPENXL_STREAM * p = (OPENXL_STREAM *)context;
    CPPPPChannel * hPC = (CPPPPChannel *)p->context;
    
    int stocksize = hPC->hSoundBuffer->GetStock();
    
    if(stocksize >= lens){
        hPC->hSoundBuffer->Read((char*)data,lens);
    }else{
        memset((char*)data,0,lens);
    }
    
    char * po = (char*)p->outputBuffer;
    memcpy(po + p->outputSize,data,lens);
    p->outputSize += lens;
    
    if(p->outputSize >= AEC_CACHE_LEN){
        hPC->hAudioPutList->Write((short*)po,GetAudioTime());
        p->outputSize -= AEC_CACHE_LEN;
        memcpy(po,
               po + AEC_CACHE_LEN,
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
	GET_LOCK(&hPC->DataWriteThreadLock);
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
	PUT_LOCK(&hPC->DataWriteThreadLock);
	return hPC->m_nNmb;
}

int FileWrite(JNIEnv * env,char *magic,void *data,unsigned int len,void * hVoid){
	char MsgStr[128] = {0};
	CPPPPChannel * hPC = (CPPPPChannel *)hVoid;
	GET_LOCK(&hPC->DataWriteThreadLock);
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
		
		//回调下载进度
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
	PUT_LOCK(&hPC->DataWriteThreadLock);
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

//获取SD卡文件数据线程,注:功能已实现，但是存在问题!!
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
	
	GET_LOCK(&hPC->fileRecvThreadLock);
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
		if (dataHead.type==0){//文件属性
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
	
	PUT_LOCK(&hPC->fileRecvThreadLock); 
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
connect:
    hPC->MsgNotify(hEnv, MSG_NOTIFY_TYPE_PPPP_STATUS, PPPP_STATUS_CONNECTING);
    
	hPC->speakerChannel = -1;
	hPC->avstremChannel =  0;
	
	Log3("[1:%s]=====>start get free session id for client connection.",
         hPC->szDID);
	//yunni p2p
	Log3("p2p_ConnectProc Begin----------------\n");
    hPC->p2p_ConnectProc(hEnv);   

	Log3("[2:%s]=====>channel init command proc here.",hPC->szDID);
	hPC->StartIOCmdChannel();
	
	Log3("[3:%s]=====>channel init fileRecv proc done.",hPC->szDID);
	hPC->StartFileRecvThread();
	
	GET_LOCK(&hPC->SessionStatusLock);
	hPC->SessionStatus = STATUS_SESSION_IDLE;
	PUT_LOCK(&hPC->SessionStatusLock);
	int ret = 0;
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

#ifdef PLATFORM_ANDROID
	if(isAttached) g_JavaVM->DetachCurrentThread();
#endif

    if(!hPC->CloseWholeThreads()){   // make sure other service thread all exit.
    	Log3("MediaCoreProcess hPC->PPPPClose()........2");
		hPC->PPPPClose();
	} 	

	GET_LOCK(&hPC->SessionStatusLock);
	hPC->SessionStatus = STATUS_SESSION_DIED;
	PUT_LOCK(&hPC->SessionStatusLock);
	
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
	
	hPC->hIOCmdBuffer->Reset();
	
    while(hPC->iocmdSending){
		if(hPC->hIOCmdBuffer == NULL){
			Log3("[X:%s]=====>Invalid hIOCmdBuffer.",hPC->szDID);
			sleep(2);
			continue;
		}

		if(hPC->hIOCmdBuffer->GetStock() >= (int)sizeof(APP_CMD_HEAD)){
			
			nBytesRead = hPC->hIOCmdBuffer->Read(hCmds,sizeof(APP_CMD_HEAD));
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
                    
                    hPC->hIOCmdBuffer->Reset();
                }

				nBytesRead = 0;
				while(nBytesRead != hCmds->CgiLens){
					
					nBytesRead = hPC->hIOCmdBuffer->Read(hCmds->CgiData,hCmds->CgiLens);

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
	
	GET_LOCK(&hPC->cmdRecvThreadLock);
	
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
		
		//hPC->ProcessCommand(hEnv,hCCH->version,hCCH->cmd,hCCH->d,hCCH->len);
		// here we process command for yunni p2p
		// ...
		Log3("ProcessCommand: hand instructions from Camera [%04x] ! \n",hCCH->cmd);
		//hPC->ProcessCommand(hEnv,hCCH->version,hCCH->cmd,hCCH->d,hCCH->len);
		hPC->ProcessCommand_EX(hEnv,hCCH->version,hCCH->cmd,hCCH->d,hCCH->len);
    }

	PUT_LOCK(&hPC->cmdRecvThreadLock);
	
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
	jbyte *	   jbyte_yuv = (jbyte *)(hEnv->GetByteArrayElements(jbyteArray_yuv,0));

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
		
		memcpy(jbyte_yuv,hPC->hVideoFrame,hPC->YUVSize);

		hPC->hVideoFrame = NULL;

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
			hPC->YUVSize,
			hPC->MW,
			hPC->MH,
			time(NULL));

		PUT_LOCK( &g_CallbackContextLock );
	}

	hEnv->ReleaseByteArrayElements(jbyteArray_yuv,jbyte_yuv,0);
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
	
	CH264Decoder * hDec = new CH264Decoder();	
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
	
	GET_LOCK(&hPC->videoRecvThreadLock);
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
	
		// decode h264 frame
		if(hDec->DecoderFrame((uint8_t *)hFrm->d,hFrm->len,hPC->MW,hPC->MH,isKeyFrame) <= 0){
			Log3("decode h.264 frame failed.");
			firstKeyFrameComming = 0;
			continue;
		}

		if(hPC->avExit == 1){	
			if(hPC->hVideoBuffer->Write(hFrm,hFrm->len + sizeof(AV_HEAD)) == 0){
				Log3("recording buffer is full.may lost frame in mp4.");
			}
		}

		if(TRY_LOCK(&hPC->DisplayLock) != 0){
			continue;
		}
		
		// get h264 yuv data
		hDec->GetYUVBuffer((uint8_t*)hYUV,hPC->YUVSize);
		hPC->hVideoFrame = hYUV;

		PUT_LOCK(&hPC->DisplayLock);
	}
	
	
	GET_LOCK(&hPC->DisplayLock);
	if(hFrm)   free(hFrm); hFrm = NULL;
	if(hYUV)   free(hYUV); hYUV = NULL;
	if(hDec) delete(hDec); hDec = NULL;
    hPC->hVideoFrame = NULL;
	
#ifdef PLATFORM_ANDROID
	if(isAttached) 
		g_JavaVM->DetachCurrentThread(); 
#endif

	firstKeyFrameComming = 0;
	isKeyFrame = 0;
	
	PUT_LOCK(&hPC->DisplayLock);
	
	PUT_LOCK(&hPC->videoRecvThreadLock);
	
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

	void * hCodec = audio_dec_init(hPC->AudioRecvFormat,8000,1);
	
	if(hCodec == NULL){
		Log3("initialize audio decodec handle failed.\n");
		return NULL;
	}
	
	char Cache[2048] = {0};	
	char Codec[8192] = {0};	
	
	AV_HEAD_YUNNI * avhead =(AV_HEAD_YUNNI *)Cache;

	hPC->hAudioBuffer->Reset();
	hPC->hSoundBuffer->Reset();
	
	
	//初始化
	void * hAgc = NULL ;
	hAgc= audio_agc_init(20,2,0,255,8000);
	
	if(hAgc == NULL){
		Log3("initialize audio agc failed.\n");
		goto jumperr;
	}

	GET_LOCK(&hPC->audioRecvThreadLock); 
	
	Log3("AudioRecvProcess  come to read buff ----------> ");
	while(hPC->audioPlaying){

		ret = PPPP_IndeedRead(hPC->sessionID,P2P_CHANNEL_AUDIO, (char *)avhead, sizeof(AV_HEAD_YUNNI));
		
		if( ret < 0 ){
			Log3("Error!!!!PPPP_IndeedRead=%d\n",ret);
			break;
		}
		
		if (avhead->startcode!=AVF_STARTCODE){
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
		
		//判断audio编码格式和采样率
		if(avhead->audiocodec!= hPC->AudioRecvFormat){
			Log3("invalid packet format for audio decoder:[%02X].",avhead->audiocodec);
			audio_dec_free(hCodec);
			Log3("initialize new audio decoder here.\n");
			hCodec = audio_dec_init(avhead->audiocodec,8000,1);
			if(hCodec == NULL){
				Log3("initialize audio decodec handle for codec:[%02X] failed.",avhead->audiocodec);
				break;
			}
			Log2("initialize new audio decoder done.\n");
			hPC->AudioRecvFormat = avhead->audiocodec;
			continue;
		}
		
		if((ret = audio_dec_process(hCodec,avhead->d,avhead->len,Codec,sizeof(Codec))) < 0){
			Log3("audio decodec process run error:%d with codec:[%02X] lens:[%d].\n",
				ret,
				hPC->AudioRecvFormat,
				avhead->len
				);
			continue;
		}
	//	Log3("Check Audio RecvBuff ret================>%d",ret);
		int times = ret/160;
		for(int i = 0; i < times; i++){
			audio_agc_proc(hAgc,&Codec[160*i]);
			hPC->hAudioBuffer->Write(&Codec[160*i],160); // for audio avi record
			hPC->hSoundBuffer->Write(&Codec[160*i],160); // for audio player callback
		}

	}

	PUT_LOCK(&hPC->audioRecvThreadLock); 
	
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

	int iRet=0;
	int length=0;
	int type=0;
	unsigned int pts=0;
	int releaseFrame=0;
	char sendBuf[1024]={0,};
	int sendLen=0;
	unsigned int frameno=0;
	GET_LOCK(&hPC->audioSendThreadLock);

	void * hCodec = audio_enc_init(hPC->AudioSendFormat,8000,1);
	
	Log3("hPC->AudioSendFormat ------------> %04x ",hPC->AudioSendFormat);

	if(hCodec == NULL){
		Log3("initialize audio encodec handle failed.\n");
		return NULL;
	}
		
	#ifdef ENABLE_AEC
		void * hAEC = audio_echo_cancellation_init(3,8000);
	#endif

	#ifdef ENABLE_NSX_O
		void * hNsx = audio_nsx_init(2,8000);

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
#if 1
	hPC->hAudioPutList = new CAudioDataList(32);
	hPC->hAudioGetList = new CAudioDataList(32);
	if(hPC->hAudioPutList == NULL || hPC->hAudioGetList == NULL){
		Log3("audio data list init failed.");
		hPC->audioPlaying = 0;
	}
	
	OPENXL_STREAM * hOSL = NULL;
	hOSL = InitOpenXLStream(
		8000,1,1,hVoid,
		recordCallback,
		playerCallback
		);
	
	if(!hOSL){
		Log3("opensl init failed.");
		hPC->audioPlaying = 0;
	}
	
#endif

	char hFrame[12*AEC_CACHE_LEN] = {0};
	char hCodecFrame[12*AEC_CACHE_LEN] = {0};

	AV_HEAD_YUNNI* hAV = (AV_HEAD_YUNNI*)hFrame;
	int hdrLen = sizeof(AV_HEAD_YUNNI);
	
	char * WritePtr = hAV->d;

	int nBytesNeed = 6*AEC_CACHE_LEN;
	int nVadFrames = 0;
	
	while(hPC->audioPlaying){

		if((hPC->voiceEnabled!=1)){
			usleep(1000);
			continue;
		}
		
		if((hPC->hAudioGetList->CheckData() != 1) || (hPC->hAudioPutList->CheckData() != 1)){
			usleep(10);
			continue;
		}
		AudioData * hCapture = hPC->hAudioGetList->Read();
		AudioData * hSpeaker = hPC->hAudioPutList->Read();

		if(hCapture == NULL || hSpeaker == NULL){
            Log3("audio data lost...");
			usleep(10);
			continue;
		}
		
		#ifdef ENABLE_AEC
				short * hAecCapturePCM = hCapture->buf;
				short * hAecSpeakerPCM = hSpeaker->buf;
		
				if (audio_echo_cancellation_farend(hAEC,(char*)hAecSpeakerPCM,80) != 0){
						Log3("WebRtcAecm_BufferFarend() failed.");
				}
				
				if (audio_echo_cancellation_proc(hAEC,(char*)hAecCapturePCM,(char*)WritePtr,80) != 0){
						Log3("WebRtcAecm_Process() failed.");
				}
		#else
				memcpy(WritePtr,hCapture->buf,80*sizeof(short));
		#endif
		
		#ifdef ENABLE_NSX_O
				audio_nsx_proc(hNsx,WritePtr);
		#endif
		
		#ifdef ENABLE_VAD
		
				int logration = audio_vad_proc(hVad,WritePtr,80);
		
				if(logration < 1024){
		//			Log3("audio detect vad actived:[%d].\n",logration);
					nVadFrames ++;
				}else{
					nVadFrames = 0;
				}
		#endif
		
		hAV->len += AEC_CACHE_LEN;
		WritePtr += AEC_CACHE_LEN;
		
		if(hAV->len < nBytesNeed){
			continue;
		}
		
		#ifdef ENABLE_VAD
			if(nVadFrames > 300){
				Log3("audio detect vad actived.\n");
				hAV->len = 0;
				WritePtr = hAV->d;
#if 1
				memset(sendBuf,0,1024);
				sendLen=0;
				usleep(10000);
#endif
				continue;
			}
		#endif

		iRet= audio_enc_process(hCodec,hAV->d,hAV->len,hCodecFrame,sizeof(hCodecFrame));
		
		if(iRet < 2){
				Log3("audio encode failed with error:%d.\n",iRet);
				hAV->len = 0;
				WritePtr = hAV->d;
				continue;
		}
		
		struct timeval tv = {0,0};
		gettimeofday(&tv,NULL);		
		pts = (tv.tv_sec * 1000) + (tv.tv_usec / 1000);
		hAV->streamid = 1;
		hAV->audiocodec = hPC->AudioSendFormat;
		hAV->sectime=pts/1000000;
		hAV->militime=pts/1000;
		hAV->len = iRet;
		hAV->frameno = frameno;
		memcpy(hAV->d,hCodecFrame,hAV->len);

	//	Log3("encode size:[%d],all size:[%d]\n",hAV->len,(hAV->len)+hdrLen);
	/*
		iRet=PPPP_IndeedWrite(hPC->sessionID,P2P_CHANNEL_AUDIO,(char *)hAV,(hAV->len)+hdrLen);

		if(iRet < 0){
			Log3("PPPP_IndeedWrite ----------->Fail  ");
			break;
		}

		hAV->len = 0;
		WritePtr = hAV->d;		
	*/
		memcpy(sendBuf+sendLen,(char *)hAV,(hAV->len)+hdrLen);
		sendLen+=(hAV->len)+hdrLen;
		hAV->len = 0;
		WritePtr = hAV->d;	
		frameno++;
		if (sendLen>=1024){
			iRet=PPPP_IndeedWrite(hPC->sessionID,P2P_CHANNEL_AUDIO,sendBuf,sendLen);
			if(iRet < 0){
				Log3("PPPP_IndeedWrite ----------->Failed\n");
				break;
				}
			else if (iRet!=sendLen){
				Log3("PPPP_IndeedWrite ----------->unfinished[ret=%d,want=%d]\n",iRet,sendLen);
				}
			memset(sendBuf,0,1024);
			sendLen=0;
			}
		else{
			continue;
		}
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
	
	PUT_LOCK(&hPC->audioSendThreadLock);

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

	CPPPPChannel * hClass = (CPPPPChannel*)Ptr;
	if(hClass == NULL){
		Log2("Invalid channel class object.");
		return NULL;
	}

	long long   ts = 0;
	int nBytesRead = 0;
	int nBytesHave = 0;

	AV_HEAD * hFrm = (AV_HEAD*)malloc(128*1024);

	hClass->hAudioBuffer->Reset();
	hClass->hVideoBuffer->Reset();

	hClass->aIdx = 0;
	hClass->vIdx = 0;

	while(hClass->avExit){
		int aBytesHave = hClass->hAudioBuffer->GetStock();
		int vBytesHave = hClass->hVideoBuffer->GetStock();
		
		if(vBytesHave > (int)(sizeof(AV_HEAD))){
			nBytesRead = hClass->hVideoBuffer->Read(hFrm,sizeof(AV_HEAD));
			while((nBytesHave = hClass->hVideoBuffer->GetStock()) < hFrm->len){
				Log3("wait video recording buffer arriver size:[%d] now:[%d].",hFrm->len,nBytesHave);
				usleep(10); continue;
			}
			nBytesRead = hClass->hVideoBuffer->Read(hFrm->d,hFrm->len);
			hClass->WriteRecorder(hFrm->d,hFrm->len,1,hFrm->type,ts);

//			Log3("video frame write size:[%d].\n",hFrm->len);
		}

		if(aBytesHave >= 640){
			nBytesRead = hClass->hAudioBuffer->Read(hFrm->d,640);
			hClass->WriteRecorder(hFrm->d,nBytesRead,0,0,ts);

//			Log3("audio frame write size:[%d].\n",nBytesRead);
		}
	}

	free(hFrm); hFrm = NULL;

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
	INT_LOCK(&videoRecvThreadLock);

	audioPlaying = 0;
	INT_LOCK(&audioRecvThreadLock);
	INT_LOCK(&audioSendThreadLock);
	voiceEnabled = 0;
	audioEnabled = 0;
	mediaEnabled = 0;
	speakEnabled = 1;
	
    fileRecving=0;
	fileRecvThread= (pthread_t)-1;
	INT_LOCK(&fileRecvThreadLock);
	INT_LOCK(&DataWriteThreadLock);
	
	mediaCoreThread = (pthread_t)-1;
	iocmdSendThread = (pthread_t)-1;
	iocmdRecvThread = (pthread_t)-1;
	videoPlayThread = (pthread_t)-1;
	videoRecvThread = (pthread_t)-1;
	audioSendThread = (pthread_t)-1;
	audioRecvThread = (pthread_t)-1;

	deviceType = -1;

	avIdx = spIdx = sessionID = -1;

    SID = -1;

	hAudioStream = NULL;
	hVideoStream = NULL;
	hAVFmtOutput = NULL;
    hAVFmtContext = NULL;
    hAVCodecContext = NULL;
	hRecordFile = NULL;

	vIdx = aIdx = 0;

	MW = 1920;
	MH = 1080;
	YUVSize = (MW * MH) + (MW * MH)/2;

	hAudioBuffer = new CCircleBuf();
	hVideoBuffer = new CCircleBuf();
	hSoundBuffer = new CCircleBuf();
	hIOCmdBuffer = new CCircleBuf();
	
    if(!hIOCmdBuffer->Create(COMMAND_BUFFER_SIZE)){
		while(1){
			Log2("create iocmd procssing buffer failed.\n");
		}
	}
	
	if(!hAudioBuffer->Create(512 * 1024)){
		while(1){
			Log2("create audio recording buffer failed.\n");
		}
	}

	if(!hSoundBuffer->Create(512 * 1024)){
		while(1){
			Log2("create sound procssing buffer failed.\n");
		}
	}

	if(!hVideoBuffer->Create((3*1024*1024))){
		while(1){
			Log2("create video recording buffer failed.\n");
		}
	}

	INT_LOCK(&AviDataLock);
	INT_LOCK(&DisplayLock);
	INT_LOCK(&SndplayLock);
	
	INT_LOCK(&SessionStatusLock);
	
	GET_LOCK(&SessionStatusLock);
	SessionStatus = STATUS_SESSION_DIED;
	PUT_LOCK(&SessionStatusLock);

	INT_LOCK(&cmdRecvThreadLock);
	INT_LOCK(&audioPlayThreadLock);
	INT_LOCK(&audioSendThreadLock);
	INT_LOCK(&videoRecvThreadLock);
	INT_LOCK(&videoPlayThreadLock);

}

CPPPPChannel::~CPPPPChannel()
{
    Log3("start free class pppp channel:[0] start.");
	
    Log3("start free class pppp channel:[1] close p2p connection and threads."); 
	Close();

	hAudioBuffer->Release();
	hVideoBuffer->Release();
	//hSoundBuffer->Release();
	hIOCmdBuffer->Release();

	Log3("start free class pppp channel:[2] free buffer.");

	delete(hAudioBuffer);
	delete(hVideoBuffer);
//	delete(hSoundBuffer);
	delete(hIOCmdBuffer);
	
/*
	hIOCmdBuffer = 
	hAudioBuffer = 
	hSoundBuffer = 
	hVideoBuffer = NULL; 
*/
    Log3("start free class pppp channel:[3] free lock.");

	DEL_LOCK(&AviDataLock);
	DEL_LOCK(&DisplayLock);
	DEL_LOCK(&SndplayLock);
	DEL_LOCK(&SessionStatusLock);
    
    Log3("start free class pppp channel:[4] close done.");
}
void CPPPPChannel::MsgNotify(
    JNIEnv * hEnv,
    int MsgType,
    int Param
){
    GET_LOCK( &g_CallbackContextLock );
    if(g_CallBack_Handle != NULL && g_CallBack_ConnectionNotify!= NULL){
        jstring jsDID = ((JNIEnv *)hEnv)->NewStringUTF(szDID);
        ((JNIEnv *)hEnv)->CallVoidMethod(g_CallBack_Handle, g_CallBack_ConnectionNotify, jsDID, MsgType, Param);
        ((JNIEnv *)hEnv)->DeleteLocalRef(jsDID);
    }
	PUT_LOCK( &g_CallbackContextLock );
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
	extCmdHead->cmdhead.len=len+4;//加4为ticket长度
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
	extCmdHead->cmdhead.len=len+4;//加4为ticket长度
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
	
	Log3("1---AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA--cmd[%02x]\n",cmdType);
	//本地需要作处理的命令
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
			PUT_LOCK(&SessionStatusLock);
			mediaEnabled = 1;
			StartVideoChannel();
		 	break;
			
		}


	cGwChannel=gwChannel;
	memset(&extCmdHead,0x00,extCmdLen);
	
	nRet=ExtCmdHeaderBuild(&extCmdHead,cGwChannel,cmdType,cmdLen);
	if (nRet<0){
		Log3("ret[%d:%d]---[%04x]",sessionID,nRet,cmdType);
		return nRet;
	}
	
	Log3("3---AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA");	
	memcpy(buf,&extCmdHead,extCmdLen);;
	if (NULL!=cmdContent){
		memcpy(buf+extCmdLen,cmdContent,cmdLen);
	}
	
	Log3("4---AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA");

	nRet=PPPP_IndeedWrite(sessionID,P2P_CHANNEL_CMMND,buf,cmdLen+extCmdLen);
	
	Log3("5---AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA %d \n",nRet);
	return 0;
}

void CPPPPChannel::p2p_ConnectProc(JNIEnv *env)
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
			return;
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
		return;
	}
	

	/* 获取pppp session 信息 */
	st_PPPP_Session1 SInfo;
	if(PPPP_Check(sessionID, &SInfo) != ERROR_PPPP_SUCCESSFUL){
	   return;
	}
	//发送验证信息

	Log3("user:%s, pass:%s",szUsr,szPwd);
	
	if(strcmp(szUsr,"") == 0){
		//首次添加默认未登陆状态
		iMsgContent=PPPP_STATUS_USER_NOT_LOGIN;
		MsgNotify(env,msgType,iMsgContent);
		return;
	}
	
	user_t userParam;
	strncpy(userParam.name,szUsr,32);
	strncpy(userParam.pass,szPwd,128);

	//SetSystemParams_yunni(0xff,CMD_SYSTEM_USER_CHK,(char *)&userParam,sizeof(user_t));
	char * Cgi = (char *)malloc(sizeof(userParam.name)+sizeof(userParam.pass));
	sprintf(Cgi,"name=%.32s&password=%.128s&",userParam.name,userParam.pass);
	Log3("sprintf Cgi result is : %s ",Cgi);
	SendCmds(sessionID,CMD_SYSTEM_USER_CHK,Cgi,(void *)this);
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
	
	//处理回应
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
void CPPPPChannel::ProcessCommand_EX(JNIEnv * env,int gwChannel,int cmd, char *pbuf, int len){
	
	Log3("cmd=%04x,len=%d",cmd,len);
	
	if(cmd == CMD_SYSTEM_USER_CHK){
		ConnectUserCheckAcK(env,pbuf, len);
		Log3("ConnectUserCheckAcK,len=%d",len);
		return;
	}
	
	GET_LOCK(&g_CallbackContextLock);
    jbyteArray jBuff=NULL;
	jstring jdid = NULL;
    if(g_CallBack_Handle != NULL)
    {     
        jdid = env->NewStringUTF(szDID);
	#ifdef PLATFORM_ANDROID
		jBuff = env->NewByteArray(len);	
		env->SetByteArrayRegion(jBuff, 0,len,(jbyte*)pbuf);
		
		env->CallVoidMethod(g_CallBack_Handle, g_CallBack_CmdRecv, jdid, sessionID,cmd,jBuff, len);

		env->DeleteLocalRef(jBuff);
	#else
		env->CallVoidMethod(g_CallBack_Handle, g_CallBack_CmdRecv, jdid, sessionID,cmd,pbuf, len);
	#endif
	
        env->DeleteLocalRef(jdid);
    }
	
	PUT_LOCK(&g_CallbackContextLock);
	
	return;

}

void CPPPPChannel::ProcessCommand(JNIEnv * env,int gwChannel,int cmd, char *pbuf, int len)
{
	//Log2("cmd=%04x,len=%d",cmd,len);
	char MsgStr[4096] = {0};
	if(cmd == CMD_SYSTEM_USER_CHK){
		ConnectUserCheckAcK(env,pbuf, len);
		Log3("ConnectUserCheckAcK,len=%d",len);
		return;
		
	}

	else if(cmd == CMD_NOTIFICATION || cmd == CMD_ALARM_NOTIFICATION){
		
		xqAlarm_t * xqALARM_T = (xqAlarm_t *)pbuf;
		
		char sTIME[64] = {0};
		char sTYPE[16] = {0};
		
		const char * wday[] = {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};
		struct tm stm;
		struct tm * p = &stm;
		time_t t;
		time(&t);
		p = localtime_r(&t,&stm);				
		sprintf(sTIME,
				"%04d-%02d-%02d-%s-%02d-%02d-%02d",
				1900+p->tm_year,
				p->tm_mon + 1,
				p->tm_mday,
				wday[p->tm_wday],
				p->tm_hour,
				p->tm_min,
				p->tm_sec);
		sprintf(sTYPE,"%d",xqALARM_T->alarmType);
		
		AlarmNotifyDoorBell(env,szDID,sTYPE,sTIME);
		Log3("ConnectUserCheckAcK,len=%d",len);
		return;
	}else if(cmd == CMD_LOG_GET){
		if(pbuf == NULL||len< sizeof(cmdHead_t)){
			sprintf(MsgStr,"%s","LogOver"); //获取停止标识
			goto JumpCallBack;
		}
		
		int LogNum = 0;
		LogNum = (len-sizeof(cmdHead_t))/sizeof(LogInfo_t);
		Log3("App Cmd Recv headlen =======>%d",LogNum);
		
		if(LogNum != 0){ 
			
			LogInfo_t logInfo[LogNum];
			memset(logInfo,0,sizeof(LogInfo_t)*LogNum);
			memcpy(logInfo,(pbuf+sizeof(cmdHead_t)),sizeof(LogInfo_t)*LogNum);	
			int i;
			for(i=0;i<LogNum;i++){
				sprintf(MsgStr,"{\"%s\":\"%d\",\"%s\":\"%d\",\"%s\":\"%d\",\"%s\":\"%d\",\"%s\":\"%s\"}",
					"mainType",logInfo[i].mainType,
					"subType",logInfo[i].subType,
					"length",logInfo[i].length,
					"timeStamp",logInfo[i].timeStamp,
					"content",logInfo[i].content
					);
				if(strlen(MsgStr) == 72){  //无效数据字符串长度为72
					sprintf(MsgStr,"%s","LogOver"); //获取停止标识
					goto JumpCallBack;
				}
				GET_LOCK(&g_CallbackContextLock);
			    if(g_CallBack_Handle != NULL)
			    {   

					jstring	jstring_did = env->NewStringUTF(szDID);
			        jstring jstring_msg = env->NewStringUTF(MsgStr);
						
					env->CallVoidMethod(g_CallBack_Handle,g_CallBack_UILayerNotify,jstring_did,cmd,jstring_msg);

					env->DeleteLocalRef(jstring_did); 
			        env->DeleteLocalRef(jstring_msg);
		    	}
				PUT_LOCK(&g_CallbackContextLock);
			}
		}else{
			sprintf(MsgStr,"%s","LogOver"); //获取停止标识
			goto JumpCallBack;
		}
		sprintf(MsgStr,"%s","LogOver"); //获取停止标识
	}else if(cmd == CMD_DATACTRL_FILELIST_GET){

		if(pbuf == NULL||len< sizeof(cmdHead_t)){
			sprintf(MsgStr,"%s","FileOver"); //获取停止标识
			goto JumpCallBack;
		}
		
		int FileNum = 0;
		FileNum = (len - sizeof(cmdHead_t)+4)/sizeof(xqFile_t);
		if(FileNum != 0){
			xqFile_t xqfile[FileNum];
			memset(xqfile,0,sizeof(xqFile_t)*FileNum);
			memcpy(xqfile,pbuf+sizeof(cmdHead_t)-4,sizeof(xqFile_t)*FileNum);
			int i;
			for(i=0;i<FileNum;i++){
				sprintf(MsgStr,"{\"%s\":\"%d\",\"%s\":\"%u\",\"%s\":\"%u\",\"%s\":\"%s\"}",
					"type",xqfile[i].type,
					"size",xqfile[i].size,
					"timeStamp",xqfile[i].timeStamp,
					"name",xqfile[i].name
					);
				GET_LOCK(&g_CallbackContextLock);
			    if(g_CallBack_Handle != NULL)
			    {   

					jstring	jstring_did = env->NewStringUTF(szDID);
			        jstring jstring_msg = env->NewStringUTF(MsgStr);
						
					env->CallVoidMethod(g_CallBack_Handle,g_CallBack_UILayerNotify,jstring_did,cmd,jstring_msg);

					env->DeleteLocalRef(jstring_did); 
			        env->DeleteLocalRef(jstring_msg);
		    	}
				PUT_LOCK(&g_CallbackContextLock);
			}
		}else{
			sprintf(MsgStr,"%s","FileOver"); //获取停止标识
			goto JumpCallBack;
		}
		sprintf(MsgStr,"%s","FileOver"); //获取停止标识
	}else if(cmd == CMD_SD_RECORDFILE_GET){

		if(pbuf == NULL || len <= 4){
			sprintf(MsgStr,"%s","RecOver"); //获取停止标识
			goto JumpCallBack;
		}
		//返回数据为字符串
		int RecordCount = 0;
		char MaxRecNum[8] = {0};
		ResolveData(MaxRecNum,pbuf+4,"RecordCount=",";");
		RecordCount = atoi(MaxRecNum);
		Log3("App Cmd Recv Get RecordCount =======>%d",RecordCount);
		int i;
		for(i=0;i<RecordCount;i++){
			char Rec_name[32] = {0};
			char Rec_size[32]  = {0};
			char Rec_name_spf[32] = {0};
			sprintf(Rec_name_spf,"%s%d%s","record_name0[",i,"]=");
			char Rec_size_spf[32] = {0};
			sprintf(Rec_size_spf,"%s%d%s","record_size0[",i,"]=");
			ResolveData(Rec_name,pbuf+4,Rec_name_spf,";");
			ResolveData(Rec_size,pbuf+4,Rec_size_spf,";");
			
			sprintf(MsgStr,"{\"%s\":\"%s\",\"%s\":\"%s\"}",
					"Rec_name",Rec_name,
					"Rec_size",Rec_size
					);
			GET_LOCK(&g_CallbackContextLock);
			    if(g_CallBack_Handle != NULL)
			    {   

					jstring	jstring_did = env->NewStringUTF(szDID);
			        jstring jstring_msg = env->NewStringUTF(MsgStr);
						
					env->CallVoidMethod(g_CallBack_Handle,g_CallBack_UILayerNotify,jstring_did,cmd,jstring_msg);

					env->DeleteLocalRef(jstring_did); 
			        env->DeleteLocalRef(jstring_msg);
		    	}
			PUT_LOCK(&g_CallbackContextLock);
		}
		sprintf(MsgStr,"%s","RecOver"); //获取停止标识
	}else if(cmd == CMD_DATACTRL_PLAYBACK_STOP){
	
		mediaEnabled = 0;
		videoPlaying = 0;   
		Log3("stop video process.");
		
	//	if(videoRecvThread != -1) pthread_join(videoRecvThread,NULL);
	//	if(videoPlayThread != -1) pthread_join(videoPlayThread,NULL);

		GET_LOCK(&SessionStatusLock);
			SessionStatus = STATUS_SESSION_IDLE;
		PUT_LOCK(&SessionStatusLock);

		return;
	}else if(cmd == CMD_NET_WIFI_SCAN){
		int ScanNum = 0;
		if(pbuf == NULL || len <= sizeof(cmdHead_t)){
			sprintf(MsgStr,"%s","WIFIOver"); //获取停止标识
			goto JumpCallBack;
		}
		ScanNum = (len - 4)/sizeof(wifiScanRet_t);
		Log3("WIFIList Scanner ScanNum =============>%d",ScanNum);
		wifiScanRet_t wifiScanRet[ScanNum];
		memset(wifiScanRet,0,sizeof(wifiScanRet_t)*ScanNum);
		memcpy(wifiScanRet,pbuf + 4,sizeof(wifiScanRet_t)*ScanNum);
		int i;
		for(i=0;i<ScanNum;i++){
			sprintf(MsgStr,"{\"%s\":\"%s\",\"%s\":\"%d\",\"%s\":\"%d\",\"%s\":\"%d\",\"%s\":\"%d\",\"%s\":\"%d\"}",
					"ssid",wifiScanRet[i].ssid,
					"security",wifiScanRet[i].security,
					"dbm0",wifiScanRet[i].dbm0,
					"dbm1",wifiScanRet[i].dbm1,
					"mode",wifiScanRet[i].mode,
					"channel",wifiScanRet[i].channel
					);
			Log3("WIFIList Scanner Result ===========>%s",MsgStr);
			GET_LOCK(&g_CallbackContextLock);
			    if(g_CallBack_Handle != NULL)
			    {   

					jstring	jstring_did = env->NewStringUTF(szDID);
			        jstring jstring_msg = env->NewStringUTF(MsgStr);
						
					env->CallVoidMethod(g_CallBack_Handle,g_CallBack_UILayerNotify,jstring_did,cmd,jstring_msg);

					env->DeleteLocalRef(jstring_did); 
			        env->DeleteLocalRef(jstring_msg);
		    	}
			PUT_LOCK(&g_CallbackContextLock);
		}
		sprintf(MsgStr,"%s","WIFIOver"); //获取停止标识
	}else{
		Rsp2Json(cmd,pbuf+4,MsgStr,sizeof(MsgStr));
	}
JumpCallBack:

	GET_LOCK(&g_CallbackContextLock);
	
	    if(g_CallBack_Handle != NULL)
	    {   

			jstring	jstring_did = env->NewStringUTF(szDID);
	        jstring jstring_msg = env->NewStringUTF(MsgStr);
				
		//	env->CallVoidMethod(g_CallBack_Handle, g_CallBack_CmdRecv, jstring_did, sessionID,cmd,pbuf, len);
			env->CallVoidMethod(g_CallBack_Handle,g_CallBack_UILayerNotify,jstring_did,cmd,jstring_msg);

			env->DeleteLocalRef(jstring_did); 
			env->DeleteLocalRef(jstring_msg); 
	    }
		
	PUT_LOCK(&g_CallbackContextLock);

	return;
}

int CPPPPChannel::PPPPClose()
{
	Log3("close connection by did:[%s] called.",szDID);
#ifdef TUTK_PPPP
	IOTC_Connect_Stop_BySID(sessionID);

	if(SID >= 0 || avIdx >= 0 || spIdx >= 0){
		Log3("close connection with session:[%d] avIdx:[%d] did:[%s].",SID,avIdx,szDID);
		
		avClientExit(SID,avIdx);
		avClientStop(avIdx);	// stop audio and video recv from device
		avServExit(SID,spIdx);	// for avServStart block
		avServStop(spIdx);		// stop audio and video send to device
		IOTC_Session_Close(sessionID); // close client session handle
	}

#else
// yunni p2p 
	int Ret = 0;
	if(sessionID >= 0){ 	   
		Ret = PPPP_ForceClose(sessionID);
	}else{
	 	Ret = PPPP_Break(szDID);
	}
	Log3("PPPP Close status value : [%d]  \n",Ret);
#endif 

	avIdx = spIdx = speakerChannel = SID = sessionID = -1;

	return 0;
}

int CPPPPChannel::Start()
{   
    Log3("start pppp connection to device with uuid:[%s].\n",szDID);

	GET_LOCK(&SessionStatusLock);
	if(SessionStatus != STATUS_SESSION_DIED){
		Log3("session status is:[%d],can't start pppp connection.\n",SessionStatus);
		PUT_LOCK(&SessionStatusLock);
		return -1;
	}
	SessionStatus = STATUS_SESSION_START;
	PUT_LOCK(&SessionStatusLock);

	StartMediaChannel();
	
    return 0;
}

void CPPPPChannel::Close()
{
	GET_LOCK(&SessionStatusLock);
	SessionStatus = STATUS_SESSION_CLOSE;
	PUT_LOCK(&SessionStatusLock);
	
	mediaLinking = 0;
	
	if(mediaCoreThread != (pthread_t)-1){
		pthread_join(mediaCoreThread,NULL);
		mediaCoreThread = (pthread_t)-1;
	}else{
		GET_LOCK(&SessionStatusLock);
		SessionStatus = STATUS_SESSION_DIED;
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
	

	SendCmds(sessionID,CMD_DATACTRL_AUDIO_STOP,NULL,(void *)this);
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
		|| !avExit
		|| !fileRecving
	){
		iocmdSending = 0;
    	iocmdRecving = 0;
		mediaEnabled = 0;
    	audioPlaying = 0;
		videoPlaying = 0;
		avExit = 0;
		fileRecving = 0;
	}

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
	int audio_recv_codec,
	int audio_send_codec,
	int video_recv_codec
){    
   // F_LOG;	
	int ret = 0;
	if(sessionID < 0) return -1;
	GET_LOCK(&SessionStatusLock);
	if(SessionStatus != STATUS_SESSION_IDLE){
		Log3("session status is:[%d],can't start living stream.\n",SessionStatus);
		ret = SessionStatus;
		PUT_LOCK(&SessionStatusLock);
		goto jumperr;
	}
	SessionStatus = STATUS_SESSION_PLAYING;
	PUT_LOCK(&SessionStatusLock);
	
	Log3("media stream start here.");

	mediaEnabled = 1;

	// pppp://usr:pwd@livingstream:[channel id]
	// pppp://usr:pwd@replay/mnt/sdcard/replay/file
	
	memset(szURL,0,sizeof(szURL));

	AudioRecvFormat = audio_recv_codec;
	AudioSendFormat = audio_send_codec;
	VideoRecvFormat = video_recv_codec;

	if(url != NULL){
		memcpy(szURL,url,strlen(url));
	}
	
	//发送视频开启指令
	Log3("Send instruction to Driver !");
	SendCmds(sessionID,CMD_DATACTRL_PLAYLIVE_START,NULL,(void *)this);
	//发送音频开启指令
	SendCmds(sessionID,CMD_DATACTRL_AUDIO_START,NULL,(void *)this);
	//发送对讲开启指令
	SendCmds(sessionID,CMD_DATACTRL_TALK_START,NULL,(void *)this);
	
	Log3("channel init video proc.");
    StartVideoChannel();
	Log3("channel init audio proc.");
	StartAudioChannel();

	ret = 0;

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
	
	return hIOCmdBuffer->Write(AppCmds,sizeof(APP_CMD_HEAD) + hCmds->CgiLens);
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

/*
AVFormatContext * hAVFmtContext;
AVOutputFormat  * hAVFmtOutput;
AVStream		* hAudioStream;
AVStream		* hVideoStream;
AVCodecContext  * hAVCodecContext;
char * hRecordFile;
*/

int CPPPPChannel::StartRecorder(
	int 		W,			// \BF\ED\B6\C8
	int 		H,			// \B8脽露\C8
	int 		FPS,		// 脰隆\C2\CA
	char *		SavePath	// 
){
	int Err = 0;

	AVCodec * hACodec = NULL; 
	AVCodec * hVCodec = NULL;

	GET_LOCK(&AviDataLock);

	if(hAVFmtContext){
		Log3("channel already in recording now.");
		PUT_LOCK(&AviDataLock);
		return -1;
	}
	
	hRecordFile  = (char*)malloc(MAX_PATHNAME_LEN);
	memset(hRecordFile,0,MAX_PATHNAME_LEN);
	memcpy(hRecordFile,SavePath,strlen(SavePath));
	
//	sprintf(hRecordFile,"%s/%d.mp4",SavePath,(int)time(NULL));

	avformat_alloc_output_context2(&hAVFmtContext, NULL, NULL, hRecordFile);
	hAVFmtOutput = hAVFmtContext->oformat;

	if(!hAVFmtOutput){
		Log3("initalize av output format failed, filename:%s.",hRecordFile);
		goto jumperr;
	}

	hAVFmtOutput->video_codec = AV_CODEC_ID_H264;

	strncpy(hAVFmtContext->filename,hRecordFile,sizeof(hAVFmtContext->filename));

	if(hAVFmtOutput->video_codec != AV_CODEC_ID_NONE){
		if(add_video_stream(&sVOs,hAVFmtContext,&hVCodec,hAVFmtOutput->video_codec,W,H,30) < 0){
			Log2("add_video_stream error");
			goto jumperr;
		}
		
		Log2("add_video_stream OK");
	}

	if(hAVFmtOutput->audio_codec != AV_CODEC_ID_NONE){
		if(add_audio_stream(&sAOs,hAVFmtContext,&hACodec,hAVFmtOutput->audio_codec,8000,1) < 0){
			Log2("add_audio_stream error");
			goto jumperr;
		}
		
		Log2("add_audio_stream OK");
	}

	if(open_video(hAVFmtContext,hVCodec,&sVOs,NULL) < 0){
	 	Log2("open_video error");
		goto jumperr;
	}
	
	if(open_audio(hAVFmtContext,hACodec,&sAOs,NULL) < 0){
		Log2("open_audio error");
		goto jumperr;
	}
	
	vCTS = aCTS = 0;		
	vLTS = aLTS = 0;		
	vPTS = aPTS = 0;		
	
	sSTS = 0;				
	vIdx = 0;				
	aIdx = 0;;				

	aLen = 0;

	Err = avio_open(&hAVFmtContext->pb,hRecordFile,AVIO_FLAG_WRITE);
	if(Err < 0){
		Log2("avio_open %s failed with err code:%d.",hRecordFile,Err);
		goto jumperr;
	}

	Err = avformat_write_header(hAVFmtContext,NULL);
	if(Err != 0){
		Log2("avformat_write_header failed with err code:%d.",Err);
		goto jumperr;
	}

	hAudioRecCaches = (char *)malloc(sAOs.st->codec->frame_size*2*2);
	if(hAudioRecCaches == NULL){
		Log2("not enough mem for AV recording.");
		goto jumperr;
	}

	avExit = 1;
	memset(&avProc,0,sizeof(avProc));

	Err = pthread_create(
		&avProc,
		NULL,
		RecordingProcess,
		this);

	if(Err != 0){
		Log2("create av recording process failed.");
		goto jumperr;
	}

	Log3("start recording process done.");

	PUT_LOCK(&AviDataLock);

	return  0;

jumperr:
	if(hAVFmtContext){
		avformat_free_context(hAVFmtContext);
		hAVFmtContext = NULL;
	}

	if(hRecordFile != NULL){
		free(hRecordFile);
	}
	hRecordFile = NULL;

	PUT_LOCK(&AviDataLock);

	return -1;
}

int CPPPPChannel::WriteRecorder(
	const char * 	FrameData,
	int				FrameSize,
	int				FrameCode, 	// audio or video codec [0|1]
	int				FrameType,	// keyframe or not [0|1]
	long long		Timestamp
){
	if(hAVFmtContext == NULL){
//		Log2("invalid av format context for wirte operation.");
		return -1;
	}

/*
	if(FrameType == 1){
		Log3("recording video I frame.");
	}
*/

	if(vIdx == 0 && FrameCode == 1 && FrameType != 1){
		Log3("wait first video key frame for avi recorder.");
		return -1;
	}

	if(vIdx == 0 && FrameCode == 0){
		Log3("wait first video key frame for avi recorder.");
		return -1;
	}

	int Err = 0;
	
//	AVRational sAVRational = {1,1000};
	AVPacket   sAVPacket = {0};
	AVStream * hAVStream = FrameCode ? sVOs.st : sAOs.st;
	
	av_init_packet(&sAVPacket);

	struct timeval tv;
	gettimeofday(&tv,NULL);

//	aCTS = vCTS = tv.tv_sec * 1000 + tv.tv_usec / 1000;
//	if(sSTS == 0) sSTS = aCTS;

	if(FrameCode){
/*
		vPTS = (vCTS - sSTS)/(1000/hAVStream->codec->time_base.den);
		if((vPTS != 0) && (vPTS <= vLTS)){
			vPTS = vPTS + 1;
		}
		vLTS = vPTS;
*/
		sAVPacket.pts = sAVPacket.dts = vIdx;
		sAVPacket.stream_index = hAVStream->index;
		sAVPacket.data = (uint8_t*)FrameData;
		sAVPacket.size = FrameSize;
		sAVPacket.flags |= FrameType ? AV_PKT_FLAG_KEY : 0;
		sAVPacket.duration = 0;
		
		Err = write_frame(hAVFmtContext, &hAVStream->codec->time_base,hAVStream, &sAVPacket);
	}else{
		memcpy(hAudioRecCaches + aLen,FrameData,FrameSize);
		aLen += FrameSize;
		if(aLen >= sAOs.st->codec->frame_size*2){
			write_audio_frame(hAVFmtContext,&sAOs,(uint8_t*)hAudioRecCaches,sAOs.st->codec->frame_size*2,0);
			aLen -= (sAOs.st->codec->frame_size*2);
			memcpy(hAudioRecCaches,hAudioRecCaches+sAOs.st->codec->frame_size*2,aLen);
		}
	}
	
	switch(FrameCode){
		case 0:
			aIdx ++;
			break;
		case 1:
			vIdx ++;
			break;
	}
	
	return 0;
}

int CPPPPChannel::CloseRecorder(){

	GET_LOCK(&AviDataLock);
	
	if(hAVFmtContext == NULL){
		Log2("invlaid av format context for record close.");
		PUT_LOCK(&AviDataLock);
		return -1;
	}

	Log3("wait avi record process exit.");
	avExit = 0;
	pthread_join(avProc,NULL);
	Log3("avi record process exit done.");

	av_write_trailer(hAVFmtContext);

	/*
	int i = 0;
	for(i;i<hAVFmtContext->nb_streams;i++){
		avcodec_free_context(&hAVFmtContext->streams[i]->codec);
		free(hAVFmtContext->streams[i]);
	}
	*/

	if(!(hAVFmtContext->oformat->flags & AVFMT_NOFILE)){
		avio_close(hAVFmtContext->pb);
	}

	close_stream(hAVFmtContext, &sVOs);
    close_stream(hAVFmtContext, &sAOs);

	avformat_free_context(hAVFmtContext);
	hAVFmtContext = NULL;

	if(hRecordFile != NULL) free(hRecordFile);
	hRecordFile = NULL;

	if(hAudioRecCaches != NULL) free(hAudioRecCaches);
	hAudioRecCaches = NULL;

	PUT_LOCK(&AviDataLock);

	return 0;
}



