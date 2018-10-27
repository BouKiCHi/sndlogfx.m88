
#include "headers.h"
#include "config.h"
#include "resource.h"

// ---------------------------------------------------------------------------

ConfigMP::ConfigMP()
{
	gate.SetDestination(PageGate, this);
}

ConfigMP::~ConfigMP()
{
	if (Log->RecordingFlag) {
		WriteS98End();
		CloseHandle(Log->LogFileHandle);
		Log->PauseFlag = Log->RecordingFlag = 0;
	}
}

// 設定の初期化を行う
bool ConfigMP::Init(HINSTANCE _hinst)
{
	strcpy_s(LogFile,"sndout.s98");
	hinst = _hinst;
	Log->SkipStartSyncSetting = true;
	Log->SkipUntilKeyOnSetting = true;
	return true;
}


bool IFCALL ConfigMP::Setup(IConfigPropBase* _base, PROPSHEETPAGE* psp)
{
	base = _base;
	
	memset(psp, 0, sizeof(PROPSHEETPAGE));
	psp->dwSize = sizeof(PROPSHEETPAGE);
	psp->dwFlags = 0;
	psp->hInstance = hinst;
	psp->pszTemplate = MAKEINTRESOURCE(IDD_CONFIG);
	psp->pszIcon = 0;
	psp->pfnDlgProc = (DLGPROC) (void*) gate;
	psp->lParam = 0;
	return true;
}

void ConfigMP::SetSeqTime(HWND hdlg)
{
	char Text[32];

	int Second = Log->TotalFrame / 100;
	wsprintf(Text, "%02d:%02d", Second/60, Second%60);
	SetWindowText(GetDlgItem(hdlg, IDC_RECTIME), Text);
}

int ConfigMP::GetLogFilename(HWND hdlg,char *fnam)
{
	OPENFILENAME ofn;
	memset(&ofn, 0, sizeof(OPENFILENAME));
	ofn.lStructSize = sizeof(OPENFILENAME);
	ofn.hwndOwner = hdlg;
	ofn.lpstrDefExt = "s98";
	ofn.lpstrFilter = "S98 files (*.s98)\0*.s98\0All files(*.*)\0*.*\0\0";
	ofn.lpstrFile = fnam;
	ofn.nMaxFile = MAX_PATH;
	ofn.Flags = OFN_OVERWRITEPROMPT;
	ofn.nMaxFileTitle = 0;
	ofn.lpstrFileTitle = NULL;
	ofn.lpstrTitle = "S98ファイルを開きます";

	if (GetSaveFileName(&ofn)) return 1;

	return 0;
}

// S98ヘッダの出力
void ConfigMP::WriteS98Header(void)
{
	DWORD ResultBytes;
	BYTE Header[0x80];
	memset(Header, 0, 0x80);

	memcpy(Header,"S981",4);
	Header[0x04]=00;
	strcpy_s((char *)(Header+0x40), 64, Log->SongName);
	Header[0x7F]=0x00;
	
	// タグ
	Header[0x10]=0x40;
	// 曲開始位置
	Header[0x14]=0x80;
	WriteFile(Log->LogFileHandle, Header, 0x80, &ResultBytes, NULL);
}

// S98終了
void ConfigMP::WriteS98End(void)
{
	DWORD ResultBytes;
	BYTE Data[0x8];
	Data[0]=0xFD;
	WriteFile(Log->LogFileHandle, Data, 1, &ResultBytes, NULL);
}

// 停止ボタン
void ConfigMP::PressedStopButton(HWND hdlg)
{
	if (!Log->RecordingFlag) return;
	Log->TotalFrame = 0;
	Log->PauseFlag = Log->RecordingFlag = 0;
	WriteS98End();
	CloseHandle(Log->LogFileHandle);
	UpdateStatus(hdlg);
	FillNextLogFilename(hdlg);
}

bool ConfigMP::GetItemBool(HWND hdlg, int ControlId) {
	return SendMessage(GetDlgItem(hdlg, ControlId), BM_GETCHECK, 0, 0);
}

void ConfigMP::SetItemBool(HWND hdlg, int ControlId, bool Checked) {
	WPARAM Param = Checked ? BM_SETCHECK : BST_UNCHECKED;
	SendMessage(GetDlgItem(hdlg, ControlId), BM_SETCHECK, Param, 0);
}


void ConfigMP::PressedRecButton(HWND hdlg)
{
	if (Log->RecordingFlag) return;

	// 各ステータスの取得
	Log->RegisterOut = Log->RegisterOutSetting = GetItemBool(hdlg, IDC_REGOUT);
	Log->SkipStartSync = Log->SkipStartSyncSetting = GetItemBool(hdlg, IDC_SKIP_START_SYNC);
	Log->SkipUntilKeyOn = Log->SkipUntilKeyOnSetting = GetItemBool(hdlg, IDC_SKIP_UNTIL_KEYON);

	GetWindowText(GetDlgItem(hdlg, IDC_LOGNAME),LogFile,MAX_PATH);
	GetWindowText(GetDlgItem(hdlg, IDC_SONGNAME),Log->SongName,0x40);

	Log->LogFileHandle = CreateFile(LogFile,GENERIC_WRITE,0,NULL,CREATE_ALWAYS,FILE_ATTRIBUTE_NORMAL,NULL);

	if (Log->LogFileHandle != INVALID_HANDLE_VALUE) {
		Log->PreviousCpuTick = CpuTime->GetCPUTick();
		Log->TotalFrame = 0;
		Log->RecordingFlag = 1;
		WriteS98Header();
	}

	UpdateStatus(hdlg);
}

