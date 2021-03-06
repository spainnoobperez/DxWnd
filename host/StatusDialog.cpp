// StatusDialog.cpp : implementation file
//

#include "stdafx.h"
#include "dxwndhost.h"
#include "StatusDialog.h"

// CStatusDialog dialog

BEGIN_MESSAGE_MAP(CStatusDialog, CDialog)
    ON_COMMAND(IDCANCEL, OnOK)
    ON_WM_TIMER()
END_MESSAGE_MAP()

IMPLEMENT_DYNAMIC(CStatusDialog, CDialog)

CStatusDialog::CStatusDialog(CWnd *pParent /*=NULL*/)
    : CDialog(CStatusDialog::IDD, pParent) {
}

CStatusDialog::~CStatusDialog() {
}

void CStatusDialog::DoDataExchange(CDataExchange *pDX) {
    CDialog::DoDataExchange(pDX);
}


#define IDStatusTIMER 1

// CStatusDialog message handlers

void CStatusDialog::OnTimer(UINT_PTR nIDEvent) {
    int DxStatus;
    int IconId;
    LPCSTR Status;
    char sMsg[1024];
    char sMsg2[1024];
    char DllVersion[21];
    DXWNDSTATUS DxWndStatus;
    extern PRIVATEMAP *pTitles;
    extern TARGETMAP *pTargets;
    TARGETMAP *Target;
    extern char *GetTSCaption(int);
    WARNTIMERS;
    GetDllVersion(DllVersion);
    DxStatus = GetHookStatus(&DxWndStatus);
    switch (DxStatus) {
    case DXW_IDLE:
        IconId = IDI_DXIDLE;
        Status = "DISABLED";
        break;
    case DXW_ACTIVE:
        IconId = IDI_DXWAIT;
        Status = "READY";
        break;
    case DXW_RUNNING:
        IconId = IDI_DXRUN;
        Status = "RUNNING";
        break;
    default:
        IconId = IDI_DXIDLE;
        Status = "???";
        break;
    }
    sprintf_s(sMsg, 1024,
              "DxWnd %s\n"
              "Hook status: %s",
              DllVersion, Status);
    if(DxStatus == DXW_RUNNING) {
        // v2.05.43: fix - use OrigIdx, not TaskIdx
        Target = &pTargets[DxWndStatus.OrigIdx];
        sprintf_s(sMsg2, 1024,
                  "\nTask=(%d): %s\n"
                  "pid=%x hwnd=%x\n"
                  "Screen = (%dx%d) Color BPP=%d\n"
                  "FullScreen = %s\nDX version = %d\n"
                  "Logging = %s",
                  (DxWndStatus.OrigIdx + 1), pTitles[DxWndStatus.OrigIdx].title,
                  DxWndStatus.dwPid, DxWndStatus.hWnd,
                  DxWndStatus.Width, DxWndStatus.Height, DxWndStatus.ColorDepth,
                  DxWndStatus.IsFullScreen ? "Yes" : "No", DxWndStatus.DXVersion,
                  DxWndStatus.isLogging ? "ON" : "OFF");
        strcat(sMsg, sMsg2);
        if(DxWndStatus.CursorX != (short) - 1) {
            sprintf_s(sMsg2, 1024,
                      "\nCursor  = (%d,%d)", DxWndStatus.CursorX, DxWndStatus.CursorY);
            strcat(sMsg, sMsg2);
        }
        if(DxWndStatus.MessageX != (short) - 1) {
            sprintf_s(sMsg2, 1024,
                      "\nMessage = (%d,%d)", DxWndStatus.MessageX, DxWndStatus.MessageY);
            strcat(sMsg, sMsg2);
        }
        if(DxWndStatus.MsgHookX != (short) - 1) {
            sprintf_s(sMsg2, 1024,
                      "\nMsgHook = (%d,%d)", DxWndStatus.MsgHookX, DxWndStatus.MsgHookY);
            strcat(sMsg, sMsg2);
        }
        if(DxWndStatus.WinProcX != (short) - 1) {
            sprintf_s(sMsg2, 1024,
                      "\nWinProc = (%d,%d)", DxWndStatus.WinProcX, DxWndStatus.WinProcY);
            strcat(sMsg, sMsg2);
        }
        if(DxWndStatus.pfd.dwFlags & DDPF_FOURCC) {
            char *fcc;
            fcc = (char *) & (DxWndStatus.pfd.dwFourCC);
            sprintf_s(sMsg2, 1024,
                      "\nFourCC = %x (%c%c%c%c)",
                      DxWndStatus.pfd.dwFourCC,
                      isprint(fcc[0]) ? fcc[0] : '.', isprint(fcc[1]) ? fcc[1] : '.', isprint(fcc[2]) ? fcc[2] : '.', isprint(fcc[3]) ? fcc[3] : '.'
                     );
            strcat(sMsg, sMsg2);
        }
        if(DxWndStatus.pfd.dwSize) {
            sprintf_s(sMsg2, 1024,
                      "\nPixel format=%s%s%s%s%s%s",
                      (DxWndStatus.pfd.dwFlags & DDPF_ALPHAPIXELS) ? "ALPHAPIXELS " : "",
                      (DxWndStatus.pfd.dwFlags & DDPF_ALPHA) ? "ALPHA " : "",
                      (DxWndStatus.pfd.dwFlags & DDPF_FOURCC) ? "FOURCC " : "",
                      (DxWndStatus.pfd.dwFlags & (DDPF_PALETTEINDEXED4 | DDPF_PALETTEINDEXEDTO8 | DDPF_PALETTEINDEXED8)) ? "PALETTEINDEXED " : "",
                      (DxWndStatus.pfd.dwFlags & DDPF_RGB) ? "RGB " : "",
                      (DxWndStatus.pfd.dwFlags & DDPF_YUV) ? "YUV " : ""
                     );
            strcat(sMsg, sMsg2);
        }
        if(DxWndStatus.pfd.dwFlags & DDPF_RGB) {
            sprintf_s(sMsg2, 1024,
                      "\nColor mask (RGBA)=(%x,%x,%x,%x)",
                      DxWndStatus.pfd.dwRBitMask, DxWndStatus.pfd.dwGBitMask, DxWndStatus.pfd.dwBBitMask, DxWndStatus.pfd.dwRGBAlphaBitMask
                     );
            strcat(sMsg, sMsg2);
        }
        // v2.05.43: FPS counter always on
        if(DxWndStatus.FPSCount) {
            sprintf(sMsg2, "\nFPS = %d", DxWndStatus.FPSCount);
            strcat(sMsg, sMsg2);
        }
        if(Target->flags2 & TIMESTRETCH) {
            if(DxWndStatus.TimeShift >= -8 && DxWndStatus.TimeShift <= 8) {
                sprintf(sMsg2, "\nTime speed %s", GetTSCaption(DxWndStatus.TimeShift));
                strcat(sMsg, sMsg2);
            }
        }
    }
    this->SetDlgItemTextA(IDC_STATUSINFO, sMsg);
}

BOOL CStatusDialog::OnInitDialog() {
    CDialog::OnInitDialog();
    SetTimer(IDStatusTIMER, 1000, NULL);
    // TODO:  Add extra initialization here
    return TRUE;  // return TRUE unless you set the focus to a control
    // EXCEPTION: OCX Property Pages should return FALSE
}

void CStatusDialog::OnOK() {
    // TODO: Add your specialized code here and/or call the base class
    KillTimer(IDStatusTIMER);
    // stop timer
    CDialog::OnOK();
}
