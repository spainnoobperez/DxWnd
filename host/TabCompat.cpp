// TabDirectX.cpp : implementation file
//

#include "stdafx.h"
#include "TargetDlg.h"
#include "TabCompat.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CTabCompat dialog

CTabCompat::CTabCompat(CWnd *pParent /*=NULL*/)
//	: CTargetDlg(pParent)
    : CDialog(CTabCompat::IDD, pParent) {
    //{{AFX_DATA_INIT(CTabCompat)
    // NOTE: the ClassWizard will add member initialization here
    //}}AFX_DATA_INIT
}

BOOL CTabCompat::PreTranslateMessage(MSG *pMsg) {
    //if(((pMsg->message == WM_KEYDOWN) || (pMsg->message == WM_KEYUP))
    if((pMsg->message == WM_KEYDOWN)
            && (pMsg->wParam == VK_RETURN))
        return TRUE;
    return CWnd::PreTranslateMessage(pMsg);
}

void CTabCompat::DoDataExchange(CDataExchange *pDX) {
    CDialog::DoDataExchange(pDX);
    CTargetDlg *cTarget = ((CTargetDlg *)(this->GetParent()->GetParent()));
    DDX_Check(pDX, IDC_FAKEVERSION, cTarget->m_FakeVersion);
    DDX_LBIndex(pDX, IDC_LISTFAKE, cTarget->m_FakeVersionId);
    DDX_Check(pDX, IDC_SINGLEPROCAFFINITY, cTarget->m_SingleProcAffinity);
    DDX_Check(pDX, IDC_USELASTCORE, cTarget->m_UseLastCore);
    DDX_Check(pDX, IDC_HANDLEEXCEPTIONS, cTarget->m_HandleExceptions);
    DDX_Check(pDX, IDC_VIRTUALHEAP, cTarget->m_VirtualHeap);
    DDX_Check(pDX, IDC_VIRTUALPROCHEAP, cTarget->m_VirtualProcHeap);
    DDX_Check(pDX, IDC_NOBAADFOOD, cTarget->m_NoBAADFOOD);
    DDX_Check(pDX, IDC_LIMITRESOURCES, cTarget->m_LimitResources);
    DDX_Check(pDX, IDC_FONTBYPASS, cTarget->m_FontBypass);
    DDX_Check(pDX, IDC_NOPERFCOUNTER, cTarget->m_NoPerfCounter);
    DDX_Check(pDX, IDC_DIABLOTWEAK, cTarget->m_DiabloTweak);
    DDX_Check(pDX, IDC_EASPORTSHACK, cTarget->m_EASportsHack);
    DDX_Check(pDX, IDC_LEGACYALLOC, cTarget->m_LegacyAlloc);
    DDX_Check(pDX, IDC_DISABLEMAXWINMODE, cTarget->m_DisableMaxWinMode);
    DDX_Check(pDX, IDC_NOIMAGEHLP, cTarget->m_NoImagehlp);
    DDX_Check(pDX, IDC_REPLACEPRIVOPS, cTarget->m_ReplacePrivOps);
    DDX_Check(pDX, IDC_BLOCKPRIORITYCLASS, cTarget->m_BlockPriorityClass);
    DDX_Check(pDX, IDC_COLORFIX, cTarget->m_ColorFix);
    DDX_Check(pDX, IDC_FIXGLOBALUNLOCK, cTarget->m_FixGlobalUnlock);
    DDX_Check(pDX, IDC_FIXFREELIBRARY, cTarget->m_FixFreeLibrary);
    DDX_Check(pDX, IDC_SKIPFREELIBRARY, cTarget->m_SkipFreeLibrary);
    DDX_Check(pDX, IDC_LOADLIBRARYERR, cTarget->m_LoadLibraryErr);
    DDX_Check(pDX, IDC_FIXALTEREDPATH, cTarget->m_FixAlteredPath);
    DDX_Check(pDX, IDC_FIXADJUSTWINRECT, cTarget->m_FixAdjustWinRect);
    DDX_Check(pDX, IDC_PRETENDVISIBLE, cTarget->m_PretendVisible);
    DDX_Check(pDX, IDC_WININSULATION, cTarget->m_WinInsulation);
    DDX_Check(pDX, IDC_DISABLEMMX, cTarget->m_DisableMMX);
    DDX_Check(pDX, IDC_SAFEALLOCS, cTarget->m_SafeAllocs);
    // GOG patches
    DDX_Check(pDX, IDC_HOOKGOGLIBS, cTarget->m_HookGOGLibs);
    DDX_Check(pDX, IDC_BYPASSGOGLIBS, cTarget->m_BypassGOGLibs);
}

BEGIN_MESSAGE_MAP(CTabCompat, CDialog)
    //{{AFX_MSG_MAP(CTabCompat)
    // NOTE: the ClassWizard will add message map macros here
    //}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CTabCompat message handlers

static struct {
    char bMajor;
    char bMinor;
    char *sName;
} WinVersions[9] = {
    {4, 0, "Windows 95"},
    {4, 10, "Windows 98/SE"},
    {4, 90, "Windows ME"},
    {5, 0, "Windows 2000"},
    {5, 1, "Windows XP"},
    {5, 2, "Windows Server 2003"},
    {6, 0, "Windows Vista"},
    {6, 1, "Windows 7"},
    {6, 2, "Windows 8"}
};

BOOL CTabCompat::OnInitDialog() {
    AfxEnableControlContainer();
    CListBox *List;
    CTargetDlg *cTarget = ((CTargetDlg *)(this->GetParent()->GetParent()));
    int i;
    List = (CListBox *)this->GetDlgItem(IDC_LISTFAKE);
    List->ResetContent();
    for(i = 0; i < 9; i++) List->AddString(WinVersions[i].sName);
    List->SetCurSel(cTarget->m_FakeVersion);
    CDialog::OnInitDialog();
    return TRUE;
}