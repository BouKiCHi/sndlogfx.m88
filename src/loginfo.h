#ifndef __LOGINFO_H__
#define __LOGINFO_H__

struct LogRecordInfo {
	HANDLE	LogFileHandle;
	int		FileSerialCount;

	int		RecordingFlag;
	int		PauseFlag;

	int		RegisterOut;
	int		SkipStartSync;
	bool	SkipUntilKeyOn;

	int		RegisterOutSetting;
	int		SkipStartSyncSetting;
	bool	SkipUntilKeyOnSetting;


	uint	PreviousCpuTick;
	uint	LeftTick;
	int		TotalFrame;
	int		Frames;
	char	SongName[0x40];
};

#endif