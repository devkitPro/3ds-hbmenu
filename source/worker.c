#include "worker.h"

#define WORKER_STACK_SIZE (16*1024)

static Thread s_workerThread;
static Handle s_workerEvent;

static volatile struct
{
	ThreadFunc func;
	void* data;

	bool exit;
} s_workerParam;

static void workerThreadProc(void* unused)
{
	for (;;)
	{
		svcWaitSynchronization(s_workerEvent, U64_MAX);
		svcClearEvent(s_workerEvent);

		if (s_workerParam.exit)
			break;

		s_workerParam.func(s_workerParam.data);
	}
}

void workerInit(void)
{
	Result res;

	res = svcCreateEvent(&s_workerEvent, 0);
	if (R_FAILED(res)) svcBreak(USERBREAK_PANIC);

	s_workerThread = threadCreate(workerThreadProc, NULL, WORKER_STACK_SIZE, 0x30, -2, true);
}

void workerExit(void)
{
	s_workerParam.exit = true;
	svcSignalEvent(s_workerEvent);
	threadJoin(s_workerThread, U64_MAX);
}

void workerSchedule(ThreadFunc func, void* data)
{
	uiEnterBusy();
	s_workerParam.func = func;
	s_workerParam.data = data;
	svcSignalEvent(s_workerEvent);
}
