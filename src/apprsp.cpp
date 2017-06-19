
//
//  apprsp.cpp
//  libvdp
//
//  Created by fork on 16/12/15.
//  Copyright ¬© 2016Âπ¥ fork. All rights reserved.
//
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "utility.h"
#include "apprsp.h"

#include "PPPP_API.h"
#include "iLnkDefine.h"
#include "iLnkRingBuf.h"
#include "PPPP_Error.h"


int RespUserGet(
int             Cmd,
void *          Msg,
char *          JsonBuffer,
int             JsonBufferSize
){
	userSetting_t * userInfoGet = (userSetting_t *)Msg;
	
	sprintf(JsonBuffer,"{\"%s\":\"%s\",\"%s\":\"%s\",\"%s\":\"%s\",\"%s\":\"%s\",\"%s\":\"%s\",\"%s\":\"%s\"}",
            "user1",userInfoGet->user1,
            "pwd1",userInfoGet->pwd1,
            "user2",userInfoGet->user2,
            "pwd2",userInfoGet->pwd2,
            "user3",userInfoGet->user3,
            "pwd3",userInfoGet->pwd3
            );
	return 0;
}

int RespSDInfoGet(
int             Cmd,
void *          Msg,
char *          JsonBuffer,
int             JsonBufferSize
){
	SDINFO_t * RespSDINFO = (SDINFO_t *)Msg;

	sprintf(JsonBuffer,"{\"%s\":\"%d\",\"%s\":\"%d\",\"%s\":\"%d\",\"%s\":\"%d\"}",
            "status",RespSDINFO->status,
            "totalSize",RespSDINFO->totalSize,
            "usedSize",RespSDINFO->usedSize,
            "badSize",RespSDINFO->badSize
            );
    
    return 0;
}
int RespWIFIInfoGet(
int             Cmd,
void *          Msg,
char *          JsonBuffer,
int             JsonBufferSize
){
	wifiParam_t * RespWIFIINFO = (wifiParam_t *)Msg;
	sprintf(JsonBuffer,"{\"%s\":\"%d\",\"%s\":\"%d\",\"%s\":\"%d\",\"%s\":\"%d\",\"%s\":\"%d\",\"%s\":\"%d\",\"%s\":\"%s\",\"%s\":\"%s\",\"%s\":\"%s\",\"%s\":\"%s\",\"%s\":\"%s\",\"%s\":\"%s\",\"%s\":\"%s\"}",
			"enable",RespWIFIINFO->enable,
			"wifiStatus",RespWIFIINFO->wifiStatus,
			"mode",RespWIFIINFO->mode,
			"channel",RespWIFIINFO->channel,
			"authtype",RespWIFIINFO->authtype,
			"dhcp",RespWIFIINFO->dhcp,
            "ssid",RespWIFIINFO->ssid,
            "psk",RespWIFIINFO->psk,
            "ip",RespWIFIINFO->ip,
            "mask",RespWIFIINFO->mask,
            "gw",RespWIFIINFO->gw,
            "dns1",RespWIFIINFO->dns1,
            "dns2",RespWIFIINFO->dns2
            );

    return 0;
	
}

int RespDateTimeGet(
int 			Cmd,
void *			Msg,
char *			JsonBuffer,
int 			JsonBufferSize
){
	datetimeParam_t * RespDatetimeParamGet = (datetimeParam_t *)Msg;
	
	sprintf(JsonBuffer,"{\"%s\":\"%d\",\"%s\":\"%d\",\"%s\":\"%d\",\"%s\":\"%d\",\"%s\":\"%s\"}",
			"now",RespDatetimeParamGet->now,
			"tz",RespDatetimeParamGet->tz,
			"ntp_enable",RespDatetimeParamGet->ntp_enable,
			"xia_ling_shi_flag_status",RespDatetimeParamGet->tz,
			"ntp_svr",RespDatetimeParamGet->ntp_svr
			);
	return 0;		
}

int RespThirdPushGet(
int 			Cmd,
void *			Msg,
char *			JsonBuffer,
int 			JsonBufferSize
){
	pushParamList_t * thirdPushList = (pushParamList_t *)Msg;
	
	//ªÒ»°µ⁄»˝∑ΩÕ∆ÀÕ
	return 0;		
}

