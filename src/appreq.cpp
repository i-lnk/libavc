#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "utility.h"
#include "appreq.h"

#include "PPPP_API.h"
#include "iLnkDefine.h"
#include "iLnkRingBuf.h"
#include "PPPP_Error.h"
#include "PPPPChannel.h"


static int Define_gwChannel = 0xff;


static char encoding_table[] = {
'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H',
'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P',
'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X',
'Y', 'Z', 'a', 'b', 'c', 'd', 'e', 'f',
'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n',
'o', 'p', 'q', 'r', 's', 't', 'u', 'v',
'w', 'x', 'y', 'z', '0', '1', '2', '3',
'4', '5', '6', '7', '8', '9', '+', '/',
};

static char * decoding_table = NULL;

static void Base64DecodeTableInit() {
    static char table[256] = {0};
    decoding_table = table;
    
    int i = 0;
    for (i = 0; i < 64; i++)
        decoding_table[(unsigned char) encoding_table[i]] = i;
}

static int Base64Decode(
	const char *data,
	char * output,
    int input_length,
    int output_size
) {
    if (decoding_table == NULL) Base64DecodeTableInit();

    if (input_length % 4 != 0) return -1;

    int output_length = input_length / 4 * 3;
    if (data[input_length - 1] == '=') output_length--;
    if (data[input_length - 2] == '=') output_length--;

    unsigned char *decoded_data = (unsigned char *)output;
    if (decoded_data == NULL) return -1;
	if (output_length > output_size) return -1;

	unsigned int i = 0;
	unsigned int j = 0;

    for (i = 0, j = 0; i < input_length;) {

        uint32_t sextet_a = data[i] == '=' ? 0 & i++ : decoding_table[data[i++]];
        uint32_t sextet_b = data[i] == '=' ? 0 & i++ : decoding_table[data[i++]];
        uint32_t sextet_c = data[i] == '=' ? 0 & i++ : decoding_table[data[i++]];
        uint32_t sextet_d = data[i] == '=' ? 0 & i++ : decoding_table[data[i++]];

        uint32_t triple = (sextet_a << 3 * 6)
        + (sextet_b << 2 * 6)
        + (sextet_c << 1 * 6)
        + (sextet_d << 0 * 6);

        if (j < output_length) decoded_data[j++] = (triple >> 2 * 8) & 0xFF;
        if (j < output_length) decoded_data[j++] = (triple >> 1 * 8) & 0xFF;
        if (j < output_length) decoded_data[j++] = (triple >> 0 * 8) & 0xFF;
    }

    return output_length;
}

//
//	string safe copy between string
//
char * GetCgiParam(
	char *			To,
	const char *	From,
	int				ToBufferSize,
	const char *	Begin,
	const char *	End
){
	if(To == NULL || From == NULL) return NULL;
	
	const char * lpBegin = NULL;
	const char * lpEnd = NULL;

	if(Begin != NULL) lpBegin = strstr(From,Begin);
	if(lpBegin == NULL){
		return NULL;
//		lpBegin = From;
	}else{
		lpBegin = lpBegin + strlen(Begin);
	}

	int CopyLens = 0;
	int	CopySize = 0;
	lpEnd = strstr(lpBegin,End);
	if(lpEnd == NULL){
		CopySize = strlen(lpBegin);
		CopyLens = ToBufferSize > CopySize ? CopySize : ToBufferSize;
	}else{
		CopySize = lpEnd - lpBegin;
		CopyLens = ToBufferSize > CopySize ? CopySize : ToBufferSize;
	}
	
	int DataLens = Base64Decode(lpBegin,To,CopyLens,ToBufferSize);
	if(DataLens < 0) return NULL;

//	Log3("GetCgiParam .....[%s%s]",Begin,To);
//	memcpy(To,lpBegin,CopyLens);

	return To;
}
char * ResolveData(
	char *			To,
	const char *	From,
	const char *	Begin,
	const char *	End)
{
	char * beginbuf = strstr(From,Begin);
	beginbuf += strlen(Begin);
    char * endbuf   = strstr(beginbuf,End);
	
    strncat(To,beginbuf,endbuf-beginbuf);

//	Log3("GetCgiParam .....[%s%s]",Begin,To);

	return To;
}

int Userlogin(
	int				sessionid,
	int				avMsgType,
	const char *	szCgi,
	void *			lpParams
){
	CPPPPChannel * hPC=(CPPPPChannel *)lpParams;
	char * Cgi = (char*)szCgi;
	user_t userlogin;
	memset(&userlogin,0,sizeof(user_t));

	char username[32] = {0};
	char userpass[128] = {0};
	
	ResolveData(username,Cgi,"name=","&");
	ResolveData(userpass,Cgi,"password=","&");

//	Log3("userlogin.name =%s , userlogin.pass = %s",username,userpass);
	
	strncpy(userlogin.name,username,sizeof(username));
	strncpy(userlogin.pass,userpass,sizeof(userpass));
	return hPC->IOCmdSend(Define_gwChannel,avMsgType,(char *)&userlogin,sizeof(user_t));
}
int UserSet(
	int				sessionid,
	int				avMsgType,
	const char *	szCgi,
	void *			lpParams
){
	CPPPPChannel * hPC=(CPPPPChannel *)lpParams;
	char * Cgi = (char*)szCgi;
	userSetting_t userSet;
	memset(&userSet,0,sizeof(userSetting_t));

	char user1[32] = {0};
	char pwd1[128] = {0};
	char user2[32] = {0};
	char pwd2[128] = {0};
	char user3[32] = {0};
	char pwd3[128] = {0};
	
	GetCgiParam(user1,Cgi,sizeof(user1),"user1=","&");
	GetCgiParam(pwd1,Cgi,sizeof(user1),"pwd1=","&");
	GetCgiParam(user2,Cgi,sizeof(user1),"user2=","&");
	GetCgiParam(pwd2,Cgi,sizeof(user1),"pwd2=","&");
	GetCgiParam(user3,Cgi,sizeof(user1),"user3=","&");
	GetCgiParam(pwd3,Cgi,sizeof(user1),"pwd3=","&");

	strncpy(userSet.user1,user1,sizeof(user1));
	strncpy(userSet.pwd1,pwd1,sizeof(pwd1));
	strncpy(userSet.user2,user2,sizeof(user2));
	strncpy(userSet.pwd2,pwd2,sizeof(pwd2));
	strncpy(userSet.user3,user3,sizeof(user3));
	strncpy(userSet.pwd3,pwd3,sizeof(pwd3));

	return hPC->IOCmdSend(Define_gwChannel,avMsgType,(char *)&userSet,sizeof(userSetting_t));
	
}
int UserGet(
	int				sessionid,
	int				avMsgType,
	const char *	szCgi,
	void *			lpParams
){
	CPPPPChannel * hPC=(CPPPPChannel *)lpParams;
	return hPC->IOCmdSend(Define_gwChannel,avMsgType,NULL,0);
}

