#include "netsender.h"

#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <errno.h>
#include <fcntl.h>
#include <netdb.h>

#include <zlib.h>

#define ZLIB_CHUNK (16 * 1024)

typedef u32 in_addr_t;

static int datafd = -1;
static struct in_addr dsaddr;
static LightEvent event;
static volatile size_t filelen, filetotal;
static volatile bool wantExit = false;
static volatile bool doneSearching = false;
#define RECEIVER_IP_LENGTH 15
static char receiverIp[RECEIVER_IP_LENGTH+1];

static void netsenderError(const char* func, int err);

static bool netsenderInit(void)
{
	return networkInit();
}

static void netsenderDeactivate(void)
{
	networkDeactivate();
}

void netsenderError(const char* func, int err)
{
	netsenderDeactivate();
	networkError(netsenderUpdate, StrId_NetSender, func, err);
}

/*---------------------------------------------------------------------------------
	Subtract the `struct timeval' values Y from X,
	storing the result in RESULT.
	Return 1 if the difference is negative, otherwise 0.

	From http://www.gnu.org/software/libtool/manual/libc/Elapsed-Time.html
---------------------------------------------------------------------------------*/
static int timeval_subtract (struct timeval *result, struct timeval *x, struct timeval *y)
{
	struct timeval tmp;
	tmp.tv_sec = y->tv_sec;
	tmp.tv_usec = y->tv_usec;

	// Perform the carry for the later subtraction by updating y.
	if (x->tv_usec < tmp.tv_usec)
	{
		int nsec = (tmp.tv_usec - x->tv_usec) / 1000000 + 1;
		tmp.tv_usec -= 1000000 * nsec;
		tmp.tv_sec += nsec;
	}

	if (x->tv_usec - tmp.tv_usec > 1000000)
	{
		int nsec = (x->tv_usec - tmp.tv_usec) / 1000000;
		tmp.tv_usec += 1000000 * nsec;
		tmp.tv_sec -= nsec;
	}

	// Compute the time remaining to wait. tv_usec is certainly positive.
	result->tv_sec = x->tv_sec - tmp.tv_sec;
	result->tv_usec = x->tv_usec - tmp.tv_usec;

	// Return 1 if result is negative.
	return x->tv_sec < tmp.tv_sec;
}

static void timeval_add (struct timeval *result, struct timeval *x, struct timeval *y)
{
	result->tv_sec = x->tv_sec + y->tv_sec;
	result->tv_usec = x->tv_usec + y->tv_usec;

	if (result->tv_usec > 1000000)
	{
		result->tv_sec += result->tv_usec / 1000000;
		result->tv_usec = result->tv_usec % 1000000;
	}
}

static struct in_addr find3DS(int retries)
{
	struct sockaddr_in s, remote, rs;
	char recvbuf[256];
	char mess[] = "3dsboot";

	int broadcastSock = socket(PF_INET, SOCK_DGRAM, 0);
	if (broadcastSock < 0)
	{
		netsenderError("broadcast socket", errno);
		remote.sin_addr.s_addr = INADDR_NONE;
		return remote.sin_addr;
	}

	memset(&s, 0, sizeof(struct sockaddr_in));
	s.sin_family = AF_INET;
	s.sin_port = htons(NETWORK_PORT);
	s.sin_addr.s_addr = INADDR_BROADCAST;

	memset(&rs, 0, sizeof(struct sockaddr_in));
	rs.sin_family = AF_INET;
	rs.sin_port = htons(NETWORK_PORT);
	rs.sin_addr.s_addr = INADDR_ANY;

	int recvSock = socket(PF_INET, SOCK_DGRAM, 0);

	if (recvSock < 0)
	{
		netsenderError("receive socket", errno);
		close(broadcastSock);
		remote.sin_addr.s_addr = INADDR_NONE;
		return remote.sin_addr;
	}

	if (bind(recvSock, (struct sockaddr*) &rs, sizeof(rs)) < 0)
	{
		netsenderError("bind receive socket", errno);
		close(broadcastSock);
		close(recvSock);
		remote.sin_addr.s_addr = INADDR_NONE;
		return remote.sin_addr;
	}

	fcntl(recvSock, F_SETFL, O_NONBLOCK);
	struct timeval wanted, now, result;

	gettimeofday(&wanted, NULL);

	int timeout = retries, len;
	while (timeout)
	{
		if (wantExit)
			break;

		gettimeofday(&now, NULL);
		if (timeval_subtract(&result,&wanted,&now))
		{
			if (sendto(broadcastSock, mess, strlen(mess), 0, (struct sockaddr *)&s, sizeof(s)) < 0)
			{
				netsenderError("sendto", errno);
				close(broadcastSock);
				close(recvSock);
				remote.sin_addr.s_addr = INADDR_NONE;
				return remote.sin_addr;
			}

			result.tv_sec=0;
			result.tv_usec=150000;
			timeval_add(&wanted,&now,&result);
			timeout--;
		}
		socklen_t socklen = sizeof(remote);
		len = recvfrom(recvSock, recvbuf, sizeof(recvbuf), 0, (struct sockaddr *)&remote, &socklen);
		if (len != -1)
		{
			if (strncmp("boot3ds",recvbuf,strlen("boot3ds")) == 0)
			{
				break;
			}
		}

		svcSleepThread(1e6);
	}