int RespOprpolityGet(
int 			Cmd,
void *			Msg,
char *			JsonBuffer,
int 			JsonBufferSize
){
	sysOprPolicy_t * sysOprGet = (sysOprPolicy_t *)Msg;
	Log3("===================>Sprintf Begin");
	sprintf(JsonBuffer,"{\"%s\":\"%hhd\",\"%s\":\"%hhd\",\"%s\":\"%s\",\"%s\":\"%d\",\"%s\":\"%d\",\"%s\":\"%d\",\"%s\":\"%hhd\",\"%s\":\"%hhd\",\"%s\":\"%d\",\"%s\":\"%hhd\",\"%s\":\"%hhd\",\"%s\":\"%d\",\"%s\":\"%hhd\",\"%s\":\"%hhd\",\"%s\":\"%hhd\",\"%s\":\"%hhd\",\"%s\":\"%hhd\",\"%s\":\"%hhd\",\"%s\":\"%s\",\"%s\":\"%d\"}",
			"adcSelect",sysOprGet->adcSelect,
			"adcChkInterval",sysOprGet->adcChkInterval,
			"reserved1",sysOprGet->reserved1,
			"gpioSelect",sysOprGet->gpioSelect,
			"gpioChkInterval",sysOprGet->gpioChkInterval,
			"sysRunTime",sysOprGet->sysRunTime,
			"rcdPicEnable",sysOprGet->rcdPicEnable,
			"rcdPicSize",sysOprGet->rcdPicSize,
			"rcdPicInterval",sysOprGet->rcdPicInterval,
			"rcdAvEnable",sysOprGet->rcdAvEnable,
			"rcdAvSize",sysOprGet->rcdAvSize,
			"rcdAvTime",sysOprGet->rcdAvTime,
			"pushEnable",sysOprGet->pushEnable,
			"alarmEnable",sysOprGet->alarmEnable,
			"wifiEnable",sysOprGet->wifiEnable,
			"reserved",sysOprGet->reserved,
			"powerMgrEnable",sysOprGet->powerMgrEnable,
			"powerMgrCountDown",sysOprGet->powerMgrCountDown,
			"reserved2",sysOprGet->reserved2,
			"powerMgrSchedule",sysOprGet->powerMgrSchedule			
	);
	
	Log3("SystemOprpolity GetInfo ========>%s",JsonBuffer);
	return 0;		
}

int RespCameraGet(
int 			Cmd,
void *			Msg,
char *			JsonBuffer,
int 			JsonBufferSize
){

	cameraParams_t* imageParamGet = (cameraParams_t *)Msg;

	sprintf(JsonBuffer,"{\"%s\":\"%d\",\"%s\":\"%d\",\"%s\":\"%d\",\"%s\":\"%d\",\"%s\":\"%d\",\"%s\":\"%d\",\"%s\":\"%d\",\"%s\":\"%d\",\"%s\":\"%d\",\"%s\":\"%d\"}",
			"resolution",imageParamGet->resolution,
			"brightness",imageParamGet->brightness,
			"contrast",imageParamGet->contrast,
			"saturation",imageParamGet->saturation,
			"sharpness",imageParamGet->sharpness,
			"frameRate",imageParamGet->frameRate,
			"bitRate",imageParamGet->bitRate,
			"rotate",imageParamGet->rotate,
			"ircut",imageParamGet->ircut,
			"mode",imageParamGet->mode
	);
	
	return 0;		
}

int RespGpioValueGet(
int 			Cmd,
void *			Msg,
char *			JsonBuffer,
int 			JsonBufferSize
){

	gpio_t * gpioValueGet = (gpio_t *)Msg;
	
	sprintf(JsonBuffer,"{\"%s\":\"%hu\",\"%s\":\"%hu\"}",
			"gpioSelect",gpioValueGet->gpioSelect,
			"gpioValue",gpioValueGet->gpioValue
	);
	
	return 0;		
}