int AudioStart(
	int				sessionid,
	int				avMsgType,
	const char *	szCgi,
	void *			lpParams
){
	CPPPPChannel * hPC=(CPPPPChannel *)lpParams;
	return hPC->IOCmdSend(Define_gwChannel,avMsgType,NULL,0);
}

int AudioStop(
	int				sessionid,
	int				avMsgType,
	const char *	szCgi,
	void *			lpParams
){
	CPPPPChannel * hPC=(CPPPPChannel *)lpParams;
	return hPC->IOCmdSend(Define_gwChannel,avMsgType,NULL,0);
}

int TalkStart(
	int				sessionid,
	int				avMsgType,
	const char *	szCgi,
	void *			lpParams
){
	CPPPPChannel * hPC=(CPPPPChannel *)lpParams;
	return hPC->IOCmdSend(Define_gwChannel,avMsgType,NULL,0);
}
int TalkStop(
	int 			sessionid,
	int 			avMsgType,
	const char *	szCgi,
	void *			lpParams
){
	CPPPPChannel * hPC=(CPPPPChannel *)lpParams;
	return hPC->IOCmdSend(Define_gwChannel,avMsgType,NULL,0);
}

int VideoStart(
	int				sessionid,
	int				avMsgType,
	const char *	szCgi,
	void *			lpParams
){
	CPPPPChannel * hPC=(CPPPPChannel *)lpParams;
	return hPC->IOCmdSend(Define_gwChannel,avMsgType,NULL,0);
}
int VideoStop(
	int				sessionid,
	int				avMsgType,
	const char *	szCgi,
	void *			lpParams
){
	CPPPPChannel * hPC=(CPPPPChannel *)lpParams;
	return hPC->IOCmdSend(Define_gwChannel,avMsgType,NULL,0);
}

int VideoPlayBackStart(
	int				sessionid,
	int				avMsgType,
	const char *	szCgi,
	void *			lpParams
){
	char * Cgi = (char *)szCgi;
	CPPPPChannel * hPC=(CPPPPChannel *)lpParams;
	fileTransParam_t fileParam ;
	memset(&fileParam,0,sizeof(fileTransParam_t));

	char offset[8] = {0};
	GetCgiParam(fileParam.filename,Cgi,sizeof(fileParam.filename),"filename=","&");
	GetCgiParam(offset,Cgi,sizeof(offset),"offset=","&");

	fileParam.offset = atoi(offset);

	return hPC->IOCmdSend(Define_gwChannel,avMsgType,(char *)&fileParam,sizeof(fileTransParam_t));
	
}

int VideoPlayBackStop(
	int				sessionid,
	int				avMsgType,
	const char *	szCgi,
	void *			lpParams
){
	CPPPPChannel * hPC=(CPPPPChannel *)lpParams;
	return hPC->IOCmdSend(Define_gwChannel,avMsgType,NULL,0);
}

