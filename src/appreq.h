#ifndef _APP_REQ_H_
#define _APP_REQ_H_

#ifdef TUTK_PPPP
#include "IOTCAPIs.h"
#include "AVAPIs.h"
#include "AVFRAMEINFO.h"
#include "AVIOCTRLDEFs.h"
#else
#include "PPPP_API.h"
#include "iLnkDefine.h"
#include "iLnkRingBuf.h"
#include "PPPP_Error.h"
#endif


typedef struct APP_BIN_DATA{
	int		lens;
	char 	d[1024];
}APP_BIN_DATA,*PAPP_BIN_DATA;

typedef int (*CMD_DO)(
	int				Idx,
	int				MsgType,
	const char *	Cgi,
	void *			Params
);

typedef struct {
	int		CmdType;
	CMD_DO	CmdCall;
}APP_CMD_CALL;

typedef struct {
    int     Magic;
	int		AppType;
	int		CgiLens;
	char	CgiData[0];
}APP_CMD_HEAD;

char * ResolveData(
	char *			To,
	const char *	From,
	const char *	Begin,
	const char *	End
);

int SendCmds(
	int 			Idx,
	int				MsgType,
	const char *	Cgi,
	void *			Params
);
#endif
