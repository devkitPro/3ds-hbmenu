#include "netloader.h"

#include <malloc.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <errno.h>
#include <fcntl.h>

#include <zlib.h>

#define ZLIB_CHUNK (16 * 1024)

static int listenfd = -1;
static int datafd   = -1;
static int udpfd    = -1;
static volatile size_t filelen, filetotal;
static volatile bool wantExit = false;

static void netloaderError(const char* func, int err);

static bool set_socket_nonblocking(int sock)
{
	int flags = fcntl(sock, F_GETFL);
	if (flags == -1) return false;
	return fcntl(sock, F_SETFL, flags | O_NONBLOCK) == 0;
}

static int recvall(int sock, void* buffer, int size, int flags)
{
	int len, sizeleft = size;

	while (sizeleft)
	{
		len = recv(sock, buffer, sizeleft, flags);
		if (!len)
		{
			size = 0;
			break;
		} else if (len < 0)
		{
			if (errno != EAGAIN && errno != EWOULDBLOCK)
			{
				netloaderError("recv", errno);
				break;
			}
		} else
		{
			sizeleft -= len;
			buffer += len;
		}
	}
	return size;
}

static bool netloaderInit(void)
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

static bool netloaderActivate(void)
{
	struct sockaddr_in serv_addr;
	// create udp socket for broadcast ping
	udpfd = socket(AF_INET, SOCK_DGRAM, 0);
	if (udpfd < 0)
	{
		netloaderError("udp socket", errno);
		return false;
	}

	memset(&serv_addr, '0', sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	serv_addr.sin_port = htons(NETLOADER_PORT);

	if (bind(udpfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0)
	{
		netloaderError("bind udp socket", errno);
		return false;
	}

	if (!set_socket_nonblocking(udpfd))
	{
		netloaderError("listen fcntl", errno);
		return false;
	}

	// create listening socket on all addresses on NETLOADER_PORT

	listenfd = socket(AF_INET, SOCK_STREAM, 0);
	if (listenfd < 0)
	{
		netloaderError("socket", errno);
		return false;
	}

	int rc = bind(listenfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr));
	if (rc != 0)
	{
		netloaderError("bind", errno);
		return false;
	}

	if (!set_socket_nonblocking(listenfd))
	{
		netloaderError("listen fcntl", errno);
		return false;
	}

	rc = listen(listenfd, 10);
	if(rc != 0)
	{
		netloaderError("listen", errno);
		return false;
	}

	return true;
}

static void netloaderDeactivate(void)
{
	if (listenfd >= 0)
	{
		close(listenfd);
		listenfd = -1;
	}

	if (datafd >= 0)
	{
		close(datafd);
		datafd = -1;
	}

	if (udpfd >= 0)
	{
		close(udpfd);
		udpfd = -1;
	}

	socExit();
}

void netloaderError(const char* func, int err)
{
	netloaderDeactivate();
	if (uiGetStateInfo()->update == netloaderUpdate)
		uiExitState();
	errorScreen(textGetString(StrId_NetLoader), textGetString(StrId_NetLoaderError), func, err);
}

static int decompress(int sock, FILE* fh, size_t filesize)
{
	static unsigned char in[ZLIB_CHUNK];
	static unsigned char out[ZLIB_CHUNK];

	int ret;
	unsigned have;
	z_stream strm;
	size_t chunksize;

	/* allocate inflate state */
	strm.zalloc = Z_NULL;
	strm.zfree = Z_NULL;
	strm.opaque = Z_NULL;
	strm.avail_in = 0;
	strm.next_in = Z_NULL;
	ret = inflateInit(&strm);
	if (ret != Z_OK)
	{
		netloaderError("inflateInit", ret);
		return ret;
	}

	size_t total = 0;
	// decompress until deflate stream ends or end of file
	do
	{
		int len = recvall(sock, &chunksize, 4, 0);

		if (len != 4)
		{
			inflateEnd(&strm);
			netloaderError("chunksize", len);
			return Z_DATA_ERROR;
		}

		strm.avail_in = recvall(sock,in,chunksize,0);

		if (strm.avail_in == 0)
		{
			inflateEnd(&strm);
			netloaderError("closed", 0);
			return Z_DATA_ERROR;
		}

		strm.next_in = in;

		// run inflate() on input until output buffer not full
		do
		{
			strm.avail_out = ZLIB_CHUNK;
			strm.next_out = out;
			ret = inflate(&strm, Z_NO_FLUSH);

			switch (ret)
			{
				case Z_NEED_DICT:
					ret = Z_DATA_ERROR; // and fall through
				case Z_DATA_ERROR:
				case Z_MEM_ERROR:
				case Z_STREAM_ERROR:
					inflateEnd(&strm);
					netloaderError("inflate", ret);
					return ret;
			}

			have = ZLIB_CHUNK - strm.avail_out;

			if (fwrite(out, 1, have, fh) != have || ferror(fh))
			{
				inflateEnd(&strm);
				netloaderError("fwrite",0);
				return Z_ERRNO;
			}

			total += have;
			filetotal = total;
			//sprintf(progress,"%zu (%d%%)",total, (100 * total) / filesize);
			//netloader_draw_progress();
		} while (strm.avail_out == 0);

		// done when inflate() says it's done
	} while (ret != Z_STREAM_END);

	// clean up and return
	inflateEnd(&strm);
	return ret == Z_STREAM_END ? Z_OK : Z_DATA_ERROR;
}

void netloaderTask(void* arg)
{
	char recvbuf[256];
	wantExit = false;
	filelen = 0;
	filetotal = 0;

	if (!netloaderInit())
	{
		errorScreen(textGetString(StrId_NetLoader), textGetString(StrId_NetLoaderUnavailable));
		return;
	}
	if (!netloaderActivate())
		return;

	uiEnterState(UI_STATE_NETLOADER);

	while (datafd < 0)
	{
		if (wantExit)
		{
			netloaderDeactivate();
			uiExitState();
			return;
		}

		struct sockaddr_in sa_udp_remote;
		socklen_t fromlen = sizeof(sa_udp_remote);

		int len = recvfrom(udpfd, recvbuf, sizeof(recvbuf), 0, (struct sockaddr*) &sa_udp_remote, &fromlen);
		if (len != -1 && strncmp(recvbuf, "3dsboot", 7) == 0)
		{
			sa_udp_remote.sin_family = AF_INET;
			sa_udp_remote.sin_port = htons(17491);
			sendto(udpfd, "boot3ds", 7, 0, (struct sockaddr*) &sa_udp_remote,sizeof(sa_udp_remote));
		}

		datafd = accept(listenfd, NULL, NULL);
		if (datafd < 0)
		{
			if (errno != -EWOULDBLOCK && errno != EWOULDBLOCK)
			{
				netloaderError("accept", errno);
				return;
			}
		} else
		{
			close(listenfd);
			listenfd = -1;
		}

		svcSleepThread(16666666ULL);
	}

	int namelen;
	int len = recvall(datafd, &namelen, 4, 0);
	if (len != 4 || namelen >= (sizeof(recvbuf)+1))
	{
		netloaderError("namelen", errno);
		return;
	}

	len = recvall(datafd, recvbuf, namelen, 0);
	if (len != namelen)
	{
		netloaderError("name", errno);
		return;
	}

	recvbuf[namelen] = 0;
	len = recvall(datafd, (int*)&filelen, 4, 0);
	if (len != 4)
	{
		netloaderError("filelen", errno);
		return;
	}

	int response = 0;

	static menuEntry_s me;
	menuEntryInit(&me, ENTRY_TYPE_FILE);
	strncpy(me.path, "sdmc:/3ds/", sizeof(me.path)-1);
	strncat(me.path, recvbuf, sizeof(me.path)-1);
	me.path[sizeof(me.path)-1] = 0;

	FILE* outf = fopen(me.path, "wb");
	if (!outf)
		response = -1;
	send(datafd, &response, sizeof(response), 0);

	if (response < 0)
	{
		netloaderError("fopen", errno);
		return;
	}

	static char fbuf[64*1024];
	setvbuf(outf, fbuf, _IOFBF, sizeof(fbuf));
	len = decompress(datafd, outf, filelen);
	fclose(outf);

	if (len != Z_OK)
		return;

	send(datafd, &response, sizeof(response), 0);

	int cmdlen;
	len = recvall(datafd, &cmdlen, 4, 0);
	if (len == 4 && cmdlen <= sizeof(fbuf))
	{
		len = recvall(datafd, fbuf, cmdlen, 0);
		if (len == cmdlen)
		{
			argData_s* ad = &me.args;
			ad->buf[0] = 0;
			ad->dst = (char*)&ad->buf[1];

			char* ptr = fbuf;
			char* ptrend = fbuf + cmdlen;
			while (ptr < ptrend)
				ptr += launchAddArg(ad, ptr);
		}
	}

	netloaderDeactivate();
	uiExitState();
	launchMenuEntry(&me);
}

void netloaderUpdate(void)
{
	if (wantExit || datafd >= 0) return;

	if (hidKeysDown() & KEY_B)
		wantExit = true;
}

void netloaderDrawBot(void)
{
	drawingSetMode(DRAW_MODE_DRAWING);
	drawingSetZ(0.4f);

	drawingWithColor(0x80FFFFFF);
	drawingDrawQuad(0.0f, 60.0f, 320.0f, 120.0f);
	drawingSubmitPrim(GPU_TRIANGLE_STRIP, 4);

	textSetColor(0xFF545454);
	textDrawInBox(textGetString(StrId_NetLoader), 0, 0.75f, 0.75f, 60.0f+25.0f, 8.0f, 320-8.0f);

	char buf[256];
	const char* text = buf;
	u32 ip = gethostid();

	if (datafd < 0)
		snprintf(buf, sizeof(buf), textGetString(StrId_NetLoaderActive), ip&0xFF, (ip>>8)&0xFF, (ip>>16)&0xFF, (ip>>24)&0xFF, NETLOADER_PORT);
	else
		snprintf(buf, sizeof(buf), textGetString(StrId_NetLoaderTransferring), filetotal/1024, filelen/1024);

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