int CameraSet(
	int				sessionid,
	int				avMsgType,
	const char *	szCgi,
	void *			lpParams
){
	char * Cgi = (char *)szCgi;
	CPPPPChannel * hPC=(CPPPPChannel *)lpParams;
	
	char cmdValue_SET[4] = {0};
	char videoChannel_SET[4] = {0};
	
	GetCgiParam(cmdValue_SET,Cgi,sizeof(cmdValue_SET),"param=","&");
	GetCgiParam(videoChannel_SET,Cgi,sizeof(videoChannel_SET),"value=","&");

	if(atoi(cmdValue_SET) == 0){
		switch(atoi(videoChannel_SET)){
            case 0: // HD
                hPC->MW = 1080;
                hPC->MH = 720;
            case 1: //VGA
                hPC->MW = 640;
                hPC->MH = 480;
            case 3: //QVGA
                hPC->MW = 320;
                hPC->MH = 240;
        }
	}
	
	camCtrl_t cameraCtrl;
	memset(&cameraCtrl,0,sizeof(camCtrl_t));

	cameraCtrl.param= atoi(cmdValue_SET);
	switch(atoi(cmdValue_SET)){
		case VIDEO_PARAM_TYPE_DEFAULT:
			break;
		case VIDEO_PARAM_TYPE_RESOLUTION:
			cameraCtrl.u.resolution = atoi(videoChannel_SET);
			break;
		case VIDEO_PARAM_TYPE_BRIGHTNESS:
			cameraCtrl.u.brightness = atoi(videoChannel_SET);
			break;
		case VIDEO_PARAM_TYPE_CONTRAST:
			cameraCtrl.u.contrast = atoi(videoChannel_SET);
			break;
		case VIDEO_PARAM_TYPE_SATURATION:
			cameraCtrl.u.saturation = atoi(videoChannel_SET);
			break;
		case VIDEO_PARAM_TYPE_SHARPNESS:
			cameraCtrl.u.sharpness = atoi(videoChannel_SET);
			break;
		case VIDEO_PARAM_TYPE_FRAMERATE:
			cameraCtrl.u.frameRate = atoi(videoChannel_SET);
			break;
		case VIDEO_PARAM_TYPE_BITRATE:
			cameraCtrl.u.bitRate = atoi(videoChannel_SET);
			break;
		case VIDEO_PARAM_TYPE_ROTATE:
			cameraCtrl.u.rotate = atoi(videoChannel_SET);
			break;
		case VIDEO_PARAM_TYPE_IRCUT:
			cameraCtrl.u.ircut = atoi(videoChannel_SET);
			break;
		case VIDEO_PARAM_TYPE_OSD:
			cameraCtrl.u.osd = atoi(videoChannel_SET);
			break;
		case VIDEO_PARAM_TYPE_MOVEDETECTION:
			cameraCtrl.u.moveDetect = atoi(videoChannel_SET);
			break;
		case VIDEO_PARAM_TYPE_MODE:
			cameraCtrl.u.mode = atoi(videoChannel_SET);
			break;
	}		

	
	return hPC->IOCmdSend(Define_gwChannel,avMsgType,(char *)&cameraCtrl,sizeof(camCtrl_t));
}

int CameraGet(
	int 			sessionid,
	int 			avMsgType,
	const char *	szCgi,
	void *			lpParams
){
	CPPPPChannel * hPC=(CPPPPChannel *)lpParams;
	return hPC->IOCmdSend(Define_gwChannel,avMsgType,NULL,0);
}

int CameraReboot(
	int				sessionid,
	int				avMsgType,
	const char *	szCgi,
	void *			lpParams
){
	CPPPPChannel * hPC=(CPPPPChannel *)lpParams;
	return hPC->IOCmdSend(Define_gwChannel,avMsgType,NULL,0);
}

int CameraShouDown(
	int				sessionid,
	int				avMsgType,
	const char *	szCgi,
	void *			lpParams
){
	CPPPPChannel * hPC=(CPPPPChannel *)lpParams;
	return hPC->IOCmdSend(Define_gwChannel,avMsgType,NULL,0);
}

int OprpolitySet(
	int				sessionid,
	int				avMsgType,
	const char *	szCgi,
	void *			lpParams
){

	char * Cgi = (char *)szCgi;	
	CPPPPChannel * hPC=(CPPPPChannel *)lpParams;
	sysOprPolicy_t Oprpolity;
	memset(&Oprpolity,0,sizeof(sysOprPolicy_t));

	char adcSelect[8] = {0};
	char adcChkInterval[8] = {0};
	char reserved1[8] = {0};
	char gpioSelect[8] = {0};
	char gpioChkInterval[8] = {0};
	char sysRunTime[8] = {0};
	char rcdPicEnable[8] = {0};
	char rcdPicSize[8] = {0};
	char rcdPicInterval[8] = {0};
	char rcdAvEnable[8] = {0};
	char rcdAvSize[8] = {0};
	char rcdAvTime[8] = {0};
	char pushEnable[8] = {0};
	char alarmEnable[8] = {0};
	char wifiEnable[8] = {0};
	char reserved[8] = {0};
	char powerMgrEnable[8] = {0};
	char powerMgrCountDown[8] = {0};
	char reserved2[8] = {0};
	char powerMgrSchedule[16] = {0};
	
	GetCgiParam(adcSelect,Cgi,sizeof(adcSelect),"adcSelect=","&");
	GetCgiParam(adcChkInterval,Cgi,sizeof(adcChkInterval),"adcChkInterval=","&");
	GetCgiParam(gpioSelect,Cgi,sizeof(gpioSelect),"gpioSelect=","&");
	GetCgiParam(gpioChkInterval,Cgi,sizeof(gpioChkInterval),"gpioChkInterval=","&");
	GetCgiParam(sysRunTime,Cgi,sizeof(sysRunTime),"sysRunTime=","&");
	GetCgiParam(rcdPicEnable,Cgi,sizeof(rcdPicEnable),"rcdPicEnable=","&");
	GetCgiParam(rcdPicSize,Cgi,sizeof(rcdPicSize),"rcdPicSize=","&");
	GetCgiParam(rcdPicInterval,Cgi,sizeof(rcdPicInterval),"rcdPicInterval=","&");
	GetCgiParam(rcdAvEnable,Cgi,sizeof(rcdAvEnable),"rcdAvEnable=","&");
	GetCgiParam(rcdAvSize,Cgi,sizeof(rcdAvSize),"rcdAvSize=","&");
	GetCgiParam(rcdAvTime,Cgi,sizeof(rcdAvTime),"rcdAvTime=","&");
	GetCgiParam(pushEnable,Cgi,sizeof(pushEnable),"pushEnable=","&");
	GetCgiParam(alarmEnable,Cgi,sizeof(alarmEnable),"alarmEnable=","&");
	GetCgiParam(wifiEnable,Cgi,sizeof(wifiEnable),"wifiEnable=","&");
	GetCgiParam(reserved,Cgi,sizeof(reserved),"reserved=","&");
	GetCgiParam(powerMgrEnable,Cgi,sizeof(powerMgrEnable),"powerMgrEnable=","&");
	GetCgiParam(powerMgrCountDown,Cgi,sizeof(powerMgrCountDown),"powerMgrCountDown=","&");
	GetCgiParam(powerMgrSchedule,Cgi,sizeof(powerMgrSchedule),"powerMgrSchedule=","&");
	
	Oprpolity.adcSelect = atoi(adcSelect);
	Oprpolity.adcChkInterval = atoi(adcChkInterval);
	Oprpolity.gpioSelect = atoi(gpioSelect);
	Oprpolity.gpioChkInterval = atoi(gpioChkInterval);
	Oprpolity.sysRunTime = atoi(sysRunTime);
	Oprpolity.rcdPicEnable = atoi(rcdPicEnable);
	Oprpolity.rcdPicSize = atoi(rcdPicSize);
	Oprpolity.rcdPicInterval = atoi(rcdPicInterval);
	Oprpolity.rcdAvEnable = atoi(rcdAvEnable);
	Oprpolity.rcdAvSize= atoi(rcdAvSize);
	Oprpolity.rcdAvTime = atoi(rcdAvTime);
	Oprpolity.pushEnable = atoi(pushEnable);
	Oprpolity.alarmEnable = atoi(alarmEnable);
	Oprpolity.wifiEnable = atoi(wifiEnable);
	Oprpolity.reserved = atoi(reserved);
	Oprpolity.powerMgrEnable = atoi(powerMgrEnable);
	Oprpolity.powerMgrCountDown = atoi(powerMgrCountDown);
	Oprpolity.powerMgrSchedule = atoi(powerMgrSchedule);

	return hPC->IOCmdSend(Define_gwChannel,avMsgType,(char *)&Oprpolity,sizeof(sysOprPolicy_t));
}

