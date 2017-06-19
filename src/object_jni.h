/* DO NOT EDIT THIS FILE - it is machine generated */
#ifndef _OBJECT_JNI_H_
#define _OBJECT_JNI_H_

#ifdef PLATFORM_ANDROID

#include <jni.h>

#define ANDROID_CLASS_PATH  "com/edwintech/vdp/jni/Avapi" 	// for vdp
//#define ANDROID_CLASS_PATH  "com/rccar/jni/Avapi"			// for remote control toys

#else

#define JNIEXPORT
#define JNICALL

class JNIEnv{
public:
    void * GetByteArrayElements(char * a, int b);
    void   ReleaseByteArrayElements(char * a, char * b, int c);
    char * GetStringUTFChars(char * a, int b);
    void   ReleaseStringUTFChars(char * a, char * b);
    char * NewStringUTF(char * a);
    void   DeleteLocalRef(char * a);
    char * NewByteArray(int b);
    void   CallVoidMethod(void * a, void * b, ...);
};

typedef void * jobject;
typedef char * jbyteArray;
typedef char * jstring;
typedef  int   jint;
typedef void * jmethodID;
typedef char   jbyte;

extern JNIEnv iOSEnv;

void CBSearchResults(int nTrue,const char * szMac,const char * szName,const char * szDID,const char * szIP,int nPort,int nType);

void CBUILayerNotify(const char * szDID,int nCmd,const char * szJson);

void CBCmdRecv(const char * szDID,int sessionID,int nCmd,char * buf,int len);

void CBVideoDataProcess(const char * szDID,char * lpImage,int nType,int nLens,int nW,int nH,int nTimestamp);

void CBConnectionNotify(const char * szDID,int nType,int nParam);

void CBAlarmNotifyByDevice(const char * szDID,const char * szSID,const char * szType,const char * szTime);

#endif

#ifdef __cplusplus
extern "C" 
{
#endif
/*
* Class:     com_magc_jni_HelloWorld12  
* Method:    DisplayHello13  
* Signature: ()V14  
*/

JNIEXPORT int  JNICALL YUV4202RGB565(JNIEnv *, jobject, jbyteArray, jbyteArray, jint, jint);

JNIEXPORT int  JNICALL PPPPSetCallbackContext(JNIEnv *, jobject, jobject);

JNIEXPORT void JNICALL AlwaysRunServer(JNIEnv*, jobject, jstring, jstring);

JNIEXPORT int JNICALL PPPPInitialize(JNIEnv *, jobject, jstring);

JNIEXPORT void JNICALL StartSearch(JNIEnv *, jobject, jstring, jstring);

JNIEXPORT void JNICALL CloseSearch(JNIEnv *, jobject);

JNIEXPORT void JNICALL PPPPManagementInit(JNIEnv *, jobject);

JNIEXPORT void JNICALL PPPPManagementFree(JNIEnv *, jobject);

JNIEXPORT int  JNICALL StartPPPP(JNIEnv *, jobject, jstring, jstring, jstring, jstring);

JNIEXPORT int  JNICALL ClosePPPP(JNIEnv *, jobject, jstring);

JNIEXPORT int  JNICALL StartPPPPLivestream(JNIEnv *, jobject, jstring, jstring, jint, jint, jint);

JNIEXPORT int  JNICALL ClosePPPPLivestream(JNIEnv *, jobject, jstring);

JNIEXPORT int  JNICALL StartRecorder(JNIEnv *, jobject, jstring, jstring);

JNIEXPORT int  JNICALL CloseRecorder(JNIEnv *, jobject, jstring);

JNIEXPORT int  JNICALL SendHexsCommand(JNIEnv *, jobject, jstring, jint, jbyteArray, jint);

JNIEXPORT int  JNICALL SendCtrlCommand(JNIEnv *, jobject, jstring, jint, jstring, jint);

JNIEXPORT int  JNICALL SendCtrlCommand_EX(JNIEnv * ,jobject,jstring,jint,jint,jobject);


JNIEXPORT int  JNICALL SetAudioStatus(JNIEnv *, jobject, jstring, jint);

JNIEXPORT int  JNICALL GetAudioStatus(JNIEnv *, jobject, jstring);

#ifdef __cplusplus
}
#endif

#endif


