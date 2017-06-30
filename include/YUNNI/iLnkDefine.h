#ifndef _Inc_ilnk_define_
#define _Inc_ilnk_define_

#define SESSION_TYPE_CLIENT 0
#define SESSION_TYPE_DEVICE 1

#define MAX_PUSH_SIZE     128

//JNI到JAVA的消息类型
#define MSG_NOTIFY_TYPE_CONNECT_PPPP_STATUS 0   	/* 连接状态 */
#define MSG_NOTIFY_TYPE_CONNECT_PPPP_MODE 1   		/* 连接模式 */
#define MSG_NOTIFY_TYPE_CONNECT_STREAM_TYPE 2
#define MSG_NOTIFY_TYPE_CONNECT_SESSIONINF 3

#define MSG_NOTIFY_TYPE_LISTEN_PPPP_STATUS 4   	/* 连接状态 */
#define MSG_NOTIFY_TYPE_LISTEN_PPPP_MODE 5   		/* 连接模式 */
#define MSG_NOTIFY_TYPE_LISTEN_STREAM_TYPE 6
#define MSG_NOTIFY_TYPE_LISTEN_SESSIONINF 7

#define MSG_NOTIFY_TYPE_SERVER_CHANGED		9
#define MSG_NOTIFY_TYPE_SYSTEM_STATUS 8

//system status
#define SYSTEM_STATUS_INSUFFIENT_RESOURCE 0


//P2p会话状态值
typedef enum{
	PPPP_STATUS_DISCONNECTED=0,//连接断开
	PPPP_STATUS_CONNECTING, //连接中
	PPPP_STATUS_CONNECTED, //已经连接
	PPPP_STATUS_ID_INVALID, //无效ID
	PPPP_STATUS_DEVICE_OFFLINE,//设备离线
	PPPP_STATUS_LOCAL_SESSION_OVERLIMIT,//本地会话超限
	PPPP_STATUS_REMOTE_SESSION_OVERLIMIT, //对端会话超限
	PPPP_STATUS_SESSIONSETUP_TIMEOUT,//会话建立超时
	PPPP_STATUS_ID_OUTOFDATE,//ID过期
	PPPP_STATUS_USER_AUTHENTICATING,//正在验证中
	PPPP_STATUS_USER_AUTHENTICATED,//验证成功
	PPPP_STATUS_USER_INVALID,//无效用户
	PPPP_STATUS_UNKNOWN_ERROR,//未知错误
	PPPP_STATUS_NOT_INITIALIZED,//未初始化
	PPPP_STATUS_DISCONNECTING,//正在断开中
	PPPP_STATUS_USER_INAUTHENTICATED,//未验证，ticket为空
	PPPP_STATUS_USER_NOT_LOGIN	//用户未登录
}msgP2pStatus_e;

//pppp mode
#define PPPP_MODE_P2P 1
#define PPPP_MODE_RELAY 2


#define P2P_CHANNEL_CMMND   0
#define P2P_CHANNEL_VIDEO    1
#define P2P_CHANNEL_AUDIO    2
#define P2P_CHANNEL_FILE     3

#define AV_MODE_LIVEPLAY 0
#define AV_MODE_PLAYBACK 1

#define MAX_P2PID_LEN	24
#define MAX_P2PSERVER_LEN	512
#define MAX_USR_NAME_LEN	32
#define MAX_USR_PWD_LEN	128
#define MAX_ROOTDIR_LEN 256


#define DEFAULT_SESSION_NMB 8
#define DEFAULT_CHANNEL_NMB 4
#define DEFAULT_CHANNEL_SIZE 128

/////////////命令定义////////////////////
#define CMD_GW_DEVLST_CLEAN			0x0100//清除网关设备列表
#define CMD_GW_DEVLST_GET 			0x0101//获取网关设备列表
#define CMD_GW_DEVLST_SET 			0x0102//添加或修改设备列表中设备信息
#define CMD_GW_DEV_SEARCH 			0x0103//让网关搜索设备
#define CMD_GW_DEV_ALLSTREAM_GET		0x0110//获取设备码流到网关RingBuf
#define CMD_GW_DEV_ALLSTREAM_CANCEL	0x0111//取消设备码流到网关RingBuf
#define CMD_GW_DEV_STREAM_REC_START	0x0112//网关实时开始录像
#define CMD_GW_DEV_STREAM_REC_STOP	0x0113//网关实时停止录像
#define CMD_GW_DEV_RECSCHEDULE_ASSOCIATE		0x0114//设置网关录像计划到某设备
#define CMD_GW_DEV_RECSCHEDULE_DISASSOCIATE		0x0115//获取网关录像计划与某设备的匹配
#define CMD_GW_RECSCHEDULE_ADD		0x0116//添加网关录像计划
#define CMD_GW_RECSCHEDULE_DEL		0x0117//按ID删除网关录像计划
#define CMD_GW_RECSCHEDULE_CHG		0x0118//按ID修改网关录像计划
#define CMD_GW_RECSCHEDULELST_GET 	0x0119//获取网关录像计划列表
#define CMD_GW_RECSCHEDULELST_CLEAN	0x0120//清除全部网关录像

#define CMD_DEV_BROADCAST 					0x0fff//设备启动时的信息广播