int OprpolityGet(
	int				sessionid,
	int				avMsgType,
	const char *	szCgi,
	void *			lpParams
){
	CPPPPChannel * hPC=(CPPPPChannel *)lpParams;
	return hPC->IOCmdSend(Define_gwChannel,avMsgType,NULL,0);
}


int Item_Recover(
	int				sessionid,
	int				avMsgType,
	const char *	szCgi,
	void *			lpParams
){
	CPPPPChannel * hPC=(CPPPPChannel *)lpParams;
	char * Cgi = (char *)szCgi;
	char Recover[4] = {0};
	GetCgiParam(Recover,Cgi,sizeof(Recover),"item_type","&");
	
	return hPC->IOCmdSend(Define_gwChannel,avMsgType,Recover,sizeof(Recover));
}

int GpioValueSet(
	int				sessionid,
	int				avMsgType,
	const char *	szCgi,
	void *			lpParams
){
	CPPPPChannel * hPC=(CPPPPChannel *)lpParams;
	char * Cgi = (char *)szCgi;
	gpio_t gpioSet;
	memset(&gpioSet,0,sizeof(gpio_t));
	
	char gpioSelect[4]  = {0};
	char gpioValue[4]	= {0};
	
	GetCgiParam(gpioSelect,Cgi,sizeof(gpioSelect),"gpioSelect","&");
	GetCgiParam(gpioValue,Cgi,sizeof(gpioValue),"gpioValue","&");

	gpioSet.gpioSelect = atoi(gpioSelect);
	gpioSet.gpioValue= atoi(gpioValue);

	return hPC->IOCmdSend(Define_gwChannel,avMsgType,(char *)&gpioSet,sizeof(gpio_t));
}

int GpioValueGet(
	int				sessionid,
	int				avMsgType,
	const char *	szCgi,
	void *			lpParams
){

	CPPPPChannel * hPC=(CPPPPChannel *)lpParams;
	return hPC->IOCmdSend(Define_gwChannel,avMsgType,NULL,0);
	
}


int SDInfoGet(
	int				sessionid,
	int				avMsgType,
	const char *	szCgi,
	void *			lpParams
){
	CPPPPChannel * hPC=(CPPPPChannel *)lpParams;
	return hPC->IOCmdSend(Define_gwChannel,avMsgType,NULL,0);
}

int SDForMat(
	int				sessionid,
	int				avMsgType,
	const char *	szCgi,
	void *			lpParams
){
	CPPPPChannel * hPC=(CPPPPChannel *)lpParams;
	return hPC->IOCmdSend(Define_gwChannel,avMsgType,NULL,0);
}

