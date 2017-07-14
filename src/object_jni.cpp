#include "stdio.h"
#include "utility.h"
#include <stdarg.h>
#include <dlfcn.h>

#include "PPPP_API.h"
#include "PPPP_Error.h"
#ifdef PLATFORM_ANDROID
#include "iLnkCmdParser.h"
#endif
#include "object_jni.h"
#include "SearchDVS.h"
#include "PPPPChannelManagement.h"

#include "appreq.h"
#include "apprsp.h"

jobject   g_CallBack_Handle = NULL;

jmethodID g_CallBack_SearchResults = NULL;
jmethodID g_CallBack_VideoDataProcess = NULL;
jmethodID g_CallBack_AlarmNotifyDoorBell = NULL;
jmethodID g_CallBack_ConnectionNotify = NULL;
jmethodID g_CallBack_UILayerNotify = NULL;
jmethodID g_CallBack_CmdRecv =	NULL;



#ifdef PLATFORM_ANDROID
#include <jni.h>
JavaVM * g_JavaVM = NULL;
static const char * classPathName = ANDROID_CLASS_PATH;
#else
void   JNIEnv::SetByteArrayRegion(char * a, int b, int c,const jbyte * d){
	memcpy(a,&d[b],c);
}

void * JNIEnv::GetByteArrayElements(char * a, int b){
    return a;
}

char * JNIEnv::GetStringUTFChars(char * a, int b){
    return a;
}

char * JNIEnv::NewStringUTF(char * a){
    char * r = NULL;
    if(a != NULL){
        if(strlen(a) > 0){
            r = (char*)malloc(strlen(a) + 1);
            memset(r,0,strlen(a) + 1);
            memcpy(r,a,strlen(a));
        }
    }
    return r;
}

void   JNIEnv::ReleaseByteArrayElements(char * a, char * b, int c){
    return;
}

void   JNIEnv::ReleaseStringUTFChars(char * a, char * b){
    return;
}

void   JNIEnv::DeleteLocalRef(char * a){
    if(a != NULL) free(a);
    return;
}

char * JNIEnv::NewByteArray(int a){
    return (char*)malloc(a);
}

void   JNIEnv::CallVoidMethod(void * a, void * b, ...){
    va_list args;
    va_start(args, b);
 
    /*
    if(b != NULL){
        void (*Call)(...) = (void (*)(...))b;
        Call(args);
        goto jumpout;
    }
    */

    if(b == g_CallBack_SearchResults){
        void (*Call)(int,const char *,const char *,const char *,const char *,int,int) = (void (*)(int,const char *,const char *,const char *,const char *,int,int))b;
        Call(va_arg(args,int),va_arg(args,const char *),va_arg(args,const char *),va_arg(args,const char *),va_arg(args,const char *),va_arg(args,int),va_arg(args,int));
        goto jumpout;
    }
    if(b == g_CallBack_UILayerNotify){
        void (*Call)(const char *,int,const char *) = (void (*)(const char *,int,const char *))b;
        Call(va_arg(args,const char *),va_arg(args,int),va_arg(args,const char *));
        goto jumpout;
    }
	
	if (b == g_CallBack_CmdRecv){
		void (*Call)(const char *,int,int,const char *,int) = (void (*)(const char *,int,int,const char *,int))b;
        Call(va_arg(args,const char *),va_arg(args,int),va_arg(args,int),va_arg(args,const char *),va_arg(args,int));
        goto jumpout;
	}

    if(b == g_CallBack_VideoDataProcess){
        void (*Call)(const char *,char *,int,int,int,int,int) = (void (*)(const char *,char *,int,int,int,int,int))b;
        Call(va_arg(args,const char *),va_arg(args,char *),va_arg(args,int),va_arg(args,int),va_arg(args,int),va_arg(args,int),va_arg(args,int));
        goto jumpout;
    }
    if(b == g_CallBack_ConnectionNotify){
        void (*Call)(const char *,int,int) = (void (*)(const char *,int,int))b;
        Call(va_arg(args,const char *),va_arg(args,int),va_arg(args,int));
        goto jumpout;
    }
    if(b == g_CallBack_AlarmNotifyDoorBell){
        void (*Call)(const char *,const char *,const char *,const char *) = (void (*)(const char *,const char *,const char *,const char *))b;
        Call(va_arg(args,const char *),va_arg(args,const char *),va_arg(args,const char *),va_arg(args,const char *));
        goto jumpout;
    }
    Log3("Invalid callback function pointer.\n");
        
jumpout:
    va_end(args);
    
    return;
}

JNIEnv iOSEnv;

#endif

COMMO_LOCK g_CallbackContextLock = PTHREAD_MUTEX_INITIALIZER;
COMMO_LOCK g_FindDevsProcessLock = PTHREAD_MUTEX_INITIALIZER;

static CSearchDVS * g_PSearchDVS = NULL;
static CPPPPChannelManagement * g_pPPPPChannelMgt = NULL;  

