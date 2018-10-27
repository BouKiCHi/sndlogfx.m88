

#include "headers.h"
#include "if/ifcommon.h"
#include "if/ifguid.h"
#include "sndlog.h"
#include "config.h"

#define EXTDEVAPI	__declspec(dllexport)

HINSTANCE hinst;

// ---------------------------------------------------------------------------

class SoundLogModule : public IModule
{
public:
	SoundLogModule();
	~SoundLogModule() {}

	LogRecordInfo Log;

	bool Init(ISystem* system);
	void IFCALL Release();

	void* IFCALL QueryIF(REFIID) { return 0; }

private:

	SoundLog Sound;
	ConfigMP Config;
	ISystem* System;
	IIOBus* IoBus;
	ICPUTime* CpuTime;
	IConfigPropBase* pb;
};

SoundLogModule::SoundLogModule() : IoBus(0), pb(0)
{
	// LogRecordInfoの初期化
	memset(&Log, 0, sizeof(Log));

	Sound.Log = &Log;
	Config.Log = &Log;

	Config.Init(hinst);
}

bool SoundLogModule::Init(ISystem* _sys)
{
	// インターフェースの接続
	System = _sys;
	IoBus = (IIOBus*) System->QueryIF(M88IID_IOBus1);
	pb = (IConfigPropBase*) System->QueryIF(M88IID_ConfigPropBase);
	CpuTime = (ICPUTime*) System->QueryIF(M88IID_CPUTime);

	if (!IoBus || !pb || !CpuTime) {
		return false;
	}

	Sound.CpuTime = Config.CpuTime = CpuTime;

	// 0x108 = PC88::pres
	const static IIOBus::Connector ConnectList[] =
	{
		{ 0x108,IIOBus::portout,SoundLog::RESET_CPU },
		{ 0x44 ,IIOBus::portout,SoundLog::OPN_OUT },
		{ 0x45 ,IIOBus::portout,SoundLog::OPN_OUT },
		{ 0x46 ,IIOBus::portout,SoundLog::OPN_OUT },
		{ 0x47 ,IIOBus::portout,SoundLog::OPN_OUT },

		{ 0, 0, 0 }
	};
	if (!IoBus->Connect(&Sound, ConnectList))
		return false;

	pb->Add(&Config);
	return true;
}

void SoundLogModule::Release()
{
	if (IoBus)
		IoBus->Disconnect(&Sound);
	if (pb)
		pb->Remove(&Config);

	Config.Release();

	delete this;
}

// ---------------------------------------------------------------------------

//	Module を作成
extern "C" EXTDEVAPI IModule* __cdecl M88CreateModule(ISystem* system)
{
	SoundLogModule* module = new SoundLogModule;
	
	if (module)
	{
		if (module->Init(system))
			return module;
		delete module;
	}
	return 0;
}

BOOL APIENTRY DllMain(HANDLE hmod, DWORD rfc, LPVOID)
{
	switch (rfc)
	{
	case DLL_PROCESS_ATTACH:
		hinst = (HINSTANCE) hmod;
		break;
	
	case DLL_THREAD_ATTACH:
		break;
	
	case DLL_THREAD_DETACH:
		break;
	
	case DLL_PROCESS_DETACH:
		break;
	}
	return true;
}