void ConfigMP::FillNextLogFilename(HWND hdlg)
{
	char NextFilename[MAX_PATH];
	while(Log->FileSerialCount < 10000) {
		sprintf_s(NextFilename, "slog%04d.s98", Log->FileSerialCount);
		DWORD Check = GetFileAttributes(NextFilename);
		if (Check == INVALID_FILE_ATTRIBUTES) {
			SetWindowText(GetDlgItem(hdlg,IDC_LOGNAME),NextFilename);
			return;
		}

		Log->FileSerialCount++;
	}
}

void ConfigMP::UpdateReadOnly(HWND hdlg, bool ReadOnly) {
	Edit_SetReadOnly(GetDlgItem(hdlg, IDC_LOGNAME), ReadOnly);
	Edit_SetReadOnly(GetDlgItem(hdlg, IDC_SONGNAME), ReadOnly);
}

void ConfigMP::UpdateStatus(HWND hdlg) {
	if (CpuTime) SetSeqTime(hdlg);
	SetItemBool(hdlg, IDC_SKIP_START_SYNC, Log->SkipStartSyncSetting);
	SetItemBool(hdlg, IDC_SKIP_UNTIL_KEYON, Log->SkipUntilKeyOnSetting);
	SetItemBool(hdlg, IDC_REGOUT, Log->RegisterOut);

	// 停止中
	if (!Log->RecordingFlag) {
		UpdateReadOnly(hdlg, FALSE);
		SetWindowText(GetDlgItem(hdlg, IDC_STATUS), "STOP");
		return;
	}

	// 記録中
	UpdateReadOnly(hdlg, TRUE);

	// 一時停止
	if (Log->PauseFlag) SetWindowText(GetDlgItem(hdlg, IDC_STATUS), "PAUSE");
	else SetWindowText(GetDlgItem(hdlg, IDC_STATUS), "REC");
}

BOOL ConfigMP::PageProc(HWND hdlg, UINT msg, WPARAM wp, LPARAM lp)
{
	switch (msg)
	{
		case WM_INITDIALOG:
			SetTimer(hdlg,0x100,1000,NULL);	

			SetWindowText(GetDlgItem(hdlg, IDC_LOGNAME),LogFile);
			FillNextLogFilename(hdlg);

			UpdateStatus(hdlg);
			return TRUE;
		break;
		case WM_TIMER:
			if (CpuTime) SetSeqTime(hdlg);

			return TRUE;
		break;
		case WM_NOTIFY:
			switch (((NMHDR*) lp)->code)
			{
			case PSN_SETACTIVE:
				base->PageSelected(this);
				break;
			case PSN_APPLY:
				base->Apply();
				return PSNRET_NOERROR;
			}
			return TRUE;
		break;
			case WM_COMMAND:
				switch(LOWORD(wp)) {
					case IDC_AUTO:
						PressedStopButton(hdlg);
						FillNextLogFilename(hdlg);
						PressedRecButton(hdlg);
					break;
					case IDC_NEXT:
						FillNextLogFilename(hdlg);
					break;
					case IDC_FILE:
						GetWindowText(GetDlgItem(hdlg, IDC_LOGNAME),LogFile,MAX_PATH);
						if (GetLogFilename(hdlg,LogFile))
						{
							SetWindowText(GetDlgItem(hdlg, IDC_LOGNAME),LogFile);
						}
					break;
					case IDC_REC:
						PressedRecButton(hdlg);
					break;
					case IDC_PAUSE:
						if (Log->RecordingFlag) {
							Log->PauseFlag = !Log->PauseFlag;
							UpdateStatus(hdlg);
						}
					break;
					case IDC_STOP:
						PressedStopButton(hdlg);
					break;

				}
				return TRUE;
			break;
			
		/*case WM_COMMAND:
			if (HIWORD(wp) == BN_CLICKED) {
					base->PageChanged(hdlg);
					return TRUE;
			}
		break; */
	}

	return FALSE;
}

BOOL CALLBACK ConfigMP::PageGate
(ConfigMP* config, HWND hwnd, UINT m, WPARAM w, LPARAM l)
{
	return config->PageProc(hwnd, m, w, l);
}

void ConfigMP::Release(void)
{
	if (!Log->RecordingFlag) return;
	WriteS98End();
	CloseHandle(Log->LogFileHandle);
	Log->PauseFlag = Log->RecordingFlag = 0;
}
