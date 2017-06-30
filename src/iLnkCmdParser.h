#ifndef _ILNK_CMDPARSER_H_
#define _ILNK_CMDPARSER_H_

#ifdef PLATFORM_ANDROID
#include <jni.h>
#endif

#include "iLnkDefine.h"
#include "PPPP_Error.h"

extern int parseLogSet(JNIEnv *env,jobject cmdParam,logSet_t *logSet);
extern int parseLogSearchCondition(JNIEnv *env,jobject cmdParam,logSearchCondition_t *logSearchCondition);

#if 1
extern int parseItemType(JNIEnv *env,jobject cmdParam,int *itemType);
extern int parseFirmwareUpgrade(JNIEnv *env,jobject cmdParam,firmwareUpgrade_t *firmwareUpgrade);
extern int parseUserChk(JNIEnv *env,jobject cmdParam,user_t *usrInf);
extern int parseUserSetting(JNIEnv *env,jobject cmdParam,userSetting_t *userSetting);
extern int parseP2PSetting(JNIEnv *env,jobject cmdParam,openP2pSetting_t *p2pSetting);
extern int parseP2pNodeExt(JNIEnv *env,jobject devObj,P2pNodeExt_t_EX *devBase);
extern int parseOprPolicy(JNIEnv *env,jobject cmdParam,sysOprPolicy_t_EX*oprPolicy);
extern int parseDatetime(JNIEnv *env,jobject cmdParam,datetimeParam_t *datetime);
extern int parseAdc(JNIEnv *env,jobject cmdParam,adc_t *strAdc);
extern int parseGpioValue(JNIEnv *env,jobject cmdParam,gpio_t *strGpio);
extern int parseGpioAction(JNIEnv *env,jobject cmdParam,gpioAction_t *strGpioAction);
extern int parseSerialSetting(JNIEnv *env,jobject cmdParam,serialParam_t *serialSetting);
extern int parseSerialData(JNIEnv *env,jobject cmdParam,serialData_t *serialData);
extern int parseWifiSetting(JNIEnv *env,jobject cmdParam,wifiParam_t *wifiParams);
extern int parsePushParam(JNIEnv *env,jobject cmdParam,pushParam_t *pushParameter);
extern int parseRecNow(JNIEnv *env,jobject cmdParam,recNow_t *recNow);
extern int parsePicCap(JNIEnv *env,jobject cmdParam,picCap_t *picCap);
extern int parseStoragePolicy(JNIEnv *env,jobject cmdParam,mStoragePolicy_t *storagePolicy);
extern int parseSdRecSchSet(JNIEnv *env,jobject cmdParam,sdRecSchSet_t *sdRecSchSet);
extern int parseSearchRecordFile(JNIEnv *env,jobject cmdParam,sdRecSearchCondition_t *searchRecFile);
extern int parsePassThroghString(JNIEnv *env,jobject cmdParam,passThroghString_t *passThroghString);
extern int parseCameraCtrl(JNIEnv *env,jobject cmdParam,camCtrl_t_EX *camCtrl);
extern int parsePtzParam(JNIEnv *env,jobject cmdParam,ptzParamSet_t *ptzCtrl);
extern int parseFileInfo(JNIEnv *env,jobject cmdParam,fileTransParam_t *fileTransParam);
extern int parseAudioParamSet(JNIEnv *env,jobject cmdParam,audioParamSet_t *audioParamSet);
#endif
#endif

