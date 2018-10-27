
#ifndef incl_config_h
#define incl_config_h

#include "if/ifcommon.h"
#include "instthnk.h"
#include "loginfo.h"

class ConfigMP : public IConfigPropSheet
{
public:
	void Release(void);
	ConfigMP();
	~ConfigMP();
	bool Init(HINSTANCE _hinst);  
	bool IFCALL Setup(IConfigPropBase*, PROPSHEETPAGE* psp);
	ICPUTime* CpuTime;
	char LogFile[MAX_PATH];
	LogRecordInfo *Log;
	

private:
	BOOL PageProc(HWND, UINT, WPARAM, LPARAM);
	static BOOL CALLBACK PageGate(ConfigMP*, HWND, UINT, WPARAM, LPARAM);
	void SetSeqTime(HWND);
	int GetLogFilename(HWND,char *);
	void WriteS98Header(void);
	void WriteS98End(void);
	void PressedStopButton(HWND hdlg);
	bool GetItemBool(HWND hdlg, int ControlId);
	void SetItemBool(HWND hdlg, int ControlId, bool Checked);
	void PressedRecButton(HWND hdlg);
	void FillNextLogFilename(HWND hdlg);
	void UpdateReadOnly(HWND hdlg, bool ReadOnly);
	void UpdateStatus(HWND hdlg);

	HINSTANCE hinst;
	IConfigPropBase* base;
	InstanceThunk gate;
};

#endif // incl_config_h