#define CMD_SYSTEM_DEFAULTSETTING_IMPORT		0x1000
#define CMD_SYSTEM_DEFAULTSETTING_EXPORT		0x1001
#define CMD_SYSTEM_DEFAULTSETTING_RECOVERY	0x1002
#define CMD_SYSTEM_ITEMDEFAULTSETTING_RECOVERY	0x1003	
#define CMD_SYSTEM_CURRENTSETTING_EXPORT	0x1004	
#define CMD_SYSTEM_CURRENTSETTING_IMPORT	0x1005	
#define CMD_SYSTEM_DEFAULTSETTING_CREATE	0x1006
#define CMD_SYSTEM_FIRMWARE_UPGRAD			0x1007
#define CMD_SYSTEM_STATUS_GET			0x1008

#define CMD_SYSTEM_SHUTDOWN			0x1010
#define CMD_SYSTEM_REBOOT			0x1011

#define CMD_SYSTEM_USER_CHK		0x1020
#define CMD_SYSTEM_USER_SET			0x1021
#define CMD_SYSTEM_USER_GET			0x1022

#define CMD_SYSTEM_P2PPARAM_SET			0x1030
#define CMD_SYSTEM_OPRPOLICY_SET		0x1031
#define CMD_SYSTEM_OPRPOLICY_GET		0x1032

#define CMD_SYSTEM_DATETIME_SET		0x1040
#define CMD_SYSTEM_DATETIME_GET		0x1041

#define CMD_INTERFACE_SADC_GET			0x1100

#define CMD_INTERFACE_GPIOVALUE_SET			0x1110
#define CMD_INTERFACE_GPIOVALUE_GET			0x1111
#define CMD_INTERFACE_GPIOACTION_SET			0x1112
#define CMD_INTERFACE_GPIOACTION_GET			0x1113

#define CMD_INTERFACE_SERIALSETTING_SET		0x1120
#define CMD_INTERFACE_SERIALSETTING_GET		0x1121
#define CMD_INTERFACE_SERIALDATA_SET	0x1122
#define CMD_INTERFACE_SERIALDATA_GET	0x1123

#define CMD_NET_WIFISETTING_SET			0x1130
#define CMD_NET_WIFISETTING_GET			0x1131
#define CMD_NET_WIFI_SCAN				0x1132
#define CMD_NET_WIREDSETTING_SET		0x1133
#define CMD_NET_WIREDSETTING_GET		0x1134

#define CMD_PUSHPARAM_SET		0x1200
#define CMD_PUSHPARAM_GET    	0x1201
#define CMD_PUSHPARAM_DEL		0x1203


#define CMD_SD_FORMAT				0x2000
#define CMD_SD_RECPOLICY_SET		0x2001
#define CMD_SD_RECPOLICY_GET		0x2002
#define CMD_SD_RECORDING_NOW		0x2003
#define CMD_SD_INFO_GET				0x2004
#define CMD_SD_RECORDFILE_GET		0x2005
#define CMD_SD_RECORDSCH_GET		0x2006
#define CMD_SD_RECORDSCH_SET		0x2007
#define CMD_SD_PICFILE_GET			0x2100
#define CMD_SD_PIC_CAPTURE			0x2101



//audio ctrl
#define CMD_DATACTRL_AUDIO_START		0x3000	
#define CMD_DATACTRL_AUDIO_STOP			0x3001
#define CMD_DATACTRL_TALK_START			0x3002
#define CMD_DATACTRL_TALK_STOP			0x3003

#define CMD_AUDIO_PARAM_SET				0x3004//设置audio参数 audioParamSet_t
#define CMD_AUDIO_PARAM_GET				0x3005//设置audio参数 audioParam_t

//video ctrl
#define CMD_DATACTRL_PLAYLIVE_START		0x3010
#define CMD_DATACTRL_PLAYLIVE_STOP		0x3011
#define CMD_DATACTRL_PLAYBACK_START		0x3012
#define CMD_DATACTRL_PLAYBACK_STOP		0x3013

#define CMD_DEV_RECORD_PLAYBACK_SEEK		0x3014
#define CMD_DEV_RECORD_PLAYBACK_SPEED		0x3015




#define CMD_DATACTRL_DOORBELL_CALL_ACCEPT	0x3020//呼叫接受
#define CMD_DATACTRL_DOORBELL_CALL_REJECT	0x3021//呼叫拒绝

//filetransfer ctrl
#define CMD_DATACTRL_FILESAVEPATH	0x4001
#define CMD_DATACTRL_FILE_GET		0x4002
#define CMD_DATACTRL_FILE_PUT		0x4003
#define CMD_DATACTRL_FILE_SET		0x4004//删除等
#define CMD_DATACTRL_FILELIST_GET	0x4005 

///////////////////////////////
#define CMD_PASSTHROUGH_STRING_PUT	0x55ff
#define CMD_CAMPARAMS_SET			0x5012
#define CMD_CAMPARAMS_GET		0x5003
#define CMD_PTZ_SET			0x5049
#define CMD_PARAM_GET			0x5002
#define CMD_LOG_GET				0x5004
#define CMD_LOG_SET				0x500d
#define CMD_MISC_GET				0x5005
#define CMD_SNAPSHOT_GET			0x5015
#define CMD_NOTIFICATION			0x5040

#define CMD_ALARM_ACTION_SET		0x5050
#define CMD_ALARM_ACTION_GET		0x5051

