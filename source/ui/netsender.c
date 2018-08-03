#include "netsender.h"

#include <malloc.h>
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
#define RECEIVER_IP_LENGTH 15
static char receiverIp[RECEIVER_IP_LENGTH+1];

static void netsenderError(const char* func, int err);

static bool netsenderInit(void)
{
	static void* SOC_buffer;

	if (!SOC_buffer)
	{
		SOC_buffer = memalign(0x1000, 0x100000);
		if (!SOC_buffer)
			return false;
	}

	Result ret = socInit(SOC_buffer, 0x100000);
	if (R_FAILED(ret))
	{
		socExit();
		return false;
	}
	return true;
}

static void netsenderDeactivate(void)
{
	socExit();
}

void netsenderError(const char* func, int err)
{
	netsenderDeactivate();
	if (uiGetStateInfo()->update == netsenderUpdate)
		uiExitState();
	errorScreen(textGetString(StrId_NetSender), textGetString(StrId_NetSenderError), func, err);
}

int sendData(int sock, int sendsize, char *buffer)
{
	while(sendsize)
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
				perror(NULL);
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
				perror(NULL);
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

	/* allocate deflate state */
	strm.zalloc = Z_NULL;
	strm.zfree = Z_NULL;
	strm.opaque = Z_NULL;
	ret = deflateInit(&strm, Z_DEFAULT_COMPRESSION);
	if (ret != Z_OK) return;

	datafd = socket(AF_INET,SOCK_STREAM,0);
	if (datafd < 0)
	{
		return;
	}

	struct sockaddr_in s;
	memset(&s, '\0', sizeof(struct sockaddr_in));
	s.sin_family = AF_INET;
	s.sin_port = htons(NETSENDER_PORT);
	s.sin_addr.s_addr = inaddr;

	if (connect(datafd, (struct sockaddr *)&s, sizeof(s)) < 0 )
	{
		struct in_addr address;
		address.s_addr = inaddr;
		fprintf(stderr,"Connection to %s failed\n",inet_ntoa(address));
		close(datafd);
		datafd = -1;
		return;
	}

	int namelen = strlen(name);

	if (sendInt32LE(datafd, namelen))
	{
		fprintf(stderr,"Failed sending filename length\n");
		close(datafd);
		datafd = -1;
		return;
	}

	if (sendData(datafd, namelen, name))
	{
		fprintf(stderr,"Failed sending filename\n");
		close(datafd);
		datafd = -1;
		return;
	}

	if (sendInt32LE(datafd, filelen))
	{
		close(datafd);
		datafd = -1;
		return;
	}

	s32 response;
	if (recvInt32LE(datafd, &response) != 0)
	{
		close(datafd);
		datafd = -1;
		return;
	}

	if (response != 0)
	{
		close(datafd);
		datafd = -1;
		return;
	}

	size_t totalsent = 0, blocks = 0;

	do {
		strm.avail_in = fread(in, 1, ZLIB_CHUNK, fh);
		filetotal += strm.avail_in;
		if (ferror(fh)) {
			(void)deflateEnd(&strm);
			return;
		}
		flush = feof(fh) ? Z_FINISH : Z_NO_FLUSH;
		strm.next_in = in;
		/* run deflate() on input until output buffer not full, finish
		   compression if all of source has been read in */
		do {
			strm.avail_out = ZLIB_CHUNK;
			strm.next_out = out;
			ret = deflate(&strm, flush);    /* no bad return value */
			have = ZLIB_CHUNK - strm.avail_out;

			if (have != 0)
			{
				if (sendInt32LE(datafd,have))
				{
					fprintf(stderr,"Failed sending chunk size\n");
					close(datafd);
					datafd = -1;
					return;
				}

				if (sendData(datafd, have, (char*)out))
				{
					fprintf(stderr,"Failed sending %s\n", name);
					(void)deflateEnd(&strm);
					close(datafd);
					datafd = -1;
					return;
				}

				totalsent += have;
				blocks++;
			}
		} while (strm.avail_out == 0);
		/* done when last data in file processed */
	} while (flush != Z_FINISH);
	(void)deflateEnd(&strm);

	if (recvInt32LE(datafd,&response)!=0)
	{
		close(datafd);
		datafd = -1;
		return;
	}

	static char cmdbuf[3072];
	memset(cmdbuf, 0, 3072);
	snprintf(&cmdbuf[4], 3072-4, "3dslink:/%s", name);
	u32 cmdlen = strlen(&cmdbuf[4])+1;

	cmdbuf[0] = cmdlen & 0xff;
	cmdbuf[1] = (cmdlen>>8) & 0xff;
	cmdbuf[2] = (cmdlen>>16) & 0xff;
	cmdbuf[3] = (cmdlen>>24) & 0xff;

	if (sendData(datafd,cmdlen+4,cmdbuf))
	{
		close(datafd);
		datafd = -1;
		return;
	}
	
	close(datafd);
	datafd = -1;
}

