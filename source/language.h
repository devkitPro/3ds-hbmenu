#pragma once
#include <3ds.h>

typedef enum
{
	StrId_Loading = 0,
	StrId_Directory,
	StrId_DefaultLongTitle,
	StrId_DefaultPublisher,
	StrId_IOError,
	StrId_CouldNotOpenFile,

	StrId_NoAppsFound_Title,
	StrId_NoAppsFound_Msg,

	StrId_Reboot,
	StrId_ReturnToHome,

	StrId_TitleSelector,
	StrId_ErrorReadingTitleMetadata,
	StrId_NoTitlesFound,
	StrId_SelectTitle,

	StrId_NoTargetTitleSupport,
	StrId_MissingTargetTitle,

	StrId_NetLoader,
	StrId_NetLoaderUnavailable,
	StrId_NetLoaderOffline,
	StrId_NetLoaderError,
	StrId_NetLoaderActive,
	StrId_NetLoaderTransferring,

	StrId_NetSender,
	StrId_NetSenderUnavailable,
	StrId_NetSenderOffline,
	StrId_NetSenderInvalidIp,
	StrId_NetSenderError,
	StrId_NetSenderActive,
	StrId_NetSenderTransferring,

	StrId_Max,
} StrId;

extern const char* const g_strings[StrId_Max][16];