#define CMD_ALARM_NOTIFICATION			0x6040

////////////本地命令/////////////////////
#define CMD_LOCAL_AVREC_START		0x5100
#define CMD_LOCAL_AVREC_STOP		0x5101
#define CMD_LOCAL_SESSION_CHECK		0x55FE

//靠靠靠靠
#define CMD_FILE_DOWNLOAD_PROGRESS		0x8001
/////////////////命令内容数据结构定义/////////////////////////
typedef struct tag_P2pParamBase{
	int iSessionNmb;//最大会话数
	int iChannelNmb;//每会话通道数
	int iChannelBufSize;//每通道底层缓冲大小KB
}P2pParamBase_t,pP2pParamBase_t;

typedef struct tag_P2pNodeBase{
	int iType;//设备类型
	char sID[MAX_P2PID_LEN];//设备的p2pID
}P2pNodeBase_t,*pP2pNodeBase_t;

typedef struct tag_P2pNodeBase_EX{
	int iType;//设备类型
	char sID[MAX_P2PID_LEN];//设备的p2pID
	char sServer[MAX_P2PSERVER_LEN];
}P2pNodeBase_t_EX,*pP2pNodeBase_t_EX;


typedef struct tag_P2pNodeExt{
	P2pNodeBase_t strNodeInf;
	char sUsr[MAX_USR_NAME_LEN];
	char sPwd[MAX_USR_PWD_LEN];
	char sServer[MAX_P2PSERVER_LEN];
}P2pNodeExt_t,*pP2pNodeExt_t;


//固件升级
typedef struct tag_firmwareUpgrade{
	int type;
	char url[128];
	}firmwareUpgrade_t;

//设备端p2p参数设置
typedef struct tag_openP2pSetting_t{
	char p2pID[24];
	char devName[64];
	char svrStr[512];
}openP2pSetting_t, *pOpenP2pSetting_t;


//系统运行参数
typedef struct{
	unsigned char adcSelect;//000000001-->sadc0,00000010-->sadc1,00000011-->all,other-->disable
	unsigned char adcChkInterval;//秒 
	unsigned char reserved1[2];
	
	unsigned short gpioSelect;
	unsigned short gpioChkInterval;//毫秒 >100

	unsigned int sysRunTime;//运行时长(秒) 0-->一直运行，n-->运行n秒
	
	unsigned char rcdPicEnable;//是否拍照 1-->是,非1不拍
	unsigned char rcdPicSize;//与码流对应 0-->720p,1-->次码流
	unsigned short rcdPicInterval;//拍照间隔(秒)
	
	unsigned char rcdAvEnable;//是否录像  1-->是,非1不录
	unsigned char rcdAvSize;//与码流对应 0-->720p,1-->次码流
	unsigned short rcdAvTime;//录像时长 0-->一直录，n-->录n秒
	
	unsigned char pushEnable;
	unsigned char alarmEnable;
	unsigned char wifiEnable;
	unsigned char reserved;

	unsigned char powerMgrEnable;//0-->无,1-->counter,2--->schedule
	unsigned char powerMgrCountDown;
	unsigned char reserved2[2];
	unsigned int  powerMgrSchedule;
	}sysOprPolicy_t;

//告警通知
typedef struct tag_xqAlarm{
	int alarmType;
	unsigned int alarmLength;
	char *alarmContent;
	struct tag_xqAlarm *pNext;
}xqAlarm_t;

//用户
typedef struct{
	char name[32];
	char pass[128];
}user_t;

typedef struct tag_P2pNodeExt_EX{
	P2pNodeBase_t_EX	node;
	user_t				user;
}P2pNodeExt_t_EX,*pP2pNodeExt_t_EX;

//用户设置
typedef struct tag_userSetting
{
    char user1[32];
    char pwd1[128];
    char user2[32];
    char pwd2[128];
    char user3[32];
    char pwd3[128];
}userSetting_t,*pUserSetting_t;


//用户验证结果
typedef struct tag_usrChkRet{
	int privilege;
	char ticket[4];
	}st_usrChkRet;


//设备状态
typedef struct tag_devStatus{
	int swVer;
	int hwVer;
	int timeZone;
	unsigned int lastShutdownTime;
	unsigned int sysUptime;
	unsigned int netUptime;

	char devName[64];
	
	unsigned char	sdStatus;
	unsigned char	p2pStatus;
	unsigned char	wifiEnableOnStart;
    unsigned char   recEnableOnStart;
	unsigned char	reserved[4];
	//net
    unsigned char   mode;     //!< 1->dhcp to get ip,   0->static ip
    unsigned char   dhcp;    //!< 1->dhcp to get dns(dhcp_ip_enable must be 1),  0->static dns
    unsigned char   mac[6];
    unsigned int    ipAddr;
    unsigned int    netmask;
	//sd
	unsigned int	lastRecTime;	
	unsigned int	totalSize;
	unsigned int	usedSize;
	}devStatus_t;

//时间设置
typedef struct tag_datetimeParam
{
    int now;
    int tz;
    int ntp_enable;
	int xia_ling_shi_flag_status;
    char ntp_svr[64];
}datetimeParam_t,*pDatetimeParam_t;

//GPIO相关
typedef struct tag_gpio
{
    unsigned short gpioSelect;
    unsigned short gpioValue;
}gpio_t, *pGpio_t;

