#include "utility.h"
#include <stdio.h>
/*
void Utils_PrintHex(const void *_data, unsigned int size) 
	{
	unsigned int n =0;
	unsigned int i=0;
	const char *data = (const char *)_data;
	unsigned int offset = 0;
	while (offset < size) {
		printf("%0xx  ", offset);
		n = size - offset;
		if (n > 16) {
			n = 16;
		}
		//UINT32 i;
		for (i = 0; i < 16; ++i) {
			if (i == 8) {
				printf(" ");
			}
			if (offset + i < size) {
				printf("%x ", data[offset + i]);
			} else {
				printf("   ");
			}
		}
		printf(" ");
		for (i = 0; i < n; ++i) {
			if (isprint(data[offset + i])) {
				printf("%c", data[offset + i]);
			} else {
				printf(".");
			}
		}
		printf("\n");
		offset += 16;
	}
}
*/
void Utils_Cmd_SysOprPolicy_Print(sysOprPolicy_t_EX sysOprCfg){
	Log3("Utils_Cmd_SysOprPolicy_Print:\n");
	Log3("adcChkInterval:%d\n",sysOprCfg.adcChkInterval);
	Log3("gpioChkInterval:%d\n",sysOprCfg.gpioChkInterval);
	Log3("sysRunTime:%d\n",sysOprCfg.sysRunTime);
	Log3("rcdPicEnable:%d\n",sysOprCfg.rcdPicEnable);
	Log3("rcdPicSize:%d\n",sysOprCfg.rcdPicSize);
	Log3("rcdPicInterval:%d\n",sysOprCfg.rcdPicInterval);
	Log3("rcdAvEnable:%d\n",sysOprCfg.rcdAvEnable);
	Log3("rcdAvSize:%d\n",sysOprCfg.rcdAvSize);
	Log3("rcdAvTime:%d\n",sysOprCfg.rcdAvTime);
	Log3("pushEnable:%d\n",sysOprCfg.pushEnable);
	Log3("alarmEnable:%d\n",sysOprCfg.alarmEnable);
	Log3("wifiEnable:%d\n",sysOprCfg.wifiEnable);
	Log3("ircut:%d\n",sysOprCfg.osdEnable);
	Log3("powerMgrEnable:%d\n",sysOprCfg.powerMgrEnable);
	Log3("powerMgrCountDown:%d\n",sysOprCfg.powerMgrCountDown);
	Log3("reserved2:%s\n",sysOprCfg.reserved2);
	Log3("powerMgrSchedule:%d\n",sysOprCfg.powerMgrSchedule);
	}

void Utils_Cmd_SdAVRecPolicy_Print(mAVRecPolicy_t AVRecPolicy){
	Log3("Utils_Cmd_SdAVRecPolicy_Print:\n");
	Log3("rcVideoWidth:%d\n",AVRecPolicy.recConf.rcVideoWidth);
	Log3("rcVideoHeight:%d\n",AVRecPolicy.recConf.rcVideoHeight);
	Log3("rcVideoRate:%d\n",AVRecPolicy.recConf.rcVideoRate);
	Log3("rcVideoMaxBitrate:%d\n",AVRecPolicy.recConf.rcVideoMaxBitrate);
	
	Log3("rcAudioSamplerate:%d\n",AVRecPolicy.recConf.rcAudioSamplerate);
	Log3("rcAudioBitWidth:%d\n",AVRecPolicy.recConf.rcAudioBitWidth);
	Log3("rcAudioMaxBitrate:%d\n",AVRecPolicy.recConf.rcAudioMaxBitrate);
	Log3("rcAudioTrack:%d\n",AVRecPolicy.recConf.rcAudioTrack);
	
	Log3("spMaxHour:%u\n",AVRecPolicy.storagePolicy.spMaxHour);
	Log3("spFullThreshold:%u\n",AVRecPolicy.storagePolicy.spFullThreshold);
	Log3("spRecycle:%d\n",AVRecPolicy.storagePolicy.spRecycle);
	Log3("spCleanData:%d\n",AVRecPolicy.storagePolicy.spCleanData);
	//printf("spReserved:%s\n",AVRecPolicy.storagePolicy.spReserved);
	}