static void YUV4202RGB565_CALL(int width, int height, const unsigned char *src, unsigned short *dst)  
{  
    int line, col, linewidth;  
    int y, u, v, yy, vr, ug, vg, ub;  
    int r, g, b;  
    const unsigned char *py, *pu, *pv;  
      
    linewidth = width >> 1;  
    py = src;  
    pu = py + (width * height);  
    pv = pu + (width * height) / 4;  
      
    y = *py++;  
    yy = y << 8;  
    u = *pu - 128;  
    ug =  88 * u;  
    ub = 454 * u;  
    v = *pv - 128;  
    vg = 183 * v;  
    vr = 359 * v;  
      
    for (line = 0; line < height; line++) {  
       for (col = 0; col < width; col++) {  
        r = (yy +      vr) >> 8;  
        g = (yy - ug - vg) >> 8;  
        b = (yy + ub     ) >> 8;  
      
        if (r < 0)   r = 0;  
        if (r > 255) r = 255;  
        if (g < 0)   g = 0;  
        if (g > 255) g = 255;  
        if (b < 0)   b = 0;  
        if (b > 255) b = 255;  
       *dst++ = (((unsigned short)r>>3)<<11) | (((unsigned short)g>>2)<<5) | (((unsigned short)b>>3)<<0);  
        
        y = *py++;  
        yy = y << 8;  
        if (col & 1) {  
         pu++;  
         pv++;  
      
         u = *pu - 128;  
         ug =  88 * u;
         ub = 454 * u;  
         v = *pv - 128;  
         vg = 183 * v;  
         vr = 359 * v;  
        }  
       } /* ..for col */  
       if ((line & 1) == 0) { // even line: rewind  
        pu -= linewidth;  
        pv -= linewidth;  
       }  
    } /* ..for line */  
}  

JNIEXPORT int JNICALL YUV4202RGB565(JNIEnv *env, jobject obj, jbyteArray yuv, jbyteArray rgb, jint width, jint height)
{
    //F_LOG ;
    //Log("NativeCaller_YUV4202RGB565: width: %d, height: %d", width, height);

    jbyte * Buf = (jbyte*)env->GetByteArrayElements(yuv, 0);
    jbyte * Pixel= (jbyte*)env->GetByteArrayElements(rgb, 0);

    YUV4202RGB565_CALL(width, height, (const unsigned char *)Buf, (unsigned short *)Pixel);

    env->ReleaseByteArrayElements(yuv, Buf, 0);        
    env->ReleaseByteArrayElements(rgb, Pixel, 0);

    return 1;
}

JNIEXPORT int JNICALL PPPPInitialize(JNIEnv *env ,jobject obj, jstring svr)
{
	Log3("start pppp init with version:[%s.%s].",__DATE__,__TIME__);
	Log3("start pppp init with version:[%s.%s].",__DATE__,__TIME__);
	Log3("start pppp init with version:[%s.%s].",__DATE__,__TIME__);
	
	char *defaultSvr=(char *)"EKPNHXIDAUAOEHLOTBSQEJSWPAARTAPKLXPGENLKLUPLHUATSVEESTPFHWIHPDIEHYAOLVEISQLNEGLPPALQHXERELIALKEHEOHZHUEKIFEEEPEJ-$$";
	int maxNmbOfSession = 64; 
	int maxNmbOfChannel = 4; 
	int maxSizeOfChannel = 128; 
	Log3("init pppp lib with server string:[%s] ", defaultSvr);
	
//	int ret = PPPP_InitializeExt(((char *)svr == NULL)?(char *)svr:defaultSvr,maxNmbOfSession,maxNmbOfChannel,maxSizeOfChannel,1024,(char *)"noName");
	int ret = PPPP_Initialize(((char *)svr == NULL)?(char *)svr:defaultSvr,0);

	if(ret == ERROR_PPPP_ALREADY_INITIALIZED){
		int ret_del = 0;
		ret_del = PPPP_DeInitialize();
		Log3("PPPP_DeInitialize ret_del ------>%d ",ret_del);
		return ret;
	}
	
	if(ret != ERROR_PPPP_SUCCESSFUL){
		Log3("PPPP_Initialize error exit...ret=%d ",ret);
	}
	
	return ret;
}

JNIEXPORT void JNICALL PPPPManagementInit(JNIEnv *env ,jobject obj)
{ 	
	Log3("call PPPPChannelMgt initialize");
	if(g_pPPPPChannelMgt == NULL){
    	g_pPPPPChannelMgt = new CPPPPChannelManagement();
	}else{
		Log3("PPPPChannelMgt already initialized.");
	}
}

JNIEXPORT void JNICALL PPPPManagementFree(JNIEnv *env ,jobject obj)
{
	Log3("stop all connected channel");
	g_pPPPPChannelMgt->StopAll();
	Log3("stop all connected channel done");
}

JNIEXPORT int JNICALL PPPPSetCallbackContext(JNIEnv *env, jobject obj, jobject context)
{
    if(context == NULL){
       	Log3("set java callback for jni layer failed.\n");
		return -1;
    }else{
    	Log3("get callback lock from PPPPSetCallbackContext");
		GET_LOCK(&g_CallbackContextLock);
		if(g_CallBack_Handle){
			PUT_LOCK(&g_CallbackContextLock);
			Log3("callback context already set.");
			Log3("get callback lock from PPPPSetCallbackContext");
			return 0;
		}

#ifdef PLATFORM_ANDROID
        g_CallBack_Handle = env->NewGlobalRef(context);
        jclass clazz = env->GetObjectClass(context);
		
        g_CallBack_SearchResults = env->GetMethodID(clazz, "SearchResult", "(ILjava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;II)V") ;
		g_CallBack_AlarmNotifyDoorBell = env->GetMethodID( clazz, "CallBack_AlarmNotifyDoorBell", "(Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;)V" );
        g_CallBack_VideoDataProcess = env->GetMethodID( clazz, "VideoData", "(Ljava/lang/String;[BIIIII)V" );

		g_CallBack_ConnectionNotify = env->GetMethodID(
			clazz, 
			"PPPPMsgNotify", 
			"(Ljava/lang/String;II)V"
			) ;

		// void UILayerNotify(String DeviceID,int MsgType,String MsgData)
		g_CallBack_UILayerNotify = env->GetMethodID(
			clazz,
			"UILayerNotify",
			"(Ljava/lang/String;ILjava/lang/String;)V"
			);
		g_CallBack_CmdRecv	=env->GetMethodID( clazz, "iLnk_CmdRecv", "(Ljava/lang/String;II[BI)V" );	
#else
        g_CallBack_Handle = (jobject)malloc(sizeof(jobject));
        g_CallBack_SearchResults = (void*)CBSearchResults;
        g_CallBack_UILayerNotify = (void*)CBUILayerNotify;
        g_CallBack_VideoDataProcess = (void*)CBVideoDataProcess;
        g_CallBack_ConnectionNotify = (void*)CBConnectionNotify;
        g_CallBack_AlarmNotifyDoorBell = (void*)CBAlarmNotifyByDevice;
		g_CallBack_CmdRecv	= (void*)CBCmdRecv;
#endif
		
		PUT_LOCK(&g_CallbackContextLock);
	Log3("get callback lock from PPPPSetCallbackContext");

    } 

    return 0;
}