typedef enum gpioAction{
	GPIO_ACTION_NONE=0,
	GPIO_ACTION_DEFAULT_RECOVERY,
	GPIO_ACTION_DEFAULT_CREATE,
	GPIO_ACTION_WIFI_RECOVERY,
	GPIO_ACTION_WIFI_CTRL,
	GPIO_ACTION_ISP_CTRL,
	GPIO_ACTION_REC_CTRL,
	GPIO_ACTION_LIGHTBOARD_CTRL,
	}gpioAction_e;

typedef struct{
	gpioAction_e action;
	int value;
	}gpioCnf_t;

typedef struct tag_gpioAction{
	int	gpioSelect;
	gpioCnf_t gpioCnf;
	}gpioAction_t;

//wifi参数
typedef struct tag_wifiParam
{
    int enable;
	int wifiStatus;
    int mode;
    int channel;
    int authtype;
	int dhcp;
    char ssid[32];
    char psk[128];

	char ip[16];
	char mask[16];
	char gw[16];
	char dns1[16];
	char dns2[16];
}wifiParam_t,*pWifiParam_t;
//扫描wifi返回结果CMD_NET_WIFI_SCAN
typedef struct tag_wifiScanRet
{
    char ssid[64];
    char mac[8];
    int security;
    int dbm0;//强度值
    int dbm1;//基准
    int mode;
    int channel;

}wifiScanRet_t,*pWifiScanRet_t;

//ADC值
typedef struct tag_adc
{
    int adcSelect;
    int adcValue1;
    int adcValue2;
}adc_t, *pAdc_t;


//日志
typedef struct tag_logSet{
	int actType;//0-->clearlog,1-->setlog
	unsigned char bAlarmLog;
	unsigned char bStartUpLog;
	unsigned char bShutdwonLog;
	unsigned char bTimerLog;
	}logSet_t;

//日志搜索条件
typedef struct tag_logSearchCondition{
	unsigned char searchType; // 0-->按类型,1-->按时间
	unsigned char mainType;
	unsigned char subType;
	unsigned char reserved;
	unsigned int startTime;
	unsigned int endTime;
}logSearchCondition_t;

//日志信息获取
typedef struct{
	unsigned char mainType;
	unsigned char subType;
	unsigned short	length;
	unsigned int timeStamp;
	unsigned char	content[120];
}LogInfo_t;
//拍照
typedef struct tag_picCap{
	int picSize;
	int ifSave;
	}picCap_t;

//录像控制
typedef struct recNow{
	int ifRecNow;
	}recNow_t;

//录像文件获取 
typedef struct tag_recFileSearchParam
{
    int starttime;
    int endtime;
}recFileSearchParam_t, *pRecFileSearchParam_t;

//ADC设置
typedef struct{
	char bEnable;
	char bNotify;
	char bPicCap;
	char bAvRec;
	short iTopTrigValue;
	short iBottomTrigValue;
	}sAdcCnf_t;

//字串透传
typedef struct tag_passThroghString{
	int strLength;
	char strContent[1024];
}passThroghString_t;

//摄像头图像设置参数
typedef enum{
	VIDEO_PARAM_TYPE_DEFAULT=0,//恢复到所有视频参数缺省值
	VIDEO_PARAM_TYPE_RESOLUTION,//分辨率
	VIDEO_PARAM_TYPE_BRIGHTNESS,//亮度
	VIDEO_PARAM_TYPE_CONTRAST,//对比度
	VIDEO_PARAM_TYPE_SATURATION,//色彩饱和度
	VIDEO_PARAM_TYPE_SHARPNESS,//锐利
	VIDEO_PARAM_TYPE_FRAMERATE,//帧率
	VIDEO_PARAM_TYPE_BITRATE,//码率
	VIDEO_PARAM_TYPE_ROTATE,//图像镜像和返转0正常,1左右,2上下,3左右上下
	VIDEO_PARAM_TYPE_IRCUT,//ircut控制 0->自动，1->enable,2->关闭
	VIDEO_PARAM_TYPE_OSD,//0->关闭1->打开
	VIDEO_PARAM_TYPE_MOVEDETECTION,//0->关闭1->打开
	VIDEO_PARAM_TYPE_MODE//flicker
}videoParamType_e;

//摄像机参数设置
typedef struct tag_camCtrl{
	unsigned short param;
	union{
		int resolution;//分辨率
		int brightness;//亮度
		int contrast;//对比度
		int saturation;//色彩饱和度
		int sharpness;//锐利
		int frameRate;//帧率
		int bitRate;//码率
		int rotate;//图像镜像和返转0正常,1左右,2上下,3左右上下
		int ircut;//ircut控制 0->自动，1->enable,2->关闭
		int osd;//0->关闭1->打开
		int moveDetect;//0->关闭1->打开
		int mode;
	}u;
	
}camCtrl_t;

//摄像机参数设置
typedef struct tag_camCtrl_EX{
	int param;
	int value;
}camCtrl_t_EX;


//摄影头参数获取
typedef struct{
	int resolution;//分辨率
	int brightness;//亮度
	int contrast;//对比度
	int saturation;//色彩饱和度
	int sharpness;//锐利
	int frameRate;//帧率
	int bitRate;//码率
	int rotate;//图像镜像和返转0正常,1左右,2上下,3左右上下
	int ircut;//ircut控制 0->自动，1->enable,2->关闭
	int mode;
}cameraParams_t;

