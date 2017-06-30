#ifndef _Inc_ilnk_define_
#define _Inc_ilnk_define_

#define SESSION_TYPE_CLIENT 0
#define SESSION_TYPE_DEVICE 1

#define MAX_PUSH_SIZE     128

//JNI��JAVA����Ϣ����
#define MSG_NOTIFY_TYPE_CONNECT_PPPP_STATUS 0   	/* ����״̬ */
#define MSG_NOTIFY_TYPE_CONNECT_PPPP_MODE 1   		/* ����ģʽ */
#define MSG_NOTIFY_TYPE_CONNECT_STREAM_TYPE 2
#define MSG_NOTIFY_TYPE_CONNECT_SESSIONINF 3

#define MSG_NOTIFY_TYPE_LISTEN_PPPP_STATUS 4   	/* ����״̬ */
#define MSG_NOTIFY_TYPE_LISTEN_PPPP_MODE 5   		/* ����ģʽ */
#define MSG_NOTIFY_TYPE_LISTEN_STREAM_TYPE 6
#define MSG_NOTIFY_TYPE_LISTEN_SESSIONINF 7

#define MSG_NOTIFY_TYPE_SERVER_CHANGED		9
#define MSG_NOTIFY_TYPE_SYSTEM_STATUS 8

//system status
#define SYSTEM_STATUS_INSUFFIENT_RESOURCE 0