int SDRecordSet(
	int				sessionid,
	int				avMsgType,
	const char *	szCgi,
	void *			lpParams
){
	CPPPPChannel * hPC=(CPPPPChannel *)lpParams;
	char * Cgi = (char *)szCgi;
	char rcVideoWidth[4] = {0};
	char rcVideoHeight[4] = {0};
	char rcVideoRate[4] = {0};
	char rcVideoMaxBitrate[4] = {0};
	char rcAudioSamplerate[4] = {0};
	char rcAudioBitWidth[4] = {0};
	char rcAudioMaxBitrate[4] = {0};
	char rcAudioTrack[4] = {0};
	char spMaxHour[4] = {0};
	char spFullThreshold[4] = {0};
	char spRecycle[4] = {0};
	char spCleanData[4] = {0};
	char spReserved[2] = {0};
	
	GetCgiParam(rcVideoWidth,Cgi,sizeof(rcVideoWidth),"rcVideoWidth","&");
	GetCgiParam(rcVideoHeight,Cgi,sizeof(rcVideoHeight),"rcVideoHeight","&");
	GetCgiParam(rcVideoRate,Cgi,sizeof(rcVideoRate),"rcVideoRate","&");
	GetCgiParam(rcVideoMaxBitrate,Cgi,sizeof(rcVideoMaxBitrate),"rcVideoMaxBitrate","&");
	GetCgiParam(rcAudioSamplerate,Cgi,sizeof(rcAudioSamplerate),"rcAudioSamplerate","&");
	GetCgiParam(rcAudioBitWidth,Cgi,sizeof(rcAudioBitWidth),"rcAudioBitWidth","&");
	GetCgiParam(rcAudioMaxBitrate,Cgi,sizeof(rcAudioMaxBitrate),"rcAudioMaxBitrate","&");
	GetCgiParam(rcAudioTrack,Cgi,sizeof(rcAudioTrack),"rcAudioTrack","&");
	
	GetCgiParam(spMaxHour,Cgi,sizeof(spMaxHour),"spMaxHour","&");
	GetCgiParam(spFullThreshold,Cgi,sizeof(spFullThreshold),"spFullThreshold","&");
	GetCgiParam(spRecycle,Cgi,sizeof(spRecycle),"spRecycle","&");
	GetCgiParam(spCleanData,Cgi,sizeof(spCleanData),"spCleanData","&");

	mRfsRecConfig_t mRfsRecConfig;
	memset(&mRfsRecConfig,0,sizeof(mRfsRecConfig_t));
	mRfsRecConfig.rcVideoWidth = atoi(rcVideoWidth);
	mRfsRecConfig.rcVideoHeight = atoi(rcVideoHeight);
	mRfsRecConfig.rcVideoRate = atoi(rcVideoRate);
	mRfsRecConfig.rcVideoMaxBitrate = atoi(rcVideoMaxBitrate);
	mRfsRecConfig.rcAudioSamplerate = atoi(rcAudioSamplerate);
	mRfsRecConfig.rcAudioBitWidth = atoi(rcAudioBitWidth);
	mRfsRecConfig.rcAudioMaxBitrate = atoi(rcAudioMaxBitrate);
	mRfsRecConfig.rcAudioTrack = atoi(rcAudioTrack);

	mRfsStoragePolicy_t mRfsStoragePolicy;
	memset(&mRfsStoragePolicy,0,sizeof(mRfsStoragePolicy_t));
	mRfsStoragePolicy.spMaxHour = atoi(spMaxHour);
	mRfsStoragePolicy.spFullThreshold = atoi(spFullThreshold);
	mRfsStoragePolicy.spRecycle = atoi(spRecycle);
	mRfsStoragePolicy.spCleanData = atoi(spCleanData);
	
	AVRecPolicy_t AVRecPolicy;
	memset(&AVRecPolicy,0,sizeof(AVRecPolicy_t));
	AVRecPolicy.recConf = mRfsRecConfig;
	AVRecPolicy.storagePolicy = mRfsStoragePolicy;
	
	return hPC->IOCmdSend(Define_gwChannel,avMsgType,(char *)&AVRecPolicy,sizeof(mRfsStoragePolicy_t)+sizeof(mRfsRecConfig_t));
}

int SDRecordGet(
	int				sessionid,
	int				avMsgType,
	const char *	szCgi,
	void *			lpParams
){
	CPPPPChannel * hPC=(CPPPPChannel *)lpParams;
	return hPC->IOCmdSend(Define_gwChannel,avMsgType,NULL,0);
}

int WIFIListGet(
	int				sessionid,
	int				avMsgType,
	const char *	szCgi,
	void *			lpParams
){
	CPPPPChannel * hPC=(CPPPPChannel *)lpParams;
	return hPC->IOCmdSend(Define_gwChannel,avMsgType,NULL,0);
}

int WIFIInfoGet(
	int				sessionid,
	int				avMsgType,
	const char *	szCgi,
	void *			lpParams
){
	CPPPPChannel * hPC=(CPPPPChannel *)lpParams;
	return hPC->IOCmdSend(Define_gwChannel,avMsgType,NULL,0);
}

int WIFIInfoSet(
	int				sessionid,
	int				avMsgType,
	const char *	szCgi,
	void *			lpParams
){
	
	CPPPPChannel * hPC=(CPPPPChannel *)lpParams;
	char * Cgi = (char *)szCgi;
	wifiParam_t wifiSet;
	memset(&wifiSet,0,sizeof(wifiParam_t));
	char enable[8] = {0};
	char wifiStatus[8] = {0};
    char mode[8] = {0} ;
    char channel[8] = {0};
    char authtype[8] = {0};
	char dhcp[8] = {0};
	GetCgiParam(enable,Cgi,sizeof(enable),"enable=","&");
	GetCgiParam(wifiStatus,Cgi,sizeof(wifiStatus),"wifiStatus=","&");
	GetCgiParam(mode,Cgi,sizeof(mode),"mode=","&");
	GetCgiParam(channel,Cgi,sizeof(channel),"channel=","&");
	GetCgiParam(authtype,Cgi,sizeof(authtype),"authtype=","&");
	GetCgiParam(dhcp,Cgi,sizeof(dhcp),"dhcp=","&");
	
	wifiSet.enable = atoi(enable);
	wifiSet.wifiStatus = atoi(wifiStatus);
	wifiSet.mode = atoi(mode);
	wifiSet.channel = atoi(channel);
	wifiSet.authtype = atoi(authtype);
	wifiSet.dhcp = atoi(dhcp);
	GetCgiParam(wifiSet.ssid,Cgi,sizeof(wifiSet.ssid),"ssid=","&");
	GetCgiParam(wifiSet.psk,Cgi,sizeof(wifiSet.psk),"psk=","&");
	GetCgiParam(wifiSet.ip,Cgi,sizeof(wifiSet.ip),"ip=","&");
	GetCgiParam(wifiSet.mask,Cgi,sizeof(wifiSet.mask),"mask=","&");
	GetCgiParam(wifiSet.gw,Cgi,sizeof(wifiSet.gw),"gw=","&");
	GetCgiParam(wifiSet.dns1,Cgi,sizeof(wifiSet.dns1),"dns1=","&");
	GetCgiParam(wifiSet.dns2,Cgi,sizeof(wifiSet.dns2),"dns2=","&");
	
	return hPC->IOCmdSend(Define_gwChannel,avMsgType,(char *)&wifiSet,sizeof(wifiParam_t));
}