//摄像机图像设置
typedef struct image_Options
{
	unsigned int	CtrlMode;	//0-->fixed 1-->auto
	unsigned int	DayMode;	//0-->night 1-->day

	unsigned int	Brightness;					/*亮度*/
	unsigned int	Contrast;					/*对比度*/
	unsigned int	ColorSaturation;					/*饱和度*/
	unsigned int	Sharpness;					/*锐利度*/

	unsigned char	Mirror;					/*0-正常 1-上下翻转 2->左右翻转 3->上下/左右翻转*/
	unsigned char	Flip;
}ImageOptions_t;

//sd卡存储策略
// 录像配置
typedef struct 
{
    unsigned int      rcVideoWidth;
    unsigned int      rcVideoHeight;
    unsigned int      rcVideoRate;        // 帧率(25)
    unsigned int      rcVideoMaxBitrate;

    unsigned int      rcAudioSamplerate;
    unsigned int      rcAudioBitWidth;    // 8/16
    unsigned int      rcAudioMaxBitrate;
    unsigned int      rcAudioTrack;       // 1-单声道, 2-立体声
} mRfsRecConfig_t;
// 存储策略
typedef struct
{
    unsigned int       spMaxHour;          // 保持多少个小时内的录像(0-不限制, 其他值-小时数)
    unsigned int      spFullThreshold;    // 临界值(剩余空间不足多少MB时认为满)
    unsigned char      spRecycle;          // 盘满后是否循环覆盖(0-不覆盖, 1-覆盖)
    unsigned char      spCleanData;        // 回收时是否清除数据(0-不清, 1-清除)
    unsigned char      spReserved[2];
} mRfsStoragePolicy_t;

typedef struct{
	mRfsRecConfig_t	recConf; 
	mRfsStoragePolicy_t	storagePolicy;
}AVRecPolicy_t;

// 录像配置
typedef struct 
{
    unsigned int      rcVideoWidth;
    unsigned int      rcVideoHeight;
    unsigned int      rcVideoRate;        // 帧率(25)
    unsigned int      rcVideoMaxBitrate;

    unsigned int      rcAudioSamplerate;
    unsigned int      rcAudioBitWidth;    // 8/16
    unsigned int      rcAudioMaxBitrate;
    unsigned int     rcAudioTrack;       // 1-单声道, 2-立体声
} mRecConfig_t;

// 存储策略
typedef struct
{
    unsigned int      spMaxHour;          // 保持多少个小时内的录像(0-不限制, 其他值-小时数)
    unsigned int      spFullThreshold;    // 临界值(剩余空间不足多少MB时认为满)
    unsigned int      spRecycle;          // 盘满后是否循环覆盖(0-不覆盖, 1-覆盖)
    unsigned int      spCleanData;        // 回收时是否清除数据(0-不清, 1-清除)
} mStoragePolicy_t;

//录像策略
typedef struct{
	mRecConfig_t	recConf; 
	mStoragePolicy_t	storagePolicy;
	}mAVRecPolicy_t;



//文件传输参数,也用作录像文件、录音文件回放
typedef struct tag_fileTransParam{
	int offset;
	char filename[128];
}fileTransParam_t;

//sd卡文件返回 CMD_FILETRANSFER_FILELIST_GET
typedef struct tag_xqFile{
int type;
unsigned int size;
unsigned int  timeStamp;
char name[64];
}xqFile_t;


//串口透传数据
typedef struct{
	int len;
	char data[1024];
	}SerialData_t;

//串口参数设置
typedef struct 
{
    int dwBaudRate;     //!< 波特率(bps)(110,300,1200,2400,4800,9600,19200,38400,57600,115200,230400,460800,921600)
    int dataBit;        //!< 数据有几位(5,6,7,8)
    int stopBit;        //!< 停止位(1,2)
    int parity;         //!< 校验(0-无校验,1-奇校验,2-偶校验 3-标志 4-空格)
    int flowCtrl;       //!< 数据流控制(0-无,1-软流控,2-硬流控)
}__attribute__ ((packed)) SerialConfig_t;


//消息推送
typedef struct tag_STRUCT_PUSH_PARAM{
    char SetType;
    char device_token[65];
    int environment;
    int pushType;
    int validity;
    //---------jpush----------
    char appkey[64];
    char master[64];
    char alias[64];
    int type;
    //-------------bpush----------
    char apikey[64];
    char ysecret_key[64];
    char channel_id[64];
    //-------------xpush------------
	unsigned int access_id;//char access_id[32];
    char xsecret_key[64];
}thirdPushParam_t, *pThrdPushParam_t;

//获取push参数
typedef struct _stJPushParam
{
	int receiverType;
	char appKey[64];
	char masterKey[64];
	char receiverValue[64];
	
}JPUSHPARAM,*PJPUSHPARAM;
	
typedef struct _stYPushParam
{
    char apikey[32];
    char secret_key[48];
    char channel_id[20];
    //unsigned char deploy_status;
}YPUSHPARAM,*PYPUSHPARAM;

typedef struct _stXPushParam
{
    //int access_id;
    char access_id[32];
    char secret_key[40];
    char device_token[68];
}XPUSHPARAM,*PXPUSHPARAM;

