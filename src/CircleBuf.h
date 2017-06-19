#ifndef _CIRCLEBUF_H_
#define _CIRCLEBUF_H_

#include "utility.h"

//��Ƶ�����У�ÿ����Ƶ���ݵ�ͷ��
struct VIDEO_BUF_HEAD
{
    unsigned int	head; 		// ͷ���������0xFF00FF 
    unsigned int	timestamp; 	// ʱ����������¼������¼���ʱ����������ʵʱ��Ƶ����Ϊ0
    unsigned int 	len;    	// ����
    unsigned int 	frametype;
	unsigned int 	streamid;
	unsigned long 	time;
	char            d[0];
};

class CCircleBuf
{    
public:
	CCircleBuf();
	~CCircleBuf();

	bool Create( int size );


	void Release();


	int Read( void* buf, int size );


	int Write( void* buf, int size );


	int GetStock();


	void Reset();


	void AllRest();


	/* ����������������ô� */
	char * ReadOneFrame( int &len );


	char * ReadOneFrame1( int &len, VIDEO_BUF_HEAD & videobufhead );


	char * ReadOneFrameX( char * pData, int DataSize );


private:
	int Read1( void* buf, int size );


protected:
	char	* m_pBuf;
	int	m_nSize;
	int	m_nStock;
	int	m_nReadPos;
	int	m_nWritePos;

	int m_nTimeout;

	COMMO_LOCK Lock;

private:

	int m_n;
};


#endif

