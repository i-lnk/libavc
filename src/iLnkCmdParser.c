/**/
#include <jni.h>
#include <string.h>
#include "stdio.h"

#include "iLnkDefine.h"
#include "PPPP_Error.h"
#include "utility.h"
#include "iLnkCmdParser.h"

int parseLogSet(JNIEnv *env,jobject cmdParam,logSet_t *logSet){
	int nRet=0;
	jclass cmdParamClass;
	
	cmdParamClass= env->FindClass("ilnk/lib/bean/LogSettingBean");
	if (cmdParamClass==0){
		Log2("cant not get cmdParamClass!!!!");
		return -1;
		}
	
	jfieldID actType = env->GetFieldID(cmdParamClass,"actType","I");
	jfieldID bAlarmLog = env->GetFieldID(cmdParamClass,"bAlarmLog","I");
	jfieldID bStartUpLog = env->GetFieldID(cmdParamClass,"bStartUpLog","I");
	jfieldID bShutdwonLog = env->GetFieldID(cmdParamClass,"bShutdwonLog","I");
	jfieldID bTimerLog = env->GetFieldID(cmdParamClass,"bTimerLog","I");

	jint jactType = env->GetIntField(cmdParam , actType); 							
	jint jbAlarmLog = env->GetIntField(cmdParam , bAlarmLog); 							
	jint jbStartUpLog = env->GetIntField(cmdParam , bStartUpLog);								
	jint jbShutdwonLog = env->GetIntField(cmdParam , bShutdwonLog); 							
	jlong jbTimerLog = env->GetIntField(cmdParam , bTimerLog); 							

	logSet->actType=(int)jactType;
	logSet->bAlarmLog=(int)jbAlarmLog;
	logSet->bStartUpLog=(int)jbStartUpLog;
	logSet->bShutdwonLog=(int)jbShutdwonLog;
	logSet->bTimerLog=(unsigned int)jbTimerLog;

	return nRet;
}

int parseLogSearchCondition(JNIEnv *env,jobject cmdParam,logSearchCondition_t *logSearchCondition){
	int nRet=0;
	jclass cmdParamClass;
	
	cmdParamClass= env->FindClass("ilnk/lib/bean/LogSearchCondition");
	if (cmdParamClass==0){
		Log2("cant not get cmdParamClass!!!!");
		return -1;
		}
	
	jfieldID searchType = env->GetFieldID(cmdParamClass,"searchType","I");
	jfieldID mainType = env->GetFieldID(cmdParamClass,"mainType","I");
	jfieldID subType = env->GetFieldID(cmdParamClass,"subType","I");
	jfieldID reserved = env->GetFieldID(cmdParamClass,"reserved","I");
	jfieldID startTime = env->GetFieldID(cmdParamClass,"startTime","J");
	jfieldID endTime = env->GetFieldID(cmdParamClass,"endTime","J");
	jint jsearchType = env->GetIntField(cmdParam , searchType);								
	jint jmainType = env->GetIntField(cmdParam , mainType);								
	jint jsubType = env->GetIntField(cmdParam , subType);								
	jint jreserved = env->GetIntField(cmdParam , reserved);								
	jlong jstartTime = env->GetLongField(cmdParam , startTime);								
	jlong jendTime = env->GetLongField(cmdParam , endTime);

	logSearchCondition->searchType=(int)jsearchType;
	logSearchCondition->mainType=(int)jmainType;
	logSearchCondition->subType=(int)jsubType;
	logSearchCondition->reserved=(int)jreserved;
	logSearchCondition->startTime=(unsigned int)jstartTime;
	logSearchCondition->endTime=(unsigned int)jendTime;

	return nRet;
}

#if 1

int parseItemType(JNIEnv *env,jobject cmdParam,int *itemType){
	int nRet=0;
	jclass cmdParamClass;
	
	cmdParamClass= env->FindClass("ilnk/lib/bean/ItemRecoveryBean");
	if (cmdParamClass==0){
		Log2("cant not get cmdParamClass!!!!");
		return -1;
		}
	
	jfieldID tz=env->GetFieldID(cmdParamClass,"item_type","I");
	jint jtz=env->GetIntField((jobject)cmdParam, tz);
	
	*itemType=(int)jtz;

	return nRet;	
	}

int parseFirmwareUpgrade(JNIEnv *env,jobject cmdParam,firmwareUpgrade_t *firmwareUpgrade){
	int nRet=0;
	jclass cmdParamClass;
	
	cmdParamClass= env->FindClass("ilnk/lib/bean/FirmwareUpgrade");
	if (cmdParamClass==0){
		Log2("cant not get cmdParamClass!!!!");
		return -1;
		}
	
	jfieldID Url = env->GetFieldID(cmdParamClass,"mUrl","Ljava/lang/String;");
	jfieldID Type = env->GetFieldID(cmdParamClass,"mType","I");
	
	jstring jUrl  = (jstring)env->GetObjectField(cmdParam ,Url);								
	const char * szUrl	= env->GetStringUTFChars(jUrl ,NULL);
	jint jType = env->GetIntField(cmdParam , Type); 							

	firmwareUpgrade->type=(int)jType;
	if (szUrl != NULL)
		strncpy(firmwareUpgrade->url,(char *)szUrl,128);
	
	env->ReleaseStringUTFChars(jUrl, szUrl);
	
	return nRet;
	}

int parseUserChk(JNIEnv *env,jobject cmdParam,user_t *usrInf){
	int nRet=0;
	jclass cmdParamClass;

	cmdParamClass= env->FindClass("ilnk/lib/bean/UserCheckBean");
	if (cmdParamClass==0){
		Log2("cant not get cmdParamClass!!!!");
		return -1;
		}

	jfieldID usrName = env->GetFieldID(cmdParamClass,"account","Ljava/lang/String;");
	jfieldID usrPwd = env->GetFieldID(cmdParamClass,"password","Ljava/lang/String;");

	jstring jusrName = (jstring)env->GetObjectField(cmdParam , usrName);								
	const char * szusrName = env->GetStringUTFChars(jusrName ,NULL);
	jstring jusrPwd = (jstring)env->GetObjectField(cmdParam , usrPwd);								
	const char * szusrPwd= env->GetStringUTFChars(jusrPwd ,NULL);

	if (szusrName != NULL){
		strncpy(usrInf->name,(char *)szusrName,32);
	}	
	if (szusrPwd != NULL){
		strncpy(usrInf->pass,(char *)szusrPwd,128);
	}	

	env->ReleaseStringUTFChars(jusrName, szusrName);
	env->ReleaseStringUTFChars(jusrPwd, szusrPwd);
		
	return nRet;
}