typedef struct _stPushParamList
{
    JPUSHPARAM stJPushParam[MAX_PUSH_SIZE];
    YPUSHPARAM stYPushParam[MAX_PUSH_SIZE];
    XPUSHPARAM stXPushParam[MAX_PUSH_SIZE];
    time_t registerTime[MAX_PUSH_SIZE];
    char environment[MAX_PUSH_SIZE];
    char devicetype[MAX_PUSH_SIZE];
    char push_data[20];
	char jpush_address[16];
	char xpush_address[16];
	char ypush_address[16];
    int pushType;
    int validity;
}pushParamList_t,*PPUSHPARAMLIST;

typedef enum tag_ENUM_VIDEO_MODE
{
	ENUM_VIDEO_MODE_MJPEG,
	ENUM_VIDEO_MODE_H264,
	ENUM_VIDEO_MODE_AUDIO
}ENUM_VIDEO_MODE_YUNNI_P2P;

typedef enum tag_ENUM_FRAME_TYPE
{
	ENUM_FRAME_TYPE_I = 0,
	ENUM_FRAME_TYPE_P = 1,
	ENUM_FRAME_TYPE_MJPEG = 3,
	ENUM_FRAME_TYPE_AUDIO = 6
}ENUM_FRAME_TYPE;

typedef enum appError
{
	APP_ERR_OK = 0,
	APP_ERR_UNAUTH,
	APP_ERR_NO_PRIVILEGE,
	APP_ERR_INVALID_PARAM,
	APP_ERR_CMDEXCUTE_FAILED,
	APP_ERR_NONE_RESULT,
	APP_ERR_UNKNOWN
}commError_e;

//文件传输相关
typedef struct tag_FILE_DATA_HEAD
{
unsigned int startcode;//0xff12ff34
unsigned char type;//0-->file attribute,1-->file content
unsigned char reserved;
unsigned short blockseq;
unsigned int len;
char magic[4];//标识同一文件
}FILE_DATA_HEAD,*PFILE_DATA_HEAD;

typedef struct tag_FILE_ATTR
{
char filename[128];
unsigned char magic[4];
unsigned char type;
unsigned char reserved[3];
unsigned int filesize;
unsigned int ctime;
}FILE_ATTR,*PFILE_ATTR;

typedef struct tag_STRU_SEARCH_SDCARD_RECORD_FILE
{
    int startPage;
    int pageNmb;
}sdRecSearchCondition_t, *PSTRU_SEARCH_SDCARD_RECORD_FILE;


//Command Channel head
#define CMD_START_CODE 0x0a01
typedef struct _CMD_CHANNEL_HEAD_NOT_NEED
{
    unsigned short startcode;
    unsigned short cmd;
    unsigned short len;
    unsigned short version;
	char 		   d[0];
}CMD_CHANNEL_HEAD_NOT_NEED, *PCMD_CHANNEL_HEAD_NOT_NEED;

typedef struct{
	CMD_CHANNEL_HEAD_NOT_NEED cmdhead;
	char ticket[4];
	}exSendCmd_t;

#define AVF_STARTCODE 0xa815aa55
#define MAX_FRAME_LENGTH 209715200 //200 * 1024 * 1024 ; 200KB
#define MAX_AUDIO_DATA_LENGTH 2048
typedef struct tag_AV_HEAD_NOt_NEED
{
    unsigned int   		startcode;	//  0xa815aa55
    
    unsigned char		type;		//  0->264 idr frame 1->264 p frame
    unsigned char  	    streamid;	//通道 0->
    unsigned short  	militime;	//  diff time
    
    unsigned int 		sectime;	//  diff time
    unsigned int    	frameno;	//  frameno
    int 		len;		//  data len
    
    unsigned char		version;	// version
    unsigned char		sessid;		//ssid 
    unsigned char		audiocodec; //音频编码格式 见audio_codec.h _AUDIO_CODEC
    unsigned char		audioSamplerate;//采样率 0->8000 1->16000 2->32000 
    
    unsigned char		audioBitrate;	//位宽    0->8000 1->16000 2->320000
    unsigned char		audioTrack;//通道 0->单, 1->双
    unsigned char		other[2];
	
    unsigned char		other1[4];
	char 				d[0];
}AV_HEAD_NOt_NEED,*PAV_HEAD_NOt_NEED;

//SD卡信息
typedef struct{
	 int status;
	 int totalSize;
	 int usedSize;
	 int badSize;
}SDINFO_t;

//云台控制参数 decodeCtrl
typedef enum PtzCtrl{
	PARAM_TYPE_DECODER_PTZ_TO=0,//云台控制方向
	PARAM_TYPE_DECODER_PTZ_PREFAB_SET,//设置预置位
	PARAM_TYPE_DECODER_PTZ_PREFAB_GET//获取预置位
}ptzCtrl_e;