//云台控制
/*
int PTZCtrlSet(
	int				sessionid,
	int				avMsgType,
	const char *	szCgi,
	void *			lpParams
){
	//云台控制
	char * Cgi = (char *)szCgi;
	CPPPPChannel * hPC=(CPPPPChannel *)lpParams;
	
	char Param[8] = {0};
	char Value[8] = {0};
		
	GetCgiParam(Param,Cgi,sizeof(Param),"param=","&");
	GetCgiParam(Value,Cgi,sizeof(Value),"value=","&");
		
	camCtrl_t cameraCtrl;
	memset(&cameraCtrl,0,sizeof(camCtrl_t));
	
	cameraCtrl.param= atoi(Param);
	cameraCtrl.value= atoi(Value);
	
	return hPC->IOCmdSend(Define_gwChannel,avMsgType,NULL,0);
}
*/
int DateTimeSet(
	int				sessionid,
	int				avMsgType,
	const char *	szCgi,
	void *			lpParams
){
	char * Cgi = (char *)szCgi;
	CPPPPChannel * hPC=(CPPPPChannel *)lpParams;
	datetimeParam_t	DatetimeParamSet;
	memset(&DatetimeParamSet,0,sizeof(datetimeParam_t));
    
	char now[32] 					 = {0};
	char  tz[32]					 = {0};
	char ntp_enable[8] 				 = {0};
	char xia_ling_shi_flag_status[8] = {0};
	char ntp_svr[64] 				 = {0};
	
	GetCgiParam(now,Cgi,sizeof(now),"now=","&");
	GetCgiParam(tz, Cgi,sizeof(tz), "tz=", "&");
	GetCgiParam(ntp_enable, Cgi,sizeof(ntp_enable), "ntp_enable=", "&");
	GetCgiParam(xia_ling_shi_flag_status, Cgi,sizeof(xia_ling_shi_flag_status), "xia_ling_shi_flag_status=", "&");
	GetCgiParam(ntp_svr, Cgi,sizeof(ntp_svr), "ntp_svr=", "&");

	DatetimeParamSet.now = atoi(now);
	DatetimeParamSet.tz = atoi(tz);
	DatetimeParamSet.ntp_enable= atoi(ntp_enable);
	DatetimeParamSet.xia_ling_shi_flag_status= atoi(xia_ling_shi_flag_status);
	strncpy(DatetimeParamSet.ntp_svr,ntp_svr,64);
	
	return hPC->IOCmdSend(Define_gwChannel,avMsgType,(char *)&DatetimeParamSet,sizeof(datetimeParam_t));
	
}
int DateTimeGet(
	int				sessionid,
	int				avMsgType,
	const char *	szCgi,
	void *			lpParams
){
	CPPPPChannel * hPC=(CPPPPChannel *)lpParams;
	
	return hPC->IOCmdSend(Define_gwChannel,avMsgType,NULL,0);

}

int ThirdPushSet(
	int				sessionid,
	int				avMsgType,
	const char *	szCgi,
	void *			lpParams
){
	char * Cgi = (char *)szCgi;
	CPPPPChannel * hPC=(CPPPPChannel *)lpParams;
	thirdPushParam_t ThirdPushParam;
	memset(&ThirdPushParam,0,sizeof(thirdPushParam_t));

	char SetType[8] = {0};
    char environment[64] = {0};
    char pushType[8] = {0};
    char validity[8] = {0};
    char type[8] = {0};
    char access_id[64] ={0};
	char deviceToken[128] = {0};
	GetCgiParam(SetType,Cgi,sizeof(SetType),"SetType=","&");
	GetCgiParam(deviceToken,Cgi,sizeof(deviceToken),"device_token=","&");
	GetCgiParam(environment,Cgi,sizeof(environment),"environment=","&");
	GetCgiParam(pushType,Cgi,sizeof(pushType),"pushType=","&");
	GetCgiParam(validity,Cgi,sizeof(validity),"validity=","&");
	GetCgiParam(ThirdPushParam.appkey,Cgi,sizeof(ThirdPushParam.appkey),"appkey=","&");
	GetCgiParam(ThirdPushParam.master,Cgi,sizeof(ThirdPushParam.master),"master=","&");
	GetCgiParam(ThirdPushParam.alias,Cgi,sizeof(ThirdPushParam.alias),"alias=","&");
	GetCgiParam(type,Cgi,sizeof(type),"type=","&");
	GetCgiParam(ThirdPushParam.apikey,Cgi,sizeof(ThirdPushParam.apikey),"apikey=","&");
	GetCgiParam(ThirdPushParam.ysecret_key,Cgi,sizeof(ThirdPushParam.ysecret_key),"ysecret_key=","&");
	GetCgiParam(ThirdPushParam.channel_id,Cgi,sizeof(ThirdPushParam.channel_id),"channel_id=","&");
	GetCgiParam(access_id,Cgi,sizeof(access_id),"access_id=","&");
	GetCgiParam(ThirdPushParam.xsecret_key,Cgi,sizeof(ThirdPushParam.xsecret_key),"xsecret_key=","&");

	strncpy(ThirdPushParam.device_token,deviceToken,sizeof(ThirdPushParam.device_token));
	ThirdPushParam.SetType = atoi(SetType);
	ThirdPushParam.environment = atoi(environment);
	ThirdPushParam.pushType = atoi(pushType);
	ThirdPushParam.validity = atoi(validity);
	ThirdPushParam.type = atoi(type);
	ThirdPushParam.access_id = atoi(access_id);
	
	return hPC->IOCmdSend(Define_gwChannel,avMsgType,(char *)&ThirdPushParam,sizeof(thirdPushParam_t));

}

