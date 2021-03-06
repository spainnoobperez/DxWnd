// TabLogs.cpp : implementation file
//

#include "stdafx.h"
#include "TargetDlg.h"
#include "TabLogs.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CTabLogs dialog

CTabLogs::CTabLogs(CWnd *pParent /*=NULL*/)
    : CDialog(CTabLogs::IDD, pParent) {
    //{{AFX_DATA_INIT(CTabLogs)
    // NOTE: the ClassWizard will add member initialization here
    //}}AFX_DATA_INIT
}

BOOL CTabLogs::PreTranslateMessage(MSG *pMsg) {
    //if(((pMsg->message == WM_KEYDOWN) || (pMsg->message == WM_KEYUP))
    if((pMsg->message == WM_KEYDOWN)
            && (pMsg->wParam == VK_RETURN))
        return TRUE;
    return CWnd::PreTranslateMessage(pMsg);
}

//BOOL CTabLogs::OnInitDialog()
//{
//	extern BOOL gbDebug;
//	CDialog::OnInitDialog();
//	(CButton *)(this->GetDlgItem(IDC_ASSERT))->EnableWindow(gbDebug ? TRUE : FALSE);
//	return TRUE;
//}

void CTabLogs::DoDataExchange(CDataExchange *pDX) {
    CDialog::DoDataExchange(pDX);
    CTargetDlg *cTarget = ((CTargetDlg *)(this->GetParent()->GetParent()));
    DDX_Radio(pDX, IDC_LOG_NONE, cTarget->m_LogMode);
    DDX_Check(pDX, IDC_OUTD3DTRACE, cTarget->m_OutD3DTrace);
    DDX_Check(pDX, IDC_OUTDDRAWTRACE, cTarget->m_OutDDRAWTrace);
    DDX_Check(pDX, IDC_OUTDEBUG, cTarget->m_OutDebug);
    DDX_Check(pDX, IDC_CURSORTRACE, cTarget->m_CursorTrace);
    //DDX_Check(pDX, IDC_LOGENABLED, cTarget->m_LogEnabled);
    DDX_Check(pDX, IDC_OUTDEBUGSTRING, cTarget->m_OutDebugString);
    //DDX_Check(pDX, IDC_ERASELOGFILE, cTarget->m_EraseLogFile);
    DDX_Check(pDX, IDC_ADDTIMESTAMP, cTarget->m_AddTimeStamp);
    DDX_Check(pDX, IDC_ADDRELATIVETIME, cTarget->m_AddRelativeTime);
    DDX_Check(pDX, IDC_ADDTHREADID, cTarget->m_AddThreadID);
    DDX_Check(pDX, IDC_OUTWINMESSAGES, cTarget->m_OutWinMessages);
    DDX_Check(pDX, IDC_OUTDWTRACE, cTarget->m_OutDWTrace);
    DDX_Check(pDX, IDC_IMPORTTABLE, cTarget->m_ImportTable);
    DDX_Check(pDX, IDC_OUTREGISTRY, cTarget->m_RegistryOp);
    DDX_Check(pDX, IDC_TRACEHOOKS, cTarget->m_TraceHooks);
    DDX_Check(pDX, IDC_OUTOGLTRACE, cTarget->m_OutOGLTrace);
    DDX_Check(pDX, IDC_OUTWGTRACE, cTarget->m_OutWGTrace);
    DDX_Check(pDX, IDC_OUTHEXTRACE, cTarget->m_OutHexTrace);
    DDX_Check(pDX, IDC_OUTSDLTRACE, cTarget->m_OutSDLTrace);
    DDX_Check(pDX, IDC_OUTTIMETRACE, cTarget->m_OutTimeTrace);
    DDX_Check(pDX, IDC_OUTSOUNDTRACE, cTarget->m_OutSoundTrace);
    DDX_Check(pDX, IDC_OUTINPUTS, cTarget->m_OutInputs);
    DDX_Check(pDX, IDC_OUTLOCALE, cTarget->m_OutLocale);
    DDX_Check(pDX, IDC_OUTSYSLIBS, cTarget->m_OutSysLibs);
    DDX_Check(pDX, IDC_OUTFPS, cTarget->m_OutFPS);
}

BEGIN_MESSAGE_MAP(CTabLogs, CDialog)
    //{{AFX_MSG_MAP(CTabLogs)
    // NOTE: the ClassWizard will add message map macros here
    //}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CTabLogs message handlers