//云台控制值
typedef enum PtzDirection{
	PARAM_VALUE_DECODER_PTZ_UP=0,//向上
	PARAM_VALUE_DECODER_PTZ_UP_STOP,//向上停
	PARAM_VALUE_DECODER_PTZ_DOWN,//向下
	PARAM_VALUE_DECODER_PTZ_DOWN_STOP,
	PARAM_VALUE_DECODER_PTZ_LEFT,
	PARAM_VALUE_DECODER_PTZ_LEFT_STOP,
	PARAM_VALUE_DECODER_PTZ_RIGHT,
	PARAM_VALUE_DECODER_PTZ_RIGHT_STOP,
	PARAM_VALUE_DECODER_PTZ_CENTER,
	PARAM_VALUE_DECODER_PTZ_UP_DOWN,
	PARAM_VALUE_DECODER_PTZ_UP_DOWN_STOP,
	PARAM_VALUE_DECODER_PTZ_LEFT_RIGHT,
	PARAM_VALUE_DECODER_PTZ_LEFT_RIGHT_STOP,
	PARAM_VALUE_DECODER_PTZ_ORIGINAL,
	
	PARAM_VALUE_DECODER_PTZ_PRESET0,
	PARAM_VALUE_DECODER_PTZ_PRESET1,
	PARAM_VALUE_DECODER_PTZ_PRESET2,
	PARAM_VALUE_DECODER_PTZ_PRESET3,
	PARAM_VALUE_DECODER_PTZ_PRESET4,
	PARAM_VALUE_DECODER_PTZ_PRESET5,
	PARAM_VALUE_DECODER_PTZ_PRESET6,
	PARAM_VALUE_DECODER_PTZ_PRESET7,
	PARAM_VALUE_DECODER_PTZ_PRESET8,
	PARAM_VALUE_DECODER_PTZ_PRESET9,
	PARAM_VALUE_DECODER_PTZ_PRESETA,
	PARAM_VALUE_DECODER_PTZ_PRESETB,
	PARAM_VALUE_DECODER_PTZ_PRESETC,
	PARAM_VALUE_DECODER_PTZ_PRESETD,
	PARAM_VALUE_DECODER_PTZ_PRESETE,
	PARAM_VALUE_DECODER_PTZ_PRESETF
	
}ptzDirection_e;
//CB_PTZ_PARAM_SET 1005//云台设置
//iParam:
typedef struct tag_ptzCtrl{
	int param;//对应参数见ptzParamType_e
	int value;//值参考ptzCtrlValueXXXX_e
	int step;
}ptzParamSet_t;
//oParam:NULL

//#define CB_PTZ_PARAM_GET//云台参数获取
//iParam:NULL
//oParam:
typedef struct{
	int status;//状态(0停止或1在动)
	int presetPosition;//先所处预置位位置
}ptzParamGet_t;
//命令头
typedef struct __CMDHEAD
{
    short 	startcode;
    short	cmd;
    short	len;
    short	version;
} cmdHead_t, *PCMDHEAD;


// 2017-06-07
// 指令数据

//串口透传数据
typedef struct{
	int len;
	char data[1024];
	}serialData_t;

//串口参数设置
typedef struct 
{
    int dwBaudRate;     //!< 波特率(bps)(110,300,1200,2400,4800,9600,19200,38400,57600,115200,230400,460800,921600)
    int dataBit;        //!< 数据有几位(5,6,7,8)
    int stopBit;        //!< 停止位(1,2)
    int parity;         //!< 校验(0-无校验,1-奇校验,2-偶校验 3-标志 4-空格)
    int flowCtrl;       //!< 数据流控制(0-无,1-软流控,2-硬流控)
}__attribute__ ((packed)) serialParam_t;

//消息推送
typedef struct tag_pushParam{
    char SetType;
    char device_token[65];
    int environment;
    int pushType;
    int validity;
    //---------jpush----------
    char appkey[64];
    char master[64];
    char alias[64];
    int type;
    //-------------bpush----------
    char apikey[64];
    char ysecret_key[64];
    char channel_id[64];
    //-------------xpush------------
    unsigned int access_id;
    char xsecret_key[64];

}pushParam_t, *pPushParam_t;

/*
云台控制相关数据结构
*/
//云台相关回调数据结构
typedef enum _ptzCtrlType{
	PTZ_PARAM_TYPE_DIRECTION=0,//云台控制方向
	PTZ_PARAM_TYPE_PREFAB//预置位
	}ptzParamType_e;

//8个方向
typedef enum _ptzCtrlValueDirection{
	PTZ_PARAM_VALUE_DIRECTION_UP=0,//向上
	PTZ_PARAM_VALUE_DIRECTION_UP_STOP,//向上停
	PTZ_PARAM_VALUE_DIRECTION_DOWN,//向下
	PTZ_PARAM_VALUE_DIRECTION_DOWN_STOP,
	PTZ_PARAM_VALUE_DIRECTION_LEFT,
	PTZ_PARAM_VALUE_DIRECTION_LEFT_STOP,
	PTZ_PARAM_VALUE_DIRECTION_RIGHT,
	PTZ_PARAM_VALUE_DIRECTION_RIGHT_STOP,
	PTZ_PARAM_VALUE_DIRECTION_CENTER,
	PTZ_PARAM_VALUE_DIRECTION_UP_DOWN,
	PTZ_PARAM_VALUE_DIRECTION_UP_DOWN_STOP,
	PTZ_PARAM_VALUE_DIRECTION_LEFT_RIGHT,
	PTZ_PARAM_VALUE_DIRECTION_LEFT_RIGHT_STOP,
	PTZ_PARAM_VALUE_DIRECTION_ORIGINAL
	}ptzCtrlValueDirection_e;