//酶鈩⒙犅好�鈥斆�藴
JNIEXPORT void JNICALL StartSearch(JNIEnv *env ,jobject obj,jstring ssid,jstring psd)
{
    
    char *gkwifiname,*gkwifipwd; 
    gkwifiname  =   (char*)env->GetStringUTFChars(ssid,0);
    gkwifipwd   =   (char*)env->GetStringUTFChars(psd,0);
    
    GET_LOCK(&g_FindDevsProcessLock);
	
    SAFE_DELETE(g_PSearchDVS);
	
    g_PSearchDVS = new CSearchDVS();
	
    g_PSearchDVS->Open(gkwifiname,gkwifipwd) ; 
    
    env->ReleaseStringUTFChars(ssid, gkwifiname);
    env->ReleaseStringUTFChars(psd, gkwifipwd);
	PUT_LOCK(&g_FindDevsProcessLock);
}

//脮拢梅蟺脌鈥斆�藴
JNIEXPORT void JNICALL CloseSearch(JNIEnv *env ,jobject obj)
{
	GET_LOCK(&g_FindDevsProcessLock);

    SAFE_DELETE(g_PSearchDVS);

	PUT_LOCK(&g_FindDevsProcessLock);
}

JNIEXPORT int JNICALL StartPPPP(JNIEnv * env, jobject obj, jstring did, jstring user, jstring pwd, jstring server)
{
    //F_LOG;

	Log3("start pppp connection by native caller.");

    if(g_pPPPPChannelMgt == NULL){    
		Log3("channel management is null.\n");
        return 0;    
    }
    
    char *szDID,*szUsr,*szPwd,*szServer; 
    szDID    =   (char*)env->GetStringUTFChars(did,0);
    szUsr    =   (char*)env->GetStringUTFChars(user,0);
    szPwd    =   (char*)env->GetStringUTFChars(pwd,0);
    szServer =   (char*)env->GetStringUTFChars(server,0);

	Log3("user:%s pass:%s uuid:%s.\n",szUsr,szPwd,szDID);
	
    int nRet = g_pPPPPChannelMgt->Start(szDID, szUsr, szPwd, szServer);

    env->ReleaseStringUTFChars(pwd, szPwd);
    env->ReleaseStringUTFChars(user, szUsr);
    env->ReleaseStringUTFChars(did, szDID);
    env->ReleaseStringUTFChars(server, szServer);
	
    return nRet;

}

JNIEXPORT int JNICALL ClosePPPP(JNIEnv *env, jobject obj, jstring did)
{
    if(g_pPPPPChannelMgt == NULL){
        return 0;
    }
    
    char * szDID;
	
    szDID = (char*)env->GetStringUTFChars(did,0);
    if(szDID == NULL)
    {
        env->ReleaseStringUTFChars(did, szDID);
        return 0;
    }

	Log3("close pppp connection by native caller with uuid:[%s].",szDID);
    
    int nRet = g_pPPPPChannelMgt->Stop(szDID);

    env->ReleaseStringUTFChars(did, szDID);

    return nRet;

}

//Request for livestream
JNIEXPORT int JNICALL StartPPPPLivestream(
	JNIEnv *	env , 
	jobject 	obj ,
	jstring 	did , 				// p2p uuid
	jstring 	url ,				// url for record replay
	jint		audio_recv_codec,	// audio recv codec
	jint		audio_send_codec,	// audio send codec
	jint		video_recv_codec	// video recv codec
){
	int r = 0;
    
    if(g_pPPPPChannelMgt == NULL)
        return 0;

	char * szURL = NULL;
	char * szDID = NULL;

    szDID = (char*)env->GetStringUTFChars(did,0);
    if(szDID == NULL){        
        env->ReleaseStringUTFChars(did, szDID);
        goto jumperr;
    }

	szURL = (char*)env->GetStringUTFChars(url,0);
	if(szURL == NULL){
		goto jumperr;
	}

    Log3("click start live stream here: %s", szDID);
    
    r = g_pPPPPChannelMgt->StartPPPPLivestream(
		szDID, 
		szURL, 
		audio_recv_codec,
		audio_send_codec,
		video_recv_codec
		);

jumperr:
	
	if(szDID){
    	env->ReleaseStringUTFChars(did, szDID);    
	}
	
	if(szURL){
		env->ReleaseStringUTFChars(did, szURL);  
	}

    return r;
}

JNIEXPORT int JNICALL ClosePPPPLivestream(
	JNIEnv *	env, 
	jobject 	obj, 
	jstring 	did
){
    //F_LOG;
    
    if(g_pPPPPChannelMgt == NULL){
		Log3("channel manager is null.");
        return 0;
    }

    char *szDID;
    szDID = (char*)env->GetStringUTFChars(did,0);
    if(szDID == NULL)
    {
        env->ReleaseStringUTFChars(did, szDID);
        return 0;
    }

	Log3("close live stream by %s.",szDID);
    
    int nRet = g_pPPPPChannelMgt->ClosePPPPLivestream(szDID);
    env->ReleaseStringUTFChars(did, szDID);

    return nRet;
}

