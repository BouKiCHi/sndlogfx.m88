

#include "headers.h"
#include "sndlog.h"

// コンストラクタ
SoundLog::SoundLog()
: Device(0)
{
}

SoundLog::~SoundLog()
{
}

// ---------------------------------------------------------------------------
//	I/O ポートを監視

void IFCALL SoundLog::ResetCpu(uint Address, uint Value)
{
	uint CpuTick = CpuTime->GetCPUTick();
	Log->PreviousCpuTick = CpuTick;
}

void IFCALL SoundLog::OutputSoundLog(uint Address, uint Value)
{
	// A0 = 0 アドレス指定 $44, $46
	// A0 = 1 データ指定 $45, $47

	int Extend = ((Address & 0xfe) == 0x46) ? 1 : 0;

	// アドレス
	if (!(Address & 1)) {
		RegisterAddressList[Extend] = Value;
		return;
	}

	int RegisterAddress = RegisterAddressList[Extend];
	if (Extend) RegisterAddress |= 0x100;


	// 録音フラグ
	if (!Log->RecordingFlag) return;

	// 一時停止
	if (Log->PauseFlag) {
		Log->PreviousCpuTick = CpuTime->GetCPUTick();
		return;
	}

	uint CpuTick = CpuTime->GetCPUTick();

	// GetCPUSpeedは4MHzであれば40
	// 40000 / 40 = 1000
	uint CpuSpeed = CpuTime->GetCPUSpeed() * 1000;

	// 最初の同期をスキップする
	bool ResetTick = Log->SkipStartSync;

	// 最初のキーオンまでスキップする
	if (Log->SkipUntilKeyOn) {
		ResetTick = true;
		// FM
		if (RegisterAddress == 0x28) {
			if (Value & 0xf0) Log->SkipUntilKeyOn = false;
		}
		// リズム
		if (RegisterAddress == 0x10) {
			if (!(Value & 0x80) && (Value & 0x1f)) Log->SkipUntilKeyOn = false;
		}

	}

	if (ResetTick) {
		Log->PreviousCpuTick = CpuTick;
		Log->SkipStartSync = 0;
	}
	
	// 差分フレーム数
	// 4,000,000 / 40,000 = 100
	uint DiffCpuTick = (CpuTick - Log->PreviousCpuTick);
	Log->PreviousCpuTick = CpuTick;

	DiffCpuTick += Log->LeftTick;
	Log->LeftTick = DiffCpuTick;

	int Frame = DiffCpuTick / CpuSpeed;
	if (Frame > 0) {
		Log->TotalFrame += Frame;
		Log->Frames += Frame;
		Log->LeftTick -= Frame * CpuSpeed;
	}


	OutputSync();
	OutputRegisterData(Extend == 0 ? 0 : 1, RegisterAddress, Value);
}

// レジスタ出力
void SoundLog::OutputRegisterData(uint Device, uint Address, uint Value)
{
	DWORD ResultBytes = 0;
	BYTE OutData[4];
	OutData[0] = Device;
	OutData[1] = Address & 0xff;
	OutData[2] = Value & 0xff;

	WriteFile(Log->LogFileHandle, OutData, 3, &ResultBytes, NULL);
}

// 同期信号
void SoundLog::OutputSync()
{
	DWORD ByteWritten = 0;
	BYTE OutData[16];

	if (Log->Frames == 0) return;
	
	// 1frame
	if (Log->Frames == 1) {
		OutData[0] = 0xFF;
		WriteFile(Log->LogFileHandle, OutData, 1, &ByteWritten, NULL);
		Log->Frames = 0;
		return;
	}
	// n-frame (出力値はn-2)
	OutData[0] = 0xFE;
	int Index = 0;
	uint Frames = Log->Frames - 2;

	do {
		BYTE LengthByte = Frames & 0x7F;
		Frames >>= 7;
		if (Frames > 0) LengthByte |= 0x80;
		OutData[1 + Index] = LengthByte;
		Index++;
	} while (Frames > 0);

	WriteFile(Log->LogFileHandle, OutData, 1 + Index, &ByteWritten, NULL);
	Log->Frames = 0;
}

// ---------------------------------------------------------------------------
//	device description
//
const Device::Descriptor SoundLog::descriptor = { /*indef*/ 0, outdef };

const Device::OutFuncPtr SoundLog::outdef[] = 
{
	STATIC_CAST(Device::OutFuncPtr, &SoundLog::ResetCpu),
	STATIC_CAST(Device::OutFuncPtr, &SoundLog::OutputSoundLog),
};

/*
const Device::InFuncPtr SoundLog::indef[] = 
{
	0,
};
*/

