#ifndef _XQP2P_ERROR_H_
#define _XQP2P_ERROR_H_


#define ERROR_PPPP_SUCCESSFUL						0
#define ERROR_PPPP_NOT_INITIALIZED					-1
#define ERROR_PPPP_ALREADY_INITIALIZED				-2
#define ERROR_PPPP_TIME_OUT							-3
#define ERROR_PPPP_INVALID_ID						-4
#define ERROR_PPPP_INVALID_PARAMETER				-5
#define ERROR_PPPP_DEVICE_NOT_ONLINE				-6
#define ERROR_PPPP_FAIL_TO_RESOLVE_NAME				-7
#define ERROR_PPPP_INVALID_PREFIX					-8
#define ERROR_PPPP_ID_OUT_OF_DATE					-9
#define ERROR_PPPP_NO_RELAY_SERVER_AVAILABLE		-10
#define ERROR_PPPP_INVALID_SESSION_HANDLE			-11
#define ERROR_PPPP_SESSION_CLOSED_REMOTE			-12
#define ERROR_PPPP_SESSION_CLOSED_TIMEOUT			-13
#define ERROR_PPPP_SESSION_CLOSED_CALLED			-14
#define ERROR_PPPP_REMOTE_SITE_BUFFER_FULL			-15
#define ERROR_PPPP_USER_LISTEN_BREAK				-16
#define ERROR_PPPP_MAX_SESSION						-17
#define ERROR_PPPP_UDP_PORT_BIND_FAILED				-18
#define ERROR_PPPP_USER_CONNECT_BREAK				-19
#define ERROR_PPPP_NO_CONF_FILE 					-20
#define ERROR_PPPP_NO_DB_CONNECTION 				-21
#define ERROR_PPPP_INSUFFICIENT_RESOURC 			-22
#define ERROR_PPPP_INVALID_CONFSTRING				-23
#define ERROR_PPPP_NO_NETCARD						-24
#define ERROR_PPPP_CONFFILE_CREATE_FAILED			-25
#define ERROR_PPPP_UNAUTHORIZ						-26
#define ERROR_PPPP_NOT_LOGIN 						-27
#define ERROR_PPPP_DB_OPERATION_FAILED				-28
#define ERROR_PPPP_SENDPKT_FAILED                   -29
#define ERROR_PPPP_BUFFER_ERROR                     -30
#define ERROR_PPPP_DEVICE_MAX_SESSION				-9001
#define ERROR_PPPP_SERVER_CHANGED					-99

#define JNI_ERR_LOCAL_BASE	-1000
#define JNI_ERR_LOCAL_NOT_INITIALED	JNI_ERR_LOCAL_BASE-1
#define JNI_ERR_LOCAL_SESSION_MGR_NULL	JNI_ERR_LOCAL_BASE-2
#define JNI_ERR_LOCAL_ID_NOMATCH	JNI_ERR_LOCAL_BASE-3
#define JNI_ERR_LOCAL_USER_UNAUTHED	JNI_ERR_LOCAL_BASE-4
#define JNI_ERR_LOCAL_INVALID_PARAM	JNI_ERR_LOCAL_BASE-5
#define JNI_ERR_LOCAL_INVALID_CMD	JNI_ERR_LOCAL_BASE-6

#endif