JNIEXPORT int JNICALL SetAudioStatus(JNIEnv *env , jobject obj, jstring did,jint status){
	char * szDID = (char*)env->GetStringUTFChars(did,0);

	int res = g_pPPPPChannelMgt->SetAudioStatus(szDID,(int)status);
	
	env->ReleaseStringUTFChars(did, szDID);

	return res;
}

JNIEXPORT int JNICALL GetAudioStatus(JNIEnv *env , jobject obj, jstring did){
	char * szDID = (char*)env->GetStringUTFChars(did,0);

	int res = g_pPPPPChannelMgt->GetAudioStatus(szDID);
	
	env->ReleaseStringUTFChars(did, szDID);

	return res;
}

JNIEXPORT int JNICALL SendHexsCommand(
	JNIEnv * 	env, 
	jobject 	obj, 
	jstring 	did, 
	jint 		msgtype, 
	jbyteArray 	msg,
	jint 		msglens
){
	char * szDID = (char*)env->GetStringUTFChars(did,0);
	jbyte * hexs = (jbyte*)env->GetByteArrayElements(msg,0);

	APP_BIN_DATA ABD;
	ABD.lens = msglens;
	memcpy(ABD.d,hexs,msglens);

	int MsgType = msgtype;
	int MsgLens = sizeof(ABD);

	g_pPPPPChannelMgt->PPPPSetSystemParams(
		szDID,MsgType,(char*)&ABD,MsgLens
		);

	env->ReleaseStringUTFChars(did,szDID);
	env->ReleaseByteArrayElements(msg,hexs,0); 

	return 0;
}

JNIEXPORT int JNICALL SendCtrlCommand(
	JNIEnv * 	env, 
	jobject 	obj, 
	jstring 	did, 
	jint 		msgtype, 
	jstring 	msg,
	jint 		msglens
){
	Log3("==========>SendCtrlCommand start with msg type:[%04x]",msgtype);

	char * szDID = (char*)env->GetStringUTFChars(did,0);
	char * szMsg = (char*)env->GetStringUTFChars(msg,0);

	int MsgType = msgtype;
	int MsgLens = msglens;

	int Ret = g_pPPPPChannelMgt->PPPPSetSystemParams(
		szDID,MsgType,szMsg,MsgLens
		);

	env->ReleaseStringUTFChars(did,szDID);
	env->ReleaseStringUTFChars(msg,szMsg);

	Log3("==========>SendCtrlCommand close with msg type:[%04x].",msgtype);

	return Ret;
}