int parseUserSetting(JNIEnv *env,jobject cmdParam,userSetting_t *userSetting){
	int nRet=0;
	jclass cmdParamClass;
	
	cmdParamClass= env->FindClass("ilnk/lib/bean/DevUsersBean");
	if (cmdParamClass==0){
		Log2("cant not get cmdParamClass!!!!");
		return -1;
		}
	
	jfieldID visitorName = env->GetFieldID(cmdParamClass,"visitorName","Ljava/lang/String;");
	jfieldID visitorPwd = env->GetFieldID(cmdParamClass,"visitorPwd","Ljava/lang/String;");
	jfieldID operatorName = env->GetFieldID(cmdParamClass,"operatorName","Ljava/lang/String;");
	jfieldID operatorPwd = env->GetFieldID(cmdParamClass,"operatorPwd","Ljava/lang/String;");
	jfieldID adminName = env->GetFieldID(cmdParamClass,"adminName","Ljava/lang/String;");
	jfieldID adminPwd = env->GetFieldID(cmdParamClass,"adminPwd","Ljava/lang/String;");
	
	jstring jvisitorName = (jstring)env->GetObjectField(cmdParam , visitorName);								
	const char * szvisitorName = env->GetStringUTFChars(jvisitorName ,NULL);
	jstring jvisitorPwd = (jstring)env->GetObjectField(cmdParam , visitorPwd);								
	const char * szvisitorPwd = env->GetStringUTFChars(jvisitorPwd ,NULL);
	jstring joperatorName = (jstring)env->GetObjectField(cmdParam , operatorName);								
	const char * szoperatorName = env->GetStringUTFChars(joperatorName ,NULL);
	jstring joperatorPwd = (jstring)env->GetObjectField(cmdParam , operatorPwd);								
	const char * szoperatorPwd = env->GetStringUTFChars(joperatorPwd ,NULL);
	jstring jadminName = (jstring)env->GetObjectField(cmdParam , adminName);								
	const char * szadminName = env->GetStringUTFChars(jadminName ,NULL);
	jstring jadminPwd = (jstring)env->GetObjectField(cmdParam , adminPwd);								
	const char * szadminPwd = env->GetStringUTFChars(jadminPwd ,NULL);
		
	if (szvisitorName != NULL){
		strncpy(userSetting->user1,(char *)szadminName,32);
	}	
	if (szvisitorPwd != NULL){
		strncpy(userSetting->pwd1,(char *)szadminPwd,128);
	}	
	if (operatorName != NULL){
		strncpy(userSetting->user2,(char *)szoperatorName,32);
	}	
	if (operatorPwd != NULL){
		strncpy(userSetting->pwd2,(char *)szoperatorPwd,128);
	}	
	if (adminName != NULL){
		strncpy(userSetting->user3,(char *)szvisitorName,32);
	}	
	if (szadminPwd != NULL){
		strncpy(userSetting->pwd3,(char *)szvisitorPwd,128);
	}	

	Log3("user0=%s,pwd0=%s,user1=%s,pwd1=%s,user2=%s,pwd2=%s,",
		userSetting->user1,userSetting->pwd1
		,userSetting->user2,userSetting->pwd2
		,userSetting->user3,userSetting->pwd3);
	
	env->ReleaseStringUTFChars(jvisitorName, szvisitorName);
	env->ReleaseStringUTFChars(joperatorName, szoperatorName);
	env->ReleaseStringUTFChars(jadminName, szadminName);
	env->ReleaseStringUTFChars(jvisitorPwd, szvisitorPwd);
	env->ReleaseStringUTFChars(joperatorPwd, szoperatorPwd);
	env->ReleaseStringUTFChars(jadminPwd, szadminPwd);
	
	return nRet;
	}

int parseP2PSetting(JNIEnv *env,jobject cmdParam,openP2pSetting_t *p2pSetting){
	int nRet=0;
	jclass cmdParamClass;
	
	cmdParamClass= env->FindClass("ilnk/lib/bean/P2pSettingBean");
	if (cmdParamClass==0){
		Log2("cant not get cmdParamClass!!!!");
		return -1;
		}
	
	jfieldID ServerString = env->GetFieldID(cmdParamClass,"ServerString","Ljava/lang/String;");
	jfieldID NameOfNode = env->GetFieldID(cmdParamClass,"NameOfNode","Ljava/lang/String;");
	jfieldID P2pIdOfNode = env->GetFieldID(cmdParamClass,"P2pIdOfNode","Ljava/lang/String;");
	
	jstring jServerString = (jstring)env->GetObjectField(cmdParam , ServerString);								
	const char * szServerString = env->GetStringUTFChars(jServerString ,NULL);
	jstring jNameOfNode = (jstring)env->GetObjectField(cmdParam , NameOfNode);								
	const char * szNameOfNode = env->GetStringUTFChars(jNameOfNode ,NULL);
	jstring jP2pIdOfNode = (jstring)env->GetObjectField(cmdParam , P2pIdOfNode);								
	const char * szP2pIdOfNode = env->GetStringUTFChars(jP2pIdOfNode ,NULL);
		
	if (szServerString != NULL){
		strncpy(p2pSetting->svrStr,(char *)szServerString,512);
	}	
	if (szNameOfNode != NULL){
		strncpy(p2pSetting->devName,(char *)szNameOfNode,64);
	}	
	if (szP2pIdOfNode != NULL){
		strncpy(p2pSetting->p2pID,(char *)szP2pIdOfNode,24);
	}	
	
	env->ReleaseStringUTFChars(jServerString, szServerString);
	env->ReleaseStringUTFChars(jNameOfNode, szNameOfNode);
	env->ReleaseStringUTFChars(jP2pIdOfNode, szP2pIdOfNode);
	
	Log2("serverString=%s\r\n,szNameOfNode=%s\r\np2pID=%s\r\n",p2pSetting->svrStr,p2pSetting->devName,p2pSetting->p2pID);

	return nRet;
}