//P2p�Ự״ֵ̬
typedef enum{
	PPPP_STATUS_DISCONNECTED=0,//���ӶϿ�
	PPPP_STATUS_CONNECTING, //������
	PPPP_STATUS_CONNECTED, //�Ѿ�����
	PPPP_STATUS_ID_INVALID, //��ЧID
	PPPP_STATUS_DEVICE_OFFLINE,//�豸����
	PPPP_STATUS_LOCAL_SESSION_OVERLIMIT,//���ػỰ����
	PPPP_STATUS_REMOTE_SESSION_OVERLIMIT, //�Զ˻Ự����
	PPPP_STATUS_SESSIONSETUP_TIMEOUT,//�Ự������ʱ
	PPPP_STATUS_ID_OUTOFDATE,//ID����
	PPPP_STATUS_USER_AUTHENTICATING,//������֤��
	PPPP_STATUS_USER_AUTHENTICATED,//��֤�ɹ�
	PPPP_STATUS_USER_INVALID,//��Ч�û�
	PPPP_STATUS_UNKNOWN_ERROR,//δ֪����
	PPPP_STATUS_NOT_INITIALIZED,//δ��ʼ��
	PPPP_STATUS_DISCONNECTING,//���ڶϿ���
	PPPP_STATUS_USER_INAUTHENTICATED,//δ��֤��ticketΪ��
	PPPP_STATUS_USER_NOT_LOGIN	//�û�δ��¼
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

/////////////�����////////////////////
#define CMD_GW_DEVLST_CLEAN			0x0100//��������豸�б�
#define CMD_GW_DEVLST_GET 			0x0101//��ȡ�����豸�б�
#define CMD_GW_DEVLST_SET 			0x0102//��ӻ��޸��豸�б����豸��Ϣ
#define CMD_GW_DEV_SEARCH 			0x0103//�����������豸
#define CMD_GW_DEV_ALLSTREAM_GET		0x0110//��ȡ�豸����������RingBuf
#define CMD_GW_DEV_ALLSTREAM_CANCEL	0x0111//ȡ���豸����������RingBuf
#define CMD_GW_DEV_STREAM_REC_START	0x0112//����ʵʱ��ʼ¼��
#define CMD_GW_DEV_STREAM_REC_STOP	0x0113//����ʵʱֹͣ¼��
#define CMD_GW_DEV_RECSCHEDULE_ASSOCIATE		0x0114//��������¼��ƻ���ĳ�豸
#define CMD_GW_DEV_RECSCHEDULE_DISASSOCIATE		0x0115//��ȡ����¼��ƻ���ĳ�豸��ƥ��
#define CMD_GW_RECSCHEDULE_ADD		0x0116//�������¼��ƻ�
#define CMD_GW_RECSCHEDULE_DEL		0x0117//��IDɾ������¼��ƻ�
#define CMD_GW_RECSCHEDULE_CHG		0x0118//��ID�޸�����¼��ƻ�
#define CMD_GW_RECSCHEDULELST_GET 	0x0119//��ȡ����¼��ƻ��б�
#define CMD_GW_RECSCHEDULELST_CLEAN	0x0120//���ȫ������¼��

#define CMD_DEV_BROADCAST 					0x0fff//�豸����ʱ����Ϣ�㲥

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

#define CMD_AUDIO_PARAM_SET				0x3004//����audio���� audioParamSet_t
#define CMD_AUDIO_PARAM_GET				0x3005//����audio���� audioParam_t

//video ctrl
#define CMD_DATACTRL_PLAYLIVE_START		0x3010
#define CMD_DATACTRL_PLAYLIVE_STOP		0x3011
#define CMD_DATACTRL_PLAYBACK_START		0x3012
#define CMD_DATACTRL_PLAYBACK_STOP		0x3013

#define CMD_DEV_RECORD_PLAYBACK_SEEK		0x3014
#define CMD_DEV_RECORD_PLAYBACK_SPEED		0x3015




#define CMD_DATACTRL_DOORBELL_CALL_ACCEPT	0x3020//���н���
#define CMD_DATACTRL_DOORBELL_CALL_REJECT	0x3021//���оܾ�

//filetransfer ctrl
#define CMD_DATACTRL_FILESAVEPATH	0x4001
#define CMD_DATACTRL_FILE_GET		0x4002
#define CMD_DATACTRL_FILE_PUT		0x4003
#define CMD_DATACTRL_FILE_SET		0x4004//ɾ����
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

////////////��������/////////////////////
#define CMD_LOCAL_AVREC_START		0x5100
#define CMD_LOCAL_AVREC_STOP		0x5101
#define CMD_LOCAL_SESSION_CHECK		0x55FE

//��������
#define CMD_FILE_DOWNLOAD_PROGRESS		0x8001
/////////////////�����������ݽṹ����/////////////////////////
typedef struct tag_P2pParamBase{
	int iSessionNmb;//���Ự��
	int iChannelNmb;//ÿ�Ựͨ����
	int iChannelBufSize;//ÿͨ���ײ㻺���СKB
}P2pParamBase_t,pP2pParamBase_t;

typedef struct tag_P2pNodeBase{
	int iType;//�豸����
	char sID[MAX_P2PID_LEN];//�豸��p2pID
}P2pNodeBase_t,*pP2pNodeBase_t;

typedef struct tag_P2pNodeBase_EX{
	int iType;//�豸����
	char sID[MAX_P2PID_LEN];//�豸��p2pID
	char sServer[MAX_P2PSERVER_LEN];
}P2pNodeBase_t_EX,*pP2pNodeBase_t_EX;


typedef struct tag_P2pNodeExt{
	P2pNodeBase_t strNodeInf;
	char sUsr[MAX_USR_NAME_LEN];
	char sPwd[MAX_USR_PWD_LEN];
	char sServer[MAX_P2PSERVER_LEN];
}P2pNodeExt_t,*pP2pNodeExt_t;


//�̼�����
typedef struct tag_firmwareUpgrade{
	int type;
	char url[128];
	}firmwareUpgrade_t;

//�豸��p2p��������
typedef struct tag_openP2pSetting_t{
	char p2pID[24];
	char devName[64];
	char svrStr[512];
}openP2pSetting_t, *pOpenP2pSetting_t;


//ϵͳ���в���
typedef struct{
	unsigned char adcSelect;//000000001-->sadc0,00000010-->sadc1,00000011-->all,other-->disable
	unsigned char adcChkInterval;//�� 
	unsigned char reserved1[2];
	
	unsigned short gpioSelect;
	unsigned short gpioChkInterval;//���� >100

	unsigned int sysRunTime;//����ʱ��(��) 0-->һֱ���У�n-->����n��
	
	unsigned char rcdPicEnable;//�Ƿ����� 1-->��,��1����
	unsigned char rcdPicSize;//��������Ӧ 0-->720p,1-->������
	unsigned short rcdPicInterval;//���ռ��(��)
	
	unsigned char rcdAvEnable;//�Ƿ�¼��  1-->��,��1��¼
	unsigned char rcdAvSize;//��������Ӧ 0-->720p,1-->������
	unsigned short rcdAvTime;//¼��ʱ�� 0-->һֱ¼��n-->¼n��
	
	unsigned char pushEnable;
	unsigned char alarmEnable;
	unsigned char wifiEnable;
	unsigned char reserved;

	unsigned char powerMgrEnable;//0-->��,1-->counter,2--->schedule
	unsigned char powerMgrCountDown;
	unsigned char reserved2[2];
	unsigned int  powerMgrSchedule;
	}sysOprPolicy_t;

//�澯֪ͨ
typedef struct tag_xqAlarm{
	int alarmType;
	unsigned int alarmLength;
	char *alarmContent;
	struct tag_xqAlarm *pNext;
}xqAlarm_t;

//�û�
typedef struct{
	char name[32];
	char pass[128];
}user_t;

typedef struct tag_P2pNodeExt_EX{
	P2pNodeBase_t_EX	node;
	user_t				user;
}P2pNodeExt_t_EX,*pP2pNodeExt_t_EX;

//�û�����
typedef struct tag_userSetting
{
    char user1[32];
    char pwd1[128];
    char user2[32];
    char pwd2[128];
    char user3[32];
    char pwd3[128];
}userSetting_t,*pUserSetting_t;


//�û���֤���
typedef struct tag_usrChkRet{
	int privilege;
	char ticket[4];
	}st_usrChkRet;


//�豸״̬
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

//ʱ������
typedef struct tag_datetimeParam
{
    int now;
    int tz;
    int ntp_enable;
	int xia_ling_shi_flag_status;
    char ntp_svr[64];
}datetimeParam_t,*pDatetimeParam_t;

//GPIO���
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

//wifi����
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
//ɨ��wifi���ؽ��CMD_NET_WIFI_SCAN
typedef struct tag_wifiScanRet
{
    char ssid[64];
    char mac[8];
    int security;
    int dbm0;//ǿ��ֵ
    int dbm1;//��׼
    int mode;
    int channel;

}wifiScanRet_t,*pWifiScanRet_t;

//ADCֵ
typedef struct tag_adc
{
    int adcSelect;
    int adcValue1;
    int adcValue2;
}adc_t, *pAdc_t;


//��־
typedef struct tag_logSet{
	int actType;//0-->clearlog,1-->setlog
	unsigned char bAlarmLog;
	unsigned char bStartUpLog;
	unsigned char bShutdwonLog;
	unsigned char bTimerLog;
	}logSet_t;

//��־��������
typedef struct tag_logSearchCondition{
	unsigned char searchType; // 0-->������,1-->��ʱ��
	unsigned char mainType;
	unsigned char subType;
	unsigned char reserved;
	unsigned int startTime;
	unsigned int endTime;
}logSearchCondition_t;

//��־��Ϣ��ȡ
typedef struct{
	unsigned char mainType;
	unsigned char subType;
	unsigned short	length;
	unsigned int timeStamp;
	unsigned char	content[120];
}LogInfo_t;
//����
typedef struct tag_picCap{
	int picSize;
	int ifSave;
	}picCap_t;

//¼�����
typedef struct recNow{
	int ifRecNow;
	}recNow_t;

//¼���ļ���ȡ 
typedef struct tag_recFileSearchParam
{
    int starttime;
    int endtime;
}recFileSearchParam_t, *pRecFileSearchParam_t;

//ADC����
typedef struct{
	char bEnable;
	char bNotify;
	char bPicCap;
	char bAvRec;
	short iTopTrigValue;
	short iBottomTrigValue;
	}sAdcCnf_t;

//�ִ�͸��
typedef struct tag_passThroghString{
	int strLength;
	char strContent[1024];
}passThroghString_t;

//����ͷͼ�����ò���
typedef enum{
	VIDEO_PARAM_TYPE_DEFAULT=0,//�ָ���������Ƶ����ȱʡֵ
	VIDEO_PARAM_TYPE_RESOLUTION,//�ֱ���
	VIDEO_PARAM_TYPE_BRIGHTNESS,//����
	VIDEO_PARAM_TYPE_CONTRAST,//�Աȶ�
	VIDEO_PARAM_TYPE_SATURATION,//ɫ�ʱ��Ͷ�
	VIDEO_PARAM_TYPE_SHARPNESS,//����
	VIDEO_PARAM_TYPE_FRAMERATE,//֡��
	VIDEO_PARAM_TYPE_BITRATE,//����
	VIDEO_PARAM_TYPE_ROTATE,//ͼ����ͷ�ת0����,1����,2����,3��������
	VIDEO_PARAM_TYPE_IRCUT,//ircut���� 0->�Զ���1->enable,2->�ر�
	VIDEO_PARAM_TYPE_OSD,//0->�ر�1->��
	VIDEO_PARAM_TYPE_MOVEDETECTION,//0->�ر�1->��
	VIDEO_PARAM_TYPE_MODE//flicker
}videoParamType_e;

//�������������
typedef struct tag_camCtrl{
	unsigned short param;
	union{
		int resolution;//�ֱ���
		int brightness;//����
		int contrast;//�Աȶ�
		int saturation;//ɫ�ʱ��Ͷ�
		int sharpness;//����
		int frameRate;//֡��
		int bitRate;//����
		int rotate;//ͼ����ͷ�ת0����,1����,2����,3��������
		int ircut;//ircut���� 0->�Զ���1->enable,2->�ر�
		int osd;//0->�ر�1->��
		int moveDetect;//0->�ر�1->��
		int mode;
	}u;
	
}camCtrl_t;

//�������������
typedef struct tag_camCtrl_EX{
	int param;
	int value;
}camCtrl_t_EX;


//��Ӱͷ������ȡ
typedef struct{
	int resolution;//�ֱ���
	int brightness;//����
	int contrast;//�Աȶ�
	int saturation;//ɫ�ʱ��Ͷ�
	int sharpness;//����
	int frameRate;//֡��
	int bitRate;//����
	int rotate;//ͼ����ͷ�ת0����,1����,2����,3��������
	int ircut;//ircut���� 0->�Զ���1->enable,2->�ر�
	int mode;
}cameraParams_t;

//�����ͼ������
typedef struct image_Options
{
	unsigned int	CtrlMode;	//0-->fixed 1-->auto
	unsigned int	DayMode;	//0-->night 1-->day

	unsigned int	Brightness;					/*����*/
	unsigned int	Contrast;					/*�Աȶ�*/
	unsigned int	ColorSaturation;					/*���Ͷ�*/
	unsigned int	Sharpness;					/*������*/

	unsigned char	Mirror;					/*0-���� 1-���·�ת 2->���ҷ�ת 3->����/���ҷ�ת*/
	unsigned char	Flip;
}ImageOptions_t;

//sd���洢����
// ¼������
typedef struct 
{
    unsigned int      rcVideoWidth;
    unsigned int      rcVideoHeight;
    unsigned int      rcVideoRate;        // ֡��(25)
    unsigned int      rcVideoMaxBitrate;

    unsigned int      rcAudioSamplerate;
    unsigned int      rcAudioBitWidth;    // 8/16
    unsigned int      rcAudioMaxBitrate;
    unsigned int      rcAudioTrack;       // 1-������, 2-������
} mRfsRecConfig_t;
// �洢����
typedef struct
{
    unsigned int       spMaxHour;          // ���ֶ��ٸ�Сʱ�ڵ�¼��(0-������, ����ֵ-Сʱ��)
    unsigned int      spFullThreshold;    // �ٽ�ֵ(ʣ��ռ䲻�����MBʱ��Ϊ��)
    unsigned char      spRecycle;          // �������Ƿ�ѭ������(0-������, 1-����)
    unsigned char      spCleanData;        // ����ʱ�Ƿ��������(0-����, 1-���)
    unsigned char      spReserved[2];
} mRfsStoragePolicy_t;

typedef struct{
	mRfsRecConfig_t	recConf; 
	mRfsStoragePolicy_t	storagePolicy;
}AVRecPolicy_t;

// ¼������
typedef struct 
{
    unsigned int      rcVideoWidth;
    unsigned int      rcVideoHeight;
    unsigned int      rcVideoRate;        // ֡��(25)
    unsigned int      rcVideoMaxBitrate;

    unsigned int      rcAudioSamplerate;
    unsigned int      rcAudioBitWidth;    // 8/16
    unsigned int      rcAudioMaxBitrate;
    unsigned int     rcAudioTrack;       // 1-������, 2-������
} mRecConfig_t;

// �洢����
typedef struct
{
    unsigned int      spMaxHour;          // ���ֶ��ٸ�Сʱ�ڵ�¼��(0-������, ����ֵ-Сʱ��)
    unsigned int      spFullThreshold;    // �ٽ�ֵ(ʣ��ռ䲻�����MBʱ��Ϊ��)
    unsigned int      spRecycle;          // �������Ƿ�ѭ������(0-������, 1-����)
    unsigned int      spCleanData;        // ����ʱ�Ƿ��������(0-����, 1-���)
} mStoragePolicy_t;

//¼�����
typedef struct{
	mRecConfig_t	recConf; 
	mStoragePolicy_t	storagePolicy;
	}mAVRecPolicy_t;



//�ļ��������,Ҳ����¼���ļ���¼���ļ��ط�
typedef struct tag_fileTransParam{
	int offset;
	char filename[128];
}fileTransParam_t;

//sd���ļ����� CMD_FILETRANSFER_FILELIST_GET
typedef struct tag_xqFile{
int type;
unsigned int size;
unsigned int  timeStamp;
char name[64];
}xqFile_t;


//����͸������
typedef struct{
	int len;
	char data[1024];
	}SerialData_t;

//���ڲ�������
typedef struct 
{
    int dwBaudRate;     //!< ������(bps)(110,300,1200,2400,4800,9600,19200,38400,57600,115200,230400,460800,921600)
    int dataBit;        //!< �����м�λ(5,6,7,8)
    int stopBit;        //!< ֹͣλ(1,2)
    int parity;         //!< У��(0-��У��,1-��У��,2-żУ�� 3-��־ 4-�ո�)
    int flowCtrl;       //!< ����������(0-��,1-������,2-Ӳ����)
}__attribute__ ((packed)) SerialConfig_t;


//��Ϣ����
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

//��ȡpush����
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

//�ļ��������
typedef struct tag_FILE_DATA_HEAD
{
unsigned int startcode;//0xff12ff34
unsigned char type;//0-->file attribute,1-->file content
unsigned char reserved;
unsigned short blockseq;
unsigned int len;
char magic[4];//��ʶͬһ�ļ�
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
    unsigned char  	    streamid;	//ͨ�� 0->
    unsigned short  	militime;	//  diff time
    
    unsigned int 		sectime;	//  diff time
    unsigned int    	frameno;	//  frameno
    int 		len;		//  data len
    
    unsigned char		version;	// version
    unsigned char		sessid;		//ssid 
    unsigned char		audiocodec; //��Ƶ�����ʽ ��audio_codec.h _AUDIO_CODEC
    unsigned char		audioSamplerate;//������ 0->8000 1->16000 2->32000 
    
    unsigned char		audioBitrate;	//λ��    0->8000 1->16000 2->320000
    unsigned char		audioTrack;//ͨ�� 0->��, 1->˫
    unsigned char		other[2];
	
    unsigned char		other1[4];
	char 				d[0];
}AV_HEAD_NOt_NEED,*PAV_HEAD_NOt_NEED;

//SD����Ϣ
typedef struct{
	 int status;
	 int totalSize;
	 int usedSize;
	 int badSize;
}SDINFO_t;

//��̨���Ʋ��� decodeCtrl
typedef enum PtzCtrl{
	PARAM_TYPE_DECODER_PTZ_TO=0,//��̨���Ʒ���
	PARAM_TYPE_DECODER_PTZ_PREFAB_SET,//����Ԥ��λ
	PARAM_TYPE_DECODER_PTZ_PREFAB_GET//��ȡԤ��λ
}ptzCtrl_e;

//��̨����ֵ
typedef enum PtzDirection{
	PARAM_VALUE_DECODER_PTZ_UP=0,//����
	PARAM_VALUE_DECODER_PTZ_UP_STOP,//����ͣ
	PARAM_VALUE_DECODER_PTZ_DOWN,//����
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
//CB_PTZ_PARAM_SET 1005//��̨����
//iParam:
typedef struct tag_ptzCtrl{
	int param;//��Ӧ������ptzParamType_e
	int value;//ֵ�ο�ptzCtrlValueXXXX_e
	int step;
}ptzParamSet_t;
//oParam:NULL

//#define CB_PTZ_PARAM_GET//��̨������ȡ
//iParam:NULL
//oParam:
typedef struct{
	int status;//״̬(0ֹͣ��1�ڶ�)
	int presetPosition;//������Ԥ��λλ��
}ptzParamGet_t;
//����ͷ
typedef struct __CMDHEAD
{
    short 	startcode;
    short	cmd;
    short	len;
    short	version;
} cmdHead_t, *PCMDHEAD;


// 2017-06-07
// ָ������

//����͸������
typedef struct{
	int len;
	char data[1024];
	}serialData_t;

//���ڲ�������
typedef struct 
{
    int dwBaudRate;     //!< ������(bps)(110,300,1200,2400,4800,9600,19200,38400,57600,115200,230400,460800,921600)
    int dataBit;        //!< �����м�λ(5,6,7,8)
    int stopBit;        //!< ֹͣλ(1,2)
    int parity;         //!< У��(0-��У��,1-��У��,2-żУ�� 3-��־ 4-�ո�)
    int flowCtrl;       //!< ����������(0-��,1-������,2-Ӳ����)
}__attribute__ ((packed)) serialParam_t;

//��Ϣ����
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
��̨����������ݽṹ
*/
//��̨��ػص����ݽṹ
typedef enum _ptzCtrlType{
	PTZ_PARAM_TYPE_DIRECTION=0,//��̨���Ʒ���
	PTZ_PARAM_TYPE_PREFAB//Ԥ��λ
	}ptzParamType_e;

//8������
typedef enum _ptzCtrlValueDirection{
	PTZ_PARAM_VALUE_DIRECTION_UP=0,//����
	PTZ_PARAM_VALUE_DIRECTION_UP_STOP,//����ͣ
	PTZ_PARAM_VALUE_DIRECTION_DOWN,//����
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

//Ԥ��16��Ԥ��λ
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
����Ƶ������ص����ݽṹ
*/

//��Դ����
typedef enum {
    AC_SR_8K   = 8000,
    AC_SR_16K  = 16000,
    AC_SR_32K  = 32000,
    AC_SR_441K = 44100,
    AC_SR_48K  = 48000,
}acSampleRate_e;

//��Դλ��
typedef enum {
    AC_BW_8  = 8,
    AC_BW_16 = 16,
    AC_BW_24 = 24,
}acBitwidth_e;

//audio��������
typedef enum{
	AUDIO_PARAM_TYPE_PROFILE=0, 		
	AUDIO_PARAM_TYPE_SOURCE,			
	AUDIO_PARAM_TYPE_MUTE,			
	AUDIO_PARAM_TYPE_CODEC, 		
	AUDIO_PARAM_TYPE_BITWIDTH,			
	AUDIO_PARAM_TYPE_INPUTGAIN, 		
	AUDIO_PARAM_TYPE_OUTPUTGAIN
}audioParamType_e;
	
//audio����
typedef struct{
	int 	Profile;					//����ONVIF profile[0-2]�������ͨ����[0-2]
	int 	Source; 					//��Դ������acSampleRate_e
	int 	Mute;						//����
	int 	Codec;						//��������0-PCMU 8-PCMA 21-G726
	int 	BitWidth;					//acBitwidth_e
	int 	InputGain;					//0~100
	int 	OutputGain; 				//0~100
}audioParam_t;

typedef struct{
	int paramType;//audioParamType_e
	union{
		int 	Profile;					//����ONVIF profile[0-2]�������ͨ����[0-2]
		int 	Source; 					//��Դ������acSampleRate_e
		int 	Mute;						//����
		int 	Codec;						//��������0-PCMU 8-PCMA 21-G726
		int 	BitWidth;					//acBitwidth_e
		int 	InputGain;					//0~100
		int 	OutputGain; 				//0~100
		}u;
}audioParamSet_t;


//SD��¼��
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

//�豸���мƻ�
	typedef struct tag_sysOprPolicy{
		unsigned int adcChkInterval;//adc�����С���ڣ��� >100
		unsigned int gpioChkInterval;//gpio�����С���ڣ����� >100
		
		unsigned int sysRunTime;//����ʱ��(��) 0-->һֱ���У�n-->����n��
		
		unsigned char rcdPicEnable;//�Ƿ����� 1-->��,��1����
		unsigned char rcdPicSize;//��������Ӧ 0-->720p,1-->������
		unsigned short rcdPicInterval;//���ռ��(��)
		
		unsigned char rcdAvEnable;//�Ƿ�¼��  1-->��,��1��¼
		unsigned char rcdAvSize;//��������Ӧ 0-->720p,1-->������
		unsigned short rcdAvTime;//¼��ʱ�� 0-->һֱ¼��n-->¼n��
		
		unsigned char pushEnable;
		unsigned char alarmEnable;
		unsigned char wifiEnable;
		unsigned char osdEnable;
	
		unsigned char powerMgrEnable;//0-->��,1-->counter,2--->schedule
		unsigned char powerMgrCountDown;
		unsigned char reserved2[2];
		unsigned int  powerMgrSchedule;
	}sysOprPolicy_t_EX,*pSysOprPolicy_t_EX;

typedef struct{
	int serialNo;
	serialData_t serialData;
}serialDataTrans_t;


#endif