	if (timeout == 0)
		remote.sin_addr.s_addr = INADDR_NONE;

	close(broadcastSock);
	close(recvSock);
	return remote.sin_addr;
}

int sendData(int sock, int sendsize, char *buffer)
{
	while (sendsize)
	{
		int len = send(sock, buffer, sendsize, 0);
		if (len == 0) break;
		if (len != -1)
		{
			sendsize -= len;
			buffer += len;
		}
		else
		{
			if (errno != EWOULDBLOCK && errno != EAGAIN)
			{
				netsenderError("send", errno);
				break;
			}
		}
	}
	return sendsize != 0;
}

int recvData(int sock, char *buffer, int size, int flags)
{
	int len, sizeleft = size;

	while (sizeleft)
	{
		len = recv(sock, buffer, sizeleft, flags);
		if (len == 0)
		{
			size = 0;
			break;
		}
		if (len != -1)
		{
			sizeleft -=len;
			buffer +=len;
		}
		else
		{
			if (errno != EWOULDBLOCK && errno != EAGAIN)
			{
				netsenderError("recv", errno);
				break;
			}
		}
	}
	return size;
}

static int sendInt32LE(int socket, u32 size)
{
	char lenbuf[4];
	lenbuf[0] = size & 0xff;
	lenbuf[1] = (size >>  8) & 0xff;
	lenbuf[2] = (size >> 16) & 0xff;
	lenbuf[3] = (size >> 24) & 0xff;

	return sendData(socket, 4, lenbuf);
}

static int recvInt32LE(int socket, s32 *data)
{
	char intbuf[4];
	int len = recvData(socket,intbuf,4,0);

	if (len == 4)
	{
		*data = (intbuf[0] & 0xff) | (intbuf[1] <<  8) | (intbuf[2] <<  16) | (intbuf[3] <<  24);
		return 0;
	}

	return -1;
}

static void send3DSXFile(in_addr_t inaddr, char *name, FILE *fh)
{
	static unsigned char in[ZLIB_CHUNK];
	static unsigned char out[ZLIB_CHUNK];

	int ret, flush;
	unsigned have;
	z_stream strm;

	// allocate deflate state
	strm.zalloc = Z_NULL;
	strm.zfree = Z_NULL;
	strm.opaque = Z_NULL;
	ret = deflateInit(&strm, Z_DEFAULT_COMPRESSION);
	if (ret != Z_OK) return;

	datafd = socket(AF_INET,SOCK_STREAM,0);
	if (datafd < 0)
	{
		netsenderError("datafd", errno);
		return;
	}

	struct sockaddr_in s;
	memset(&s, 0, sizeof(struct sockaddr_in));
	s.sin_family = AF_INET;
	s.sin_port = htons(NETWORK_PORT);
	s.sin_addr.s_addr = inaddr;

	if (connect(datafd, (struct sockaddr *)&s, sizeof(s)) < 0 )
	{
		netsenderError("connect", errno);
		goto send3DSXFileExit;
	}

	int namelen = strlen(name);

	if (sendInt32LE(datafd, namelen))
	{
		netsenderError("filenameLen", errno);
		goto send3DSXFileExit;
	}

	if (sendData(datafd, namelen, name))
	{
		netsenderError("filename", errno);
		goto send3DSXFileExit;
	}

	if (sendInt32LE(datafd, filelen))
	{
		netsenderError("filelen", errno);
		goto send3DSXFileExit;
	}

	s32 response;
	if (recvInt32LE(datafd, &response) != 0)
	{
		netsenderError("response", 0);
		goto send3DSXFileExit;
	}

	if (response != 0)
	{
		switch (response) {
			case -1:
				netsenderError("create file", 0);
				break;
			case -2:
				netsenderError("no space", 0);
				break;
			case -3:
				netsenderError("no memory", 0);
				break;
			default:
				netsenderError("response:", response);
				break;
		}
		goto send3DSXFileExit;
	}

	size_t totalsent = 0, blocks = 0;

	do
	{
		strm.avail_in = fread(in, 1, ZLIB_CHUNK, fh);
		filetotal += strm.avail_in;
		if (ferror(fh))
		{
			netsenderError("ferror", 0);
			deflateEnd(&strm);
			goto send3DSXFileExit;
		}
		flush = feof(fh) ? Z_FINISH : Z_NO_FLUSH;
		strm.next_in = in;
		// run deflate() on input until output buffer not full, finish compression if all of source has been read in
		do
		{
			strm.avail_out = ZLIB_CHUNK;
			strm.next_out = out;
			ret = deflate(&strm, flush);    // no bad return value
			have = ZLIB_CHUNK - strm.avail_out;

			if (have != 0)
			{
				if (sendInt32LE(datafd,have))
				{
					netsenderError("chunk size", errno);
					goto send3DSXFileExit;
				}

				if (sendData(datafd, have, (char*)out))
				{
					netsenderError("have", errno);
					deflateEnd(&strm);
					goto send3DSXFileExit;
				}

				totalsent += have;
				blocks++;
			}
		} while (strm.avail_out == 0);
		// done when last data in file processed
	} while (flush != Z_FINISH);
	deflateEnd(&strm);

	if (recvInt32LE(datafd,&response)!=0)
		goto send3DSXFileExit;

	static char cmdbuf[3072];
	memset(cmdbuf, 0, 3072);
	snprintf(&cmdbuf[4], 3072-4, "3dslink:/%s", name);
	u32 cmdlen = strlen(&cmdbuf[4])+1;

	cmdbuf[0] = cmdlen & 0xff;
	cmdbuf[1] = (cmdlen>>8) & 0xff;
	cmdbuf[2] = (cmdlen>>16) & 0xff;
	cmdbuf[3] = (cmdlen>>24) & 0xff;

	if (sendData(datafd,cmdlen+4,cmdbuf))
		netsenderError("cmdbuf", errno);
	
	send3DSXFileExit:
	close(datafd);
	datafd = -1;
}