int parseP2pNodeExt(JNIEnv *env,jobject devObj,P2pNodeExt_t_EX * devBase){
	jclass devClass;
	
	jfieldID p2pID;
	jfieldID svrStr;
	jfieldID usr;
	jfieldID pwd;

	jstring jszDID;
	jstring jSvr;
	jstring jUsr;
	jstring jPwd;
	
    const char *szDID;
	const char *szSvr;
	const char *szUsr;
	const char *szPwd;

	devClass = env->FindClass("ilnk/lib/bean/P2pNodeExt");
	if (devClass==0){
		Log2("cant not get devClass!!!!");
		return -1;
		}
	
	p2pID=env->GetFieldID(devClass,"sID","Ljava/lang/String;");
	svrStr=env->GetFieldID(devClass,"sServer","Ljava/lang/String;");	
	usr=env->GetFieldID(devClass,"sUsr","Ljava/lang/String;");	
	pwd=env->GetFieldID(devClass,"sPwd","Ljava/lang/String;");
	
	jszDID = (jstring)env->GetObjectField(devObj ,p2pID);								
	jSvr = (jstring)env->GetObjectField(devObj ,svrStr);								
	jUsr = (jstring)env->GetObjectField(devObj ,usr);								
	jPwd = (jstring)env->GetObjectField(devObj,pwd);	
	
	szDID = env->GetStringUTFChars(jszDID ,NULL);
	szSvr = env->GetStringUTFChars(jSvr ,NULL);
	szUsr = env->GetStringUTFChars(jUsr ,NULL);
	szPwd = env->GetStringUTFChars(jPwd ,NULL);
	
    if(szDID != NULL){        
        strncpy(devBase->node.sID,szDID,24);
    	}
    if(szSvr != NULL){        
        strncpy(devBase->node.sServer,szSvr,512);
    	}
    if(szUsr != NULL){        
        strncpy(devBase->user.name,szUsr,32);
    	}
    if(szPwd != NULL){        
        strncpy(devBase->user.pass,szPwd,128);
    	}

    env->ReleaseStringUTFChars(jszDID, szDID);    
	env->ReleaseStringUTFChars(jSvr, szSvr);
	env->ReleaseStringUTFChars(jUsr, szUsr);
	env->ReleaseStringUTFChars(jPwd, szPwd);

	return 0;
	}

int parseOprPolicy(JNIEnv *env,jobject cmdParam,sysOprPolicy_t_EX *oprPolicy){
	int nRet=0;
	jclass cmdParamClass;
	
	cmdParamClass= env->FindClass("ilnk/lib/bean/SysOprBean");
	if (cmdParamClass==0){
		Log2("cant not get cmdParamClass!!!!");
		return -1;
		}
	
	//jfieldID adcSelect=env->GetFieldID(cmdParamClass,"adcSelect","I");
	jfieldID adcChkInterval=env->GetFieldID(cmdParamClass,"adcChkInterval","I");
	//jfieldID gpioSelect=env->GetFieldID(cmdParamClass,"gpioSelect","I");
	jfieldID gpioChkInterval=env->GetFieldID(cmdParamClass,"gpioChkInterval","I");
	jfieldID sysRunTime=env->GetFieldID(cmdParamClass,"sysRunTime","I");
	jfieldID rcdPicEnable=env->GetFieldID(cmdParamClass,"rcdPicEnable","I");
	jfieldID rcdPicSize=env->GetFieldID(cmdParamClass,"rcdPicSize","I");
	jfieldID rcdPicInterval=env->GetFieldID(cmdParamClass,"rcdPicInterval","I");
	jfieldID rcdAvEnable=env->GetFieldID(cmdParamClass,"rcdAvEnable","I");
	jfieldID rcdAvSize=env->GetFieldID(cmdParamClass,"rcdAvSize","I");
	jfieldID rcdAvTime=env->GetFieldID(cmdParamClass,"rcdAvTime","I");
	jfieldID alarmEnable=env->GetFieldID(cmdParamClass,"alarmEnable","I");
	jfieldID wifiEnable=env->GetFieldID(cmdParamClass,"wifiEnable","I");
	jfieldID pushEnable=env->GetFieldID(cmdParamClass,"pushEnable","I");
	jfieldID osdEnable=env->GetFieldID(cmdParamClass,"osdEnable","I");
	jfieldID powerMgrEnable=env->GetFieldID(cmdParamClass,"powerMgrEnable","I");
	jfieldID powerMgrCountDown=env->GetFieldID(cmdParamClass,"powerMgrCountDown","I");
	jfieldID powerMgrSchedule=env->GetFieldID(cmdParamClass,"powerMgrSchedule","I");
	//jint jadcSelect=env->GetIntField(cmdParam, adcSelect);
	jint jadcChkInterval=env->GetIntField(cmdParam, adcChkInterval);
	//jint jgpioSelect=env->GetIntField(cmdParam, gpioSelect);
	jint jgpioChkInterval=env->GetIntField((jobject)cmdParam, gpioChkInterval);
	jint jsysRunTime=env->GetIntField((jobject)cmdParam, sysRunTime);
	jint jrcdPicEnable=env->GetIntField((jobject)cmdParam, rcdPicEnable);
	jint jrcdPicSize=env->GetIntField((jobject)cmdParam, rcdPicSize);
	jint jrcdPicInterval=env->GetIntField((jobject)cmdParam, rcdPicInterval);
	jint jrcdAvEnable=env->GetIntField((jobject)cmdParam, rcdAvEnable);
	jint jrcdAvSize=env->GetIntField((jobject)cmdParam, rcdAvSize);
	jint jrcdAvTime=env->GetIntField((jobject)cmdParam, rcdAvTime);
	jint jalarmEnable=env->GetIntField((jobject)cmdParam, alarmEnable);
	jint jwifiEnable=env->GetIntField((jobject)cmdParam, wifiEnable);
	jint josdEnable=env->GetIntField((jobject)cmdParam, osdEnable);
	jint jpushEnable=env->GetIntField((jobject)cmdParam, pushEnable);
	jint jpowerMgrEnable=env->GetIntField((jobject)cmdParam, powerMgrEnable);
	jint jpowerMgrCountDown=env->GetIntField((jobject)cmdParam, powerMgrCountDown);
	jint jpowerMgrSchedule=env->GetIntField((jobject)cmdParam, powerMgrSchedule);
	
	oprPolicy->adcChkInterval=(int)jadcChkInterval;
	//oprPolicy->adcSelect=(int)jadcSelect;
	oprPolicy->gpioChkInterval=(int)jgpioChkInterval;
	//oprPolicy->gpioSelect=(int)jgpioSelect;
	oprPolicy->powerMgrCountDown=(int)jpowerMgrCountDown;
	oprPolicy->powerMgrEnable=(int)jpowerMgrEnable;
	oprPolicy->powerMgrSchedule=(int)jpowerMgrSchedule;
	oprPolicy->pushEnable=(int)jpushEnable;
	oprPolicy->alarmEnable=(int)jalarmEnable;
	oprPolicy->wifiEnable=(int)jwifiEnable;
	oprPolicy->osdEnable=(int)josdEnable;
	oprPolicy->rcdAvEnable=(int)jrcdAvEnable;
	oprPolicy->rcdAvSize=(int)jrcdAvSize;
	oprPolicy->rcdAvTime=(int)jrcdAvTime;
	oprPolicy->rcdPicEnable=(int)jrcdPicEnable;
	oprPolicy->rcdPicInterval=(int)jrcdPicInterval;
	oprPolicy->rcdPicSize=(int)jrcdPicSize;
	oprPolicy->sysRunTime=(int)jsysRunTime;
	
	return nRet;
	}

