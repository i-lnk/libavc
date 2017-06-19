/****************************************************************************
* Copyright 2012 - 2015, IPCamera Tech. Co., Ltd.
* ALL rights reserved
* �ļ���: RingBuf.h
* �ļ�����: RTP����ѭ��������
*
* ��ǰ�汾:	1.1
* ����:scott
* ��������: 2012-01-01
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

/*��ȡnal����*/
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
#define SENDER_BUF_LENGTH			(1024*1024*4)	/*4M�ڴ� ����1[3072]+����2[1536]+����3[256]+��Ƶ[256]=4864Kb/8 = 640KB, ����6�� 640x8=3840KB*/		
#define SENDER_INDEX_DEEP			6400			/*������ȳ��������ֵ (1024*4K/(64+32)K)*95 = 4053��ʵ�ʲ��Խ��Ϊ3000����*/
#define TCP_SYNC_SECONDS			3				/*ͬ��ʱ��Ϊ3��*/

#define  RINGBBUF_INIT_FILE			"/tmp/RingBuf"

#define SystemSleep(X) usleep(X*1000)
/*�ӻ�������ȡ����ģʽ*/
typedef enum get_tcpbuf_m
{
    RNGBLOCK		= 0,
    NO_RNGBLOCK	= 1,
}GET_TCPBUF_M;

/*��������ѭ���������ṹ*/
struct CHN_LIVE_BUF
{
   //unsigned char  *data;							/*����������ݵ��ڴ�ָ��*/
   unsigned int   data_length;				/*�������ĳ���*/
   struct packet_index     
   {
   		int onUsing;
 	 	unsigned int   record_time;     /*д��ʱ��*/
     	unsigned int   pos;							/*���������data����׵�ַ*/
     	unsigned int   type;						/*����*/
     	unsigned int   pts;							/*ͬ��ʱ��*/
     	int			   length;						/*���� �������ʱ����0, ��ʼ��ʱ����0, ���ͷ�ʱ����-1*/
		unsigned int   chno;							/*chno ��������Ƶ:0,������Ƶ:1,��Ƶ:2,����:3,*/
   } index[SENDER_INDEX_DEEP];				/*�������飬ÿ��������Ӧһ��Frame*/
   volatile unsigned int head;				/*����ͷ �����֡ ��ʼλ��*/
   volatile unsigned int tail;				/*����β ������֡ ��ʼλ��*/
   unsigned int current_index;				/*��ǰ����*/
   unsigned int current_pos;					/*��ǰд���־*/
   unsigned int capability;         	/*ʣ����������ʼ�������������ֵRAWDATA_LENGTH*/
   unsigned int readers;					/*���߸���*/
   unsigned int synchronous;			/*��������ͬ��Դ*/
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

/*RINGBUF���̼�ͬ���ӿ�*/
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