//预设16个预置位
typedef enum _ptzCtrlValuePrefab{
	PTZ_PARAM_VALUE_PRESET_0=0,
	PTZ_PARAM_VALUE_PRESET_1,
	PTZ_PARAM_VALUE_PRESET_2,
	PTZ_PARAM_VALUE_PRESET_3,
	PTZ_PARAM_VALUE_PRESET_4,
	PTZ_PARAM_VALUE_PRESET_5,
	PTZ_PARAM_VALUE_PRESET_6,
	PTZ_PARAM_VALUE_PRESET_7,
	PTZ_PARAM_VALUE_PRESET_8,
	PTZ_PARAM_VALUE_PRESET_9,
	PTZ_PARAM_VALUE_PRESET_A,
	PTZ_PARAM_VALUE_PRESET_B,
	PTZ_PARAM_VALUE_PRESET_C,
	PTZ_PARAM_VALUE_PRESET_D,
	PTZ_PARAM_VALUE_PRESET_E,
	PTZ_PARAM_VALUE_PRESET_F
}ptzCtrlValuePrefab_e;

/*
音视频处理相关的数据结构
*/

//音源采样
typedef enum {
    AC_SR_8K   = 8000,
    AC_SR_16K  = 16000,
    AC_SR_32K  = 32000,
    AC_SR_441K = 44100,
    AC_SR_48K  = 48000,
}acSampleRate_e;

//音源位宽
typedef enum {
    AC_BW_8  = 8,
    AC_BW_16 = 16,
    AC_BW_24 = 24,
}acBitwidth_e;

//audio参数类型
typedef enum{
	AUDIO_PARAM_TYPE_PROFILE=0, 		
	AUDIO_PARAM_TYPE_SOURCE,			
	AUDIO_PARAM_TYPE_MUTE,			
	AUDIO_PARAM_TYPE_CODEC, 		
	AUDIO_PARAM_TYPE_BITWIDTH,			
	AUDIO_PARAM_TYPE_INPUTGAIN, 		
	AUDIO_PARAM_TYPE_OUTPUTGAIN
}audioParamType_e;
	
//audio参数
typedef struct{
	int 	Profile;					//兼容ONVIF profile[0-2]代表编码通道号[0-2]
	int 	Source; 					//音源采样率acSampleRate_e
	int 	Mute;						//静音
	int 	Codec;						//编码类型0-PCMU 8-PCMA 21-G726
	int 	BitWidth;					//acBitwidth_e
	int 	InputGain;					//0~100
	int 	OutputGain; 				//0~100
}audioParam_t;

typedef struct{
	int paramType;//audioParamType_e
	union{
		int 	Profile;					//兼容ONVIF profile[0-2]代表编码通道号[0-2]
		int 	Source; 					//音源采样率acSampleRate_e
		int 	Mute;						//静音
		int 	Codec;						//编码类型0-PCMU 8-PCMA 21-G726
		int 	BitWidth;					//acBitwidth_e
		int 	InputGain;					//0~100
		int 	OutputGain; 				//0~100
		}u;
}audioParamSet_t;


//SD与录像
typedef struct  tag_sdRecSchSet
{
    int record_schedule_sun_0;
    int record_schedule_sun_1; 
    int record_schedule_sun_2; 
    int record_schedule_mon_0; 
    int record_schedule_mon_1; 
    int record_schedule_mon_2; 
    int record_schedule_tue_0; 
    int record_schedule_tue_1; 
    int record_schedule_tue_2; 
    int record_schedule_wed_0;
    int record_schedule_wed_1; 
    int record_schedule_wed_2; 
    int record_schedule_thu_0; 
    int record_schedule_thu_1; 
    int record_schedule_thu_2; 
    int record_schedule_fri_0; 
    int record_schedule_fri_1; 
    int record_schedule_fri_2; 
    int record_schedule_sat_0; 
    int record_schedule_sat_1; 
    int record_schedule_sat_2;  
}sdRecSchSet_t, *pSdRecSchSet_t;

//设备运行计划
	typedef struct tag_sysOprPolicy{
		unsigned int adcChkInterval;//adc检测最小周期，秒 >100
		unsigned int gpioChkInterval;//gpio检测最小周期，毫秒 >100
		
		unsigned int sysRunTime;//运行时长(秒) 0-->一直运行，n-->运行n秒
		
		unsigned char rcdPicEnable;//是否拍照 1-->是,非1不拍
		unsigned char rcdPicSize;//与码流对应 0-->720p,1-->次码流
		unsigned short rcdPicInterval;//拍照间隔(秒)
		
		unsigned char rcdAvEnable;//是否录像  1-->是,非1不录
		unsigned char rcdAvSize;//与码流对应 0-->720p,1-->次码流
		unsigned short rcdAvTime;//录像时长 0-->一直录，n-->录n秒
		
		unsigned char pushEnable;
		unsigned char alarmEnable;
		unsigned char wifiEnable;
		unsigned char osdEnable;
	
		unsigned char powerMgrEnable;//0-->无,1-->counter,2--->schedule
		unsigned char powerMgrCountDown;
		unsigned char reserved2[2];
		unsigned int  powerMgrSchedule;
	}sysOprPolicy_t_EX,*pSysOprPolicy_t_EX;

typedef struct{
	int serialNo;
	serialData_t serialData;
}serialDataTrans_t;


#endif