int parseDatetime(JNIEnv *env,jobject cmdParam,datetimeParam_t *datetime){
	int nRet=0;
	jclass cmdParamClass;
	
	cmdParamClass= env->FindClass("ilnk/lib/bean/DateBean");
	if (cmdParamClass==0){
		Log2("cant not get cmdParamClass!!!!");
		return -1;
		}
	
	jfieldID ntp_ser=env->GetFieldID(cmdParamClass,"ntp_ser","Ljava/lang/String;");
	jfieldID ntp_enable=env->GetFieldID(cmdParamClass,"ntp_enable","I");
	jfieldID tz=env->GetFieldID(cmdParamClass,"tz","I");
	jfieldID now=env->GetFieldID(cmdParamClass,"now","I");
	
	jstring jntp_ser = (jstring)env->GetObjectField((jobject)cmdParam , ntp_ser);								
	const char * szntp_ser = env->GetStringUTFChars(jntp_ser ,NULL);
	jint jntp_enable=env->GetIntField((jobject)cmdParam, ntp_enable);
	jint jtz=env->GetIntField((jobject)cmdParam, tz);
	jint jnow=env->GetIntField((jobject)cmdParam, now);
	
	datetime->now=(int)jnow;
	datetime->tz=(int)jtz;
	datetime->ntp_enable=(int)jntp_enable;
	if (szntp_ser!=NULL){
		strncpy(datetime->ntp_svr,(char *)szntp_ser,64);
		}
		
	env->ReleaseStringUTFChars(jntp_ser, szntp_ser);

	return nRet;
	}

int parseAdc(JNIEnv *env,jobject cmdParam,adc_t *strAdc){
	int nRet=0;
	jclass cmdParamClass;
	
	cmdParamClass= env->FindClass("ilnk/lib/bean/AdcValueBean");
	if (cmdParamClass==0){
		Log2("cant not get cmdParamClass!!!!");
		return -1;
		}
	
	jfieldID adcSelect=env->GetFieldID(cmdParamClass,"adcSelect","I");
	int _adcSelect=(int)env->GetIntField((jobject)cmdParam, adcSelect);

	strAdc->adcSelect=_adcSelect;

	return nRet;	
	}

int parseGpioValue(JNIEnv *env,jobject cmdParam,gpio_t *strGpio){
	int nRet=0;
	jclass cmdParamClass;
	
	cmdParamClass= env->FindClass("ilnk/lib/bean/GpioValueBean");
	if (cmdParamClass==0){
		Log2("cant not get cmdParamClass!!!!");
		return -1;
		}

	jfieldID gpioSelect=env->GetFieldID(cmdParamClass,"gpioSelect","I");
	jfieldID gpioValue=env->GetFieldID(cmdParamClass,"gpioValue","I");
	int _gpioSelect=(int)env->GetIntField((jobject)cmdParam, gpioSelect);
	int _gpioValue=(int)env->GetIntField((jobject)cmdParam, gpioValue);

	strGpio->gpioSelect=_gpioSelect;
	strGpio->gpioValue=_gpioValue;

	return nRet;	
	}

int parseGpioAction(JNIEnv *env,jobject cmdParam,gpioAction_t *strGpioAction){
	int nRet=0;
	jclass cmdParamClass;
	
	cmdParamClass= env->FindClass("ilnk/lib/bean/GpioActionBean");
	if (cmdParamClass==0){
		Log2("cant not get cmdParamClass!!!!");
		return -1;
		}

	jfieldID gpioSelect=env->GetFieldID(cmdParamClass,"gpioSelect","I");
	jfieldID gpioAction=env->GetFieldID(cmdParamClass,"gpioAction","I");
	jfieldID gpioValue=env->GetFieldID(cmdParamClass,"gpioTiger","I");
	int _gpioSelect=(int)env->GetIntField((jobject)cmdParam, gpioSelect);
	int _gpioValue=(int)env->GetIntField((jobject)cmdParam, gpioValue);
	int _gpioAction=(int)env->GetIntField((jobject)cmdParam, gpioAction);

	strGpioAction->gpioSelect=_gpioSelect;
	strGpioAction->gpioCnf.action=(gpioAction_e)_gpioAction;
	strGpioAction->gpioCnf.value=_gpioValue;
	Log2("strGpioAction:select=%d,action=%d,value=%d!!!!",
		strGpioAction->gpioSelect,strGpioAction->gpioCnf.action,strGpioAction->gpioCnf.value);

	return nRet;	
	}

int parseSerialSetting(JNIEnv *env,jobject cmdParam,serialParam_t *serialSetting){
	int nRet=0;
	jclass cmdParamClass;
	
	cmdParamClass= env->FindClass("ilnk/lib/bean/SerialSettingBean");
	if (cmdParamClass==0){
		Log2("cant not get cmdParamClass!!!!");
		return -1;
		}

	jfieldID dataBit=env->GetFieldID(cmdParamClass,"dataBit","I");
	jfieldID dwBaudRate=env->GetFieldID(cmdParamClass,"dwBaudRate","I");
	jfieldID flowCtrl=env->GetFieldID(cmdParamClass,"flowCtrl","I");
	jfieldID parity=env->GetFieldID(cmdParamClass,"parity","I");
	jfieldID stopBit=env->GetFieldID(cmdParamClass,"stopBit","I");
	int _dataBit=(int)env->GetIntField((jobject)cmdParam, dataBit);
	int _dwBaudRate=(int)env->GetIntField((jobject)cmdParam, dwBaudRate);
	int _flowCtrl=(int)env->GetIntField((jobject)cmdParam, flowCtrl);
	int _parity=(int)env->GetIntField((jobject)cmdParam, parity);
	int _stopBit=(int)env->GetIntField((jobject)cmdParam, stopBit);

	serialSetting->dataBit=_dataBit;
	serialSetting->dwBaudRate=_dwBaudRate;
	serialSetting->flowCtrl=_flowCtrl;
	serialSetting->parity=_parity;
	serialSetting->stopBit=_stopBit;

	return nRet;	
	}


