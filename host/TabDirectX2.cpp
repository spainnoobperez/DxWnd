// TabDirectX2.cpp : implementation file
//

#include "stdafx.h"
#include "TargetDlg.h"
#include "TabDirectX2.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CTabDirectX2 dialog

CTabDirectX2::CTabDirectX2(CWnd *pParent /*=NULL*/)
    : CDialog(CTabDirectX2::IDD, pParent) {
    //{{AFX_DATA_INIT(CTabDirectX2)
    // NOTE: the ClassWizard will add member initialization here
    //}}AFX_DATA_INIT
}

BOOL CTabDirectX2::PreTranslateMessage(MSG *pMsg) {
    //if(((pMsg->message == WM_KEYDOWN) || (pMsg->message == WM_KEYUP))
    if((pMsg->message == WM_KEYDOWN)
            && (pMsg->wParam == VK_RETURN))
        return TRUE;
    return CWnd::PreTranslateMessage(pMsg);
}

void CTabDirectX2::DoDataExchange(CDataExchange *pDX) {
    CDialog::DoDataExchange(pDX);
    CTargetDlg *cTarget = ((CTargetDlg *)(this->GetParent()->GetParent()));
    // Ddraw tweaks
    // DDX_Check(pDX, IDC_NOSYSMEMPRIMARY, cTarget->m_NoSysMemPrimary);
    //DDX_Check(pDX, IDC_NOSYSMEMBACKBUF, cTarget->m_NoSysMemBackBuf);
    DDX_Check(pDX, IDC_FIXPITCH, cTarget->m_FixPitch);
    DDX_Check(pDX, IDC_POWER2WIDTH, cTarget->m_Power2Width);
    DDX_Check(pDX, IDC_FIXREFCOUNTER, cTarget->m_FixRefCounter);
    DDX_Check(pDX, IDC_NOHALDEVICE, cTarget->m_NoHALDevice);
    DDX_Check(pDX, IDC_NOTNLDEVICE, cTarget->m_NoTnLDevice);
    DDX_Check(pDX, IDC_MINIMALCAPS, cTarget->m_MinimalCaps);
    DDX_Check(pDX, IDC_SETZBUFFERBITDEPTHS, cTarget->m_SetZBufferBitDepths);
    DDX_Check(pDX, IDC_LIMITDDRAW, cTarget->m_LimitDdraw);
    DDX_Check(pDX, IDC_SUPPRESSOVERLAY, cTarget->m_SuppressOverlay);
    DDX_Check(pDX, IDC_USERGB565, cTarget->m_UseRGB565);
    DDX_CBIndex(pDX, IDC_DDWAWLIMITCOMBO, cTarget->m_MaxDdrawInterface);
    DDX_Check(pDX, IDC_CLEARTEXTUREFOURCC, cTarget->m_ClearTextureFourCC);
    DDX_Check(pDX, IDC_SUPPRESSFOURCCBLT, cTarget->m_SuppressFourCCBlt);
    DDX_Check(pDX, IDC_NODDEXCLUSIVEMODE, cTarget->m_NoDDExclusiveMode);
    DDX_Check(pDX, IDC_LOCKFULLSCREENCOOP, cTarget->m_LockFullscreenCoop);
    DDX_Check(pDX, IDC_CREATEDESKTOP, cTarget->m_CreateDesktop);
    DDX_Check(pDX, IDC_SAFEPALETTEUSAGE, cTarget->m_SafePaletteUsage);
    //DDX_Check(pDX, IDC_ALLOWSYSMEMON3DDEV, cTarget->m_AllowSysmemOn3DDev);
    DDX_Check(pDX, IDC_LOADGAMMARAMP, cTarget->m_LoadGammaRamp);
    // Vsync
    //DDX_Check(pDX, IDC_SAVELOAD, cTarget->m_SaveLoad);
    //DDX_Check(pDX, IDC_FORCEVSYNC, cTarget->m_ForceVSync);
    //DDX_Check(pDX, IDC_FORCENOVSYNC, cTarget->m_ForceNoVSync);
    //DDX_Check(pDX, IDC_FORCEWAIT, cTarget->m_ForceWait);
    //DDX_Check(pDX, IDC_FORCENOWAIT, cTarget->m_ForceNoWait);
    DDX_Radio(pDX, IDC_VSYNCDEFAULT, cTarget->m_VSyncMode);
    DDX_Radio(pDX, IDC_WAITDEFAULT, cTarget->m_WaitMode);
    DDX_Radio(pDX, IDC_VSYNCHW, cTarget->m_VSyncImpl);
    DDX_Text(pDX, IDC_SCANLINE, cTarget->m_ScanLine);
    // Clipper
    DDX_Radio(pDX, IDC_CLIPPERNONE, cTarget->m_ClipperMode);
    // FourCC handling
    DDX_Radio(pDX, IDC_NOFOURCC, cTarget->m_FourCCMode);
}

BEGIN_MESSAGE_MAP(CTabDirectX2, CDialog)
    //{{AFX_MSG_MAP(CTabDirectX2)
    // NOTE: the ClassWizard will add message map macros here
    //}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CTabDirectX2 message handlers