JNIEXPORT int JNICALL SendCtrlCommand_EX(
	JNIEnv * 	env, 
	jobject 	obj, 
	jstring 	did, 
	jint 		gwChannel,
	jint 		cmdType,
	jobject 	cmdParam
){
	int nRet=0;
	char *szDID;

	if(g_pPPPPChannelMgt == NULL)
		return nRet;

	szDID	=	(char*)env->GetStringUTFChars(did,0);
	if(szDID == NULL)
	{		 
		env->ReleaseStringUTFChars(did, szDID);
		return 0;
	}
	int len=0;
	char *param=NULL;
	
	switch ((int)cmdType){
			case CMD_GW_DEVLST_CLEAN://清理网关设备列表
				break;
			case CMD_GW_DEVLST_GET://获取网关设备列表
				break;
			case CMD_GW_DEVLST_SET://添加或修改设备列表中设备信息
				break;
			case CMD_GW_DEV_SEARCH://让网关搜索设备
				break;
			case CMD_GW_DEV_ALLSTREAM_GET://获取设备码流到网关RingBuf
				break;
			case CMD_GW_DEV_ALLSTREAM_CANCEL://取消设备码流到网关RingBuf
				break;
			case CMD_GW_DEV_STREAM_REC_START://网关实时开始录像
				break;
			case CMD_GW_DEV_STREAM_REC_STOP://网关实时停止录像
				break;
			case CMD_GW_DEV_RECSCHEDULE_ASSOCIATE://设置网关录像计划到某设备
				break;
			case CMD_GW_DEV_RECSCHEDULE_DISASSOCIATE://取消网关录像计划与某设备的匹配
				break;
			case CMD_GW_RECSCHEDULE_ADD://添加网关录像计划
				break;
			case CMD_GW_RECSCHEDULE_DEL://按ID删除网关录像计划
				break;
			case CMD_GW_RECSCHEDULE_CHG://按ID修改网关录像计划
				break;
			case CMD_GW_RECSCHEDULELST_GET://获取网关录像计划列表
				break;
			case CMD_GW_RECSCHEDULELST_CLEAN://清除全部网关录像
				break;
			case CMD_DEV_BROADCAST://设备启动时的信息广播10S内
				break;
			case CMD_SYSTEM_DEFAULTSETTING_EXPORT:
			case CMD_SYSTEM_DEFAULTSETTING_RECOVERY:
			case CMD_SYSTEM_DEFAULTSETTING_CREATE:
			case CMD_SYSTEM_CURRENTSETTING_EXPORT:
			case CMD_SYSTEM_STATUS_GET:
			case CMD_SYSTEM_SHUTDOWN:
			case CMD_SYSTEM_REBOOT:
			case CMD_SYSTEM_DATETIME_GET:
			case CMD_SYSTEM_USER_GET:
			case CMD_SYSTEM_OPRPOLICY_GET:
			case CMD_INTERFACE_SERIALSETTING_GET:
			case CMD_NET_WIFISETTING_GET:
			case CMD_PARAM_GET:
			case CMD_MISC_GET:
			case CMD_SNAPSHOT_GET:
			case CMD_CAMPARAMS_GET:
			case CMD_SD_RECORDSCH_GET:
			case CMD_SD_RECPOLICY_GET:
			case CMD_SD_FORMAT:
			case CMD_SD_INFO_GET:
			case CMD_PUSHPARAM_GET:
			case CMD_NET_WIREDSETTING_GET:
			case CMD_NET_WIFI_SCAN:
			case CMD_INTERFACE_GPIOACTION_GET:
			case CMD_INTERFACE_SERIALDATA_GET:
//			case CMD_AUDIO_PARAM_GET:
				break;
			case CMD_SYSTEM_DEFAULTSETTING_IMPORT:
				break;
			case CMD_SYSTEM_CURRENTSETTING_IMPORT:
				break;
			case CMD_LOG_SET:{
				len=sizeof(logSet_t);
				#ifdef PLATFORM_ANDROID
					logSet_t logSet;
					memset(&logSet,0x00,len);
					
					nRet=parseLogSet(env,cmdParam,&logSet);
					if (nRet<0)goto toRet;
					param=(char *)&logSet;
				#else
					param =(char *)cmdParam;
				#endif
			}
			break;
			
			case CMD_LOG_GET:{
				len=sizeof(logSearchCondition_t);
				#ifdef PLATFORM_ANDROID
					logSearchCondition_t logSearchCondition;
					memset(&logSearchCondition,0x00,len);
					nRet=parseLogSearchCondition(env,cmdParam,&logSearchCondition);
					if (nRet<0)goto toRet;
					param=(char *)&logSearchCondition;
				#else
					param =(char *)cmdParam;
				#endif
			
			}
			break;
		
			case CMD_SYSTEM_ITEMDEFAULTSETTING_RECOVERY:
				{
				len=sizeof(int);
				#ifdef PLATFORM_ANDROID
					int itemType=0;
					nRet=parseItemType(env,cmdParam,&itemType);
					if (nRet<0)goto toRet;
					param=(char *)&itemType;
				#else
					param=(char *)cmdParam;
				#endif
				}
				break;
			case CMD_SYSTEM_FIRMWARE_UPGRAD:
				{
				len=sizeof(firmwareUpgrade_t);
				#ifdef PLATFORM_ANDROID
					firmwareUpgrade_t firmwareUpgrade;
					memset(&firmwareUpgrade,0x00,len);
					nRet=parseFirmwareUpgrade(env,cmdParam,&firmwareUpgrade);
					if (nRet<0)goto toRet;
					param=(char *)&firmwareUpgrade;
				#else
					param=(char *)cmdParam;
				#endif
				}
				break;
			case CMD_SYSTEM_USER_CHK:
				{
				len=sizeof(user_t);
				#ifdef PLATFORM_ANDROID
					user_t user;
					memset(&user,0x00,len);
					nRet=parseUserChk(env,cmdParam,&user);
					if (nRet<0)goto toRet;
					param=(char *)&user;
				#else
					param=(char *)cmdParam;
				#endif
				}
				break;
			case CMD_SYSTEM_USER_SET:
				{
				len=sizeof(userSetting_t);
				#ifdef PLATFORM_ANDROID
					userSetting_t userSetting;
					memset(&userSetting,0x00,len);
					nRet=parseUserSetting(env,cmdParam,&userSetting);
					if (nRet<0)goto toRet;
					param=(char *)&userSetting;
				#else
					param=(char *)cmdParam;
				#endif
				}
				break;
			case CMD_SYSTEM_P2PPARAM_SET:
				{
				len=sizeof(openP2pSetting_t);
				#ifdef PLATFORM_ANDROID
					openP2pSetting_t p2pSetting;
					memset(&p2pSetting,0x00,len);
					nRet=parseP2PSetting(env,cmdParam,&p2pSetting);
					if (nRet<0)goto toRet;
					//paramToPrint(CMD_SYSTEM_P2PPARAM_SET,(void *)&p2pSetting);
					param=(char *)&p2pSetting;
				#else
					param=(char *)cmdParam;
				#endif
				}
				break;
			case CMD_SYSTEM_OPRPOLICY_SET:
				{
				len=sizeof(sysOprPolicy_t_EX);
				#ifdef PLATFORM_ANDROID
					sysOprPolicy_t_EX	oprPolicy;
					memset(&oprPolicy,0x00,len);
					nRet=parseOprPolicy(env,cmdParam,&oprPolicy);
					if (nRet<0)goto toRet;
					Utils_Cmd_SysOprPolicy_Print(oprPolicy);
					param=(char *)&oprPolicy;
				#else
					param=(char *)cmdParam;
				#endif
				}
				break;
			case CMD_SYSTEM_DATETIME_SET:
				{
				len=sizeof(datetimeParam_t);
				#ifdef PLATFORM_ANDROID
					datetimeParam_t datetime;
					memset(&datetime,0x00,len);
					nRet=parseDatetime(env,cmdParam,&datetime);
					if (nRet<0)goto toRet;
					param=(char *)&datetime;
				#else
					param=(char *)cmdParam;
				#endif
				}
				break;
			case CMD_INTERFACE_SADC_GET:
				{
				len=sizeof(adc_t);
				#ifdef PLATFORM_ANDROID
					adc_t strAdc;
					memset(&strAdc,0x00,len);
					nRet=parseAdc(env,cmdParam,&strAdc);
					if (nRet<0)goto toRet;
					param=(char *)&strAdc;
				#else
					param=(char *)cmdParam;
				#endif
				}
				break;
			case CMD_INTERFACE_GPIOVALUE_SET:
			case CMD_INTERFACE_GPIOVALUE_GET:
				{
				len=sizeof(gpio_t);
				#ifdef PLATFORM_ANDROID
					gpio_t strGpio;
					memset(&strGpio,0x00,len);
					nRet=parseGpioValue(env,cmdParam,&strGpio);
					if (nRet<0)goto toRet;
				
					param=(char *)&strGpio;
				#else
					param=(char *)cmdParam;
				#endif
				}
				break;
			case CMD_INTERFACE_GPIOACTION_SET:
				{
				len=sizeof(gpioAction_t);
				#ifdef PLATFORM_ANDROID
					gpioAction_t strGpioAction;
					memset(&strGpioAction,0x00,len);
					nRet=parseGpioAction(env,cmdParam,&strGpioAction);
					if (nRet<0)goto toRet;
					//paramToPrint(CMD_INTERFACE_GPIOACTION_SET,(void *)&strGpioAction);
					param=(char *)&strGpioAction;
				#else
					param=(char *)cmdParam;
				#endif
				}
				break;
			case CMD_INTERFACE_SERIALSETTING_SET:
				{
				len=sizeof(serialParam_t);
				#ifdef PLATFORM_ANDROID
					serialParam_t serialSetting;
					memset(&serialSetting,0x00,len);
					nRet=parseSerialSetting(env,cmdParam,&serialSetting);
					if (nRet<0)goto toRet;
					//paramToPrint(CMD_INTERFACE_SERIALSETTING_SET,(void *)&serialSetting);
					param=(char *)&serialSetting;
				#else
					param=(char *)cmdParam;
				#endif
				}
				break;
			case CMD_INTERFACE_SERIALDATA_SET:
				{
				len=sizeof(serialData_t);
				#ifdef PLATFORM_ANDROID
					serialData_t serialData;
					memset(&serialData,0x00,len);
					nRet=parseSerialData(env,cmdParam,&serialData);
					
					if (nRet<0)goto toRet;
				#else
					serialData_t * cmdData = (serialData_t *)cmdParam;
					serialData_t serialData;
					memset(&serialData,0,sizeof(serialData));
					serialData.len = cmdData->len;
					memcpy(serialData.data,cmdData->data,sizeof(serialData.data));
					
				#endif
					char sendBuf[1024];
					memset(sendBuf,0,1024);
					serialDataTrans_t *serialDataTrans=(serialDataTrans_t *)sendBuf;
					serialDataTrans->serialNo=0;
					memcpy(&serialDataTrans->serialData,&serialData,nRet);
					len=nRet+4;
					//paramToPrint(CMD_INTERFACE_SERIALDATA_SET,(void *)&serialData);
					param=(char *)sendBuf;
				
				}
				break;
				/*
			case CMD_INTERFACE_SERIALDATA_GET:
				{
				serialData_t serialData;
				len=sizeof(serialData_t);
				memset(&serialData,0x00,len);
				nRet=parseSerialData(env,cmdParam,&serialData);
				if (nRet<0)goto toRet;
				len=nRet;
				paramToPrint(CMD_INTERFACE_SERIALDATA_SET,(void *)&serialData);
				param=(char *)&serialData;
				}
				break;
				*/
			case CMD_NET_WIFISETTING_SET:
				{
				len=sizeof(wifiParam_t);
				#ifdef PLATFORM_ANDROID
					wifiParam_t wifiParams;
					memset(&wifiParams,0x00,len);
					nRet=parseWifiSetting(env,cmdParam,&wifiParams);
					if (nRet<0)goto toRet;
					//paramToPrint(CMD_NET_WIFISETTING_SET,(void *)&wifiParams);
					param=(char *)&wifiParams;
				#else
					param=(char *)cmdParam;
				#endif
				}
				break;
			case CMD_NET_WIREDSETTING_SET:
				break;
			case CMD_PUSHPARAM_SET:
				{
				len=sizeof(pushParam_t);
				#ifdef PLATFORM_ANDROID
					pushParam_t pushParameter;
					memset(&pushParameter,0x00,len);
					nRet=parsePushParam(env,cmdParam,&pushParameter);
					if (nRet<0)goto toRet;
					//paramToPrint(CMD_PUSHPARAM_SET,(void *)&pushParameter);
					param=(char *)&pushParameter;
				#else
					param=(char *)cmdParam;
				#endif
				}
				break;
			case CMD_PUSHPARAM_DEL:
				{
				len=sizeof(pushParam_t);
				#ifdef PLATFORM_ANDROID
					pushParam_t pushParameter;
					memset(&pushParameter,0x00,len);
					nRet=parsePushParam(env,cmdParam,&pushParameter);
					if (nRet<0)goto toRet;
					//paramToPrint(CMD_PUSHPARAM_SET,(void *)&pushParameter);
					param=(char *)&pushParameter;
				#else
					param=(char *)cmdParam;
				#endif
				}
				break;
			case CMD_SD_RECORDING_NOW:
				{
				len=sizeof(recNow_t);
				#ifdef PLATFORM_ANDROID
					recNow_t recNow;
					memset(&recNow,0x00,len);
					nRet=parseRecNow(env,cmdParam,&recNow);
					if (nRet<0)goto toRet;
					//paramToPrint(CMD_SD_RECORDING_NOW,(void *)&recNow);
					param=(char *)&recNow;
				#else
					param=(char *)cmdParam;
				#endif
				}
				break;
			case CMD_SD_PIC_CAPTURE:
				{
				len=sizeof(picCap_t);
				#ifdef PLATFORM_ANDROID
					picCap_t picCap;
					memset(&picCap,0x00,len);
					nRet=parsePicCap(env,cmdParam,&picCap);
					if (nRet<0)goto toRet;
					//paramToPrint(CMD_SD_RECORDING_NOW,(void *)&picCap);
					param=(char *)&picCap;
				#else
					param=(char *)cmdParam;
				#endif
				}
				break;
			case CMD_SD_RECPOLICY_SET:
				{
				len=sizeof(mAVRecPolicy_t);
				#ifdef PLATFORM_ANDROID
					mStoragePolicy_t storagePolicy;
					memset(&storagePolicy,0x00,len);
					nRet=parseStoragePolicy(env,cmdParam,&storagePolicy);
					if (nRet<0)goto toRet;

					mAVRecPolicy_t avRecPolicy;
					avRecPolicy.storagePolicy=storagePolicy;

					Utils_Cmd_SdAVRecPolicy_Print(avRecPolicy);
					param=(char *)&avRecPolicy;
				#else
					param=(char *)cmdParam;
				#endif
				}
				break;
			case CMD_SD_RECORDFILE_GET:
				{
				len=sizeof(sdRecSearchCondition_t);
				#ifdef PLATFORM_ANDROID
					sdRecSearchCondition_t searchRecFile;
					memset(&searchRecFile,0x00,len);
					nRet=parseSearchRecordFile(env,cmdParam,&searchRecFile);
					if (nRet<0)goto toRet;
					//paramToPrint(CMD_SD_RECORDFILE_GET,(void *)&searchRecFile);
					param=(char *)&searchRecFile;
				#else
					param=(char *)cmdParam;
				#endif
				}
				break;
			case CMD_SD_RECORDSCH_SET:
				{
				len=sizeof(sdRecSchSet_t);
				#ifdef PLATFORM_ANDROID
					sdRecSchSet_t sdRecSchSet;
					memset(&sdRecSchSet,0x00,len);
					nRet=parseSdRecSchSet(env,cmdParam,&sdRecSchSet);
					if (nRet<0)goto toRet;
					//paramToPrint(CMD_SD_RECORDSCH_SET,(void *)&sdRecSchSet);
					param=(char *)&sdRecSchSet;
				#else
					param=(char *)cmdParam;
				#endif
				}
				break;
			case CMD_SD_PICFILE_GET:
				break;
			case CMD_PASSTHROUGH_STRING_PUT:
				{
				len=sizeof(sdRecSearchCondition_t);
				#ifdef PLATFORM_ANDROID
					passThroghString_t passThroghString;
					memset(&passThroghString,0x00,len);
					nRet=parsePassThroghString(env,cmdParam,&passThroghString);
					if (nRet<0)goto toRet;
					//paramToPrint(CMD_PASSTHROUGH_STRING_PUT,(void *)&passThroghString);
					param=(char *)&passThroghString;
				#else
					param=(char *)cmdParam;
				#endif
				}
				break;
			case CMD_CAMPARAMS_SET:
				{
				len=sizeof(camCtrl_t_EX);	
				#ifdef PLATFORM_ANDROID
					camCtrl_t_EX	camCtrl;
					memset(&camCtrl,0x00,len);
					nRet=parseCameraCtrl(env,cmdParam,&camCtrl);
					if (nRet<0)goto toRet;
					//paramToPrint(CMD_CAMPARAMS_SET,(void *)&camCtrl);
					param=(char *)&camCtrl;
				#else
					param=(char *)cmdParam;
				#endif
				}
				break;
			case CMD_PTZ_SET:
				{
				len=sizeof(ptzParamSet_t);
				#ifdef PLATFORM_ANDROID
					ptzParamSet_t ptzParam;
					memset(&ptzParam,0x00,len);
					nRet=parsePtzParam(env,cmdParam,&ptzParam);
					if (nRet<0)goto toRet;
					//paramToPrint(CMD_PTZ_SET,(void *)&ptzParam);
					param=(char *)&ptzParam;
				#else
					param=(char *)cmdParam;
				#endif
				}
				break;
			case CMD_NOTIFICATION:
				break;
			case CMD_LOCAL_SESSION_CHECK:
				break;
			case CMD_DATACTRL_AUDIO_START:
			case CMD_DATACTRL_AUDIO_STOP:
			case CMD_DATACTRL_TALK_START:
			case CMD_DATACTRL_TALK_STOP:
			case CMD_DATACTRL_PLAYLIVE_START:
			case CMD_DATACTRL_PLAYLIVE_STOP:
			case CMD_DATACTRL_PLAYBACK_START:
			case CMD_DATACTRL_PLAYBACK_STOP:
			case CMD_DEV_RECORD_PLAYBACK_SEEK:
			case CMD_DATACTRL_FILESAVEPATH:
			case CMD_DATACTRL_FILE_GET:
			case CMD_DATACTRL_FILE_PUT:
			case CMD_DATACTRL_FILE_SET:
			case CMD_DATACTRL_FILELIST_GET:
			case CMD_LOCAL_AVREC_START:
				Log3("local video recording start\n");
			case CMD_LOCAL_AVREC_STOP:
				Log3("local video recording stop\n");
			case CMD_DATACTRL_DOORBELL_CALL_ACCEPT:
			case CMD_DATACTRL_DOORBELL_CALL_REJECT:
				{
				#ifdef PLATFORM_ANDROID
					fileTransParam_t fileTransParam;
					//len=sizeof(fileTransParam_t);
					memset(&fileTransParam,0x00,len);
					if (NULL!=cmdParam)len=parseFileInfo(env,cmdParam,&fileTransParam);
					nRet=len;
					if (len<0)goto toRet;
					if (len==sizeof(fileTransParam_t))param=(char *)&fileTransParam;
				#else
					len = sizeof(fileTransParam_t);
					param = (char *)cmdParam;
				#endif
				}
				break;
			case CMD_AUDIO_PARAM_SET:
				{
				len=sizeof(audioParamSet_t);
				#ifdef PLATFORM_ANDROID
					audioParamSet_t audioParamSet;
					memset(&audioParamSet,0x00,len);
					nRet=parseAudioParamSet(env,cmdParam,&audioParamSet);
					if (nRet<0)goto toRet;
					//paramToPrint(CMD_AUDIO_PARAM_SET,(void *)&audioParamSet);
					param=(char *)&audioParamSet;
				#else
					param = (char *)cmdParam;
				#endif
				}			
				break;

			case CMD_ALARM_ACTION_SET:
			case CMD_ALARM_ACTION_GET:	
			default:
				return JNI_ERR_LOCAL_INVALID_CMD;
			}

	
	nRet = g_pPPPPChannelMgt->CmdExcute(szDID,(int)gwChannel,(int)cmdType,(char*)param, len);

	env->ReleaseStringUTFChars(did,szDID);

toRet:

	return nRet;
}