int parseSerialData(JNIEnv *env,jobject cmdParam,serialData_t *serialData){
	//int nRet=0;
	jclass cmdParamClass;
	
	cmdParamClass= env->FindClass("ilnk/lib/bean/SerialDataBean");
	if (cmdParamClass==0){
		Log2("cant not get cmdParamClass!!!!");
		return -1;
		}

	jfieldID data = env->GetFieldID(cmdParamClass, "data", "[B");
	jfieldID len=env->GetFieldID(cmdParamClass,"len","I");
	int _len=(int)env->GetIntField((jobject)cmdParam, len);
	jbyteArray jDataArr = (jbyteArray) env->GetObjectField(cmdParam, data); 
	jbyte *jb= env->GetByteArrayElements(jDataArr, 0); 
	
	serialData->len=_len;
	strncpy(serialData->data,(char *)jb,_len);
	Log2("serial: len=%d,data=%s\n",serialData->len,serialData->data);
	
	env->ReleaseByteArrayElements(jDataArr,jb,0);
	
	return _len+4;	
	}

int parseWifiSetting(JNIEnv *env,jobject cmdParam,wifiParam_t *wifiParams){
	int nRet=0;
	jclass cmdParamClass;
	
	cmdParamClass= env->FindClass("ilnk/lib/bean/WifiSettingBean");
	if (cmdParamClass==0){
		Log2("cant not get cmdParamClass!!!!");
		return -1;
		}
	
	jfieldID ssid=env->GetFieldID(cmdParamClass,"ssid","Ljava/lang/String;");
	jfieldID wpa_psk=env->GetFieldID(cmdParamClass,"psk","Ljava/lang/String;");
	jfieldID enable=env->GetFieldID(cmdParamClass,"enable","I");
	jfieldID channel=env->GetFieldID(cmdParamClass,"channel","I");
	jfieldID mode=env->GetFieldID(cmdParamClass,"mode","I");
	jfieldID authtype=env->GetFieldID(cmdParamClass,"authtype","I");
	jfieldID dhcp=env->GetFieldID(cmdParamClass,"dhcp","I");
	
	
	//added by 20160728 
	jfieldID ip=env->GetFieldID(cmdParamClass,"ip","Ljava/lang/String;");
	jfieldID mask=env->GetFieldID(cmdParamClass,"mask","Ljava/lang/String;");
	jfieldID gw=env->GetFieldID(cmdParamClass,"gw","Ljava/lang/String;");
	jfieldID dns1=env->GetFieldID(cmdParamClass,"dns1","Ljava/lang/String;");
	jfieldID dns2=env->GetFieldID(cmdParamClass,"dns2","Ljava/lang/String;");
	jstring jszIP = (jstring)env->GetObjectField((jobject)cmdParam , ip);								
	const char * szIP = env->GetStringUTFChars(jszIP ,NULL);
	jstring jszmask = (jstring)env->GetObjectField((jobject)cmdParam , mask);								
	const char * szmask = env->GetStringUTFChars(jszmask ,NULL);
	jstring jszgw = (jstring)env->GetObjectField((jobject)cmdParam , gw);								
	const char * szgw = env->GetStringUTFChars(jszgw ,NULL);
	jstring jszdns1 = (jstring)env->GetObjectField((jobject)cmdParam , dns1);								
	const char * szdns1 = env->GetStringUTFChars(jszdns1 ,NULL);
	jstring jszdns2 = (jstring)env->GetObjectField((jobject)cmdParam , dns2);								
	const char * szdns2 = env->GetStringUTFChars(jszdns2 ,NULL);
	jint jdhcp=env->GetIntField((jobject)cmdParam, dhcp);
	
	jstring jszSSID = (jstring)env->GetObjectField((jobject)cmdParam , ssid);								
	const char * szSSID = env->GetStringUTFChars(jszSSID ,NULL);
	jstring jszWpa_psk = (jstring)env->GetObjectField((jobject)cmdParam , wpa_psk); 							
	const char * szWpa_psk = env->GetStringUTFChars(jszWpa_psk ,NULL);
	jint jenable=env->GetIntField((jobject)cmdParam, enable);
	jint jchannel=env->GetIntField((jobject)cmdParam, channel);
	jint jmode=env->GetIntField((jobject)cmdParam, mode);
	jint jauthtype=env->GetIntField((jobject)cmdParam, authtype);

	
	strcpy(wifiParams->ssid, szSSID);

	strcpy(wifiParams->psk, szWpa_psk);
	wifiParams->enable = (int)jenable;
	wifiParams->channel = (int)jchannel;
	wifiParams->mode = (int)jmode;
	wifiParams->authtype = (int)jauthtype;
	wifiParams->dhcp = (int)jdhcp;

	//20160728
	strcpy(wifiParams->ip, szIP);
	strcpy(wifiParams->mask, szmask);
	strcpy(wifiParams->gw, szgw);
	strcpy(wifiParams->dns1, szdns1);
	strcpy(wifiParams->dns2, szdns2);
	
	env->ReleaseStringUTFChars(jszSSID, szSSID);
	env->ReleaseStringUTFChars(jszWpa_psk, szWpa_psk);	
	//20160728
	env->ReleaseStringUTFChars(jszIP, szIP);	
	env->ReleaseStringUTFChars(jszmask, szmask);	
	env->ReleaseStringUTFChars(jszgw, szgw);	
	env->ReleaseStringUTFChars(jszdns1, szdns1);	
	env->ReleaseStringUTFChars(jszdns2, szdns2);	

	return nRet;	
	}