void netsenderTask(void* arg)
{
	wantExit = false;
	filelen = 0;
	filetotal = 0;
	dsaddr.s_addr = INADDR_NONE;
	doneSearching = false;

	if (!netsenderInit())
	{
		errorScreen(textGetString(StrId_NetSender), textGetString(StrId_NetSenderUnavailable));
		return;
	}

	uiEnterState(UI_STATE_NETSENDER);

	dsaddr = find3DS(10);
	if (wantExit)
	{
		netsenderDeactivate();
		uiExitState();
		return;
	}

	doneSearching = true;
	if (dsaddr.s_addr == INADDR_NONE)
	{
		LightEvent_Init(&event, RESET_ONESHOT);
		LightEvent_Wait(&event);
		LightEvent_Clear(&event);
	}

	if (wantExit || dsaddr.s_addr == INADDR_NONE)
	{
		netsenderDeactivate();
		uiExitState();
		return;
	}

	menuEntry_s* me = (menuEntry_s*)arg;
	char* filename = strdup(me->path);
	char* basename = strrchr(filename,'/') + 1;  // assume it doesnt fail

	FILE* inf = fopen(me->path, "rb");
	fseek(inf, 0, SEEK_END);
	filelen = ftell(inf);
	fseek(inf, 0, SEEK_SET);

	send3DSXFile(dsaddr.s_addr, basename, inf);

	fclose(inf);
	free(filename);

	netsenderDeactivate();
	uiExitState();
}

static bool validateIp(const char* ip, size_t len)
{
	if (len < 7) // if it's not long enough to hold 4 digits and 3 separators, it's too short.
		return false;

	struct addrinfo *info;
	if (getaddrinfo(ip, NULL, NULL, &info) == 0) // check it's a valid address first
	{
		dsaddr = ((struct sockaddr_in*)info->ai_addr)->sin_addr;
		freeaddrinfo(info);
		// this won't error, so 0.0.0.0 (the default ip) will return false and force the user to put something else
		return inet_addr(ip) != 0;
	}

	return false;
}

static SwkbdCallbackResult callback(void *user, const char **ppMessage, const char *text, size_t textlen)
{
	bool validIp = validateIp(text, textlen);
	if (!validIp)
	{
		*ppMessage = textGetString(StrId_NetSenderInvalidIp);
		return SWKBD_CALLBACK_CONTINUE;
	}

	return SWKBD_CALLBACK_OK;
}

void netsenderUpdate(void)
{
	if (wantExit || datafd >= 0) return;

	if (hidKeysDown() & KEY_B)
		wantExit = true;

	if (doneSearching && dsaddr.s_addr == INADDR_NONE)
	{
		SwkbdState swkbd;
		swkbdInit(&swkbd, SWKBD_TYPE_NUMPAD, 2, RECEIVER_IP_LENGTH);
		swkbdSetNumpadKeys(&swkbd, L'.', 0);
		swkbdSetFeatures(&swkbd, SWKBD_FIXED_WIDTH);
		swkbdSetValidation(&swkbd, SWKBD_NOTEMPTY, 0, 0);
		swkbdSetInitialText(&swkbd, "000.000.000.000");
		swkbdSetFilterCallback(&swkbd, callback, NULL);
		SwkbdButton button = swkbdInputText(&swkbd, receiverIp, RECEIVER_IP_LENGTH+1);

		if (button == SWKBD_BUTTON_LEFT) // Cancel
			wantExit = true;

		LightEvent_Signal(&event);
	}
}

void netsenderExit(void)
{
	wantExit = true;
}

void netsenderDrawBot(void)
{
	char buf[256];
	const char* text = NULL;
	if (!doneSearching)
	{
		snprintf(buf, sizeof(buf), textGetString(StrId_NetSenderActive));
		text = buf;
	}

	networkDrawBot(StrId_NetSender, text, (datafd >= 0 && filelen), filelen, filetotal);
}