JNIEXPORT int JNICALL StartRecorder(
	JNIEnv *env, jobject obj, jstring did, jstring filepath
){
	if(g_pPPPPChannelMgt == NULL) return 0;

	char * szDID 		= (char*)env->GetStringUTFChars(did,0);
	char * szSavePath = (char*)env->GetStringUTFChars(filepath,0);
	
	if(szDID == NULL){
		env->ReleaseStringUTFChars(did,szDID);
		return 0;
	}

	Log2("Nativecaller Call StartRecorder on DID:%s.",szDID);

	int ret = g_pPPPPChannelMgt->StartRecorderByDID(szDID,szSavePath);

	env->ReleaseStringUTFChars(did,szDID);
	env->ReleaseStringUTFChars(filepath,szSavePath);
	
	return ret == 0 ? 1 : 0;
}

JNIEXPORT int JNICALL CloseRecorder(
	JNIEnv *env, jobject obj, jstring did
){	
	if(g_pPPPPChannelMgt == NULL) return 0;

	char * szDID = (char*)env->GetStringUTFChars(did,0);
	if(szDID == NULL){
		env->ReleaseStringUTFChars(did,szDID);
		return 0;
	}

	Log2("Nativecaller Call CloseRecorder on DID:%s.",szDID);

	int ret = g_pPPPPChannelMgt->CloseRecorderByDID(szDID);

	env->ReleaseStringUTFChars(did,szDID);
	
	return ret == 0 ? 1 : 0;
}