int parsePushParam(JNIEnv *env,jobject cmdParam,pushParam_t *pushParameter){
	int nRet=0;
	jclass cmdParamClass;
	
	cmdParamClass= env->FindClass("ilnk/lib/bean/PushBean");
	if (cmdParamClass==0){
		Log2("cant not get cmdParamClass!!!!");
		return -1;
		}

	jfieldID deviceToken=env->GetFieldID(cmdParamClass,"deviceToken","Ljava/lang/String;");
	jfieldID appkey=env->GetFieldID(cmdParamClass,"appkey","Ljava/lang/String;");
	jfieldID masterkey=env->GetFieldID(cmdParamClass,"masterkey","Ljava/lang/String;");
	jfieldID alias=env->GetFieldID(cmdParamClass,"alias","Ljava/lang/String;");
	jfieldID apikey=env->GetFieldID(cmdParamClass,"apikey","Ljava/lang/String;");
	jfieldID ysecretkey=env->GetFieldID(cmdParamClass,"ysecretkey","Ljava/lang/String;");
	jfieldID xsecretkey=env->GetFieldID(cmdParamClass,"xsecretkey","Ljava/lang/String;");
	jfieldID channelid=env->GetFieldID(cmdParamClass,"channelid","Ljava/lang/String;");
	jfieldID setType=env->GetFieldID(cmdParamClass,"setType","I");
	jfieldID evironment=env->GetFieldID(cmdParamClass,"evironment","I");
	jfieldID pushTYpe=env->GetFieldID(cmdParamClass,"pushTYpe","I");
	jfieldID validity=env->GetFieldID(cmdParamClass,"validity","I");
	jfieldID type=env->GetFieldID(cmdParamClass,"type","I");
	jfieldID accessid=env->GetFieldID(cmdParamClass,"accessid","I");

	jstring jdeviceToken = (jstring)env->GetObjectField((jobject)cmdParam , deviceToken);								
	const char * szdeviceToken = env->GetStringUTFChars(jdeviceToken ,NULL);
	jstring jappkey = (jstring)env->GetObjectField((jobject)cmdParam , appkey); 							
	const char * szappkey = env->GetStringUTFChars(jappkey ,NULL);
	jstring jmasterkey = (jstring)env->GetObjectField((jobject)cmdParam , masterkey);								
	const char * szmasterkey = env->GetStringUTFChars(jmasterkey ,NULL);
	jstring jalias = (jstring)env->GetObjectField((jobject)cmdParam , alias);								
	const char * szalias = env->GetStringUTFChars(jalias ,NULL);
	jstring japikey = (jstring)env->GetObjectField((jobject)cmdParam , apikey); 							
	const char * szapikey = env->GetStringUTFChars(japikey ,NULL);
	jstring jysecretkey = (jstring)env->GetObjectField((jobject)cmdParam , ysecretkey); 							
	const char * szysecretkey = env->GetStringUTFChars(jysecretkey ,NULL);
	jstring jxsecretkey = (jstring)env->GetObjectField((jobject)cmdParam , xsecretkey); 							
	const char * szxsecretkey = env->GetStringUTFChars(jxsecretkey ,NULL);
	jstring jchannelid = (jstring)env->GetObjectField((jobject)cmdParam , channelid);								
	const char * szchannelid = env->GetStringUTFChars(jchannelid ,NULL);
	jint jsetType=env->GetIntField((jobject)cmdParam, setType);
	jint jevironment=env->GetIntField((jobject)cmdParam, evironment);
	jint jpushTYpe=env->GetIntField((jobject)cmdParam, pushTYpe);
	jint jvalidity=env->GetIntField((jobject)cmdParam, validity);
	jint jtype=env->GetIntField((jobject)cmdParam, type);
	jint jaccessid=env->GetIntField((jobject)cmdParam, accessid);
	
	pushParameter->SetType=(int)jsetType;	

	//pushParameter.type=(int)jtype;	
	//pushParameter.access_id=(unsigned int)jaccessid;	

	if (szdeviceToken != NULL){
		strcpy(pushParameter->device_token, szdeviceToken);
	}	
	
	pushParameter->environment=(int)jevironment; 
	pushParameter->pushType=(int)jpushTYpe; 
	pushParameter->validity=(int)jvalidity;
	
	if (pushParameter->pushType & 0x01)
	{
		Log2("Jpush regiter");
		if(szappkey != NULL){
			Log2("------APPKEY:%s", szappkey);
			strcpy(pushParameter->appkey, szappkey);
			Log2("------APPKEY:%s", pushParameter->appkey);
		}
	
		if( szmasterkey != NULL){
			strcpy(pushParameter->master, szmasterkey);
		}
	
		if(szalias != NULL){
			strcpy(pushParameter->alias, szalias);
		}
	
		pushParameter->type = (int)jtype;	
	}

	if ((pushParameter->pushType >>1)&0x01)
	{
		Log2("Bpush regiter");
		strcpy(pushParameter->apikey, szapikey);
		strcpy(pushParameter->ysecret_key, szysecretkey);
		strcpy(pushParameter->channel_id, szchannelid);
	}
	
	if ((pushParameter->pushType >>2)&0x01)
	{
		Log2("Xpush regiter");
		pushParameter->access_id = (unsigned int)jaccessid; 
	
		strcpy(pushParameter->xsecret_key, szxsecretkey);
	}	
/*
	if (szappkey != NULL){
		strcpy(pushParameter.appkey, szappkey);
	}
	if (szmasterkey != NULL){
		strcpy(pushParameter.master, szmasterkey);
	}
	if (szalias != NULL){
		strcpy(pushParameter.alias, szalias);
	}
	if (szapikey != NULL){
		strcpy(pushParameter.apikey, szapikey);
	}
	if (szysecretkey != NULL){
		strcpy(pushParameter.ysecret_key, szysecretkey);
	}
	if (szxsecretkey != NULL){
		strcpy(pushParameter.xsecret_key, szxsecretkey);
	}
	if (szchannelid != NULL){
		strcpy(pushParameter.channel_id, szchannelid);
	}
	*/

	env->ReleaseStringUTFChars(jdeviceToken, szdeviceToken);
	env->ReleaseStringUTFChars(jappkey, szappkey);
	env->ReleaseStringUTFChars(jmasterkey, szmasterkey);
	env->ReleaseStringUTFChars(jalias, szalias);
	env->ReleaseStringUTFChars(japikey, szapikey);
	env->ReleaseStringUTFChars(jysecretkey, szysecretkey);
	env->ReleaseStringUTFChars(jxsecretkey, szxsecretkey);
	env->ReleaseStringUTFChars(jchannelid, szchannelid);

	return nRet;	
	}

int parseRecNow(JNIEnv *env,jobject cmdParam,recNow_t *recNow){
	int nRet=0;
	jclass cmdParamClass;
	
	cmdParamClass= env->FindClass("ilnk/lib/bean/SdRecCtrlBean");
	if (cmdParamClass==0){
		Log2("cant not get cmdParamClass!!!!");
		return -1;
		}
	
	jfieldID bRecNow=env->GetFieldID(cmdParamClass,"bRecNow","I");
	jint jbRecNow=env->GetIntField(cmdParam, bRecNow);

	recNow->ifRecNow=(int)jbRecNow;

	return nRet;	
	}

int parsePicCap(JNIEnv *env,jobject cmdParam,picCap_t *picCap){
	int nRet=0;
	jclass cmdParamClass;
	
	cmdParamClass= env->FindClass("ilnk/lib/bean/SdPicCap");
	if (cmdParamClass==0){
		Log2("cant not get cmdParamClass!!!!");
		return -1;
		}
	
	jfieldID picSize=env->GetFieldID(cmdParamClass,"picSize","I");
	jfieldID ifSave=env->GetFieldID(cmdParamClass,"ifSave","I");
	jint jpicSize=env->GetIntField(cmdParam, picSize);
	jint jifSave=env->GetIntField(cmdParam, ifSave);

	picCap->picSize=(int)jpicSize;
	picCap->ifSave=(int)jifSave;

	return nRet;	
	}