int ThirdPushGet(
	int				sessionid,
	int				avMsgType,
	const char *	szCgi,
	void *			lpParams
){

	CPPPPChannel * hPC=(CPPPPChannel *)lpParams;
	return hPC->IOCmdSend(Define_gwChannel,avMsgType,NULL,0);

}

int ThirdPushDel(
	int				sessionid,
	int				avMsgType,
	const char *	szCgi,
	void *			lpParams
){
	char * Cgi = (char *)szCgi;
	CPPPPChannel * hPC=(CPPPPChannel *)lpParams;
	char device_token[128] = {0};
	GetCgiParam(device_token,Cgi,sizeof(device_token),"device_token=","&");
	
	return hPC->IOCmdSend(Define_gwChannel,avMsgType,device_token,sizeof(device_token));

}

int LogInfoGet(
	int				sessionid,
	int				avMsgType,
	const char *	szCgi,
	void *			lpParams
){
	char * Cgi = (char *)szCgi;
	
	CPPPPChannel * hPC=(CPPPPChannel *)lpParams;
	logSearchCondition_t logSearch ;
	memset(&logSearch,0,sizeof(logSearchCondition_t));
	
	char searchType[8] = {0};
	char mainType[8]   = {0};
	char subType[8]	   = {0};
	char reserved[8]   = {0};
	char startTime[32] = {0};
	char endTime[32]   = {0};
	
	GetCgiParam(searchType,Cgi,sizeof(searchType),"searchType=","&");
	GetCgiParam(mainType,Cgi,sizeof(mainType),"mainType=","&");
	GetCgiParam(subType,Cgi,sizeof(subType),"subType=","&");
	GetCgiParam(reserved,Cgi,sizeof(reserved),"reserved=","&");
	GetCgiParam(startTime,Cgi,sizeof(startTime),"startTime=","&");
	GetCgiParam(endTime,Cgi,sizeof(endTime),"endTime=","&");

	logSearch.searchType = atoi(searchType);
	logSearch.mainType   = atoi(mainType);
	logSearch.subType	 = atoi(subType);
	logSearch.reserved   = atoi(reserved);
	logSearch.startTime  = atoi(startTime);
	logSearch.endTime    = atoi(endTime);
	
	
	return hPC->IOCmdSend(Define_gwChannel,avMsgType,(char *)&logSearch,sizeof(logSearchCondition_t));

}

int LogInfoSet(
	int				sessionid,
	int				avMsgType,
	const char *	szCgi,
	void *			lpParams
){

	
	char * Cgi = (char *)szCgi;
	CPPPPChannel * hPC=(CPPPPChannel *)lpParams;
	logSet_t logSet;
	memset(&logSet,0,sizeof(logSet_t));

	char actType[8]   = {0};
	char bAlarmLog[8] = {0};
	char bStartUpLog[8] = {0};
	char bShutdwonLog[8] = {0};
	char bTimerLog[8] = {0};

	GetCgiParam(actType,Cgi,sizeof(actType),"actType=","&");
	GetCgiParam(bAlarmLog,Cgi,sizeof(bAlarmLog),"bAlarmLog=","&");
	GetCgiParam(bStartUpLog,Cgi,sizeof(bStartUpLog),"bStartUpLog=","&");
	GetCgiParam(bShutdwonLog,Cgi,sizeof(bShutdwonLog),"bShutdwonLog=","&");
	GetCgiParam(bTimerLog,Cgi,sizeof(bTimerLog),"bTimerLog=","&");

	logSet.actType = atoi(actType);
	logSet.bAlarmLog = atoi(bAlarmLog);
	logSet.bStartUpLog = atoi(bStartUpLog);
	logSet.bShutdwonLog = atoi(bShutdwonLog);
	logSet.bTimerLog = atoi(bTimerLog);
	
	return hPC->IOCmdSend(Define_gwChannel,avMsgType,(char *)&logSet,sizeof(logSet_t));
	
}

int FileDataGet(
	int				sessionid,
	int				avMsgType,
	const char *	szCgi,
	void *			lpParams
){

	char * Cgi = (char *)szCgi;
//	Log3("File DownLoad Cgi ==================>%s",Cgi);
	CPPPPChannel * hPC=(CPPPPChannel *)lpParams;
	fileTransParam_t fileList;
	memset(&fileList,0,sizeof(fileTransParam_t));

	char offset[8] = {0};
	char FilePath[256] = {0};
	char FileDir[256]  = {0};
	char FileName[256] = {0};
	char FileFullName[256] = {0};
	GetCgiParam(FilePath,Cgi,sizeof(FilePath),"filepath=","&");
	strncpy(hPC->FileDataPath,FilePath,sizeof(hPC->FileDataPath));
	
	GetCgiParam(FileDir,Cgi,sizeof(FileDir),"fileDir=","&");
	strncpy(hPC->FileDataDir,FileDir,sizeof(hPC->FileDataDir));

	GetCgiParam(FileName,Cgi,sizeof(FileName),"filename=","&");
	strncpy(hPC->FileDataName,FileName,sizeof(hPC->FileDataName));
	sprintf(FileFullName,"%s%s",FileDir,FileName);
	strncpy(fileList.filename,FileFullName,sizeof(fileList.filename));
	
	GetCgiParam(offset,Cgi,sizeof(offset),"offset=","&");
	fileList.offset = atoi(offset);
	
	return hPC->IOCmdSend(Define_gwChannel,avMsgType,(char *)&fileList,sizeof(fileTransParam_t));
	
}

