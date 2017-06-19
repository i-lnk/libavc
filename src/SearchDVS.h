// SearchDVS.h: interface for the CSearchDVS class.
//
//////////////////////////////////////////////////////////////////////

#ifdef PLATFORM_ANDROID
#include <linux/socket.h>
#include <linux/tcp.h>
#include <linux/in.h>
#endif

#include "object_jni.h"

#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

#include <pthread.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <time.h>
#include <sys/stat.h>
#include <sys/types.h>

#define MAX_BIND_TIME 10

#define BROADCAST_SEND_PORT0			0x9888
#define BROADCAST_RECV_PORT0			0x8888

#define BROADCAST_SEND_PORT1			8600
#define BROADCAST_RECV_PORT1			6801

#define	SERVER_PACK_FLAG				0x03404324

typedef unsigned short 	WORD;
typedef unsigned char 	BYTE;
typedef int   			BOOL;
typedef unsigned int	DWORD;

typedef struct tagJBNV_SERVER_PACK{
	char			szIp[16];				//������Ip
	WORD			wMediaPort;				//���˿�
	WORD			wWebPort;				//Http�˿ں�
	WORD			wChannelCount;			//ͨ������
	char			szServerName[32];		//��������
	DWORD			dwDeviceType;			//����������
	DWORD			dwServerVersion;		//�������汾
	WORD			wChannelStatic;			//ͨ��״̬(�Ƿ���Ƶ��ʧ)
	WORD			wSensorStatic;			//̽ͷ״̬
	WORD			wAlarmOutStatic;		//�������״̬
}JBNV_SERVER_PACK;

typedef struct tagJBNV_SERVER_PACK_EX{
	JBNV_SERVER_PACK jspack;
	BYTE	      	bMac[6]; 			//MAC��ַ
	BOOL	        bEnableDHCP;		//DHCPʹ��
	BOOL			bEnableDNS; 		//DNSʹ��
	DWORD			dwNetMask; 			//��������
	DWORD			dwGateway; 			//����
	DWORD			dwDNS;  			//DNS������
	DWORD			dwComputerIP;		//����IP����ʱ���ã�
	BOOL	        bEnableCenter; 		//����ƽ̨ʹ�ܣ���ʱ���ã�
	DWORD			dwCenterIpAddress; 	//����ƽ̨IP��ַ
	DWORD			dwCenterPort; 		//����ƽ̨�˿�
	char			csServerNo[64]; 	//���������к�
	int				bEncodeAudio; 		//��Ƶ����ʹ��
}JBNV_SERVER_PACK_EX;

/*
typedef struct tagSEARCH_CMD{
	unsigned int dwFlag; 			//0x4844
	unsigned int dwCmd; 			//������������ӦCMD_GET������
	
}SEARCH_CMD;
*/

//*********GK_Search***************************
typedef struct tagPHONE_INFO{
	char ssid[64];
	char psd[64];
}PHONE_INFO;

typedef struct tagSEARCH_CMD{
	unsigned int dwFlag;
	unsigned int dwCmd;
	PHONE_INFO search_info; 			 			
}SEARCH_CMD;


typedef struct tagJBNV_SERVER_MSG_DATA_EX{
	DWORD					dwSize;
	DWORD					dwPackFlag; // == SERVER_PACK_FLAG
	JBNV_SERVER_PACK_EX		jbServerPack;
}JBNV_SERVER_MSG_DATA_EX;

#pragma pack(push,1)

typedef struct _stBcastParam{
    char           szIpAddr[16];     //IP��ַ�������޸�
    char           szMask[16];       //�������룬�����޸�
    char           szGateway[16];    //���أ������޸�
    char           szDns1[16];       //dns1�������޸�
    char           szDns2[16];       //dns2�������޸�
    char           szMacAddr[6];     //�豸MAC��ַ
    unsigned short nPort;            //�豸�˿�
    char           dwDeviceID[32];   //platform deviceid
    char           szDevName[80];    //�豸����
    char           sysver[16];       //�̼��汾
    char           appver[16];       //����汾
    char           szUsrName[32];   //�޸�ʱ����û���֤
    char           szPassword[32];   //�޸�ʱ����û���֤
    char           sysmode;          //0->baby 1->HDIPCAM
    char           dhcp;             //0->��ֹdhcp,1->����DHCP
    char           other[2];         //other
    char           other1[20];       //other1
}BCASTPARAM, *PBCASTPARAM;

#pragma pack(pop)

/************************************************************************/
/*ͨѶ��ʽ��
	STARTCODE+CMD+BCASTPARAM
	���壺
	STARTCODE�� short�ͣ��ַ���HD��,0x4844
	CMD��short�ͣ�0x0101 ��ʾ��ȡ
	0x0801 ��ʾ��ȡʱ�ķ���
	0x0102 ��ʾ����
	0x0802 ��ʾ����ʱ�ķ��� */
/************************************************************************/

#define STARTCODE  0x4844  //HD
#define CMD_GET  0x0101
#define CMD_PHONE_SEARCH  0x0109
#define CMD_GET_RESPONSE  0x0801 
#define CMD_SET  0x0102
#define CMD_SET_RESPONSE  0x0802

#define MAX_SOCK 3

typedef struct{
	int 	sock_port;
	int 	sock_type;
	int 	broadcast;
}T_SOCK_ATTR,*PT_SOCK_ATTR;

class CSearchDVS  
{
public:
	CSearchDVS();
	virtual ~CSearchDVS();

public:
	char WifiName[64];
	char WifiPsd[64];
	int Open(char *ssid,char *psd);
	void Close();

	int SearchDVS();
private:	
	
	void OnMessageProc(const char *pszMessage, int iBufferSize, const char *pszIp);
	void ProcMessage(short nType, unsigned short nMessageLen, const char *pszMessage);
	void ProcMessageGoke(const char * lpMsg);
	void ProcMessageShenZhou(const char * lpMsg);
	void CloseSocket();
	void * PPPPSearchThread();
private:
	static void * RecvThread(void * param);
	void RecvProcess();

    static void * SendThread(void * param);
    void SendProcess();

	static void * PPPPSearchThread(void * param);
    void YunniSearchProcess();
	
private:
	int			socks[MAX_SOCK];
	T_SOCK_ATTR sockattrs[MAX_SOCK];
	
	bool		m_bRecvThreadRuning;
	pthread_t 	m_RecvThreadID;

    int m_bSendThreadRuning;
    pthread_t m_SendThreadID;

	int Yunni_SearchThreadRunning;
	pthread_t Yunni_searchThreadID;

	JNIEnv * m_envLocal;
};