int parseStoragePolicy(JNIEnv *env,jobject cmdParam,mStoragePolicy_t *storagePolicy){
	int nRet=0;
	jclass cmdParamClass;
	
	cmdParamClass= env->FindClass("ilnk/lib/bean/SdStoragePolicyBean");
	if (cmdParamClass==0){
		Log2("cant not get cmdParamClass!!!!");
		return -1;
		}
	
	jfieldID spMaxHour=env->GetFieldID(cmdParamClass,"spMaxHour","I");
	jfieldID spFullThreshold=env->GetFieldID(cmdParamClass,"spFullThreshold","I");
	jfieldID spRecycle=env->GetFieldID(cmdParamClass,"spRecycle","I");
	jfieldID spCleanData=env->GetFieldID(cmdParamClass,"spCleanData","I");
	jint jspMaxHour=env->GetIntField(cmdParam, spMaxHour);
	jint jspFullThreshold=env->GetIntField(cmdParam, spFullThreshold);
	jint jspRecycle=env->GetIntField(cmdParam, spRecycle);
	jint jspCleanData=env->GetIntField(cmdParam, spCleanData);

	storagePolicy->spCleanData=(int)jspCleanData;
	storagePolicy->spFullThreshold=(int)jspFullThreshold;
	storagePolicy->spMaxHour=(int)jspMaxHour;
	storagePolicy->spRecycle=(int)jspRecycle;

	return nRet;	
	}

int parseSdRecSchSet(JNIEnv *env,jobject cmdParam,sdRecSchSet_t *sdRecSchSet){
	int nRet=0;
	jclass cmdParamClass;
	
	cmdParamClass= env->FindClass("ilnk/lib/bean/SdRecScheduleBean");
	if (cmdParamClass==0){
		Log2("cant not get cmdParamClass!!!!");
		return -1;
		}
	
	jfieldID sun0=env->GetFieldID(cmdParamClass,"record_schedule_sun_0","I");
	jfieldID sun1=env->GetFieldID(cmdParamClass,"record_schedule_sun_1","I");
	jfieldID sun2=env->GetFieldID(cmdParamClass,"record_schedule_sun_2","I");
	
	jfieldID mon0=env->GetFieldID(cmdParamClass,"record_schedule_mon_0","I");
	jfieldID mon1=env->GetFieldID(cmdParamClass,"record_schedule_mon_1","I");
	jfieldID mon2=env->GetFieldID(cmdParamClass,"record_schedule_mon_2","I");

	jfieldID tue0=env->GetFieldID(cmdParamClass,"record_schedule_tue_0","I");
	jfieldID tue1=env->GetFieldID(cmdParamClass,"record_schedule_tue_1","I");
	jfieldID tue2=env->GetFieldID(cmdParamClass,"record_schedule_tue_2","I");

	jfieldID wed0=env->GetFieldID(cmdParamClass,"record_schedule_wed_0","I");
	jfieldID wed1=env->GetFieldID(cmdParamClass,"record_schedule_wed_1","I");
	jfieldID wed2=env->GetFieldID(cmdParamClass,"record_schedule_wed_2","I");

	jfieldID thu0=env->GetFieldID(cmdParamClass,"record_schedule_thu_0","I");
	jfieldID thu1=env->GetFieldID(cmdParamClass,"record_schedule_thu_1","I");
	jfieldID thu2=env->GetFieldID(cmdParamClass,"record_schedule_thu_2","I");

	jfieldID fri0=env->GetFieldID(cmdParamClass,"record_schedule_fri_0","I");
	jfieldID fri1=env->GetFieldID(cmdParamClass,"record_schedule_fri_1","I");
	jfieldID fri2=env->GetFieldID(cmdParamClass,"record_schedule_fri_2","I");

	jfieldID sat0=env->GetFieldID(cmdParamClass,"record_schedule_sat_0","I");
	jfieldID sat1=env->GetFieldID(cmdParamClass,"record_schedule_sat_1","I");
	jfieldID sat2=env->GetFieldID(cmdParamClass,"record_schedule_sat_2","I");

	jint jsun0=env->GetIntField(cmdParam, sun0);
	jint jsun1=env->GetIntField(cmdParam, sun1);
	jint jsun2=env->GetIntField(cmdParam, sun2);

	jint jmon0=env->GetIntField(cmdParam, mon0);
	jint jmon1=env->GetIntField(cmdParam, mon1);
	jint jmon2=env->GetIntField(cmdParam, mon2);

	jint jtue0=env->GetIntField(cmdParam, tue0);
	jint jtue1=env->GetIntField(cmdParam, tue1);
	jint jtue2=env->GetIntField(cmdParam, tue2);

	jint jwed0=env->GetIntField(cmdParam, wed0);
	jint jwed1=env->GetIntField(cmdParam, wed1);
	jint jwed2=env->GetIntField(cmdParam, wed2);

	jint jthu0=env->GetIntField(cmdParam, thu0);
	jint jthu1=env->GetIntField(cmdParam, thu1);
	jint jthu2=env->GetIntField(cmdParam, thu2);

	jint jfri0=env->GetIntField(cmdParam, fri0);
	jint jfri1=env->GetIntField(cmdParam, fri1);
	jint jfri2=env->GetIntField(cmdParam, fri2);

	jint jsat0=env->GetIntField(cmdParam, sat0);
	jint jsat1=env->GetIntField(cmdParam, sat1);
	jint jsat2=env->GetIntField(cmdParam, sat2);


	sdRecSchSet->record_schedule_sun_0=(int)jsun0;
	sdRecSchSet->record_schedule_sun_1=(int)jsun1;
	sdRecSchSet->record_schedule_sun_2=(int)jsun2;

	sdRecSchSet->record_schedule_mon_0=(int)jmon0;
	sdRecSchSet->record_schedule_mon_1=(int)jmon1;
	sdRecSchSet->record_schedule_mon_2=(int)jmon2;

	sdRecSchSet->record_schedule_tue_0=(int)jtue0;
	sdRecSchSet->record_schedule_tue_1=(int)jtue1;
	sdRecSchSet->record_schedule_tue_2=(int)jtue2;

	sdRecSchSet->record_schedule_wed_0=(int)jwed0;
	sdRecSchSet->record_schedule_wed_1=(int)jwed1;
	sdRecSchSet->record_schedule_wed_2=(int)jwed2;

	sdRecSchSet->record_schedule_thu_0=(int)jthu0;
	sdRecSchSet->record_schedule_thu_1=(int)jthu1;
	sdRecSchSet->record_schedule_thu_2=(int)jthu2;

	sdRecSchSet->record_schedule_fri_0=(int)jfri0;
	sdRecSchSet->record_schedule_fri_1=(int)jfri1;
	sdRecSchSet->record_schedule_fri_2=(int)jfri2;

	sdRecSchSet->record_schedule_sat_0=(int)jsat0;
	sdRecSchSet->record_schedule_sat_1=(int)jsat1;
	sdRecSchSet->record_schedule_sat_2=(int)jsat2;

	return nRet;	
	}


