

#ifndef incl_mem_h
#define incl_mem_h

#include "device.h"
#include "if/ifcommon.h"
#include "loginfo.h"

// ---------------------------------------------------------------------------

class SoundLog : public Device
{
public:
	enum
	{
		RESET_CPU = 0,
		OPN_OUT,
	};

public:
	SoundLog();
	~SoundLog();

	
	const Descriptor* IFCALL GetDesc() const { return &descriptor; }

	// interface Config
	LogRecordInfo *Log;
	ICPUTime* CpuTime;


	// I/O port functions
	void IOCALL OutputSoundLog(uint Address, uint Value);
	void IOCALL ResetCpu(uint Address, uint Value);


private:

	void OutputRegisterData(uint Device, uint Address, uint Data);
	void OutputSync();

	unsigned char RegisterAddressList[4];

	static const Descriptor descriptor;
//	static const InFuncPtr  indef[];
	static const OutFuncPtr outdef[];
};

#endif // incl_mem_h