void netsenderTask(void* arg)
{
	wantExit = false;
	filelen = 0;
	filetotal = 0;
	dsaddr.s_addr = INADDR_NONE;

	if (!netsenderInit())
	{
		errorScreen(textGetString(StrId_NetSender), textGetString(StrId_NetSenderUnavailable));
		return;
	}

	LightEvent_Init(&event, RESET_ONESHOT);

	uiEnterState(UI_STATE_NETSENDER);

	LightEvent_Wait(&event);
	LightEvent_Clear(&event);

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
	return;
}

static bool validateIp(const char* ip, size_t len)
{
	if (len < 7) // if it's not long enough to hold 4 digits and 3 separators, it's too short.
		return false;

	struct addrinfo *info;
	if (getaddrinfo(ip, NULL, NULL, &info) == 0)
	{
		dsaddr = ((struct sockaddr_in*)info->ai_addr)->sin_addr;
		freeaddrinfo(info);
		return true;
	}
	else
	{
		return false;
	}
}

static SwkbdCallbackResult callback(void *user, const char **ppMessage, const char *text, size_t textlen)
{
	(void)user;

	bool validIp = validateIp(text, textlen);
	if (!validIp)
	{
		*ppMessage = textGetString(StrId_NetSenderInvalidIp);
		return SWKBD_CALLBACK_CONTINUE;
	}
	else
	{
		return SWKBD_CALLBACK_OK;
	}
}

void netsenderUpdate(void)
{
	if (wantExit || datafd >= 0) return;

	if (hidKeysDown() & KEY_B)
	{
		wantExit = true;
	}

	SwkbdState swkbd;
	swkbdInit(&swkbd, SWKBD_TYPE_NUMPAD, 2, RECEIVER_IP_LENGTH);
	swkbdSetNumpadKeys(&swkbd, L'.', 0);
	swkbdSetFeatures(&swkbd, SWKBD_FIXED_WIDTH);
	swkbdSetValidation(&swkbd, SWKBD_NOTEMPTY, 0, 0);
	swkbdSetInitialText(&swkbd, "000.000.000.000");
	swkbdSetFilterCallback(&swkbd, callback, NULL);
	SwkbdButton button = swkbdInputText(&swkbd, receiverIp, RECEIVER_IP_LENGTH+1);
	if(button == SWKBD_BUTTON_LEFT) // Cancel
	{
		wantExit = true;
	}
	LightEvent_Signal(&event);
}

void netsenderExit(void)
{
	wantExit = true;
}

void netsenderDrawBot(void)
{
	drawingSetMode(DRAW_MODE_DRAWING);
	drawingSetZ(0.4f);

	drawingWithColor(0x80FFFFFF);
	drawingDrawQuad(0.0f, 60.0f, 320.0f, 120.0f);
	drawingSubmitPrim(GPU_TRIANGLE_STRIP, 4);

	textSetColor(0xFF545454);
	textDrawInBox(textGetString(StrId_NetSender), 0, 0.75f, 0.75f, 60.0f+25.0f, 8.0f, 320-8.0f);

	char buf[256];
	const char* text = buf;
	u32 ip = gethostid();

	if (ip == 0)
		snprintf(buf, sizeof(buf), textGetString(StrId_NetSenderOffline));
	else
		snprintf(buf, sizeof(buf), textGetString(StrId_NetSenderTransferring), filetotal/1024, filelen/1024);

	textDraw(8.0f, 60.0f+25.0f+8.0f, 0.5f, 0.5f, false, text);

	if (datafd >= 0 && filelen)
	{
		float progress = (float)filetotal / filelen;
		float width = progress*320;

		drawingWithColor(0xC000E000);
		drawingDrawQuad(0.0f, 60.0f+120.0f-16.0f, width, 16.0f);
		drawingWithColor(0xC0C0C0C0);
		drawingDrawQuad(width, 60.0f+120.0f-16.0f, 320.0f-width, 16.0f);

		snprintf(buf, sizeof(buf), "%.02f%%", progress*100);
		textSetColor(0xFF000000);
		textDrawInBox(buf, 0, 0.5f, 0.5f, 60.0f+120.0f-3.0f, 0.0f, 320.0f);
	}
}