int parseSearchRecordFile(JNIEnv *env,jobject cmdParam,sdRecSearchCondition_t *searchRecFile){
	int nRet=0;
	jclass cmdParamClass;
	
	cmdParamClass= env->FindClass("ilnk/lib/bean/SdRecfileSearchBean");
	if (cmdParamClass==0){
		Log2("cant not get cmdParamClass!!!!");
		return -1;
		}

	jfieldID offset=env->GetFieldID(cmdParamClass,"offSet","I");
	jfieldID nmb=env->GetFieldID(cmdParamClass,"nmb","I");
	jint joffset=env->GetIntField((jobject)cmdParam, offset);
	jint jnmb=env->GetIntField((jobject)cmdParam, nmb);

	searchRecFile->startPage=(int)joffset;
	searchRecFile->pageNmb=(int)jnmb;

	return nRet;
}

int parsePassThroghString(JNIEnv *env,jobject cmdParam,passThroghString_t *passThroghString){
	int nRet=0;
	jclass cmdParamClass;
	
	cmdParamClass= env->FindClass("ilnk/lib/bean/PassThroughBean");
	if (cmdParamClass==0){
		Log2("cant not get cmdParamClass!!!!");
		return -1;
		}

	jfieldID len=env->GetFieldID(cmdParamClass,"len","I");
	jfieldID msg=env->GetFieldID(cmdParamClass,"msg","Ljava/lang/String;");
	jint jlen=env->GetIntField((jobject)cmdParam, len);
	jstring jmsg = (jstring)env->GetObjectField((jobject)cmdParam , msg);								
	const char * szmsg = env->GetStringUTFChars(jmsg ,NULL);
	passThroghString->strLength=(int)jlen;
	if (NULL!=szmsg){
		strncpy(passThroghString->strContent,szmsg,1024);
		}
	else{
		nRet=JNI_ERR_LOCAL_INVALID_PARAM;
		}
	if (passThroghString->strLength>1024)nRet=JNI_ERR_LOCAL_INVALID_PARAM;
	
	env->ReleaseStringUTFChars(jmsg, szmsg);
	return nRet;
}


int parseCameraCtrl(JNIEnv *env,jobject cmdParam,camCtrl_t_EX *camCtrl){
	int nRet=0;
	jclass cmdParamClass;
	
	cmdParamClass= env->FindClass("ilnk/lib/bean/CameraCtrlBean");
	if (cmdParamClass==0){
		Log2("cant not get cmdParamClass!!!!");
		return -1;
		}
	
	jfieldID paramType=env->GetFieldID(cmdParamClass,"paramType","I");
	jfieldID paramValue=env->GetFieldID(cmdParamClass,"paramValue","I");
	jint jparamType=env->GetIntField(cmdParam, paramType);
	jint jparamValue=env->GetIntField(cmdParam, paramValue);

	camCtrl->param=(int)jparamType;
	camCtrl->value=(int)jparamValue;

	return nRet;	
	}

int parsePtzParam(JNIEnv *env,jobject cmdParam,ptzParamSet_t *ptzCtrl){
	int nRet=0;
	jclass cmdParamClass;

	if (cmdParam==NULL)return 0;
	
	cmdParamClass= env->FindClass("ilnk/lib/bean/PtzCtrlBean");
	if (cmdParamClass==0){
		Log2("cant not get cmdParamClass!!!!");
		return -1;
		}
	
	jfieldID paramType=env->GetFieldID(cmdParamClass,"param","I");
	jfieldID paramValue=env->GetFieldID(cmdParamClass,"value","I");
	jfieldID paramStep=env->GetFieldID(cmdParamClass,"step","I");
	
	jint jparamType=env->GetIntField(cmdParam, paramType);
	jint jparamValue=env->GetIntField(cmdParam, paramValue);
	jint jparamStep=env->GetIntField(cmdParam, paramStep);

	ptzCtrl->param=(int)jparamType;
	ptzCtrl->value=(int)jparamValue;
	ptzCtrl->step=(int)jparamStep;

	return nRet;	
	}


int parseAudioParamSet(JNIEnv *env,jobject cmdParam,audioParamSet_t *audioParamSet){
	int nRet=0;
	jclass cmdParamClass;

	if (cmdParam==NULL)return 0;
	
	cmdParamClass= env->FindClass("ilnk/lib/bean/DevAudioSetBean");
	if (cmdParamClass==0){
		Log2("cant not get cmdParamClass!!!!");
		return -1;
		}
	
	jfieldID paramType=env->GetFieldID(cmdParamClass,"param","I");
	jfieldID paramValue=env->GetFieldID(cmdParamClass,"value","I");
	jint jparamType=env->GetIntField(cmdParam, paramType);
	jint jparamValue=env->GetIntField(cmdParam, paramValue);

	audioParamSet->paramType=(int)jparamType;
	audioParamSet->u.OutputGain=(int)jparamValue;

	return nRet;	
	}

int parseFileInfo(JNIEnv *env,jobject cmdParam,fileTransParam_t *fileTransParam){
	int nRet=0;
	jclass cmdParamClass;
	jstring jfilename =NULL;
	jint joffset=0;
	
	cmdParamClass= env->FindClass("ilnk/lib/bean/FileTransferBean");
	if (cmdParamClass==0){
		Log2("cant not get cmdParamClass!!!!");
		return -1;
		}

	jfieldID filename=env->GetFieldID(cmdParamClass,"filename","Ljava/lang/String;");
	jfieldID offset=env->GetFieldID(cmdParamClass,"offset","I");

	if (NULL!=cmdParam){
		jfilename = (jstring)env->GetObjectField((jobject)cmdParam , filename); 							
		const char * szfilename = env->GetStringUTFChars(jfilename ,NULL);
		joffset=env->GetIntField((jobject)cmdParam, offset);

		fileTransParam->offset=(int)joffset;
		Log3("-------offset--------->[%d] \n",fileTransParam->offset);
		if (NULL!=szfilename){
			strncpy(fileTransParam->filename,szfilename,128);
			Log2("get file:%s\n\r",szfilename);
			}
		nRet=sizeof(fileTransParam_t);	
		env->ReleaseStringUTFChars(jfilename, szfilename);
	}else{
		Log3("cmdParam is NULL\n\r");
		nRet=0;
	}
	return nRet;	
	}
#endif