#ifdef PLATFORM_ANDROID

static JNINativeMethod Calls[] = {
	{"YUV4202RGB565", "([B[BII)I", (void*)YUV4202RGB565},
	{"AlwaysRunServer","(Ljava/lang/String;Ljava/lang/String;)V", (void*)AlwaysRunServer},
	{"PPPPInitialize", "(Ljava/lang/String;)I", (void*)PPPPInitialize},
	{"StartSearch", "(Ljava/lang/String;Ljava/lang/String;)V", (void*)StartSearch},
	{"CloseSearch", "()V", (void*)CloseSearch},
	{"PPPPManagementInit", "()V", (void*)PPPPManagementInit},
	{"PPPPManagementFree", "()V", (void*)PPPPManagementFree},    
	{"PPPPSetCallbackContext", "(Landroid/content/Context;)I", (void*)PPPPSetCallbackContext},
	{"StartPPPP", "(Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;)I", (void*)StartPPPP},
	{"ClosePPPP", "(Ljava/lang/String;)I", (void*)ClosePPPP},
	{"StartPPPPLivestream", "(Ljava/lang/String;Ljava/lang/String;III)I", (void*)StartPPPPLivestream},
	{"ClosePPPPLivestream", "(Ljava/lang/String;)I", (void*)ClosePPPPLivestream},
	{"StartRecorder", "(Ljava/lang/String;Ljava/lang/String;)I", (void*)StartRecorder},
	{"CloseRecorder", "(Ljava/lang/String;)I", (void*)CloseRecorder},
	{"SendHexsCommand", "(Ljava/lang/String;I[BI)I", (void*)SendHexsCommand},
	{"SendCtrlCommand", "(Ljava/lang/String;ILjava/lang/String;I)I", (void*)SendCtrlCommand},
	{"SendCtrlCommand_EX", "(Ljava/lang/String;IILjava/lang/Object;)I", (void*)SendCtrlCommand_EX},
	{"SetAudioStatus", "(Ljava/lang/String;I)I", (void*)SetAudioStatus},
	{"GetAudioStatus", "(Ljava/lang/String;)I", (void*)GetAudioStatus},
};