int FileListGet(
	int				sessionid,
	int				avMsgType,
	const char *	szCgi,
	void *			lpParams
){
	char * Cgi = (char *)szCgi;
	CPPPPChannel * hPC=(CPPPPChannel *)lpParams;
	fileTransParam_t fileList;
	memset(&fileList,0,sizeof(fileTransParam_t));

	char offset[8] = {0};

	GetCgiParam(offset,Cgi,sizeof(offset),"offset=","&");
	GetCgiParam(fileList.filename,Cgi,sizeof(fileList.filename),"filename=","&");
	
	fileList.offset = atoi(offset);
	
	return hPC->IOCmdSend(Define_gwChannel,avMsgType,(char *)&fileList,sizeof(fileTransParam_t));

}

int VideoRecListGet(
	int				sessionid,
	int				avMsgType,
	const char *	szCgi,
	void *			lpParams
){
	char * Cgi = (char *)szCgi;
	CPPPPChannel * hPC=(CPPPPChannel *)lpParams;
	
	recFileSearchParam_t videoRecSearch;
	memset(&videoRecSearch,0,sizeof(recFileSearchParam_t));

	char starttime[16] = {0};
	char endtime[16]   = {0};

	GetCgiParam(starttime,Cgi,sizeof(starttime),"starttime=","&");
	GetCgiParam(endtime,Cgi,sizeof(endtime),"endtime=","&");
	
	videoRecSearch.starttime = atoi(starttime);
	videoRecSearch.endtime	 = atoi(endtime);
		
	return hPC->IOCmdSend(Define_gwChannel,avMsgType,(char *)&videoRecSearch,sizeof(recFileSearchParam_t));

}

// fucntion list for each command

static APP_CMD_CALL hACC[] = {

	//用户注册、设置、获取，音视频开启、关闭,视频回放、停止,设备端实时拍照、录像
	{CMD_SYSTEM_USER_CHK,Userlogin},
	{CMD_SYSTEM_USER_SET,UserSet},
	{CMD_SYSTEM_USER_GET,UserGet},
	{CMD_DATACTRL_AUDIO_START,AudioStart},
	{CMD_DATACTRL_AUDIO_STOP,AudioStop},
	{CMD_DATACTRL_TALK_START,TalkStart},
	{CMD_DATACTRL_TALK_STOP,TalkStop},
	{CMD_DATACTRL_PLAYLIVE_START,VideoStart},
	{CMD_DATACTRL_PLAYLIVE_STOP,VideoStop},
	{CMD_DATACTRL_PLAYBACK_START,VideoPlayBackStart},
	{CMD_DATACTRL_PLAYBACK_STOP,VideoPlayBackStop},
	
	//设置摄像机图像参数,设备重启、设备关闭、系统运行策略设置、获取,设置GPIO值、获取GPIO状态
	{CMD_CAMPARAMS_GET,CameraGet},
	{CMD_CAMPARAMS_SET,CameraSet},	
	{CMD_SYSTEM_REBOOT,CameraReboot},
	{CMD_SYSTEM_SHUTDOWN,CameraShouDown},
	{CMD_SYSTEM_OPRPOLICY_SET,OprpolitySet},
	{CMD_SYSTEM_OPRPOLICY_GET,OprpolityGet},
	{CMD_SYSTEM_ITEMDEFAULTSETTING_RECOVERY,Item_Recover},
	{CMD_INTERFACE_GPIOVALUE_SET,GpioValueSet},
	{CMD_INTERFACE_GPIOVALUE_GET,GpioValueGet},
	//获取SD卡状态、格式化SD卡、SD卡录像存储策略设置、获取
	{CMD_SD_INFO_GET,SDInfoGet},
	{CMD_SD_FORMAT,SDForMat},
	{CMD_SD_RECPOLICY_SET,SDRecordSet},
	{CMD_SD_RECPOLICY_GET,SDRecordGet},
	//获取WIFI信息
	{CMD_NET_WIFISETTING_SET,WIFIInfoSet},
	{CMD_NET_WIFISETTING_GET,WIFIInfoGet},
	{CMD_NET_WIFI_SCAN,WIFIListGet},
	//PTZ云台控制
	//{CMD_PTZ_SET,PTZCtrlSet},
	
	//设置设备时间、获取设备时间
	{CMD_SYSTEM_DATETIME_SET,DateTimeSet},
	{CMD_SYSTEM_DATETIME_GET,DateTimeGet},

	//设置第三方推送
	{CMD_PUSHPARAM_SET,ThirdPushSet},
	{CMD_PUSHPARAM_GET,ThirdPushGet},
	{CMD_PUSHPARAM_DEL,ThirdPushDel},

	//获取日志
	{CMD_LOG_GET,LogInfoGet},
	{CMD_LOG_SET,LogInfoSet},

	//获取文件、文件列表
	{CMD_DATACTRL_FILE_GET,FileDataGet},
	{CMD_DATACTRL_FILELIST_GET,FileListGet},

	//获取录像列表
	{ CMD_SD_RECORDFILE_GET,VideoRecListGet},
	{0,NULL}
};

int SendCmds(
	int 			Idx,
	int				MsgType,
	const char *	Cgi,
	void *			Params
){
//	Log3("CCCCCCCCCCCCall SendCmds:[%08X].\n",MsgType);

	int i = 0;
	for(i = 0;hACC[i].CmdCall != NULL;i++){
		if(MsgType == hACC[i].CmdType){

//			Log3("app cmd send IOCTRL cmd:[%03X].",hACC[i].CmdType);
			
			return hACC[i].CmdCall(
				Idx,
				hACC[i].CmdType,
				Cgi,
				Params);
		}
	}

	return -1;
}