int RespSDRecordGet(
int 			Cmd,
void *			Msg,
char *			JsonBuffer,
int 			JsonBufferSize
){

	AVRecPolicy_t * AVRecPolicy = (AVRecPolicy_t *)Msg;

	mRfsRecConfig_t	mRfsRecConfig;
	memset(&mRfsRecConfig,0,sizeof(mRfsRecConfig_t));
	mRfsRecConfig = AVRecPolicy->recConf;

	mRfsStoragePolicy_t	mRfsStoragePolicy;
	memset(&mRfsStoragePolicy,0,sizeof(mRfsStoragePolicy_t));
	mRfsStoragePolicy = AVRecPolicy->storagePolicy;

	sprintf(JsonBuffer,"{\"%s\":\"%d\",\"%s\":\"%d\",\"%s\":\"%d\",\"%s\":\"%d\",\"%s\":\"%d\",\"%s\":\"%d\",\"%s\":\"%d\",\"%s\":\"%hhu\",\"%s\":\"%d\",\"%s\":\"%d\",\"%s\":\"%hhu\",\"%s\":\"%hhu\",\"%s\":\"%s\"}",
			"rcVideoWidth",mRfsRecConfig.rcVideoWidth,
			"rcVideoHeight",mRfsRecConfig.rcVideoHeight,
			"rcVideoRate",mRfsRecConfig.rcVideoRate,
			"rcVideoMaxBitrate",mRfsRecConfig.rcVideoMaxBitrate,
			"rcAudioSamplerate",mRfsRecConfig.rcAudioSamplerate,
			"rcAudioBitWidth",mRfsRecConfig.rcAudioBitWidth,
			"rcAudioMaxBitrate",mRfsRecConfig.rcAudioMaxBitrate,
			"rcAudioTrack",mRfsRecConfig.rcAudioTrack,

			"spMaxHour",mRfsStoragePolicy.spMaxHour,
			"spFullThreshold",mRfsStoragePolicy.spFullThreshold,
			"spRecycle",mRfsStoragePolicy.spRecycle,
			"spCleanData",mRfsStoragePolicy.spCleanData,
			"spReserved",mRfsStoragePolicy.spReserved
	);
	
	return 0;		
}

static APP_CMD_RESP hACR[] = { 
	//”√ªßªÒ»°
	{CMD_SYSTEM_USER_GET,RespUserGet},
	//SD–≈œ¢∑µªÿ
	{CMD_SD_INFO_GET,RespSDInfoGet},
	//WIFI–≈œ¢∑µªÿ
	{CMD_NET_WIFISETTING_GET,RespWIFIInfoGet},
	//ªÒ»°…Ë±∏ ±º‰
	{CMD_SYSTEM_DATETIME_GET,RespDateTimeGet},
	//µ⁄»˝∑ΩÕ∆ÀÕªÒ»°
	{CMD_PUSHPARAM_GET,RespThirdPushGet},
	//ªÒ»°œµÕ≥‘À––≤Œ ˝
	{CMD_SYSTEM_OPRPOLICY_GET,RespOprpolityGet},
	//ªÒ»°…„œÒÕ∑ÕºœÒ≤Œ ˝
	{CMD_CAMPARAMS_GET,RespCameraGet},
	//ªÒ»°GPIO÷µ
	{CMD_INTERFACE_GPIOVALUE_GET,RespGpioValueGet},
	//ªÒ»°¬ºœÒ¥Ê¥¢≤ﬂ¬‘
	{CMD_SD_RECPOLICY_GET,RespSDRecordGet},
	{0,NULL}
};

int Rsp2Json(
    int             Cmd,
    void *          Msg,
    char *          JsonBuffer,
    int             JsonBufferSize
){
    //  Log3("CCCCCCCCCCCCall RecvCmds:[%08X].\n",Cmd);
    
    int i = 0;
    for(i = 0;hACR[i].RspJson != NULL;i++){
        if(Cmd == hACR[i].CmdType){
            
            Log3("app cmd recv IOCTRL cmd:[%03X].",hACR[i].CmdType);
            
            return hACR[i].RspJson(
                       Cmd,
                       Msg,
                       JsonBuffer,
                       JsonBufferSize);
        }
    }
    
    return -1;
}