static int RegisterNativeMethods(JNIEnv* env, const char* className, JNINativeMethod * gMethods, int numMethods) {
    jclass clazz;
    clazz = env->FindClass(className);
    if (clazz == NULL) {
		Log3("can't find class by name:[%s].\n",className);
        return JNI_FALSE;
    }
    if (env->RegisterNatives(clazz, gMethods, numMethods) < 0) {
		Log3("can't regist natives for class:[%s].\n",className);
        return JNI_FALSE;
    }

    return JNI_TRUE;
}

static int RegisterNatives(JNIEnv* env){
    if (!RegisterNativeMethods(env, classPathName, Calls, sizeof(Calls) / sizeof(Calls[0]))){
        return JNI_FALSE;
    }

    return JNI_TRUE;
}

jint JNI_OnLoad(JavaVM* vm, void* reserved) 
{  
    g_JavaVM = vm ;

	JNIEnv* env = 0;
	jint result = -1;

	if (vm->GetEnv((void**) &env, JNI_VERSION_1_4) != JNI_OK) {
		goto bail;
	}

	if (!RegisterNatives(env)){
        goto bail;
    }
	
	result = JNI_VERSION_1_4;
	
    env->GetJavaVM(&g_JavaVM);   
bail:
	
	return result;
}

void JNI_OnUnload(JavaVM *vm, void *reserved)
{
   //F_LOG;
}

#endif


 
