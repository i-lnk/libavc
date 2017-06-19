/****************************************************************************
* Copyright 2012 - 2015, IPCamera Tech. Co., Ltd.
* ALL rights reserved
* 文件名: RingBuf.h
* 文件描述: RTP码流循环缓冲区
*
* 当前版本:	1.1
* 作者:scott
* 创建日期: 2012-01-01
*****************************************************************************/
#ifndef __RTPBUF_H__
#define __RTPBUF_H__
/*
#include <rtthread.h>
#include <lwip/netdb.h>
#include <lwip/sockets.h> 
#include <lwip/api.h>
#include <lwip/init.h>
#include "FHAdv_Pub.h"
#include "pubdef_in.h"
#include "taskdef.h"
#include "osalBaseApi.h"
*/
#include "pthread.h"
/*
#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif 
*/
#define RINGBUF_ERR_OK		0
#define RINGBUF_ERR_BUFNULL		-1
#define RINGBUF_ERR_ONUSING		-2
#define RINGBUF_ERR_UNSYNC		-3
#define RINGBUF_ERR_NODATA		-4


#define AUDIO_STREAM				0xFFA0
#define VIDEO_H264_STREAM			0xFFB1
#define VIDEO_MJPEG_STREAM			0xFFC2
#define METADATA_STREAM				0xFFD3

/*获取nal类型*/
#define H264_Get_NalType(c)  ( (c) & 0x1F )  

/*Nal Type Define*/
#define NAL_TYPE_SLICE      1
#define NAL_TYPE_IDR        5
#define NAL_TYPE_SEI        6
#define NAL_TYPE_SPS        7
#define NAL_TYPE_PPS        8
#define NAL_TYPE_SEQ_END    9
#define NAL_TYPE_STREAM_END 10

/***************************************************************************/
#define SENDER_BUF_LENGTH			(1024*1024*4)	/*4M内存 码流1[3072]+码流2[1536]+码流3[256]+音频[256]=4864Kb/8 = 640KB, 缓存6秒 640x8=3840KB*/		
#define SENDER_INDEX_DEEP			6400			/*索引深度超过，最大值 (1024*4K/(64+32)K)*95 = 4053，实际测试结果为3000左右*/
#define TCP_SYNC_SECONDS			3				/*同步时间为3秒*/

#define  RINGBBUF_INIT_FILE			"/tmp/RingBuf"

#define SystemSleep(X) usleep(X*1000)
/*从缓冲区获取码流模式*/
typedef enum get_tcpbuf_m
{
    RNGBLOCK		= 0,
    NO_RNGBLOCK	= 1,
}GET_TCPBUF_M;

/*保存码流循环缓冲区结构*/
struct CHN_LIVE_BUF
{
   //unsigned char  *data;							/*存放码流数据的内存指针*/
   unsigned int   data_length;				/*缓冲区的长度*/
   struct packet_index     
   {
   		int onUsing;
 	 	unsigned int   record_time;     /*写入时间*/
     	unsigned int   pos;							/*存放在数组data里的首地址*/
     	unsigned int   type;						/*类型*/
     	unsigned int   pts;							/*同步时间*/
     	int			   length;						/*长度 填充数据时大于0, 初始化时等于0, 被释放时等于-1*/
		unsigned int   chno;							/*chno 主码流视频:0,子码视频:1,音频:2,报警:3,*/
   } index[SENDER_INDEX_DEEP];				/*索引数组，每个索引对应一个Frame*/
   volatile unsigned int head;				/*索引头 即最旧帧 起始位置*/
   volatile unsigned int tail;				/*索引尾 即最新帧 起始位置*/
   unsigned int current_index;				/*当前索引*/
   unsigned int current_pos;					/*当前写入标志*/
   unsigned int capability;         	/*剩余容量，初始化给与最大容量值RAWDATA_LENGTH*/
   unsigned int readers;					/*读者个数*/
   unsigned int synchronous;			/*编码重启同步源*/
};

typedef struct XqRingBuffer{
	struct CHN_LIVE_BUF bufCtrl;
	unsigned char	*bufData;
	pthread_mutex_t	bufWriteLock;
	}xqRingBuffer_t;

xqRingBuffer_t *XqRingBuf_Get(unsigned int bufSize);
void XqRingBuf_Free(xqRingBuffer_t *xqRingBuf);
int XqRingBuf_GetData(xqRingBuffer_t *xqRingBuf,int chno, int *pframe, unsigned char **buf, int *length, int *type, unsigned int *pts, int block);
int XqRingBuf_PutData(xqRingBuffer_t *xqRingBuf,int chno, char *buf, int length, unsigned int stream_type, unsigned int pts);
void XqRingBuf_ReleaseData(
	xqRingBuffer_t *xqRingBuf,
	int frameIndex
);

/*RINGBUF进程间同步接口*/
void RingBufReaderAdd(xqRingBuffer_t *xqRingBuf);
void RingBufReaderDec(xqRingBuffer_t *xqRingBuf);
void RingBufSynchronousClean(xqRingBuffer_t *xqRingBuf);
void RingBufSynchronousSet(xqRingBuffer_t *xqRingBuf);
/*
#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif
*/
#endif

