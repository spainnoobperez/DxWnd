#define _WIN32_WINNT 0x0600
#define WIN32_LEAN_AND_MEAN
#define _CRT_SECURE_NO_WARNINGS
#define _CRT_NON_CONFORMING_SWPRINTFS

#include <stdio.h>
#include <stdlib.h>
#include "dxwnd.h"
#include "dxwcore.hpp"
#include "dxhook.h"
#include "hddraw.h"
#include "dxhelper.h"
#include "shareddc.hpp"
#include <Wingdi.h>
#include <Winuser.h>
#include "dlgstretch.h"
#include "h_user32.h"
#include "dxwlocale.h"


#ifndef DXW_NOTRACES
#define TraceError() OutTraceE("%s: ERROR err=%d\n", ApiRef, GetLastError())
#define IfTraceError() if(!res) TraceError()
extern void DumpBitmap(char *, HBITMAP);
#else
#define TraceError()
#define IfTraceError()
#endif

//#define TRACEALL
//#define TRACEPOINTS
//#define TRACEWAITOBJECTS
//#define TRACESYSCALLS
//#define TRACEIMAGES
//#define TRACEMENUS

#ifdef TRACEALL
#define TRACEPOINTS
#define TRACEWAITOBJECTS
#define TRACESYSCALLS
#define TRACEWINDOWS
#define TRACEIMAGES
#define TRACEMENUS
#endif // TRACEALL

#ifdef TRACESYSCALLS
FindWindowA_Type pFindWindowA;
SetActiveWindow_Type pSetActiveWindow;
IsWindow_Type pIsWindow;
IsWindow_Type pIsWindowEnabled;
IsIconic_Type pIsIconic;
IsZoomed_Type pIsZoomed;
GetAncestor_Type pGetAncestor;
LoadBitmapA_Type pLoadBitmapA;
GetWindow_Type pGetWindow;
GetWindowThreadProcessId_Type pGetWindowThreadProcessId;
SetScrollInfo_Type pSetScrollInfo;
GetScrollInfo_Type pGetScrollInfo;
SetScrollPos_Type pSetScrollPos;
GetScrollPos_Type pGetScrollPos;
GetScrollRange_Type pGetScrollRange;
SetScrollRange_Type pSetScrollRange;
TranslateMessage_Type pTranslateMessage;
IsDialogMessageA_Type pIsDialogMessageA;
SetCursor_Type pSetCursor;
SetCapture_Type pSetCapture;
ReleaseCapture_Type pReleaseCapture;
//TranslateMessage_Type pTranslateMessage DXWINITIALIZED;
//GetTopWindow_Type pGetTopWindow DXWINITIALIZED;
//EnumDisplayMonitors_Type pEnumDisplayMonitors;
#endif // TRACESYSCALLS

//#define TRANSLATEMESSAGEHOOK
#define FAKEKILLEDWIN 0xDEADDEAD
//#define HOOKWINDOWSHOOKPROCS TRUE
//#define KEEPWINDOWSTATES TRUE
#define HACKVIRTUALKEYS
#ifdef KEEPWINDOWSTATES
HWND hForegroundWindow = NULL;
HWND hActiveWindow = NULL;
#endif

typedef BOOL (WINAPI *DrawEdge_Type)(HDC, LPRECT, UINT, UINT);
DrawEdge_Type pDrawEdge;
BOOL WINAPI extDrawEdge(HDC, LPRECT, UINT, UINT);

typedef int (WINAPI *SetWindowRgn_Type)(HWND, HRGN, BOOL);
SetWindowRgn_Type pSetWindowRgn;
int WINAPI extSetWindowRgn(HWND, HRGN, BOOL);
typedef int (WINAPI *GetWindowRgn_Type)(HWND, HRGN);
GetWindowRgn_Type pGetWindowRgn;
int WINAPI extGetWindowRgn(HWND, HRGN);
GetMessagePos_Type pGetMessagePos;

#define _Warn(s) MessageBox(0, s, "to do", MB_ICONEXCLAMATION)

BOOL IsChangeDisplaySettingsHotPatched = FALSE;
BOOL InMainWinCreation = FALSE;
HWND CurrentActiveMovieWin = NULL;

extern BOOL bFlippedDC;
extern HDC hFlippedDC;
extern void HandleHotKeys(HWND, UINT, LPARAM, WPARAM);

#ifdef TRACEWAITOBJECTS
typedef DWORD (WINAPI *MsgWaitForMultipleObjects_Type)(DWORD, const HANDLE *, BOOL, DWORD, DWORD);
DWORD WINAPI extMsgWaitForMultipleObjects(DWORD, const HANDLE *, BOOL, DWORD, DWORD);
MsgWaitForMultipleObjects_Type pMsgWaitForMultipleObjects;
#endif // TRACEWAITOBJECTS

#ifdef HACKVIRTUALKEYS
typedef UINT (WINAPI *MapVirtualKeyA_Type)(UINT, UINT);
UINT WINAPI extMapVirtualKeyA(UINT, UINT);
MapVirtualKeyA_Type pMapVirtualKeyA;
#endif // HACKVIRTUALKEYS

#ifdef TRACEPOINTS
typedef BOOL (WINAPI *SetRect_Type)(LPRECT, int, int, int, int);
SetRect_Type pSetRect;
BOOL WINAPI extSetRect(LPRECT, int, int, int, int);
typedef BOOL (WINAPI *PtInRect_Type)(CONST RECT *, POINT);
PtInRect_Type pPtInRect;
BOOL WINAPI extPtInRect(CONST RECT *, POINT);
typedef BOOL (WINAPI *SetRectEmpty_Type)(LPRECT);
SetRectEmpty_Type pSetRectEmpty;
BOOL WINAPI extSetRectEmpty(LPRECT);
typedef BOOL (WINAPI *OffsetRect_Type)(LPRECT, int, int);
OffsetRect_Type pOffsetRect;
BOOL WINAPI extOffsetRect(LPRECT, int, int);
typedef BOOL (WINAPI *InflateRect_Type)(LPRECT, int, int);
InflateRect_Type pInflateRect;
BOOL WINAPI extInflateRect(LPRECT, int, int);
typedef BOOL (WINAPI *OperateRect_Type)(LPRECT, CONST RECT *, CONST RECT *);
OperateRect_Type pIntersectRect, pUnionRect, pSubtractRect;
BOOL WINAPI extIntersectRect(LPRECT, CONST RECT *, CONST RECT *);
BOOL WINAPI extUnionRect(LPRECT, CONST RECT *, CONST RECT *);
BOOL WINAPI extSubtractRect(LPRECT, CONST RECT *, CONST RECT *);
typedef BOOL (WINAPI *IsRectEmpty_Type)(CONST RECT *);
IsRectEmpty_Type pIsRectEmpty;
BOOL WINAPI extIsRectEmpty(CONST RECT *);
typedef BOOL (WINAPI *CopyRect_Type)(LPRECT, CONST RECT *);
CopyRect_Type pCopyRect;
BOOL WINAPI extCopyRect(LPRECT, CONST RECT *);
#endif // TRACEPOINTS

#ifdef TRACEWINDOWS
typedef BOOL (WINAPI *EnableWindow_Type)(HWND, BOOL);
EnableWindow_Type pEnableWindow;
BOOL WINAPI extEnableWindow(HWND, BOOL);
HWND WINAPI extGetFocus(void);
typedef HWND (WINAPI *GetFocus_Type)(void);
GetFocus_Type pGetFocus;
typedef HWND (WINAPI *SetFocus_Type)(HWND);
SetFocus_Type pSetFocus;
HWND WINAPI extSetFocus(HWND);
typedef BOOL (WINAPI *EnumChildWindows_Type)(HWND, WNDENUMPROC, LPARAM);
EnumChildWindows_Type pEnumChildWindows;
BOOL WINAPI extEnumChildWindows(HWND, WNDENUMPROC, LPARAM);
#endif // TRACEWINDOWS

#ifdef TRACEMENUS
typedef HMENU (WINAPI *CreateMenu_Type)(void);
CreateMenu_Type pCreateMenu, pCreatePopupMenu;
HMENU WINAPI extCreateMenu(void);
HMENU WINAPI extCreatePopupMenu(void);
typedef UINT (WINAPI *GetMenuState_Type)(HMENU, UINT, UINT);
GetMenuState_Type pGetMenuState;
UINT WINAPI extGetMenuState(HMENU, UINT, UINT);
typedef BOOL (WINAPI *GetMenuItemInfoA_Type)(HMENU, UINT, BOOL, LPMENUITEMINFOA);
GetMenuItemInfoA_Type pGetMenuItemInfoA;
BOOL WINAPI extGetMenuItemInfoA(HMENU, UINT, BOOL, LPMENUITEMINFOA);
typedef BOOL (WINAPI *GetMenuItemInfoW_Type)(HMENU, UINT, BOOL, LPMENUITEMINFOW);
GetMenuItemInfoW_Type pGetMenuItemInfoW;
BOOL WINAPI extGetMenuItemInfoW(HMENU, UINT, BOOL, LPMENUITEMINFOW);
typedef BOOL (WINAPI *SetMenuItemInfoA_Type)(HMENU, UINT, BOOL, LPCMENUITEMINFOA);
SetMenuItemInfoA_Type pSetMenuItemInfoA;
BOOL WINAPI extSetMenuItemInfoA(HMENU, UINT, BOOL, LPCMENUITEMINFOA);
#endif // TRACEMENUS

typedef HMENU (WINAPI *GetMenu_Type)(HWND);
GetMenu_Type pGetMenu;
HMENU WINAPI extGetMenu(HWND);
typedef BOOL (WINAPI *SetMenu_Type)(HWND, HMENU);
SetMenu_Type pSetMenu;
BOOL WINAPI extSetMenu(HWND, HMENU);
typedef BOOL (WINAPI *SetWindowTextA_Type)(HWND, LPCSTR);
SetWindowTextA_Type pSetWindowTextA;
BOOL WINAPI extSetWindowTextA(HWND, LPCSTR);
typedef int (WINAPI *GetWindowTextA_Type)(HWND, LPSTR, int);
GetWindowTextA_Type pGetWindowTextA;
int WINAPI extGetWindowTextA(HWND, LPSTR, int);
LPSTR WINAPI extCharPrev(LPCSTR, LPCSTR);
LPSTR WINAPI extCharNext(LPCSTR);
typedef BOOL (WINAPI *OemToCharA_Type)(LPCSTR, LPSTR);
OemToCharA_Type pOemToCharA;
BOOL WINAPI extOemToCharA(LPCSTR, LPSTR);
typedef BOOL (WINAPI *CharToOemA_Type)(LPCSTR, LPSTR);
OemToCharA_Type pCharToOemA;
BOOL WINAPI extCharToOemA(LPCSTR, LPSTR);
typedef HRESULT (WINAPI *MessageBoxA_Type)(HWND, LPCSTR, LPCSTR, UINT);
MessageBoxA_Type pMessageBoxA = 0;
HRESULT WINAPI extMessageBoxA(HWND, LPCSTR, LPCSTR, UINT);
typedef HRESULT (WINAPI *MessageBoxW_Type)(HWND, LPCWSTR, LPCWSTR, UINT);
MessageBoxW_Type pMessageBoxW = 0;
HRESULT WINAPI extMessageBoxW(HWND, LPCWSTR, LPCWSTR, UINT);
typedef LRESULT (WINAPI *DefProc_Type)(HWND, UINT, WPARAM, LPARAM);
DefProc_Type pDefFrameProcA;
LRESULT WINAPI extDefFrameProcA(HWND, UINT, WPARAM, LPARAM);
LRESULT WINAPI extDefMDIChildProcA(HWND, UINT, WPARAM, LPARAM);
LRESULT WINAPI extDefDlgProcA(HWND, UINT, WPARAM, LPARAM);
typedef int (WINAPI *GetClassNameA_Type)(HWND, LPSTR, int);
GetClassNameA_Type pGetClassNameA;
int WINAPI extGetClassNameA(HWND, LPSTR, int);
typedef BOOL (WINAPI *GetClassInfoA_Type)(HINSTANCE, LPCSTR, LPWNDCLASSA);
GetClassInfoA_Type pGetClassInfoA;
BOOL WINAPI extGetClassInfoA(HINSTANCE, LPCSTR, LPWNDCLASSA);
typedef BOOL (WINAPI *UnregisterClassA_Type)(LPCSTR, HINSTANCE);
UnregisterClassA_Type pUnregisterClassA;
BOOL WINAPI extUnregisterClassA(LPCSTR, HINSTANCE);

static HookEntryEx_Type Hooks[] = {
#ifdef HACKVIRTUALKEYS
    {HOOK_IAT_CANDIDATE, 0, "MapVirtualKeyA", (FARPROC)MapVirtualKeyA, (FARPROC *) &pMapVirtualKeyA, (FARPROC)extMapVirtualKeyA},
#endif // HACKVIRTUALKEYS
    {HOOK_HOT_CANDIDATE, 0, "FillRect", (FARPROC)NULL, (FARPROC *) &pFillRect, (FARPROC)extFillRect},
    {HOOK_IAT_CANDIDATE, 0, "UpdateWindow", (FARPROC)UpdateWindow, (FARPROC *) &pUpdateWindow, (FARPROC)extUpdateWindow}, // v2.04.04: needed for "Hide Desktop" option
    {HOOK_HOT_CANDIDATE, 0x25, "ChangeDisplaySettingsA", (FARPROC)ChangeDisplaySettingsA, (FARPROC *) &pChangeDisplaySettingsA, (FARPROC)extChangeDisplaySettingsA},
    {HOOK_HOT_CANDIDATE, 0x26, "ChangeDisplaySettingsExA", (FARPROC)ChangeDisplaySettingsExA, (FARPROC *) &pChangeDisplaySettingsExA, (FARPROC)extChangeDisplaySettingsExA},
    {HOOK_HOT_CANDIDATE, 0x28, "ChangeDisplaySettingsW", (FARPROC)NULL, (FARPROC *) &pChangeDisplaySettingsW, (FARPROC)extChangeDisplaySettingsW}, // ref. by Knights of Honor
    {HOOK_HOT_CANDIDATE, 0x27, "ChangeDisplaySettingsExW", (FARPROC)NULL, (FARPROC *) &pChangeDisplaySettingsExW, (FARPROC)extChangeDisplaySettingsExW},
    {HOOK_HOT_CANDIDATE, 0, "GetMonitorInfoA", (FARPROC)GetMonitorInfoA, (FARPROC *) &pGetMonitorInfoA, (FARPROC)extGetMonitorInfoA},
    {HOOK_HOT_CANDIDATE, 0, "GetMonitorInfoW", (FARPROC)GetMonitorInfoW, (FARPROC *) &pGetMonitorInfoW, (FARPROC)extGetMonitorInfoW},
    {HOOK_HOT_CANDIDATE, 0, "ShowCursor", (FARPROC)ShowCursor, (FARPROC *) &pShowCursor, (FARPROC)extShowCursor},
    {HOOK_IAT_CANDIDATE, 0, "CreateDialogIndirectParamA", (FARPROC)CreateDialogIndirectParamA, (FARPROC *) &pCreateDialogIndirectParamA, (FARPROC)extCreateDialogIndirectParamA},
    {HOOK_IAT_CANDIDATE, 0, "CreateDialogParamA", (FARPROC)CreateDialogParamA, (FARPROC *) &pCreateDialogParam, (FARPROC)extCreateDialogParamA},
    {HOOK_IAT_CANDIDATE, 0, "DialogBoxParamA", (FARPROC)DialogBoxParamA, (FARPROC *) &pDialogBoxParamA, (FARPROC)extDialogBoxParamA},
    {HOOK_HOT_CANDIDATE, 0, "DialogBoxIndirectParamA", (FARPROC)DialogBoxIndirectParamA, (FARPROC *) &pDialogBoxIndirectParamA, (FARPROC)extDialogBoxIndirectParamA},
    {HOOK_HOT_CANDIDATE, 0, "MoveWindow", (FARPROC)MoveWindow, (FARPROC *) &pMoveWindow, (FARPROC)extMoveWindow},
    {HOOK_HOT_CANDIDATE, 0, "SetWindowPos", (FARPROC)SetWindowPos, (FARPROC *) &pSetWindowPos, (FARPROC)extSetWindowPos},
    {HOOK_HOT_CANDIDATE, 0, "EnumDisplaySettingsA", (FARPROC)EnumDisplaySettingsA, (FARPROC *) &pEnumDisplaySettings, (FARPROC)extEnumDisplaySettings},
    {HOOK_IAT_CANDIDATE, 0, "GetClipCursor", (FARPROC)GetClipCursor, (FARPROC *) &pGetClipCursor, (FARPROC)extGetClipCursor},
    {HOOK_HOT_CANDIDATE, 0, "ClipCursor", (FARPROC)ClipCursor, (FARPROC *) &pClipCursor, (FARPROC)extClipCursor},
    // DefWindowProcA is HOOK_HOT_REQUIRED for nls support ....
    {HOOK_HOT_REQUIRED,  0, "DefWindowProcA", (FARPROC)DefWindowProcA, (FARPROC *) &pDefWindowProcA, (FARPROC)extDefWindowProcA},
    {HOOK_IAT_CANDIDATE, 0, "DefWindowProcW", (FARPROC)DefWindowProcW, (FARPROC *) &pDefWindowProcW, (FARPROC)extDefWindowProcW},
    // CreateWindowExA & CreateWindowExW are HOOK_HOT_REQUIRED for nls support ....
    {HOOK_HOT_CANDIDATE, 0, "CreateWindowExA", (FARPROC)CreateWindowExA, (FARPROC *) &pCreateWindowExA, (FARPROC)extCreateWindowExA},
    {HOOK_HOT_CANDIDATE, 0, "CreateWindowExW", (FARPROC)CreateWindowExW, (FARPROC *) &pCreateWindowExW, (FARPROC)extCreateWindowExW},
    {HOOK_IAT_CANDIDATE, 0, "RegisterClassExA", (FARPROC)RegisterClassExA, (FARPROC *) &pRegisterClassExA, (FARPROC)extRegisterClassExA},
    {HOOK_IAT_CANDIDATE, 0, "RegisterClassA", (FARPROC)RegisterClassA, (FARPROC *) &pRegisterClassA, (FARPROC)extRegisterClassA},
    {HOOK_IAT_CANDIDATE, 0, "RegisterClassExW", (FARPROC)RegisterClassExW, (FARPROC *) &pRegisterClassExW, (FARPROC)extRegisterClassExW},
    {HOOK_IAT_CANDIDATE, 0, "RegisterClassW", (FARPROC)RegisterClassW, (FARPROC *) &pRegisterClassW, (FARPROC)extRegisterClassW},
    {HOOK_HOT_CANDIDATE, 0, "GetSystemMetrics", (FARPROC)GetSystemMetrics, (FARPROC *) &pGetSystemMetrics, (FARPROC)extGetSystemMetrics},
    {HOOK_HOT_CANDIDATE, 0, "GetDesktopWindow", (FARPROC)GetDesktopWindow, (FARPROC *) &pGetDesktopWindow, (FARPROC)extGetDesktopWindow},
    {HOOK_IAT_CANDIDATE, 0, "CloseWindow", (FARPROC)NULL, (FARPROC *) &pCloseWindow, (FARPROC)extCloseWindow},
    {HOOK_IAT_CANDIDATE, 0, "DestroyWindow", (FARPROC)NULL, (FARPROC *) &pDestroyWindow, (FARPROC)extDestroyWindow},
    {HOOK_IAT_CANDIDATE, 0, "SetSysColors", (FARPROC)NULL, (FARPROC *) &pSetSysColors, (FARPROC)extSetSysColors},
    {HOOK_HOT_CANDIDATE, 0, "SetWindowLongA", (FARPROC)SetWindowLongA, (FARPROC *) &pSetWindowLongA, (FARPROC)extSetWindowLongA},
    {HOOK_HOT_CANDIDATE, 0, "GetWindowLongA", (FARPROC)GetWindowLongA, (FARPROC *) &pGetWindowLongA, (FARPROC)extGetWindowLongA},
    {HOOK_HOT_CANDIDATE, 0, "SetWindowLongW", (FARPROC)SetWindowLongW, (FARPROC *) &pSetWindowLongW, (FARPROC)extSetWindowLongW},
    {HOOK_HOT_CANDIDATE, 0, "GetWindowLongW", (FARPROC)GetWindowLongW, (FARPROC *) &pGetWindowLongW, (FARPROC)extGetWindowLongW},
    {HOOK_IAT_CANDIDATE, 0, "IsWindowVisible", (FARPROC)IsWindowVisible, (FARPROC *) &pIsWindowVisible, (FARPROC)extIsWindowVisible}, // ref. in dxw.SetClipper, CreateWindowCommon
    {HOOK_IAT_CANDIDATE, 0, "GetTopWindow", (FARPROC)GetTopWindow, (FARPROC *) &pGetTopWindow, (FARPROC)extGetTopWindow},
    // hot by MinHook since v2.03.07
    {HOOK_HOT_CANDIDATE, 0, "SystemParametersInfoA", (FARPROC)SystemParametersInfoA, (FARPROC *) &pSystemParametersInfoA, (FARPROC)extSystemParametersInfoA},
    {HOOK_HOT_CANDIDATE, 0, "SystemParametersInfoW", (FARPROC)SystemParametersInfoW, (FARPROC *) &pSystemParametersInfoW, (FARPROC)extSystemParametersInfoW},
    {HOOK_HOT_CANDIDATE, 0, "BringWindowToTop", (FARPROC)BringWindowToTop, (FARPROC *) &pBringWindowToTop, (FARPROC)extBringWindowToTop},
    {HOOK_HOT_CANDIDATE, 0, "SetForegroundWindow", (FARPROC)SetForegroundWindow, (FARPROC *) &pSetForegroundWindow, (FARPROC)extSetForegroundWindow},
    {HOOK_HOT_CANDIDATE, 0, "ChildWindowFromPoint", (FARPROC)ChildWindowFromPoint, (FARPROC *) &pChildWindowFromPoint, (FARPROC)extChildWindowFromPoint},
    {HOOK_HOT_CANDIDATE, 0, "ChildWindowFromPointEx", (FARPROC)ChildWindowFromPointEx, (FARPROC *) &pChildWindowFromPointEx, (FARPROC)extChildWindowFromPointEx},
    {HOOK_HOT_CANDIDATE, 0, "WindowFromPoint", (FARPROC)WindowFromPoint, (FARPROC *) &pWindowFromPoint, (FARPROC)extWindowFromPoint},
    {HOOK_HOT_REQUIRED,  0, "SetWindowsHookExA", (FARPROC)SetWindowsHookExA, (FARPROC *) &pSetWindowsHookExA, (FARPROC)extSetWindowsHookExA},
    {HOOK_HOT_REQUIRED,  0, "SetWindowsHookExW", (FARPROC)SetWindowsHookExW, (FARPROC *) &pSetWindowsHookExW, (FARPROC)extSetWindowsHookExW},
    {HOOK_HOT_REQUIRED,  0, "UnhookWindowsHookEx", (FARPROC)UnhookWindowsHookEx, (FARPROC *) &pUnhookWindowsHookEx, (FARPROC)extUnhookWindowsHookEx},
    {HOOK_IAT_CANDIDATE, 0, "GetDC", (FARPROC)GetDC, (FARPROC *) &pGDIGetDC, (FARPROC)extGDIGetDC},
    {HOOK_IAT_CANDIDATE, 0, "GetDCEx", (FARPROC)GetDCEx, (FARPROC *) &pGDIGetDCEx, (FARPROC)extGDIGetDCEx},
    {HOOK_IAT_CANDIDATE, 0, "GetWindowDC", (FARPROC)GetWindowDC, (FARPROC *) &pGDIGetWindowDC, (FARPROC)extGDIGetWindowDC},
    {HOOK_IAT_CANDIDATE, 0, "ReleaseDC", (FARPROC)ReleaseDC, (FARPROC *) &pGDIReleaseDC, (FARPROC)extGDIReleaseDC},
    {HOOK_HOT_CANDIDATE, 0, "BeginPaint", (FARPROC)BeginPaint, (FARPROC *) &pBeginPaint, (FARPROC)extBeginPaint},
    {HOOK_HOT_CANDIDATE, 0, "EndPaint", (FARPROC)EndPaint, (FARPROC *) &pEndPaint, (FARPROC)extEndPaint},
    {HOOK_HOT_CANDIDATE, 0, "ScrollDC", (FARPROC)NULL, (FARPROC *) &pScrollDC, (FARPROC)extScrollDC},
    // ShowScrollBar and DrawMenuBar both added to fix the Galapagos menu bar, but with no success !!!!
    {HOOK_HOT_CANDIDATE, 0, "ShowScrollBar", (FARPROC)ShowScrollBar, (FARPROC *) &pShowScrollBar, (FARPROC)extShowScrollBar},
    {HOOK_HOT_CANDIDATE, 0, "DrawMenuBar", (FARPROC)DrawMenuBar, (FARPROC *) &pDrawMenuBar, (FARPROC)extDrawMenuBar},
    {HOOK_HOT_CANDIDATE, 0, "EnumDisplayDevicesA", (FARPROC)EnumDisplayDevicesA, (FARPROC *) &pEnumDisplayDevicesA, (FARPROC)extEnumDisplayDevicesA},
    // EnumDisplayDevicesW used by "Battleground Europe" ...
    {HOOK_HOT_CANDIDATE, 0, "EnumDisplayDevicesW", (FARPROC)EnumDisplayDevicesW, (FARPROC *) &pEnumDisplayDevicesW, (FARPROC)extEnumDisplayDevicesW},
    {HOOK_IAT_CANDIDATE, 0, "EnumWindows", (FARPROC)NULL, (FARPROC *) &pEnumWindows, (FARPROC)extEnumWindows},
    {HOOK_IAT_CANDIDATE, 0, "AdjustWindowRect", (FARPROC)NULL, (FARPROC *) &pAdjustWindowRect, (FARPROC)extAdjustWindowRect},
    {HOOK_IAT_CANDIDATE, 0, "AdjustWindowRectEx", (FARPROC)AdjustWindowRectEx, (FARPROC *) &pAdjustWindowRectEx, (FARPROC)extAdjustWindowRectEx},
    {HOOK_HOT_CANDIDATE, 0, "GetActiveWindow", (FARPROC)GetActiveWindow, (FARPROC *) &pGetActiveWindow, (FARPROC)extGetActiveWindow},
    {HOOK_HOT_CANDIDATE, 0, "GetForegroundWindow", (FARPROC)GetForegroundWindow, (FARPROC *) &pGetForegroundWindow, (FARPROC)extGetForegroundWindow},
    {HOOK_HOT_CANDIDATE, 0, "ShowWindow", (FARPROC)ShowWindow, (FARPROC *) &pShowWindow, (FARPROC)extShowWindow},
    {HOOK_HOT_CANDIDATE, 0, "DeferWindowPos", (FARPROC)DeferWindowPos, (FARPROC *) &pGDIDeferWindowPos, (FARPROC)extDeferWindowPos},
    {HOOK_HOT_CANDIDATE, 0, "CallWindowProcA", (FARPROC)CallWindowProcA, (FARPROC *) &pCallWindowProcA, (FARPROC)extCallWindowProcA},
    {HOOK_HOT_CANDIDATE, 0, "CallWindowProcW", (FARPROC)CallWindowProcW, (FARPROC *) &pCallWindowProcW, (FARPROC)extCallWindowProcW},
#ifdef BYPASSEDAPI
    //{HOOK_IAT_CANDIDATE, 0, "GetWindowPlacement", (FARPROC)NULL, (FARPROC *)&pGetWindowPlacement, (FARPROC)extGetWindowPlacement},
    //{HOOK_IAT_CANDIDATE, 0, "SetWindowPlacement", (FARPROC)NULL, (FARPROC *)&pSetWindowPlacement, (FARPROC)extSetWindowPlacement},
    //{HOOK_IAT_CANDIDATE, 0, "GetWindowTextA", (FARPROC)GetWindowTextA, (FARPROC *)&pGetWindowTextA, (FARPROC)extGetWindowTextA},
    //{HOOK_HOT_CANDIDATE, 0, "EnumDisplayMonitors", (FARPROC)EnumDisplayMonitors, (FARPROC *)&pEnumDisplayMonitors, (FARPROC)extEnumDisplayMonitors},
#endif // BYPASSEDAPI
#ifdef TRACEWAITOBJECTS
    {HOOK_IAT_CANDIDATE, 0, "MsgWaitForMultipleObjects", (FARPROC)MsgWaitForMultipleObjects, (FARPROC *) &pMsgWaitForMultipleObjects, (FARPROC)extMsgWaitForMultipleObjects},
#endif // TRACEWAITOBJECTS
#ifdef TRACESYSCALLS
    {HOOK_IAT_CANDIDATE, 0, "SetCapture", (FARPROC)SetCapture, (FARPROC *) &pSetCapture, (FARPROC)extSetCapture},
    {HOOK_IAT_CANDIDATE, 0, "ReleaseCapture", (FARPROC)ReleaseCapture, (FARPROC *) &pReleaseCapture, (FARPROC)extReleaseCapture},
    {HOOK_IAT_CANDIDATE, 0, "FindWindowA", (FARPROC)FindWindowA, (FARPROC *) &pFindWindowA, (FARPROC)extFindWindowA},
    {HOOK_IAT_CANDIDATE, 0, "SetActiveWindow", (FARPROC)SetActiveWindow, (FARPROC *) &pSetActiveWindow, (FARPROC)extSetActiveWindow},
    {HOOK_IAT_CANDIDATE, 0, "IsWindow", (FARPROC)IsWindow, (FARPROC *) &pIsWindow, (FARPROC)extIsWindow},
    {HOOK_IAT_CANDIDATE, 0, "IsWindowEnabled", (FARPROC)IsWindowEnabled, (FARPROC *) &pIsWindowEnabled, (FARPROC)extIsWindowEnabled},
    {HOOK_IAT_CANDIDATE, 0, "IsZoomed", (FARPROC)NULL, (FARPROC *) &pIsZoomed, (FARPROC)extIsZoomed},
    {HOOK_HOT_CANDIDATE, 0, "IsIconic", (FARPROC)IsIconic, (FARPROC *) &pIsIconic, (FARPROC)extIsIconic},
    {HOOK_HOT_CANDIDATE, 0, "GetAncestor", (FARPROC)GetAncestor, (FARPROC *) &pGetAncestor, (FARPROC)extGetAncestor},
    {HOOK_IAT_CANDIDATE, 0, "LoadBitmapA", (FARPROC)NULL, (FARPROC *) &pLoadBitmapA, (FARPROC)extLoadBitmapA},
    {HOOK_HOT_CANDIDATE, 0, "GetWindow", (FARPROC)GetWindow, (FARPROC *) &pGetWindow, (FARPROC)extGetWindow},
    {HOOK_HOT_CANDIDATE, 0, "GetWindowThreadProcessId", (FARPROC)GetWindowThreadProcessId, (FARPROC *) &pGetWindowThreadProcessId, (FARPROC)extGetWindowThreadProcessId},
    {HOOK_HOT_CANDIDATE, 0, "SetScrollInfo", (FARPROC)SetScrollInfo, (FARPROC *) &pSetScrollInfo, (FARPROC)extSetScrollInfo},
    {HOOK_HOT_CANDIDATE, 0, "GetScrollInfo", (FARPROC)GetScrollInfo, (FARPROC *) &pGetScrollInfo, (FARPROC)extGetScrollInfo},
    {HOOK_HOT_CANDIDATE, 0, "SetScrollPos", (FARPROC)SetScrollPos, (FARPROC *) &pSetScrollPos, (FARPROC)extSetScrollPos},
    {HOOK_HOT_CANDIDATE, 0, "GetScrollPos", (FARPROC)GetScrollPos, (FARPROC *) &pGetScrollPos, (FARPROC)extGetScrollPos},
    {HOOK_HOT_CANDIDATE, 0, "SetScrollRange", (FARPROC)SetScrollRange, (FARPROC *) &pSetScrollRange, (FARPROC)extSetScrollRange},
    {HOOK_HOT_CANDIDATE, 0, "GetScrollRange", (FARPROC)GetScrollRange, (FARPROC *) &pGetScrollRange, (FARPROC)extGetScrollRange},
    {HOOK_IAT_CANDIDATE, 0, "TranslateMessage", (FARPROC)TranslateMessage, (FARPROC *) &pTranslateMessage, (FARPROC)extTranslateMessage},
    {HOOK_IAT_CANDIDATE, 0, "IsDialogMessageA", (FARPROC)IsDialogMessageA, (FARPROC *) &pIsDialogMessageA, (FARPROC)extIsDialogMessageA},
    {HOOK_IAT_CANDIDATE, 0, "SetCursor", (FARPROC)SetCursor, (FARPROC *) &pSetCursor, (FARPROC)extSetCursor},
#endif // TRACESYSCALLS
#ifdef TRACEPOINTS
    {HOOK_IAT_CANDIDATE, 0, "SetRect", (FARPROC)SetRect, (FARPROC *) &pSetRect, (FARPROC)extSetRect},
    {HOOK_IAT_CANDIDATE, 0, "PtInRect", (FARPROC)PtInRect, (FARPROC *) &pPtInRect, (FARPROC)extPtInRect},
    {HOOK_IAT_CANDIDATE, 0, "SetRectEmpty", (FARPROC)SetRectEmpty, (FARPROC *) &pSetRectEmpty, (FARPROC)extSetRectEmpty},
    {HOOK_IAT_CANDIDATE, 0, "OffsetRect", (FARPROC)OffsetRect, (FARPROC *) &pOffsetRect, (FARPROC)extOffsetRect},
    {HOOK_IAT_CANDIDATE, 0, "InflateRect", (FARPROC)InflateRect, (FARPROC *) &pInflateRect, (FARPROC)extInflateRect},
    {HOOK_IAT_CANDIDATE, 0, "IntersectRect", (FARPROC)IntersectRect, (FARPROC *) &pIntersectRect, (FARPROC)extIntersectRect},
    {HOOK_IAT_CANDIDATE, 0, "UnionRect", (FARPROC)UnionRect, (FARPROC *) &pUnionRect, (FARPROC)extUnionRect},
    {HOOK_IAT_CANDIDATE, 0, "SubtractRect", (FARPROC)SubtractRect, (FARPROC *) &pSubtractRect, (FARPROC)extSubtractRect},
    {HOOK_IAT_CANDIDATE, 0, "CopyRect", (FARPROC)CopyRect, (FARPROC *) &pCopyRect, (FARPROC)extCopyRect},
    {HOOK_IAT_CANDIDATE, 0, "IsRectEmpty", (FARPROC)IsRectEmpty, (FARPROC *) &pIsRectEmpty, (FARPROC)extIsRectEmpty},
#endif // TRACEPOINTS
#ifdef TRACEWINDOWS
    {HOOK_IAT_CANDIDATE, 0, "EnableWindow", (FARPROC)EnableWindow, (FARPROC *) &pEnableWindow, (FARPROC)extEnableWindow},
    {HOOK_HOT_CANDIDATE, 0, "GetFocus", (FARPROC)GetFocus, (FARPROC *) &pGetFocus, (FARPROC)extGetFocus},
    {HOOK_HOT_CANDIDATE, 0, "SetFocus", (FARPROC)SetFocus, (FARPROC *) &pSetFocus, (FARPROC)extSetFocus},
    {HOOK_HOT_CANDIDATE, 0, "EnumChildWindows", (FARPROC)EnumChildWindows, (FARPROC *) &pEnumChildWindows, (FARPROC)extEnumChildWindows},
#endif // TRACEWINDOWS
#ifdef TRACEMENUS
    {HOOK_IAT_CANDIDATE, 0, "CreateMenu", (FARPROC)CreateMenu, (FARPROC *) &pCreateMenu, (FARPROC)extCreateMenu},
    {HOOK_IAT_CANDIDATE, 0, "CreatePopupMenu", (FARPROC)CreatePopupMenu, (FARPROC *) &pCreatePopupMenu, (FARPROC)extCreatePopupMenu},
    {HOOK_IAT_CANDIDATE, 0, "GetMenuState", (FARPROC)GetMenuState, (FARPROC *) &pGetMenuState, (FARPROC)extGetMenuState},
    {HOOK_IAT_CANDIDATE, 0, "GetMenuItemInfoA", (FARPROC)GetMenuItemInfoA, (FARPROC *) &pGetMenuItemInfoA, (FARPROC)extGetMenuItemInfoA},
    {HOOK_IAT_CANDIDATE, 0, "GetMenuItemInfoW", (FARPROC)GetMenuItemInfoW, (FARPROC *) &pGetMenuItemInfoW, (FARPROC)extGetMenuItemInfoW},
    {HOOK_IAT_CANDIDATE, 0, "SetMenuItemInfoA", (FARPROC)SetMenuItemInfoA, (FARPROC *) &pSetMenuItemInfoA, (FARPROC)extSetMenuItemInfoA},
#endif // TRACEMENUS
    {HOOK_IAT_CANDIDATE, 0, "GetMenu", (FARPROC)GetMenu, (FARPROC *) &pGetMenu, (FARPROC)extGetMenu},
    {HOOK_IAT_CANDIDATE, 0, "SetMenu", (FARPROC)SetMenu, (FARPROC *) &pSetMenu, (FARPROC)extSetMenu},
    {HOOK_IAT_CANDIDATE, 0, "GetAsyncKeyState", (FARPROC)GetAsyncKeyState, (FARPROC *) &pGetAsyncKeyState, (FARPROC)extGetAsyncKeyState},
    {HOOK_IAT_CANDIDATE, 0, 0, NULL, 0, 0} // terminator
};

static HookEntryEx_Type RawInputHooks[] = {
    {HOOK_HOT_REQUIRED,  0, "GetRawInputData", (FARPROC)GetRawInputData, (FARPROC *) &pGetRawInputData, (FARPROC)extGetRawInputData},
    {HOOK_HOT_REQUIRED,  0, "GetRawInputBuffer", (FARPROC)GetRawInputBuffer, (FARPROC *) &pGetRawInputBuffer, (FARPROC)extGetRawInputBuffer},
    {HOOK_HOT_REQUIRED,  0, "RegisterRawInputDevices", (FARPROC)RegisterRawInputDevices, (FARPROC *) &pRegisterRawInputDevices, (FARPROC)extRegisterRawInputDevices},
    {HOOK_HOT_REQUIRED,  0, "GetRawInputDeviceInfoA", (FARPROC)GetRawInputDeviceInfoA, (FARPROC *) &pGetRawInputDeviceInfoA, (FARPROC)extGetRawInputDeviceInfoA},
    {HOOK_HOT_REQUIRED,  0, "GetRawInputDeviceInfoW", (FARPROC)GetRawInputDeviceInfoW, (FARPROC *) &pGetRawInputDeviceInfoW, (FARPROC)extGetRawInputDeviceInfoW},
    {HOOK_HOT_REQUIRED,  0, "GetRawInputDeviceList", (FARPROC)GetRawInputDeviceList, (FARPROC *) &pGetRawInputDeviceList, (FARPROC)extGetRawInputDeviceList},
    {HOOK_IAT_CANDIDATE, 0, 0, NULL, 0, 0} // terminator
};

static HookEntryEx_Type RemapHooks[] = {
    {HOOK_HOT_CANDIDATE, 0, "ScreenToClient", (FARPROC)ScreenToClient, (FARPROC *) &pScreenToClient, (FARPROC)extScreenToClient},
    {HOOK_HOT_CANDIDATE, 0, "ClientToScreen", (FARPROC)ClientToScreen, (FARPROC *) &pClientToScreen, (FARPROC)extClientToScreen},
    {HOOK_HOT_CANDIDATE, 0, "GetClientRect", (FARPROC)GetClientRect, (FARPROC *) &pGetClientRect, (FARPROC)extGetClientRect},
    {HOOK_HOT_CANDIDATE, 0, "GetWindowRect", (FARPROC)GetWindowRect, (FARPROC *) &pGetWindowRect, (FARPROC)extGetWindowRect},
    {HOOK_HOT_CANDIDATE, 0, "MapWindowPoints", (FARPROC)MapWindowPoints, (FARPROC *) &pMapWindowPoints, (FARPROC)extMapWindowPoints},
    {HOOK_HOT_CANDIDATE, 0, "GetUpdateRgn", (FARPROC)GetUpdateRgn, (FARPROC *) &pGetUpdateRgn, (FARPROC)extGetUpdateRgn},
    {HOOK_IAT_CANDIDATE, 0, "GetUpdateRect", (FARPROC)GetUpdateRect, (FARPROC *) &pGetUpdateRect, (FARPROC)extGetUpdateRect},
    {HOOK_IAT_CANDIDATE, 0, "RedrawWindow", (FARPROC)RedrawWindow, (FARPROC *) &pRedrawWindow, (FARPROC)extRedrawWindow},
    {HOOK_HOT_CANDIDATE, 0, "InvalidateRect", (FARPROC)InvalidateRect, (FARPROC *) &pInvalidateRect, (FARPROC)extInvalidateRect},
    {HOOK_HOT_CANDIDATE, 0, "SetWindowRgn", (FARPROC)SetWindowRgn, (FARPROC *) &pSetWindowRgn, (FARPROC)extSetWindowRgn},
    {HOOK_HOT_CANDIDATE, 0, "GetWindowRgn", (FARPROC)GetWindowRgn, (FARPROC *) &pGetWindowRgn, (FARPROC)extGetWindowRgn},
    {HOOK_IAT_CANDIDATE, 0, 0, NULL, 0, 0} // terminator
};

static HookEntryEx_Type SyscallHooks[] = {
    {HOOK_IAT_CANDIDATE, 0, "FrameRect", (FARPROC)FrameRect, (FARPROC *) &pFrameRect, (FARPROC)extFrameRect},
    {HOOK_HOT_CANDIDATE, 0, "InvalidateRgn", (FARPROC)InvalidateRgn, (FARPROC *) &pInvalidateRgn, (FARPROC)extInvalidateRgn},
    {HOOK_IAT_CANDIDATE, 0, "TabbedTextOutA", (FARPROC)TabbedTextOutA, (FARPROC *) &pTabbedTextOutA, (FARPROC)extTabbedTextOutA},
    {HOOK_IAT_CANDIDATE, 0, "TabbedTextOutW", (FARPROC)TabbedTextOutW, (FARPROC *) &pTabbedTextOutW, (FARPROC)extTabbedTextOutW},
    {HOOK_IAT_CANDIDATE, 0, "DrawTextA", (FARPROC)DrawTextA, (FARPROC *) &pDrawTextA, (FARPROC)extDrawTextA},
    {HOOK_IAT_CANDIDATE, 0, "DrawTextExA", (FARPROC)DrawTextExA, (FARPROC *) &pDrawTextExA, (FARPROC)extDrawTextExA},
    {HOOK_IAT_CANDIDATE, 0, "DrawTextW", (FARPROC)DrawTextW, (FARPROC *) &pDrawTextW, (FARPROC)extDrawTextW},
    {HOOK_IAT_CANDIDATE, 0, "DrawTextExW", (FARPROC)DrawTextExW, (FARPROC *) &pDrawTextExW, (FARPROC)extDrawTextExW},
    {HOOK_HOT_CANDIDATE, 0, "InvertRect", (FARPROC)NULL, (FARPROC *) &pInvertRect, (FARPROC)extInvertRect},
    {HOOK_HOT_CANDIDATE, 0, "DrawIcon", (FARPROC)NULL, (FARPROC *) &pDrawIcon, (FARPROC)extDrawIcon},
    {HOOK_IAT_CANDIDATE, 0, "DrawIconEx", (FARPROC)NULL, (FARPROC *) &pDrawIconEx, (FARPROC)extDrawIconEx},
    {HOOK_HOT_CANDIDATE, 0, "DrawCaption", (FARPROC)NULL, (FARPROC *) &pDrawCaption, (FARPROC)extDrawCaption},
    {HOOK_HOT_CANDIDATE, 0, "DrawEdge", (FARPROC)NULL, (FARPROC *) &pDrawEdge, (FARPROC)extDrawEdge},
    //TODO {HOOK_HOT_CANDIDATE, 0, "DrawFocusRect", (FARPROC)NULL, (FARPROC *)&pDrawFocusRect, (FARPROC)extDrawFocusRect},
    //TODO {HOOK_HOT_CANDIDATE, 0, "DrawFrameControl", (FARPROC)NULL, (FARPROC *)&pDrawFrameControl, (FARPROC)extDrawFrameControl},
    //TODO {HOOK_HOT_CANDIDATE, 0, "DrawStateA", (FARPROC)NULL, (FARPROC *)&pDrawStateA, (FARPROC)extDrawStateA},
    //TODO {HOOK_HOT_CANDIDATE, 0, "DrawStateW", (FARPROC)NULL, (FARPROC *)&pDrawStateW, (FARPROC)extDrawStateW},
    //TODO {HOOK_HOT_CANDIDATE, 0, "GrayStringA", (FARPROC)NULL, (FARPROC *)&pGrayStringA, (FARPROC)extGrayStringA},
    //TODO {HOOK_HOT_CANDIDATE, 0, "GrayStringW", (FARPROC)NULL, (FARPROC *)&pGrayStringW, (FARPROC)extGrayStringW},
    //TODO {HOOK_HOT_CANDIDATE, 0, "PaintDesktop", (FARPROC)NULL, (FARPROC *)&pPaintDesktop, (FARPROC)extPaintDesktop},
    {HOOK_IAT_CANDIDATE, 0, 0, NULL, 0, 0} // terminator
};

static HookEntryEx_Type ScaledHooks[] = {
    {HOOK_HOT_CANDIDATE, 0, "ValidateRect", (FARPROC)ValidateRect, (FARPROC *) &pValidateRect, (FARPROC)extValidateRect},
    {HOOK_HOT_CANDIDATE, 0, "ValidateRgn", (FARPROC)ValidateRgn, (FARPROC *) &pValidateRgn, (FARPROC)extValidateRgn},
    {HOOK_IAT_CANDIDATE, 0, "ScrollWindow", (FARPROC)ScrollWindow, (FARPROC *) &pScrollWindow, (FARPROC)extScrollWindow},
    {HOOK_IAT_CANDIDATE, 0, 0, NULL, 0, 0} // terminator
};

static HookEntryEx_Type MouseHooks[] = {
    {HOOK_HOT_CANDIDATE, 0, "GetCursorPos", (FARPROC)GetCursorPos, (FARPROC *) &pGetCursorPos, (FARPROC)extGetCursorPos},
    {HOOK_HOT_CANDIDATE, 0, "SetCursorPos", (FARPROC)SetCursorPos, (FARPROC *) &pSetCursorPos, (FARPROC)extSetCursorPos},
    {HOOK_IAT_CANDIDATE, 0, "GetCursorInfo", (FARPROC)GetCursorInfo, (FARPROC *) &pGetCursorInfo, (FARPROC)extGetCursorInfo},
    {HOOK_IAT_CANDIDATE, 0, "SendMessageA", (FARPROC)SendMessageA, (FARPROC *) &pSendMessageA, (FARPROC)extSendMessageA},
    {HOOK_IAT_CANDIDATE, 0, "SendMessageW", (FARPROC)SendMessageW, (FARPROC *) &pSendMessageW, (FARPROC)extSendMessageW},
    {HOOK_HOT_REQUIRED,  0, "mouse_event", (FARPROC)mouse_event, (FARPROC *) &pmouse_event, (FARPROC)extmouse_event},
    //{HOOK_IAT_CANDIDATE, 0, "SetPhysicalCursorPos", NULL, (FARPROC *)&pSetCursor, (FARPROC)extSetCursor}, // ???
    {HOOK_HOT_CANDIDATE, 0, "SendInput", (FARPROC)SendInput, (FARPROC *) &pSendInput, (FARPROC)extSendInput},
    {HOOK_HOT_CANDIDATE, 0, "GetMessagePos", (FARPROC)GetMessagePos, (FARPROC *) &pGetMessagePos, (FARPROC)extGetMessagePos},
    {HOOK_IAT_CANDIDATE, 0, 0, NULL, 0, 0} // terminator
};

static HookEntryEx_Type TimeHooks[] = {
    {HOOK_IAT_CANDIDATE, 0, "SetTimer", (FARPROC)SetTimer, (FARPROC *) &pSetTimer, (FARPROC)extSetTimer},
    {HOOK_IAT_CANDIDATE, 0, "KillTimer", (FARPROC)KillTimer, (FARPROC *) &pKillTimer, (FARPROC)extKillTimer},
    {HOOK_IAT_CANDIDATE, 0, 0, NULL, 0, 0} // terminator
};

static HookEntryEx_Type DesktopHooks[] = { // currently unused, needed for X-Files
    {HOOK_IAT_CANDIDATE, 0, "CreateDesktopA", (FARPROC)CreateDesktopA, (FARPROC *) &pCreateDesktop, (FARPROC)extCreateDesktop},
    {HOOK_IAT_CANDIDATE, 0, "SwitchDesktop", (FARPROC)SwitchDesktop, (FARPROC *) &pSwitchDesktop, (FARPROC)extSwitchDesktop},
    {HOOK_IAT_CANDIDATE, 0, "OpenDesktopA", (FARPROC)OpenDesktopA, (FARPROC *) &pOpenDesktop, (FARPROC)extOpenDesktop},
    {HOOK_IAT_CANDIDATE, 0, "CloseDesktop", (FARPROC)CloseDesktop, (FARPROC *) &pCloseDesktop, (FARPROC)extCloseDesktop},
    {HOOK_IAT_CANDIDATE, 0, 0, NULL, 0, 0} // terminator
};

static HookEntryEx_Type MsgLoopHooks[] = {
    {HOOK_HOT_CANDIDATE, 0, "PeekMessageA", (FARPROC)PeekMessageA, (FARPROC *) &pPeekMessageA, (FARPROC)extPeekMessageA},
    {HOOK_HOT_CANDIDATE, 0, "PeekMessageW", (FARPROC)PeekMessageW, (FARPROC *) &pPeekMessageW, (FARPROC)extPeekMessageW},
    {HOOK_HOT_CANDIDATE, 0, "GetMessageA", (FARPROC)GetMessageA, (FARPROC *) &pGetMessageA, (FARPROC)extGetMessageA},
    {HOOK_HOT_CANDIDATE, 0, "GetMessageW", (FARPROC)GetMessageW, (FARPROC *) &pGetMessageW, (FARPROC)extGetMessageW},
    {HOOK_IAT_CANDIDATE, 0, "PostMessageA", (FARPROC)PostMessageA, (FARPROC *) &pPostMessageA, (FARPROC)extPostMessageA},
    {HOOK_IAT_CANDIDATE, 0, "PostMessageW", (FARPROC)PostMessageW, (FARPROC *) &pPostMessageW, (FARPROC)extPostMessageW},
    {HOOK_IAT_CANDIDATE, 0, 0, NULL, 0, 0} // terminator
};

static HookEntryEx_Type MessageBoxHooks[] = {
    {HOOK_HOT_REQUIRED, 0, "MessageBoxTimeoutA", (FARPROC)NULL, (FARPROC *) &pMessageBoxTimeoutA, (FARPROC)extMessageBoxTimeoutA},
    {HOOK_HOT_REQUIRED, 0, "MessageBoxTimeoutW", (FARPROC)NULL, (FARPROC *) &pMessageBoxTimeoutW, (FARPROC)extMessageBoxTimeoutW},
    {HOOK_IAT_CANDIDATE, 0, 0, NULL, 0, 0} // terminator
};

static HookEntryEx_Type FakeIOHooks[] = {
    {HOOK_IAT_CANDIDATE, 0, "SendMessageA", (FARPROC)SendMessageA, (FARPROC *) &pSendMessageA, (FARPROC)extSendMessageA},
    {HOOK_IAT_CANDIDATE, 0, "LoadImageA", (FARPROC)LoadImageA, (FARPROC *) &pLoadImageA, (FARPROC)extLoadImageA},
    {HOOK_IAT_CANDIDATE, 0, 0, NULL, 0, 0} // terminator
};

static HookEntryEx_Type NlsHooks[] = {
    {HOOK_HOT_REQUIRED, 0, "CharPrev", (FARPROC)CharPrev, (FARPROC *)NULL, (FARPROC)extCharPrev},
    {HOOK_HOT_REQUIRED, 0, "CharNext", (FARPROC)CharNext, (FARPROC *)NULL, (FARPROC)extCharNext}, //
    {HOOK_HOT_REQUIRED, 0, "SetWindowTextA", (FARPROC)SetWindowTextA, (FARPROC *) &pSetWindowTextA, (FARPROC)extSetWindowTextA},
    {HOOK_HOT_REQUIRED, 0, "GetWindowTextA", (FARPROC)GetWindowTextA, (FARPROC *) &pGetWindowTextA, (FARPROC)extGetWindowTextA},
    //	{HOOK_HOT_REQUIRED, 0, "DefFrameProcA", (FARPROC)DefFrameProcA, (FARPROC *)&pDefFrameProcA, (FARPROC)extDefFrameProcA},
    //	{HOOK_HOT_REQUIRED, 0, "DefMDIChildProcA", (FARPROC)DefMDIChildProcA, (FARPROC *)NULL, (FARPROC)extDefMDIChildProcA},
    //	{HOOK_HOT_REQUIRED, 0, "DefDlgProcA", (FARPROC)DefDlgProcA, (FARPROC *)NULL, (FARPROC)extDefDlgProcA},
    {HOOK_HOT_REQUIRED, 0, "GetClassNameA", (FARPROC)GetClassNameA, (FARPROC *) &pGetClassNameA, (FARPROC)extGetClassNameA},
    {HOOK_HOT_REQUIRED, 0, "GetClassInfoA", (FARPROC)GetClassInfoA, (FARPROC *) &pGetClassInfoA, (FARPROC)extGetClassInfoA},
    {HOOK_HOT_REQUIRED, 0, "UnregisterClassA", (FARPROC)UnregisterClassA, (FARPROC *) &pUnregisterClassA, (FARPROC)extUnregisterClassA},
    // tracing only ....
    {HOOK_HOT_REQUIRED, 0, "OemToCharA", (FARPROC)OemToCharA, (FARPROC *) &pOemToCharA, (FARPROC)extOemToCharA},
    {HOOK_HOT_REQUIRED, 0, "CharToOemA", (FARPROC)CharToOemA, (FARPROC *) &pCharToOemA, (FARPROC)extCharToOemA},
    {HOOK_HOT_REQUIRED, 0, "MessageBoxTimeoutA", (FARPROC)NULL, (FARPROC *) &pMessageBoxTimeoutA, (FARPROC)extMessageBoxTimeoutA},
    {HOOK_HOT_REQUIRED, 0, "MessageBoxA", (FARPROC)MessageBoxA, (FARPROC *) &pMessageBoxA, (FARPROC)extMessageBoxA},
    {HOOK_IAT_CANDIDATE, 0, 0, NULL, 0, 0} // terminator
};

static char *libname = "user32.dll";

void HookUser32(HMODULE hModule) {
    HookLibraryEx(hModule, Hooks, libname);
    HookLibraryEx(hModule, MsgLoopHooks, libname);
    if (dxw.GDIEmulationMode != GDIMODE_NONE) HookLibraryEx(hModule, SyscallHooks, libname);
    if (dxw.dwFlags2 & GDISTRETCHED)	HookLibraryEx(hModule, ScaledHooks, libname);
    if (dxw.dwFlags1 & CLIENTREMAPPING) HookLibraryEx(hModule, RemapHooks, libname);
    if ((dxw.dwFlags1 & (MODIFYMOUSE | SLOWDOWN | KEEPCURSORWITHIN)) || (dxw.dwFlags2 & KEEPCURSORFIXED)) HookLibraryEx(hModule, MouseHooks, libname);
    if (dxw.dwFlags2 & TIMESTRETCH) HookLibraryEx(hModule, TimeHooks, libname);
    if (dxw.dwFlags9 & NODIALOGS) HookLibraryEx(hModule, MessageBoxHooks, libname);
    if (dxw.dwFlags10 & FIXMOUSERAWINPUT) HookLibraryEx(hModule, RawInputHooks, libname);
    if (dxw.dwFlags10 & (FAKECDDRIVE | FAKEHDDRIVE)) HookLibraryEx(hModule, FakeIOHooks, libname);
    if (dxw.dwFlags11 & CUSTOMLOCALE) HookLibraryEx(hModule, NlsHooks, libname);
    IsChangeDisplaySettingsHotPatched = IsHotPatchedEx(Hooks, "ChangeDisplaySettingsExA") || IsHotPatchedEx(Hooks, "ChangeDisplaySettingsExW");
    return;
}

void HookUser32Init() {
    HookLibInitEx(Hooks);
    HookLibInitEx(SyscallHooks);
    HookLibInitEx(ScaledHooks);
    HookLibInitEx(RemapHooks);
    HookLibInitEx(MouseHooks);
}

FARPROC Remap_user32_ProcAddress(LPCSTR proc, HMODULE hModule) {
    FARPROC addr;
    if (addr = RemapLibraryEx(proc, hModule, Hooks)) return addr;
    if (addr = RemapLibraryEx(proc, hModule, MsgLoopHooks)) return addr;
    if (dxw.dwFlags1 & CLIENTREMAPPING)
        if (addr = RemapLibraryEx(proc, hModule, RemapHooks)) return addr;
    if (dxw.GDIEmulationMode != GDIMODE_NONE)
        if(addr = RemapLibraryEx(proc, hModule, SyscallHooks)) return addr;
    if (dxw.dwFlags2 & GDISTRETCHED)
        if (addr = RemapLibraryEx(proc, hModule, ScaledHooks)) return addr;
    if ((dxw.dwFlags1 & (MODIFYMOUSE | SLOWDOWN | KEEPCURSORWITHIN)) || (dxw.dwFlags2 & KEEPCURSORFIXED))
        if (addr = RemapLibraryEx(proc, hModule, MouseHooks)) return addr;
    if((dxw.dwFlags2 & TIMESTRETCH) && (dxw.dwFlags4 & STRETCHTIMERS))
        if (addr = RemapLibraryEx(proc, hModule, TimeHooks)) return addr;
    if(dxw.dwFlags10 & FIXMOUSERAWINPUT)
        if (addr = RemapLibraryEx(proc, hModule, RawInputHooks)) return addr;
    if (dxw.dwFlags10 & (FAKECDDRIVE | FAKEHDDRIVE))
        if (addr = RemapLibraryEx(proc, hModule, FakeIOHooks)) return addr;
    if (dxw.dwFlags11 & CUSTOMLOCALE)
        if (addr = RemapLibraryEx(proc, hModule, NlsHooks)) return addr;
    return NULL;
}

/* ------------------------------------------------------------------------------ */
// auxiliary (static) functions
/* ------------------------------------------------------------------------------ */

static void Stopper(char *s, int line) {
    char sMsg[81];
    sprintf(sMsg, "break: \"%s\"", s);
    MessageBox(0, sMsg, "break", MB_OK | MB_ICONEXCLAMATION);
}

//#define STOPPER_TEST // comment out to eliminate
#ifdef STOPPER_TEST
#define STOPPER(s) Stopper(s, __LINE__)
#else
#define STOPPER(s)
#endif

static LPCSTR sTemplateName(LPCSTR tn) {
    static char sBuf[20 + 1];
    if((DWORD)tn >> 16)
        return tn;
    else {
        sprintf(sBuf, "ID:(%#x)", ((DWORD)tn & 0x0000FFFF));
        return sBuf;
    }
}

// --------------------------------------------------------------------------
//
// globals, externs, static functions...
//
// --------------------------------------------------------------------------

// PrimHDC: DC handle of the selected DirectDraw primary surface. NULL when invalid.
HDC PrimHDC = NULL;

LPRECT lpClipRegion = NULL;
RECT ClipRegion;
int LastCurPosX, LastCurPosY;

extern GetDC_Type pGetDC;
extern ReleaseDC_Type pReleaseDC1;
extern HRESULT WINAPI sBlt(int, Blt_Type, char *, LPDIRECTDRAWSURFACE, LPRECT, LPDIRECTDRAWSURFACE, LPRECT, DWORD, LPDDBLTFX, BOOL);


LONG WINAPI MyChangeDisplaySettings(char *fname, BOOL WideChar, void *lpDevMode, DWORD dwflags) {
    HRESULT res;
    DWORD dmFields, dmBitsPerPel, dmPelsWidth, dmPelsHeight;
    if(dwflags & CDS_TEST) {
        OutTraceDW("%s: TEST res=DISP_CHANGE_SUCCESSFUL\n", fname);
        return DISP_CHANGE_SUCCESSFUL;
    }
    // v2.02.32: reset the emulated DC used in GDIEMULATEDC mode
    dxw.ResetEmulatedDC();
    if(lpDevMode) {
        if(WideChar) {
            dmFields = ((DEVMODEW *)lpDevMode)->dmFields;
            dmPelsWidth = ((DEVMODEW *)lpDevMode)->dmPelsWidth;
            dmPelsHeight = ((DEVMODEW *)lpDevMode)->dmPelsHeight;
            dmBitsPerPel = ((DEVMODEW *)lpDevMode)->dmBitsPerPel;
        } else {
            dmFields = ((DEVMODEA *)lpDevMode)->dmFields;
            dmPelsWidth = ((DEVMODEA *)lpDevMode)->dmPelsWidth;
            dmPelsHeight = ((DEVMODEA *)lpDevMode)->dmPelsHeight;
            dmBitsPerPel = ((DEVMODEA *)lpDevMode)->dmBitsPerPel;
        }
    } else {
        if(dwflags == 0) {
            // fake default registry settings
            dmFields = DM_PELSWIDTH | DM_PELSHEIGHT | DM_BITSPERPEL;
            dmPelsWidth = dxw.dwDefaultScreenWidth;
            dmPelsHeight = dxw.dwDefaultScreenHeight;
            dmBitsPerPel = dxw.dwDefaultColorDepth;
            lpDevMode = (void *) - 1; // any not null value to avoid if-conditions below
        }
    }
    // v2.04.37: do not change primary device settings while in Windowed mode. Fixes "Fastlane Pinball"
    if(dwflags & CDS_SET_PRIMARY) dwflags &= ~CDS_SET_PRIMARY;
    // v2.01.89: save desired settings first
    // v2.01.95: protect when lpDevMode is null (closing game... Jedi Outcast)
    // v2.02.23: consider new width/height only when dmFields flags are set.
    if(lpDevMode && (dmFields & (DM_PELSWIDTH | DM_PELSHEIGHT))) {
        BOOL bGoFullscreen;
        dxw.SetScreenSize(dmPelsWidth, dmPelsHeight);
        // v2.02.31: when main win is bigger that expected resolution, you're in windowed fullscreen mode
        // v2.04.71: if main window is not defined yet, go FULLSCREEN in any case
        if(dxw.GethWnd()) {
            RECT client;
            (*pGetClientRect)(dxw.GethWnd(), &client);
            OutTraceDW("%s: current hWnd=%#x size=(%d,%d)\n", fname, dxw.GethWnd(), client.right, client.bottom);
            bGoFullscreen = ((client.right >= (LONG)dmPelsWidth) && (client.bottom >= (LONG)dmPelsHeight));
        } else
            bGoFullscreen = TRUE;
        if(bGoFullscreen) {
            OutTraceDW("%s: entering FULLSCREEN mode\n", fname);
            dxw.SetFullScreen(TRUE);
        } else
            OutTraceDW("%s: current mode: %s\n", fname, dxw.IsFullScreen() ? "FULLSCREEN" : "WINDOWED");
    }
    // v2.04.90: suppress CDS_RESET flag, that can force a video mode change also without CDS_FULLSCREEN
    // specification. Fixes "LEGO Racers" between intro movies and game main menu panel.
    dwflags &= ~CDS_RESET;
    // v2.03.61: bypass display mode changes also for CDS_UPDATEREGISTRY flag
    // used by "Severance: Blade of Darkness" OpenGL renderer
    // v2.04.90: CDS_FULLSCREEN could be OR-ed with other flags, e.g. CDS_RESET in "LEGO Racers".
    if ((dwflags == 0 || (dwflags & CDS_FULLSCREEN) || (dwflags & CDS_UPDATEREGISTRY)) && lpDevMode) {
        if (dxw.IsEmulated || !(dmFields & DM_BITSPERPEL)) {
            OutTraceDW("%s: BYPASS res=DISP_CHANGE_SUCCESSFUL\n", fname);
            return DISP_CHANGE_SUCCESSFUL;
        } else {
            DEVMODEA NewMode;
            if(dwflags == CDS_FULLSCREEN) dwflags = 0; // no FULLSCREEN
            (*pEnumDisplaySettings)(NULL, ENUM_CURRENT_SETTINGS, &NewMode);
            OutTraceDW("ChangeDisplaySettings: CURRENT wxh=(%dx%d) BitsPerPel=%d -> %d\n",
                       NewMode.dmPelsWidth, NewMode.dmPelsHeight, NewMode.dmBitsPerPel, dmBitsPerPel);
            NewMode.dmFields = DM_BITSPERPEL | DM_PELSWIDTH | DM_PELSHEIGHT;
            NewMode.dmBitsPerPel = dmBitsPerPel;
            res = (*pChangeDisplaySettingsExA)(NULL, &NewMode, NULL, 0, NULL);
#ifndef DXW_NOTRACES
            if(res) OutTraceE("ChangeDisplaySettings: ERROR err=%d at %d\n", GetLastError(), __LINE__);
#endif // DXW_NOTRACES
            return res;
        }
    }
    if(WideChar)
        res = (*pChangeDisplaySettingsExW)(NULL, (LPDEVMODEW)lpDevMode, NULL, dwflags, NULL);
    else
        res = (*pChangeDisplaySettingsExA)(NULL, (LPDEVMODEA)lpDevMode, NULL, dwflags, NULL);
    if(dxw.bAutoScale) dxw.AutoScale();
    return res;
}

void dxwFixWindowPos(char *ApiName, HWND hwnd, LPARAM lParam) {
    LPWINDOWPOS wp;
    int MaxX, MaxY;
    wp = (LPWINDOWPOS)lParam;
    MaxX = dxw.iSizX;
    MaxY = dxw.iSizY;
    if (!MaxX) MaxX = dxw.GetScreenWidth();
    if (!MaxY) MaxY = dxw.GetScreenHeight();
    static int iLastCX, iLastCY;
    static int BorderX = -1;
    static int BorderY = -1;
    int cx, cy;
    OutTraceDW("%s: GOT hwnd=%#x pos=(%d,%d) dim=(%d,%d) Flags=%#x(%s)\n",
               ApiName, hwnd, wp->x, wp->y, wp->cx, wp->cy, wp->flags, ExplainWPFlags(wp->flags));
    // if nothing to be moved, do nothing
    if ((wp->flags & (SWP_NOMOVE | SWP_NOSIZE)) == (SWP_NOMOVE | SWP_NOSIZE)) return; //v2.02.13
    if (dxw.dwFlags1 & PREVENTMAXIMIZE) {
        int UpdFlag = 0;
        WINDOWPOS MaxPos;
        dxw.CalculateWindowPos(hwnd, MaxX, MaxY, &MaxPos);
        if(wp->cx > MaxPos.cx) {
            wp->cx = MaxPos.cx;
            UpdFlag = 1;
        }
        if(wp->cy > MaxPos.cy) {
            wp->cy = MaxPos.cy;
            UpdFlag = 1;
        }
#ifndef DXW_NOTRACES
        if (UpdFlag)
            OutTraceDW("%s: SET max size=(%dx%d)\n", ApiName, wp->cx, wp->cy);
#endif // DXW_NOTRACES
    }
    if (dxw.IsFullScreen() && (hwnd == dxw.GethWnd())) {
        if (dxw.dwFlags1 & LOCKWINPOS) {
            dxw.CalculateWindowPos(hwnd, MaxX, MaxY, wp);
            OutTraceDW("%s: LOCK pos=(%d,%d) size=(%dx%d)\n", ApiName, wp->x, wp->y, wp->cx, wp->cy);
        }
        // v2.03.95: locked size
        if (dxw.dwFlags2 & LOCKEDSIZE) {
            WINDOWPOS MaxPos;
            dxw.CalculateWindowPos(hwnd, MaxX, MaxY, &MaxPos);
            wp->cx = MaxPos.cx;
            wp->cy = MaxPos.cy;
            OutTraceDW("%s: SET locked size=(%dx%d)\n", ApiName, wp->cx, wp->cy);
        }
        if (dxw.dwFlags7 & ANCHORED) {
            WINDOWPOS MaxPos;
            dxw.CalculateWindowPos(hwnd, MaxX, MaxY, &MaxPos);
            wp->cx = MaxPos.cx;
            wp->cy = MaxPos.cy;
            wp->x  = MaxPos.x;
            wp->y  = MaxPos.y;
            OutTraceDW("%s: SET anchored pos=(%d,%d) size=(%dx%d)\n", ApiName, wp->x, wp->y, wp->cx, wp->cy);
        }
    }
    if ((dxw.dwFlags2 & KEEPASPECTRATIO) && dxw.IsFullScreen() && (hwnd == dxw.GethWnd())) {
        // note: while keeping aspect ration, resizing from one corner doesn't tell
        // which coordinate is prevalent to the other. We made an arbitrary choice.
        // note: v2.1.93: compensation must refer to the client area, not the wp
        // window dimensions that include the window borders.
        if(BorderX == -1) {
            // v2.02.92: Fixed for AERO mode, where GetWindowRect substantially LIES!
            RECT client, full;
            LONG dwStyle, dwExStyle;
            HMENU hMenu;
            extern GetWindowLong_Type pGetWindowLong;
            (*pGetClientRect)(hwnd, &client);
            full = client;
            dwStyle = (*pGetWindowLong)(hwnd, GWL_STYLE);
            dwExStyle = (*pGetWindowLong)(hwnd, GWL_EXSTYLE);
            hMenu = (dwStyle & WS_CHILD) ? NULL : GetMenu(hwnd);
            (*pAdjustWindowRectEx)(&full, dwStyle, (hMenu != NULL), dwExStyle);
            if (hMenu && (hMenu != (HMENU) - 1)) __try {
                    CloseHandle(hMenu);
                } __except(EXCEPTION_EXECUTE_HANDLER) {};
            BorderX = full.right - full.left - client.right;
            BorderY = full.bottom - full.top - client.bottom;
            OutTraceDW("%s: KEEPASPECTRATIO window borders=(%d,%d)\n", ApiName, BorderX, BorderY);
        }
        extern LRESULT LastCursorPos;
        switch (LastCursorPos) {
        case HTBOTTOM:
        case HTTOP:
        case HTBOTTOMLEFT:
        case HTBOTTOMRIGHT:
        case HTTOPLEFT:
        case HTTOPRIGHT:
            cx = BorderX + ((wp->cy - BorderY) * dxw.iRatioX) / dxw.iRatioY;
            if(cx != wp->cx) {
                OutTraceDW("%s: KEEPASPECTRATIO adjusted cx=%d->%d\n", ApiName, wp->cx, cx);
                wp->cx = cx;
            }
            break;
        case HTLEFT:
        case HTRIGHT:
            cy = BorderY + ((wp->cx - BorderX) * dxw.iRatioY) / dxw.iRatioX;
            if(cy != wp->cy) {
                OutTraceDW("%s: KEEPASPECTRATIO adjusted cy=%d->%d\n", ApiName, wp->cy, cy);
                wp->cy = cy;
            }
            break;
        }
    }
    if ((dxw.dwDFlags & CENTERTOWIN) && dxw.IsFullScreen() && (hwnd == dxw.GethWnd())) {
        RECT wrect;
        LONG dwStyle, dwExStyle;
        HMENU hMenu;
        int minx, miny;
        wrect = dxw.GetScreenRect();
        dwStyle = (*pGetWindowLong)(hwnd, GWL_STYLE);
        dwExStyle = (*pGetWindowLong)(hwnd, GWL_EXSTYLE);
        hMenu = (dwStyle & WS_CHILD) ? NULL : GetMenu(hwnd);
        (*pAdjustWindowRectEx)(&wrect, dwStyle, (hMenu != NULL), dwExStyle);
        minx = wrect.right - wrect.left;
        miny = wrect.bottom - wrect.top;
        if(wp->cx < minx) wp->cx = minx;
        if(wp->cy < miny) wp->cy = miny;
    }
    iLastCX = wp->cx;
    iLastCY = wp->cy;
}

void dxwFixMinMaxInfo(char *ApiName, HWND hwnd, LPARAM lParam) {
    if (dxw.dwFlags1 & PREVENTMAXIMIZE) {
        LPMINMAXINFO lpmmi;
        lpmmi = (LPMINMAXINFO)lParam;
        OutTraceDW("%s: GOT MaxPosition=(%d,%d) MaxSize=(%d,%d)\n", ApiName,
                   lpmmi->ptMaxPosition.x, lpmmi->ptMaxPosition.y, lpmmi->ptMaxSize.x, lpmmi->ptMaxSize.y);
        lpmmi->ptMaxPosition.x = 0;
        lpmmi->ptMaxPosition.y = 0;
        lpmmi->ptMaxSize.x = dxw.GetScreenWidth();
        lpmmi->ptMaxSize.y = dxw.GetScreenHeight();
        OutTraceDW("%s: SET PREVENTMAXIMIZE MaxPosition=(%d,%d) MaxSize=(%d,%d)\n", ApiName,
                   lpmmi->ptMaxPosition.x, lpmmi->ptMaxPosition.y, lpmmi->ptMaxSize.x, lpmmi->ptMaxSize.y);
    }
    // v2.1.75: added logic to fix win coordinates to selected ones.
    // fixes the problem with "Achtung Spitfire", that can't be managed through PREVENTMAXIMIZE flag.
    if (dxw.dwFlags1 & LOCKWINPOS) {
        LPMINMAXINFO lpmmi;
        lpmmi = (LPMINMAXINFO)lParam;
        OutTraceDW("%s: GOT MaxPosition=(%d,%d) MaxSize=(%d,%d)\n", ApiName,
                   lpmmi->ptMaxPosition.x, lpmmi->ptMaxPosition.y, lpmmi->ptMaxSize.x, lpmmi->ptMaxSize.y);
        lpmmi->ptMaxPosition.x = dxw.iPosX;
        lpmmi->ptMaxPosition.y = dxw.iPosY;
        lpmmi->ptMaxSize.x = dxw.iSizX ? dxw.iSizX : dxw.GetScreenWidth();
        lpmmi->ptMaxSize.y = dxw.iSizY ? dxw.iSizY : dxw.GetScreenHeight();
        OutTraceDW("%s: SET LOCKWINPOS MaxPosition=(%d,%d) MaxSize=(%d,%d)\n", ApiName,
                   lpmmi->ptMaxPosition.x, lpmmi->ptMaxPosition.y, lpmmi->ptMaxSize.x, lpmmi->ptMaxSize.y);
    }
}

static LRESULT WINAPI FixWindowProc(char *ApiName, HWND hwnd, UINT Msg, WPARAM wParam, LPARAM *lpParam) {
    LPARAM lParam;
    lParam = *lpParam;
    OutTraceW("%s: hwnd=%#x msg=[%#x]%s(%#x,%#x)\n",
              ApiName, hwnd, Msg, ExplainWinMessage(Msg), wParam, lParam);
    switch(Msg) {
    case WM_NCHITTEST:
        // v2.02.71 fix: when processing WM_NCHITTEST messages whith fixed coordinates avoid calling
        // the *pDefWindowProc call
        // fixes "Microsoft Motocross Madness" mouse handling
        if((dxw.dwFlags2 & FIXNCHITTEST) && (dxw.dwFlags1 & MODIFYMOUSE)) { // mouse processing
            OutTraceDW("%s: suppress WM_NCHITTEST\n", ApiName);
            return TRUE;
        }
        break;
    case WM_ERASEBKGND:
        OutTraceDW("%s: prevent erase background\n", ApiName);
        return TRUE; // 1=erased
        break; // useless
    case WM_GETMINMAXINFO:
        dxwFixMinMaxInfo(ApiName, hwnd, lParam);
        break;
    case WM_WINDOWPOSCHANGING:
    case WM_WINDOWPOSCHANGED:
        if(dxw.dwFlags5 & NOWINPOSCHANGES) return TRUE;
        dxwFixWindowPos(ApiName, hwnd, lParam);
        break;
    case WM_MOVING:
    case WM_MOVE:
        if(dxw.dwFlags5 & NOWINPOSCHANGES) return TRUE;
        break;
    case WM_STYLECHANGING:
    case WM_STYLECHANGED:
        dxw.FixStyle(ApiName, hwnd, wParam, lParam);
        break;
    case WM_SIZE:
        if ((dxw.dwFlags1 & LOCKWINPOS) && dxw.IsFullScreen()) return 0;
        if (dxw.dwFlags1 & PREVENTMAXIMIZE) {
            if ((wParam == SIZE_MAXIMIZED) || (wParam == SIZE_MAXSHOW)) {
                OutTraceDW("%s: prevent screen SIZE to fullscreen wparam=%d(%s) size=(%d,%d)\n", ApiName,
                           wParam, ExplainResizing(wParam), HIWORD(lParam), LOWORD(lParam));
                return 0; // checked
                //lParam = MAKELPARAM(dxw.GetScreenWidth(), dxw.GetScreenHeight());
                //OutTraceDW("%s: updated SIZE wparam=%d(%s) size=(%d,%d)\n", ApiName,
                //	wParam, ExplainResizing(wParam), HIWORD(lParam), LOWORD(lParam));
            }
        }
        break;
    default:
        break;
    }
    // marker to run hooked function
    return(-1);
}

// see https://msdn.microsoft.com/en-us/library/windows/desktop/ms632679%28v=vs.85%29.aspx

// from MSDN:
// If an overlapped window is created with the WS_VISIBLE style bit set and the x parameter is set to CW_USEDEFAULT,
// then the y parameter determines how the window is shown. If the y parameter is CW_USEDEFAULT, then the window
// manager calls ShowWindow with the SW_SHOW flag after the window has been created. If the y parameter is some other
// value, then the window manager calls ShowWindow with that value as the nCmdShow parameter.

static BOOL IsFullscreenWindow(
    DWORD dwStyle,
    DWORD dwExStyle,
    HWND hWndParent,
    int x,
    int y,
    int nWidth,
    int nHeight) {
    if (dwExStyle & WS_EX_CONTROLPARENT) return FALSE; // "Diablo" fix
    if ((dwStyle & WS_CHILD) && (!dxw.IsDesktop(hWndParent))) return FALSE; // Diablo fix
    // if maximized.
    if(dwStyle & WS_MAXIMIZE) return TRUE;
    // go through here only when WS_CHILD of desktop window
    // v2.04.87: some programs use values different from CW_USEDEFAULT, but still with CW_USEDEFAULT bit set
    // in this case better use the bitwinse AND rather than the equal operator to find big windows
    if((x & CW_USEDEFAULT) && (dwStyle & (WS_POPUP | WS_CHILD))) x = y = 0;
    if(nWidth & CW_USEDEFAULT) {
        if (dwStyle & (WS_POPUP | WS_CHILD)) nWidth = nHeight = 0;
        else nWidth = dxw.GetScreenWidth() - x;
    }
    // msdn undocumented case: x,y=(-1000, CW_USEDEFAULT) w,h=(CW_USEDEFAULT,CW_USEDEFAULT) in "Imperialism"
    if(nHeight & CW_USEDEFAULT) {
        y = 0;
        nHeight = dxw.GetScreenHeight();
    }
    // if bigger than screen ...
    if((x <= 0) &&
            (y <= 0) &&
            (nWidth >= (int)dxw.GetScreenWidth()) &&
            (nHeight >= (int)dxw.GetScreenHeight())) return TRUE;
    // v2.04.68.fx1: if there is no main window yet and the IsFullScreen flag is set ...
    if(!dxw.GethWnd() && (dxw.dwFlags3 & FULLSCREENONLY)) return TRUE;
    return FALSE;
}

static BOOL IsRelativePosition(DWORD dwStyle, DWORD dwExStyle, HWND hWndParent) {
    // IsRelativePosition TRUE:
    // tested on Gangsters: coordinates must be window-relative!!!
    // Age of Empires....
    // IsRelativePosition FALSE:
    // needed for "Diablo", that creates a new WS_EX_CONTROLPARENT window that must be
    // overlapped to the directdraw surface.
    // needed for "Riven", that creates a new WS_POPUP window with the menu bar that must be
    // overlapped to the directdraw surface.
    if ((dwStyle & WS_CHILD) && !dxw.IsRealDesktop(hWndParent) && !(dwStyle & WS_POPUP))
        return TRUE;
    else
        return FALSE;
}

// --------------------------------------------------------------------------
//
// user32 API hookers
//
// --------------------------------------------------------------------------


BOOL WINAPI extInvalidateRect(HWND hwnd, RECT *lpRect, BOOL bErase) {
    ApiName("InvalidateRect");
#ifndef DXW_NOTRACES
    if(IsTraceSYS) {
        char sRect[81];
        if(lpRect) sprintf(sRect, "(%d,%d)-(%d,%d)", lpRect->left, lpRect->top, lpRect->right, lpRect->bottom);
        else strcpy(sRect, "NULL");
        OutTrace("%s: hwnd=%#x rect=%s erase=%#x\n", ApiRef, hwnd, sRect, bErase);
    }
#endif
    if(dxw.dwFlags11 & INVALIDATEFULLRECT) {
        lpRect = NULL;
        OutTraceDW("%s: INVALIDATEFULLRECT fixed rect=NULL\n", ApiRef);
    }
    if(dxw.Windowize) {
        if(dxw.IsRealDesktop(hwnd))
            hwnd = dxw.GethWnd();
        RECT ScaledRect;
        if(dxw.IsFullScreen()) {
            switch(dxw.GDIEmulationMode) {
            case GDIMODE_STRETCHED:
            case GDIMODE_SHAREDDC:
            case GDIMODE_EMULATED:
                if(lpRect) {
                    // v2.03.55: the lpRect area must NOT be altered by the call
                    // effect visible in partial updates of Deadlock 2 main menu buttons
                    ScaledRect = *lpRect;
                    dxw.MapClient(&ScaledRect);
                    lpRect = &ScaledRect;
                    OutTraceDW("%s: fixed rect=(%d,%d)-(%d,%d)\n",
                               ApiRef, lpRect->left, lpRect->top, lpRect->right, lpRect->bottom);
                }
                break;
            default:
                break;
            }
        }
    }
    return (*pInvalidateRect)(hwnd, lpRect, bErase);
}

BOOL WINAPI extShowWindow(HWND hwnd, int nCmdShow) {
    BOOL res;
    ApiName("ShowWindow");
    extern HWND hTrayWnd;
    static long iLastSizX, iLastSizY;
    int nOrigCmd;
    //static long iLastPosX, iLastPosY;
    OutTraceSYS("%s: hwnd=%#x, CmdShow=%#x(%s)\n", ApiRef, hwnd, nCmdShow, ExplainShowCmd(nCmdShow));
    if((dxw.dwFlags9 & KILLBLACKWIN) && (hwnd == (HWND)FAKEKILLEDWIN)) {
        OutTraceDW("%s: ignore killed window\n", ApiRef);
        return TRUE;
    }
    if((dxw.dwFlags4 & PREVENTMINIMIZE) &&
            dxw.Windowize &&
            (hwnd == dxw.GethWnd()) &&
            (nCmdShow == SW_MINIMIZE)) {
        OutTraceDW("%s: suppress main window minimize\n", ApiRef);
        return TRUE;
    }
    if(dxw.Windowize && (hwnd == hTrayWnd) && (nCmdShow == SW_HIDE)) {
        // v2.03.85: suppress attempts to hide the tray window
        OutTraceDW("%s: suppress tray window hide\n", ApiRef);
        return TRUE;
    }
    if(dxw.Windowize && dxw.IsFullScreen() && dxw.IsDesktop(hwnd)) {
        if(dxw.dwFlags1 & CLIPCURSOR) {
            OutTraceDW("%s: clipper on main win %s\n", ApiRef, (nCmdShow == SW_HIDE) ? "OFF" : "ON");
            (nCmdShow == SW_HIDE) ? dxw.EraseClipCursor() : dxw.SetClipCursor();
        }
    }
    nOrigCmd = nCmdShow;
    if ((dxw.dwFlags2 & WINDOWIZE) && // v2.04.89: trim window size only in WINDOWIZE mode
            ((dxw.dwFlags9 & EMULATEMAXIMIZE) || (dxw.dwFlags1 & PREVENTMAXIMIZE))) {
        BOOL bMustTrim = FALSE;
        // v2.04.32: check conditions
        if(nCmdShow == SW_MAXIMIZE) bMustTrim = TRUE;
        if(nCmdShow == SW_SHOWDEFAULT) {
            STARTUPINFO StartupInfo;
            GetStartupInfo(&StartupInfo);
            OutTraceDW("%s DEBUG: StartupInfo dwFlags=%#x ShowWindow=%#x\n", ApiRef, StartupInfo.dwFlags, StartupInfo.wShowWindow);
            if((StartupInfo.dwFlags & STARTF_USESHOWWINDOW) && (StartupInfo.wShowWindow == SW_MAXIMIZE))bMustTrim = TRUE;
        }
        // v2.04.32: trim the maximized window and set it to main window state
        if(bMustTrim) {
            if (dxw.dwFlags1 & PREVENTMAXIMIZE) {
                OutTraceDW("%s: suppress SW_MAXIMIZE maximize\n", ApiRef);
                nCmdShow = SW_SHOWNORMAL;
            }
            if (dxw.dwFlags9 & EMULATEMAXIMIZE) {
                OutTraceDW("%s: emulate SW_MAXIMIZE maximize\n", ApiRef);
                nCmdShow = SW_SHOWNORMAL;
                (*pSetWindowPos)(hwnd, 0, dxw.iPosX, dxw.iPosY, dxw.iSizX, dxw.iSizY, SWP_NOZORDER | SWP_SHOWWINDOW);
            }
            // v2.04.32: since it should maximize, then it is a main window
            // v2.04.90: only if there's no other main window active
            if(!dxw.GethWnd()) {
                dxw.SethWnd(hwnd);
                dxw.FixWindowFrame(hwnd);
            }
        }
    }
    res = (*pShowWindow)(hwnd, nCmdShow);
    // v2.03.95: force zero size when minimize and refresh window coordinates
    if(hwnd == dxw.GethWnd()) {
        if(nCmdShow == SW_MINIMIZE) {
            dxw.IsVisible = FALSE;
            iLastSizX = dxw.iSizX;
            iLastSizY = dxw.iSizY;
            dxw.iSizX = dxw.iSizY = 0;
        } else {
            dxw.IsVisible = TRUE;
            if((dxw.iSizX == 0) && (dxw.iSizY == 0)) {
                dxw.iSizX = iLastSizX;
                dxw.iSizY = iLastSizY;
            }
        }
    }
    //dxw.UpdateDesktopCoordinates();
    OutTraceSYS("%s: res=%#x\n", ApiRef, res);
    return res;
}

LONG WINAPI extGetWindowLong(GetWindowLong_Type pGetWindowLong, char *ApiName, HWND hwnd, int nIndex) {
    LONG res;
    res = (*pGetWindowLong)(hwnd, nIndex);
    OutDebugDW("%s: hwnd=%#x, Index=%#x(%s) res=%#x\n", ApiName, hwnd, nIndex, ExplainSetWindowIndex(nIndex), res);
    // v2.04.46: handle DWL_DLGPROC only when flag is set
    if((nIndex == GWL_WNDPROC) ||
            ((nIndex == DWL_DLGPROC) && (dxw.dwFlags8 & HOOKDLGWIN))) {
        WNDPROC wp;
        wp = dxwws.GetProc(hwnd);
        OutTraceDW("%s: remapping WindowProc res=%#x -> %#x\n", ApiName, res, (LONG)wp);
        if(wp) res = (LONG)wp; // if not found, don't alter the value.
    }
    return res;
}

LONG WINAPI extGetWindowLongA(HWND hwnd, int nIndex) {
    return extGetWindowLong(pGetWindowLongA, "GetWindowLongA", hwnd, nIndex);
}
LONG WINAPI extGetWindowLongW(HWND hwnd, int nIndex) {
    return extGetWindowLong(pGetWindowLongW, "GetWindowLongW", hwnd, nIndex);
}

LONG WINAPI extSetWindowLong(HWND hwnd, int nIndex, LONG dwNewLong, SetWindowLong_Type pSetWindowLong, GetWindowLong_Type pGetWindowLong) {
    LONG res;
    ApiName("SetWindowLong");
    OutTraceDW("%s: hwnd=%#x, Index=%#x(%s) Val=%#x\n",
               ApiRef, hwnd, nIndex, ExplainSetWindowIndex(nIndex), dwNewLong);
    if (dxw.Windowize) {
        if(dxw.dwFlags1 & LOCKWINSTYLE) {
            if(nIndex == GWL_STYLE) {
                OutTraceDW("%s: Lock GWL_STYLE=%#x\n", ApiRef, dwNewLong);
                return (*pGetWindowLong)(hwnd, nIndex);
            }
            if(nIndex == GWL_EXSTYLE) {
                OutTraceDW("%s: Lock GWL_EXSTYLE=%#x\n", ApiRef, dwNewLong);
                return (*pGetWindowLong)(hwnd, nIndex);
            }
        }
        if (dxw.dwFlags1 & PREVENTMAXIMIZE) {
            if(nIndex == GWL_STYLE) {
                dwNewLong &= ~WS_MAXIMIZE;
                if(dxw.IsDesktop(hwnd)) {
                    OutTraceDW("%s: GWL_STYLE %#x suppress MAXIMIZE\n", ApiRef, dwNewLong);
                    dwNewLong |= WS_OVERLAPPEDWINDOW;
                    dwNewLong &= ~(WS_DLGFRAME | WS_MAXIMIZE | WS_VSCROLL | WS_HSCROLL | WS_CLIPSIBLINGS);
                }
            }
        }
        if(dxw.IsDesktop(hwnd) && (nIndex == GWL_EXSTYLE)) {
            // v2.02.32: disable topmost for main window only
            if(dxw.dwFlags5 & UNLOCKZORDER) {
                OutTraceDW("%s: GWL_EXSTYLE %#x suppress TOPMOST\n", ApiRef, dwNewLong);
                dwNewLong &= ~(WS_EX_TOPMOST);
            }
            // v2.04.31: forces topmost for main window only
            if(dxw.dwFlags9 & LOCKTOPZORDER) {
                OutTraceDW("%s: GWL_EXSTYLE %#x forces TOPMOST\n", ApiRef, dwNewLong);
                dwNewLong |= WS_EX_TOPMOST ;
            }
        }
        // v2.04.44: revised ...
        if((nIndex == GWL_STYLE) && !(dwNewLong & WS_CHILD) && dxw.IsDesktop(hwnd))
            dwNewLong = dxw.FixWinStyle(dwNewLong);
        if((nIndex == GWL_EXSTYLE) && !(dwNewLong & WS_CHILD) && dxw.IsDesktop(hwnd))
            dwNewLong = dxw.FixWinExStyle(dwNewLong);
    }
    // v2.03.94.fx2: removed dxw.IsFullScreen() check here ... WinProc routine must be verified in all conditions
    // fixes "Nascar Racing 3" that was setting the WinProc while still in non fullscreen mode!
    // v2.04.46: handle DWL_DLGPROC only when flag is set
    if((nIndex == GWL_WNDPROC) ||
            ((nIndex == DWL_DLGPROC) && (dxw.dwFlags8 & HOOKDLGWIN))) {
        LONG lres;
        WNDPROC OldProc;
        DWORD WinStyle;
        BOOL bHooked = FALSE;
        // fix ....
        extern LRESULT CALLBACK dw_Hider_Message_Handler(HWND, UINT, WPARAM, LPARAM);
        if(dwNewLong == (LONG)dw_Hider_Message_Handler)
            return (*pSetWindowLong)(hwnd, nIndex, (LONG)dw_Hider_Message_Handler);
        // GPL fix
        // v2.03.94.fx2: moved dxw.IsFullScreen() check here ...
        if(dxw.IsRealDesktop(hwnd) && dxw.Windowize && dxw.IsFullScreen()) {
            hwnd = dxw.GethWnd();
            OutTraceDW("%s: DESKTOP hwnd, FIXING hwnd=%#x\n", ApiRef, hwnd);
        }
        // end of GPL fix
        OldProc = (WNDPROC)(*pGetWindowLong)(hwnd, nIndex);
        WinStyle = (*pGetWindowLong)(hwnd, GWL_STYLE);
        while(TRUE) { // fake loop
            lres = -1; // initialize with not 0 value since 0 means error
            if(!(dxw.dwDFlags2 & NOWINDOWHOOKS)) {
                // hook extWindowProc to main win ....
                if(dxw.IsDesktop(hwnd)) {
                    if(OldProc == extWindowProc) OldProc = dxwws.GetProc(hwnd);
                    dxwws.PutProc(hwnd, (WNDPROC)dwNewLong);
                    res = (LONG)OldProc;
                    SetLastError(0);
                    lres = (*pSetWindowLong)(hwnd, nIndex, (LONG)extWindowProc);
                    OutTraceDW("%s: DESKTOP hooked %#x->%#x\n", ApiRef, dwNewLong, extWindowProc);
                    break;
                }
                // hook extDlgWindowProc to dialog win ....
                if((WinStyle & DWL_DLGPROC) && (dxw.dwFlags8 & HOOKDLGWIN)) {
                    if(OldProc == extDialogWindowProc) OldProc = dxwws.GetProc(hwnd);
                    dxwws.PutProc(hwnd, (WNDPROC)dwNewLong);
                    res = (LONG)OldProc;
                    SetLastError(0);
                    lres = (*pSetWindowLong)(hwnd, nIndex, (LONG)extDialogWindowProc);
                    OutTraceDW("%s: DIALOG hooked %#x->%#x\n", ApiRef, dwNewLong, extDialogWindowProc);
                    break;
                }
                // hook extChildWindowProc to child win ....
                if((WinStyle & WS_CHILD) && (dxw.dwFlags1 & HOOKCHILDWIN)) {
                    if(OldProc == extChildWindowProc) OldProc = dxwws.GetProc(hwnd);
                    dxwws.PutProc(hwnd, (WNDPROC)dwNewLong);
                    res = (LONG)OldProc;
                    SetLastError(0);
                    lres = (*pSetWindowLong)(hwnd, nIndex, (LONG)extChildWindowProc);
                    OutTraceDW("%s: CHILD hooked %#x->%#x\n", ApiRef, dwNewLong, extChildWindowProc);
                    break;
                }
            }
            // hook dwNewLong if not done otherwise
            res = (*pSetWindowLong)(hwnd, nIndex, dwNewLong);
            break;
        }
#ifndef DXW_NOTRACES
        if(!lres && GetLastError()) OutTraceE("%s: ERROR err=%d at %d\n", ApiRef, GetLastError(), __LINE__);
#endif // DXW_NOTRACES
    } else {
        // through here for any message different from GWL_WNDPROC or DWL_DLGPROC
        res = (*pSetWindowLong)(hwnd, nIndex, dwNewLong);
    }
    OutTraceDW("%s: hwnd=%#x, nIndex=%#x, Val=%#x, res=%#x\n", ApiRef, hwnd, nIndex, dwNewLong, res);
    return res;
}

LONG WINAPI extSetWindowLongA(HWND hwnd, int nIndex, LONG dwNewLong) {
    return extSetWindowLong(hwnd, nIndex, dwNewLong, pSetWindowLongA, pGetWindowLongA);
}
LONG WINAPI extSetWindowLongW(HWND hwnd, int nIndex, LONG dwNewLong) {
    return extSetWindowLong(hwnd, nIndex, dwNewLong, pSetWindowLongW, pGetWindowLongW);
}

#ifndef DXW_NOTRACES
char *ExplainSWPFlags(UINT c) {
    static char eb[256];
    unsigned int l;
    strcpy(eb, "SWP_");
    if (c & SWP_NOSIZE) strcat(eb, "NOSIZE+");
    if (c & SWP_NOMOVE) strcat(eb, "NOMOVE+");
    if (c & SWP_NOZORDER) strcat(eb, "NOZORDER+");
    if (c & SWP_NOREDRAW) strcat(eb, "NOREDRAW+");
    if (c & SWP_NOACTIVATE) strcat(eb, "NOACTIVATE+");
    if (c & SWP_FRAMECHANGED) strcat(eb, "FRAMECHANGED+");
    if (c & SWP_SHOWWINDOW) strcat(eb, "SHOWWINDOW+");
    if (c & SWP_HIDEWINDOW) strcat(eb, "HIDEWINDOW+");
    if (c & SWP_NOCOPYBITS) strcat(eb, "NOCOPYBITS+");
    if (c & SWP_NOOWNERZORDER) strcat(eb, "NOOWNERZORDER+");
    if (c & SWP_NOSENDCHANGING) strcat(eb, "NOSENDCHANGING+");
    if (c & SWP_DEFERERASE) strcat(eb, "DEFERERASE+");
    if (c & SWP_ASYNCWINDOWPOS) strcat(eb, "ASYNCWINDOWPOS+");
    l = strlen(eb);
    if (l > strlen("SWP_")) eb[l - 1] = 0; // delete last '+' if any
    else eb[0] = 0;
    return(eb);
}
#endif // DXW_NOTRACES

BOOL WINAPI extSetWindowPos(HWND hwnd, HWND hWndInsertAfter, int X, int Y, int cx, int cy, UINT uFlags) {
    BOOL res;
    ApiName("SetWindowPos");
    BOOL bMustFixPos = FALSE;
    DWORD dwStyle, dwExStyle;
    HWND hParent;
#ifdef DXWHIDEWINDOWUPDATES
    int origx, origy, origw, origh;
#endif
    OutTraceSYS("%s: hwnd=%#x%s insertafter=%#x pos=(%d,%d) dim=(%d,%d) Flags=%#x(%s)\n",
                ApiRef, hwnd, dxw.IsFullScreen() ? "(FULLSCREEN)" : "",
                hWndInsertAfter,
                X, Y, cx, cy,
                uFlags, ExplainSWPFlags(uFlags));
#ifdef DXWHIDEWINDOWUPDATES
    origx = X;
    origy = Y;
    origw = cx;
    origh = cy;
#endif
    OutDebugDW("%s: fullscreen=%#x desktop=%#x inmainwincreation=%#x\n",
               ApiRef, dxw.IsFullScreen(), dxw.IsDesktop(hwnd), InMainWinCreation);
    // when not in fullscreen mode, just proxy the call
    // v2.04.91: ... unless the window is trying to grow quite bigger than desktop: in this case
    // when there's no better main win is better promote it. Ref. "Space Clash"
    if (!dxw.IsFullScreen()) {
        if(IsFullscreenWindow(
                    (*pGetWindowLong)(hwnd, GWL_STYLE),
                    (*pGetWindowLong)(hwnd, GWL_EXSTYLE),
                    GetParent(hwnd),
                    Y, Y, cx, cy
                )) {
            OutTraceDW("%s: expanding fullscreen candidate\n", ApiRef);
            if(!dxw.GethWnd()) {
                dxw.SetFullScreen(TRUE); // v2.05.15: also set fullscreen mode
                dxw.SethWnd(hwnd);
                dxw.FixWindowFrame(hwnd);
            }
            bMustFixPos = TRUE;
        } else {
            res = (*pSetWindowPos)(hwnd, hWndInsertAfter, X, Y, cx, cy, uFlags);
#ifndef DXW_NOTRACES
            if(!res) OutTraceE("%s: ERROR err=%d at %d\n", ApiRef, GetLastError(), __LINE__);
#endif // DXW_NOTRACES
            return res;
        }
    }
    // in fullscreen, but a child window inside .....
    dwStyle = (*pGetWindowLong)(hwnd, GWL_STYLE);
    dwExStyle = (*pGetWindowLong)(hwnd, GWL_EXSTYLE);
    hParent = GetParent(hwnd);
    //if (!dxw.IsDesktop(hwnd) && !InMainWinCreation){
    if (!dxw.IsDesktop(hwnd)) { // v2.04.97 - "Fallen Haven"
        if(IsRelativePosition(dwStyle, dwExStyle, hParent)) {
            dxw.MapClient(&X, &Y, &cx, &cy);
            // v2.04.87: inner child size can't exceed desktop size
            if(cx > dxw.iSizX) cx = dxw.iSizX;
            if(cy > dxw.iSizY) cy = dxw.iSizY;
            OutTraceDW("%s: REMAPPED pos=(%d,%d) dim=(%d,%d)\n", ApiRef, X, Y, cx, cy);
            if(dxw.IsRealDesktop(hParent)) {
                OutTraceDW("%s: REMAPPED hparent=%#x->%#x\n", ApiRef, hParent, dxw.GethWnd());
                SetParent(hwnd, dxw.GethWnd());
            }
            if(dxw.IsRealDesktop(hWndInsertAfter)) {
                OutTraceDW("%s: REMAPPED hWndInsertAfter=%#x->%#x\n", ApiRef, hWndInsertAfter, dxw.GethWnd());
                hWndInsertAfter = dxw.GethWnd();
                uFlags &= ~SWP_NOOWNERZORDER;
            }
#ifdef DXWHIDEWINDOWUPDATES
            if(dxw.dwFlags10 & HIDEWINDOWCHANGES) {
                DWORD lpWinCB;
                res = (*pSetWindowPos)(hwnd, hWndInsertAfter, origx, origy, origw, origh, uFlags);
                lpWinCB = (*pGetWindowLong)(hwnd, GWL_WNDPROC);
                (*pSetWindowLong)(hwnd, GWL_WNDPROC, NULL);
                res = (*pSetWindowPos)(hwnd, hWndInsertAfter, X, Y, cx, cy, uFlags);
                (*pSetWindowLong)(hwnd, GWL_WNDPROC, lpWinCB);
            } else
                res = (*pSetWindowPos)(hwnd, hWndInsertAfter, X, Y, cx, cy, uFlags);
#else
            res = (*pSetWindowPos)(hwnd, hWndInsertAfter, X, Y, cx, cy, uFlags);
#endif
#ifndef DXW_NOTRACES
            if(!res)OutTraceE("%s: ERROR err=%d at %d\n", ApiRef, GetLastError(), __LINE__);
#endif // DXW_NOTRACES
            return res;
        } else
            dxw.MapWindow(&X, &Y, &cx, &cy);
    }
    if (dxw.IsDesktop(hwnd) && (dxw.dwFlags1 & LOCKWINPOS) && !bMustFixPos) {
        // Note: any attempt to change the window position, no matter where and how, through the
        // SetWindowPos API is causing resizing to the default 1:1 pixel size in Commandos.
        // in such cases, there is incompatibility between LOCKWINPOS and LOCKWINSTYLE.
        OutTraceDW("%s: locked position\n", ApiRef);
        // v2.04.90: LOCKWINPOS should NOT prevent other window changes but the position and size.
        // the SetWindowPos can't be skipped, but the new coordinates can be ignored.
        uFlags |= (SWP_NOSIZE | SWP_NOMOVE);
    }
    // v2.04.90: it's no use to trim the size when SWP_NOSIZE is set
    if ((dxw.dwFlags1 & PREVENTMAXIMIZE) && !(uFlags & SWP_NOSIZE)) {
        int UpdFlag = 0;
        int MaxX, MaxY;
        // v2.03.96: in PREVENTMAXIMIZE mode don't exceed the initial size
        MaxX = dxw.iSiz0X;
        MaxY = dxw.iSiz0Y;
        // v2.04.90: here we have real position & sizes
        //if (!MaxX) MaxX = dxw.GetScreenWidth();
        //if (!MaxY) MaxY = dxw.GetScreenHeight();
        if (!MaxX) MaxX = dxw.iSizX;
        if (!MaxY) MaxY = dxw.iSizY;
        if(cx > MaxX) {
            cx = MaxX;
            UpdFlag = 1;
        }
        if(cy > MaxY) {
            cy = MaxY;
            UpdFlag = 1;
        }
#ifndef DXW_NOTRACES
        if (UpdFlag)
            OutTraceDW("%s: using max dim=(%d,%d)\n", ApiRef, cx, cy);
#endif // DXW_NOTRACES
    }
    // v2.04.90: when moving a big window, set position coordinates. Fixes "Space Clash" intro movies.
    if(bMustFixPos && !(uFlags & SWP_NOMOVE)) {
        X = dxw.iPosX;
        Y = dxw.iPosY;
    }
    // useful??? to be demonstrated....
    // when altering main window in fullscreen mode, fix the coordinates for borders
    if(!(uFlags & SWP_NOSIZE)) {
        DWORD dwCurStyle, dwExStyle;
        HMENU hMenu;
        RECT rect;
        rect.top = rect.left = 0;
        rect.right = cx;
        rect.bottom = cy;
        dwCurStyle = (*pGetWindowLong)(hwnd, GWL_STYLE);
        dwExStyle = (*pGetWindowLong)(hwnd, GWL_EXSTYLE);
        // BEWARE: from MSDN -  If the window is a child window, the return value is undefined.
        hMenu = (dwCurStyle & WS_CHILD) ? NULL : GetMenu(hwnd);
        (*pAdjustWindowRectEx)(&rect, dwCurStyle, (hMenu != NULL), dwExStyle);
        if (hMenu && (hMenu != (HMENU) - 1)) __try {
                CloseHandle(hMenu);
            } __except(EXCEPTION_EXECUTE_HANDLER) {};
        cx = rect.right;
        cy = rect.bottom;
        OutTraceDW("%s: main form hwnd=%#x fixed size=(%d,%d)\n", ApiRef, hwnd, cx, cy);
    }
#ifdef DXWHIDEWINDOWUPDATES
    if(dxw.dwFlags10 & HIDEWINDOWCHANGES) {
        DWORD lpWinCB;
        res = (*pSetWindowPos)(hwnd, hWndInsertAfter, origx, origy, origw, origh, uFlags);
        lpWinCB = (*pGetWindowLong)(hwnd, GWL_WNDPROC);
        (*pSetWindowLong)(hwnd, GWL_WNDPROC, NULL);
        res = (*pSetWindowPos)(hwnd, hWndInsertAfter, X, Y, cx, cy, uFlags);
        (*pSetWindowLong)(hwnd, GWL_WNDPROC, lpWinCB);
    } else
        res = (*pSetWindowPos)(hwnd, hWndInsertAfter, X, Y, cx, cy, uFlags);
#else
    res = (*pSetWindowPos)(hwnd, hWndInsertAfter, X, Y, cx, cy, uFlags);
#endif
#ifndef DXW_NOTRACES
    if(!res)OutTraceE("%s: ERROR err=%d at %d\n", ApiRef, GetLastError(), __LINE__);
#endif // DXW_NOTRACES
    if(dxw.bAutoScale) dxw.AutoScale();
    return res;
}

HDWP WINAPI extDeferWindowPos(HDWP hWinPosInfo, HWND hwnd, HWND hWndInsertAfter, int X, int Y, int cx, int cy, UINT uFlags) {
    // v2.02.31: heavily used by "Imperialism II" !!!
    HDWP res;
    ApiName("DeferWindowPos");
    OutTraceSYS("%s: hwnd=%#x%s insertafter=%#x pos=(%d,%d) dim=(%d,%d) Flags=%#x(%s)\n",
                ApiRef, hwnd, dxw.IsFullScreen() ? "(FULLSCREEN)" : "",
                hWndInsertAfter,
                X, Y, cx, cy,
                uFlags, ExplainSWPFlags(uFlags));
    OutDebugDW("%s: fullscreen=%#x desktop=%#x inmainwincreation=%#x\n",
               ApiRef, dxw.IsFullScreen(), dxw.IsDesktop(hwnd), InMainWinCreation);
    if(dxw.IsFullScreen()) {
        dxw.MapClient(&X, &Y, &cx, &cy);
        OutTraceDW("%s: remapped pos=(%d,%d) dim=(%d,%d)\n", ApiRef, X, Y, cx, cy);
    }
    res = (*pGDIDeferWindowPos)(hWinPosInfo, hwnd, hWndInsertAfter, X, Y, cx, cy, uFlags);
#ifndef DXW_NOTRACES
    if(!res)OutTraceE("%s: ERROR err=%d at %d\n", ApiRef, GetLastError(), __LINE__);
#endif // DXW_NOTRACES
    if(dxw.bAutoScale) dxw.AutoScale();
    return res;
}

#ifndef DXW_NOTRACES
static char *ExplainScrollMode(WORD m) {
    char *s;
    switch(m) {
    case SB_LINEUP:
        s = "LINEUP/LEFT";
        break;
    case SB_LINEDOWN:
        s = "LINEDOWN/RIGHT";
        break;
    case SB_PAGEUP:
        s = "PAGEUP/LEFT";
        break;
    case SB_PAGEDOWN:
        s = "PAGEDOWN/RIGHT";
        break;
    case SB_THUMBPOSITION:
        s = "THUMBPOSITION";
        break;
    case SB_THUMBTRACK:
        s = "THUMBTRACK";
        break;
    case SB_TOP:
        s = "TOP/LEFT";
        break;
    case SB_BOTTOM:
        s = "BOTTOM/RIGHT";
        break;
    case SB_ENDSCROLL:
        s = "ENDSCROLL";
        break;
    default:
        s = "???";
    }
    return s;
}
#endif // DXW_NOTRACES

LRESULT WINAPI extSendMessage(char *apiname, SendMessage_Type pSendMessage, HWND hwnd, UINT Msg, WPARAM wParam, LPARAM lParam) {
    LRESULT ret;
    OutTraceW("%s: hwnd=%#x WinMsg=[%#x]%s(%#x,%#x)\n",
              apiname, hwnd, Msg, ExplainWinMessage(Msg), wParam, lParam);
    if(dxw.dwFlags1 & MODIFYMOUSE) {
        switch (Msg) {
        case WM_MOUSEMOVE:
        case WM_MOUSEWHEEL:
        case WM_LBUTTONDOWN:
        case WM_LBUTTONUP:
        case WM_LBUTTONDBLCLK:
        case WM_RBUTTONDOWN:
        case WM_RBUTTONUP:
        case WM_RBUTTONDBLCLK:
        case WM_MBUTTONDOWN:
        case WM_MBUTTONUP:
        case WM_MBUTTONDBLCLK:
            // revert here the WindowProc mouse correction
            POINT prev, curr;
            RECT rect;
            prev.x = LOWORD(lParam);
            prev.y = HIWORD(lParam);
            (*pGetClientRect)(dxw.GethWnd(), &rect);
            curr.x = (prev.x * rect.right) / dxw.GetScreenWidth();
            curr.y = (prev.y * rect.bottom) / dxw.GetScreenHeight();
            if (Msg == WM_MOUSEWHEEL) { // v2.02.33 mousewheel fix
                POINT upleft = {0, 0};
                (*pClientToScreen)(dxw.GethWnd(), &upleft);
                curr = dxw.AddCoordinates(curr, upleft);
            }
            lParam = MAKELPARAM(curr.x, curr.y);
            OutTraceC("%s: hwnd=%#x pos XY=(%d,%d)->(%d,%d)\n", apiname, hwnd, prev.x, prev.y, curr.x, curr.y);
            break;
        case WM_FONTCHANGE:
            // suppress WM_FONTCHANGE avoids "Warhammer: Shadow of the Horned Rat" crash when entering battle
            OutTraceDW("%s: WM_FONTCHANGE suppressed\n", apiname);
            return 0;
            break;
        case WM_VSCROLL:
        case WM_HSCROLL:
            OutTraceW("%s: %s pos=%d scroll=%#x(%s) handle=%#x\n",
                      apiname,
                      (Msg == WM_VSCROLL) ? "WM_VSCROLL" : "WM_HSCROLL",
                      HIWORD(wParam),
                      LOWORD(wParam), ExplainScrollMode(LOWORD(wParam)),
                      lParam);
            break;
        default:
            break;
        }
    }
#ifndef MCIWNDM_OPENA
#define MCIWNDM_OPENA (WM_USER + 153) // from Vfw.h
#endif
    if(dxw.dwFlags10 & (FAKECDDRIVE | FAKEHDDRIVE)) {
        if(Msg == MCIWNDM_OPENA) {
            OutTrace("%s: mapping path=\"%s\"\n", apiname, (char *)lParam);
            extern LPCSTR dxwTranslatePathA(LPCSTR, DWORD *);
            lParam = (WPARAM)dxwTranslatePathA((LPCSTR)lParam, NULL);
        }
    }
    ret = (*pSendMessage)(hwnd, Msg, wParam, lParam);
    OutTraceW("%s: lresult=%#x\n", apiname, ret);
    return ret;
}

LRESULT WINAPI extSendMessageA(HWND hwnd, UINT Msg, WPARAM wParam, LPARAM lParam) {
    return extSendMessage("SendMessageA", pSendMessageA, hwnd, Msg, wParam, lParam);
}
LRESULT WINAPI extSendMessageW(HWND hwnd, UINT Msg, WPARAM wParam, LPARAM lParam) {
    return extSendMessage("SendMessageW", pSendMessageW, hwnd, Msg, wParam, lParam);
}

BOOL WINAPI extGetCursorPos(LPPOINT lppoint) {
    HRESULT res;
    static int PrevX, PrevY;
    POINT prev;
    if(dxw.dwFlags1 & SLOWDOWN) dxw.DoSlow(2);
    if (pGetCursorPos)
        res = (*pGetCursorPos)(lppoint);
    else {
        lppoint->x = 0;
        lppoint->y = 0;
        res = 1;
    }
    if(dxw.dwFlags1 & MODIFYMOUSE) {
        dxw.UpdateDesktopCoordinates();
        prev = *lppoint;
        dxw.UnmapWindow(lppoint);
        dxw.FixCursorClipper(lppoint);
        OutTraceC("GetCursorPos: FIXED pos=(%d,%d)->(%d,%d)\n", prev.x, prev.y, lppoint->x, lppoint->y);
    } else
        OutTraceC("GetCursorPos: pos=(%d,%d)\n", lppoint->x, lppoint->y);
    GetHookInfo()->CursorX = (short)lppoint->x;
    GetHookInfo()->CursorY = (short)lppoint->y;
    if((dxw.dwFlags1 & HIDEHWCURSOR) && dxw.IsFullScreen()) while((*pShowCursor)(0) >= 0);
    if(dxw.dwFlags2 & SHOWHWCURSOR) while((*pShowCursor)(1) < 0);
    return res;
}

BOOL WINAPI extSetCursorPos(int x, int y) {
    BOOL res;
    ApiName("SetCursorPos");
    int PrevX, PrevY;
    OutDebugC("%s: XY=(%d,%d)\n", ApiRef, x, y);
    PrevX = x;
    PrevY = y;
    // v2.05.15: SetCursorPos is disabled also when the window loses focus. Requested for "Yu No"
    if(!dxw.bActive || (dxw.dwFlags2 & KEEPCURSORFIXED)) {
        OutTraceC("%s: SUPPRESS pos=(%d,%d)\n", ApiRef, x, y);
        LastCurPosX = x;
        LastCurPosY = y;
        return 1;
    }
    if(dxw.dwFlags1 & SLOWDOWN) dxw.DoSlow(2);
    if(dxw.dwFlags1 & KEEPCURSORWITHIN) {
        // Intercept SetCursorPos outside screen boundaries (used as Cursor OFF in some games)
        if ((y < 0) || (y >= (int)dxw.GetScreenHeight()) || (x < 0) || (x >= (int)dxw.GetScreenWidth())) return 1;
    }
    if(dxw.dwFlags1 & MODIFYMOUSE) {
        // v2.03.41
        dxw.UpdateDesktopCoordinates();
        dxw.MapWindow(&x, &y);
    }
    // v2.04.30: hint by gsky916 - "In specific case SetCursorPos could cause crazy mouse movement
    // that made the mouse click out of player's control. While the game call this function quite frequently,
    // if I add some sleep to slow down the crazy mouse movement, the whole game become lag.
    // Finally I got an idea is using mouse_event to emulate SetCursorPos, and it works"
#if 0
    if((dxw.dwFlags9 & MOUSEMOVEBYEVENT) && pmouse_event) {
        // If MOUSEEVENTF_ABSOLUTE value is specified, dx and dy contain normalized
        // absolute coordinates between 0 and 65,535. The event procedure maps these
        // coordinates onto the display surface. Coordinate (0,0) maps onto the
        // upper-left corner of the display surface; coordinate (65535,65535) maps onto
        // the lower-right corner. In a multimonitor system, the coordinates map to the
        // primary monitor.
        DWORD nx = x * 65535 / GetSystemMetrics(SM_CXSCREEN);
        DWORD ny = y * 65535 / GetSystemMetrics(SM_CYSCREEN);
        (*pmouse_event)(MOUSEEVENTF_ABSOLUTE | MOUSEEVENTF_MOVE, nx, ny, 0, 0);
#else
    if((dxw.dwFlags9 & MOUSEMOVEBYEVENT) && pSendInput) {
        INPUT in;
        in.type = INPUT_MOUSE;
        in.mi.dx = x * 65535 / GetSystemMetrics(SM_CXSCREEN);
        in.mi.dy = y * 65535 / GetSystemMetrics(SM_CYSCREEN);
        in.mi.dwFlags = MOUSEEVENTF_ABSOLUTE | MOUSEEVENTF_MOVE;
        in.mi.mouseData = 0;
        in.mi.dwExtraInfo = 0;
        in.mi.time = 0;
        if((*pSendInput)(1, &in, sizeof(in)) != 1)
            OutTraceE("%s: SendInput ERROR err=%d\n", ApiRef, GetLastError());
#endif
        return TRUE; // ok
    }
    res = 0;
    if (pSetCursorPos) res = (*pSetCursorPos)(x, y);
    OutTraceC("%s: res=%#x XY=(%d,%d)->(%d,%d)\n", ApiRef, res, PrevX, PrevY, x, y);
    return res;
}

static BOOL WINAPI extPeekMessage(char *api, PeekMessage_Type pPeekMessage, LPMSG lpMsg, HWND hwnd, UINT wMsgFilterMin, UINT wMsgFilterMax, UINT wRemoveMsg) {
    BOOL res;
    char *sLabel;
    if(dxw.dwFlags3 & PEEKALLMESSAGES) {
        sLabel = "(ANY) ";
        if((wMsgFilterMin == 0) && (wMsgFilterMax == 0)) {
            // no filtering, everything is good
            res = (*pPeekMessage)(lpMsg, hwnd, wMsgFilterMin, wMsgFilterMax, (wRemoveMsg & 0x000F));
        } else {
            MSG Dummy;
            // better eliminate all messages before and after the selected range !!!!
            //if(wMsgFilterMin)(*pPeekMessage)(&Dummy, hwnd, 0, wMsgFilterMin-1, TRUE);
            if(wMsgFilterMin > 0x0F)(*pPeekMessage)(&Dummy, hwnd, 0x0F, wMsgFilterMin - 1, TRUE);
            res = (*pPeekMessage)(lpMsg, hwnd, wMsgFilterMin, wMsgFilterMax, (wRemoveMsg & 0x000F));
            if(wMsgFilterMax < WM_KEYFIRST)(*pPeekMessage)(&Dummy, hwnd, wMsgFilterMax + 1, WM_KEYFIRST - 1, TRUE); // don't touch above WM_KEYFIRST !!!!
        }
    } else {
        sLabel = "";
        res = (*pPeekMessage)(lpMsg, hwnd, wMsgFilterMin, wMsgFilterMax, (wRemoveMsg & 0x000F));
    }
#ifndef DXW_NOTRACES
    if(IsTraceW || IsTraceC)  {
        if(res) {
            OutTrace(
                "%s: %slpmsg=%#x hwnd=%#x filter=(%#x-%#x) remove=%#x(%s) res=%#x "
                "msg={message=%#x(%s) hwnd=%#x wparam=%#x lparam=%#x pt=(%d,%d) time=%#x}\n",
                api, sLabel, lpMsg, hwnd, wMsgFilterMin, wMsgFilterMax, wRemoveMsg, ExplainPeekRemoveMsg(wRemoveMsg), res,
                lpMsg->message, ExplainWinMessage(lpMsg->message & 0xFFFF), lpMsg->hwnd,
                lpMsg->wParam, lpMsg->lParam, lpMsg->pt.x, lpMsg->pt.y, lpMsg->time);
        } else {
            // v2.04.47: trace void peeks only in debug mode, they can outnumber other logs
            OutDebugW("%s: %slpmsg=%#x hwnd=%#x filter=(%#x-%#x) remove=%#x(%s) res=%#x\n",
                      api, sLabel, lpMsg, hwnd, wMsgFilterMin, wMsgFilterMax, wRemoveMsg, ExplainPeekRemoveMsg(wRemoveMsg), res);
        }
    }
#endif
    // v2.04.60: new SWALLOWMOUSEMOVE flag to eliminate mouse movement messages received too often.
    // In "Akte Europa" the reception of too many movement messages blocked the gameplay while the mouse was moved.
    if(res && (dxw.dwFlags5 & SWALLOWMOUSEMOVE)) {
        static DWORD LastMouseMessage = 0;
        if(lpMsg->message == WM_MOUSEMOVE) {
            DWORD now = (*pGetTickCount)();
            // 50 mSec is for sure greater than any sync delay and gives 20 updates / sec.
            // v2.05.27: Kayel Gee fix - use MaxFPS when defined
            if((now - LastMouseMessage) > ((dxw.dwFlags2 & LIMITFPS) ? dxw.MaxFPS : 50)) {
                OutDebugW("%s: passing WM_MOUSEMOVE at %#x\n", api, now);
                LastMouseMessage = (*pGetTickCount)();
            } else {
                OutDebugW("%s: swallow WM_MOUSEMOVE at %#x\n", api, now);
                res = (*pPeekMessage)(lpMsg, hwnd, WM_MOUSEMOVE, WM_MOUSEMOVE, TRUE);
                return 0;
            }
        }
    }
    // v2.04.34: when res==0 no message is extracted from the queue, but lpMsg can point to the last valid message
    // so that a loop of failed extractions from queue would keep fixing the last message coordinates.
    // Useless though apparently not harmful, a if(res) condition was added here.
    if(res && dxw.GethWnd() && dxw.MustFixCoordinates) {
        POINT point;
        //res=(*pGetCursorPos)(&point); // can't do this. Why?
        point = lpMsg->pt;
        point = dxw.ScreenToClient(point);
        point = dxw.FixCursorPos(point);
        OutTraceC("%s: FIXED pos=(%d,%d)->(%d,%d)\n", api, lpMsg->pt.x, lpMsg->pt.y, point.x, point.y);
        lpMsg->pt = point;
        if(dxw.dwFlags12 & FIXMOUSELPARAM) {
            switch(lpMsg->message) {
            case WM_MOUSEMOVE:
            case WM_LBUTTONDOWN:
            case WM_LBUTTONUP:
            case WM_LBUTTONDBLCLK:
            case WM_RBUTTONDOWN:
            case WM_RBUTTONUP:
            case WM_RBUTTONDBLCLK:
            case WM_MBUTTONDOWN:
            case WM_MBUTTONUP:
            case WM_MBUTTONDBLCLK:
            case WM_MOUSEWHEEL:
            case WM_XBUTTONDOWN:
            case WM_XBUTTONUP:
            case WM_XBUTTONDBLCLK:
            case WM_MOUSEHWHEEL:
                lpMsg->lParam = MAKELPARAM(point.x, point.y);
                OutTraceC("%s: fixed lParam=(%d,%d)\n", api, (short)LOWORD(lpMsg->lParam), (short)HIWORD(lpMsg->lParam));
                break;
            }
        }
    }
    // perform hot keys processing on messages that would be deleted otherwise ...
    if( wRemoveMsg && (dxw.dwFlags4 & ENABLEHOTKEYS)) {
        switch(lpMsg->message) {
        case WM_SYSKEYDOWN:
        case WM_KEYDOWN:
            OutTraceDW("%s: event %s wparam=%#x lparam=%#x\n",
                       api, (lpMsg->message == WM_SYSKEYDOWN) ? "WM_SYSKEYDOWN" : "WM_KEYDOWN", lpMsg->wParam, lpMsg->lParam);
            HandleHotKeys(hwnd, lpMsg->message, lpMsg->lParam, lpMsg->wParam);
            break;
        }
    }
    GetHookInfo()->MessageX = (short)lpMsg->pt.x;
    GetHookInfo()->MessageY = (short)lpMsg->pt.y;
    if(dxw.dwFlags1 & SLOWDOWN) dxw.DoSlow(1);
    return res;
}

BOOL WINAPI extPeekMessageA(LPMSG lpMsg, HWND hwnd, UINT wMsgFilterMin, UINT wMsgFilterMax, UINT wRemoveMsg) {
    return extPeekMessage("PeekMessageA", pPeekMessageA, lpMsg, hwnd, wMsgFilterMin, wMsgFilterMax, wRemoveMsg);
}
BOOL WINAPI extPeekMessageW(LPMSG lpMsg, HWND hwnd, UINT wMsgFilterMin, UINT wMsgFilterMax, UINT wRemoveMsg) {
    return extPeekMessage("PeekMessageW", pPeekMessageW, lpMsg, hwnd, wMsgFilterMin, wMsgFilterMax, wRemoveMsg);
}

static BOOL WINAPI extGetMessage(char *api, GetMessage_Type pGetMessage, LPMSG lpMsg, HWND hwnd, UINT wMsgFilterMin, UINT wMsgFilterMax) {
    BOOL res;
    POINT point;
    res = (*pGetMessage)(lpMsg, hwnd, wMsgFilterMin, wMsgFilterMax);
    OutTraceW("%s: lpmsg=%#x hwnd=%#x filter=(%#x-%#x) msg=%#x(%s) wparam=%#x, lparam=%#x pt=(%d,%d) res=%#x\n",
              api, lpMsg, lpMsg->hwnd, wMsgFilterMin, wMsgFilterMax,
              lpMsg->message, ExplainWinMessage(lpMsg->message & 0xFFFF),
              lpMsg->wParam, lpMsg->lParam, lpMsg->pt.x, lpMsg->pt.y, res);
    if((dxw.dwFlags11 & REMAPNUMKEYPAD) &&
            ((lpMsg->message == WM_KEYDOWN) || (lpMsg->message == WM_KEYUP))
      ) {
        switch(lpMsg->wParam) {
        case VK_INSERT:
            lpMsg->wParam = VK_NUMPAD0;
            break;
        case VK_END:
            lpMsg->wParam = VK_NUMPAD1;
            break;
        case VK_DOWN:
            lpMsg->wParam = VK_NUMPAD2;
            break;
        case VK_NEXT:
            lpMsg->wParam = VK_NUMPAD3;
            break;
        case VK_LEFT:
            lpMsg->wParam = VK_NUMPAD4;
            break;
        case VK_RIGHT:
            lpMsg->wParam = VK_NUMPAD6;
            break;
        case VK_HOME:
            lpMsg->wParam = VK_NUMPAD7;
            break;
        case VK_UP:
            lpMsg->wParam = VK_NUMPAD8;
            break;
        case VK_PRIOR:
            lpMsg->wParam = VK_NUMPAD9;
            break;
        }
    }
    if(dxw.dwFlags1 & MODIFYMOUSE) {
        point = lpMsg->pt;
        point = dxw.ScreenToClient(point);
        point = dxw.FixCursorPos(point);
        OutTraceC("%s: FIXED pos=(%d,%d)->(%d,%d)\n", api, lpMsg->pt.x, lpMsg->pt.y, point.x, point.y);
        lpMsg->pt = point;
#ifdef FIXMOUSEMESSAGESLPARAM
        switch(lpMsg->message) {
        case WM_MOUSEMOVE:
        case WM_LBUTTONDOWN:
        case WM_LBUTTONUP:
        case WM_LBUTTONDBLCLK:
        case WM_RBUTTONDOWN:
        case WM_RBUTTONUP:
        case WM_RBUTTONDBLCLK:
        case WM_MBUTTONDOWN:
        case WM_MBUTTONUP:
        case WM_MBUTTONDBLCLK:
        case WM_MOUSEWHEEL:
        case WM_XBUTTONDOWN:
        case WM_XBUTTONUP:
        case WM_XBUTTONDBLCLK:
        case WM_MOUSEHWHEEL:
            lpMsg->lParam = MAKELPARAM(point.x, point.y);
            OutTraceC("%s: fixed lParam=(%d,%d)\n", api, (short)LOWORD(lpMsg->lParam), (short)HIWORD(lpMsg->lParam));
            break;
        }
#endif // FIXMOUSEMESSAGESLPARAM
    }
    if (lpMsg->message == WM_SYSKEYDOWN) {
        // v2.05.11 fix: added here to manage hot keys & Alt-F4 in GetMessage loops. Fixes "Road Rash".
        if ((dxw.dwFlags1 & HANDLEALTF4) && (lpMsg->wParam == VK_F4)) {
            OutTraceDW("WindowProc: WM_SYSKEYDOWN(ALT-F4) - terminating process\n");
            TerminateProcess(GetCurrentProcess(), 0);
        }
        if (dxw.dwFlags4 & ENABLEHOTKEYS) {
            switch(lpMsg->message) {
            case WM_SYSKEYDOWN:
            case WM_KEYDOWN:
                OutTrace("%s: event %s wparam=%#x lparam=%#x\n",
                         api, (lpMsg->message == WM_SYSKEYDOWN) ? "WM_SYSKEYDOWN" : "WM_KEYDOWN", lpMsg->wParam, lpMsg->lParam);
                HandleHotKeys(hwnd, lpMsg->message, lpMsg->lParam, lpMsg->wParam);
                break;
            }
        }
    }
    GetHookInfo()->MessageX = (short)lpMsg->pt.x;
    GetHookInfo()->MessageY = (short)lpMsg->pt.y;
    return res;
}

BOOL WINAPI extGetMessageA(LPMSG lpMsg, HWND hwnd, UINT wMsgFilterMin, UINT wMsgFilterMax) {
    return extGetMessage("GetMessageA", pGetMessageA, lpMsg, hwnd, wMsgFilterMin, wMsgFilterMax);
}
BOOL WINAPI extGetMessageW(LPMSG lpMsg, HWND hwnd, UINT wMsgFilterMin, UINT wMsgFilterMax) {
    return extGetMessage("GetMessageW", pGetMessageW, lpMsg, hwnd, wMsgFilterMin, wMsgFilterMax);
}

BOOL WINAPI extPostMessage(char *api, PostMessage_Type pPostMessage, HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam) {
    BOOL res;
    res = (*pPostMessage)(hWnd, Msg, wParam, lParam);
    OutTraceW("%s: hwnd=%#x msg=%#x(%s) wparam=%#x, lparam=%#x res=%#x\n",
              api, hWnd, Msg, ExplainWinMessage(Msg), wParam, lParam, res);
    return res;
}

BOOL WINAPI extPostMessageA(HWND hwnd, UINT Msg, WPARAM wParam, LPARAM lParam) {
    return extPostMessage("PostMessageA", pPostMessageA, hwnd, Msg, wParam, lParam);
}
BOOL WINAPI extPostMessageW(HWND hwnd, UINT Msg, WPARAM wParam, LPARAM lParam) {
    return extPostMessage("PostMessageW", pPostMessageW, hwnd, Msg, wParam, lParam);
}

BOOL WINAPI extClientToScreen(HWND hwnd, LPPOINT lppoint) {
    // v2.02.10: fully revised to handle scaled windows
    BOOL res;
    ApiName("ClientToScreen");
    OutDebugSYS("%s: hwnd=%#x hWnd=%#x FullScreen=%#x point=(%d,%d)\n",
                ApiRef, hwnd, dxw.GethWnd(), dxw.IsFullScreen(), lppoint->x, lppoint->y);
    if (lppoint && dxw.IsFullScreen()) {
        // optimization: in fullscreen mode, coordinate conversion for the desktop window
        // should always keep the same values inaltered
        if(hwnd != dxw.GethWnd())
            *lppoint = dxw.AddCoordinates(*lppoint, dxw.ClientOffset(hwnd));
        OutDebugDW("%s: FIXED point=(%d,%d)\n", ApiRef, lppoint->x, lppoint->y);
        res = TRUE;
    } else
        res = (*pClientToScreen)(hwnd, lppoint);
    return res;
}

BOOL WINAPI extScreenToClient(HWND hwnd, LPPOINT lppoint) {
    // v2.02.10: fully revised to handle scaled windows
    BOOL res;
    ApiName("ScreenToClient");
    OutDebugSYS("%s: hwnd=%#x hWnd=%#x FullScreen=%#x point=(%d,%d)\n",
                ApiRef, hwnd, dxw.GethWnd(), dxw.IsFullScreen(), lppoint->x, lppoint->y);
    if (lppoint && (lppoint->x == -32000) && (lppoint->y == -32000)) return 1;
    if (lppoint && dxw.IsFullScreen()) {
        // optimization: in fullscreen mode, coordinate conversion for the desktop window
        // should always keep the same values inaltered
        if(hwnd != dxw.GethWnd()) {
            *lppoint = dxw.SubCoordinates(*lppoint, dxw.ClientOffset(hwnd));
            OutDebugDW("%s: FIXED point=(%d,%d)\n", ApiRef, lppoint->x, lppoint->y);
        }
        res = TRUE;
    } else
        res = (*pScreenToClient)(hwnd, lppoint);
    OutDebugSYS("%s: returned point=(%d,%d)\n", ApiRef, lppoint->x, lppoint->y);
    return res;
}

BOOL WINAPI extGetClientRect(HWND hwnd, LPRECT lpRect) {
    BOOL ret;
    ApiName("GetClientRect");
    OutDebugDW("%s: whnd=%#x FullScreen=%#x\n", ApiRef, hwnd, dxw.IsFullScreen());
    if(!lpRect) return 0;
    // v2.04.30: fix for Avernum3 - when GetWindowRect is called before main window hWnd is set,
    // simply return the emulated desktop size.
    if(dxw.IsDesktop(hwnd) && dxw.IsFullScreen()) {
        *lpRect = dxw.GetScreenRect();
        OutDebugDW("%s: virtual desktop rect=(%d,%d)-(%d,%d) at %d\n",
                   ApiRef, lpRect->left, lpRect->top, lpRect->right, lpRect->bottom, __LINE__);
        return TRUE;
    }
    // proxed call
    ret = (*pGetClientRect)(hwnd, lpRect);
    if(!ret) {
        OutTraceE("%s: ERROR hwnd=%#x err=%d at %d\n", ApiRef, hwnd, GetLastError(), __LINE__);
        return ret;
    }
    OutDebugDW("%s: actual rect=(%d,%d)-(%d,%d)\n", ApiRef, lpRect->left, lpRect->top, lpRect->right, lpRect->bottom);
    if (dxw.IsDesktop(hwnd)) {
        *lpRect = dxw.GetScreenRect();
        OutDebugDW("%s: desktop rect=(%d,%d)-(%d,%d)\n", ApiRef, lpRect->left, lpRect->top, lpRect->right, lpRect->bottom);
    } else if (dxw.IsFullScreen()) {
        *lpRect = dxw.GetClientRect(*lpRect);
        OutDebugDW("%s: fixed rect=(%d,%d)-(%d,%d)\n", ApiRef, lpRect->left, lpRect->top, lpRect->right, lpRect->bottom);
    }
    return ret;
}

BOOL WINAPI extGetWindowRect(HWND hwnd, LPRECT lpRect) {
    BOOL ret;
    ApiName("GetWindowRect");
    OutDebugDW("%s: hwnd=%#x hWnd=%#x FullScreen=%#x\n", ApiRef, hwnd, dxw.GethWnd(), dxw.IsFullScreen());
    if(!lpRect) return 0;
    // v2.04.30: fix for Avernum3 - when GetWindowRect is called before main window hWnd is set,
    // simply return the emulated desktop size.
    if(dxw.IsDesktop(hwnd) && dxw.IsFullScreen()) {
        *lpRect = dxw.GetScreenRect();
        OutDebugDW("%s: virtual desktop rect=(%d,%d)-(%d,%d) at %d\n",
                   ApiRef, lpRect->left, lpRect->top, lpRect->right, lpRect->bottom, __LINE__);
        return TRUE;
    }
    if(dxw.IsRealDesktop(hwnd)) {
        // v2.03.52, v2.03.61: fix for "Storm Angel" and "Geneforge" :
        // replace the real desktop with the virtual one only if that doesn't cause troubles.
        HWND hwnd_try = dxw.GethWnd();
        if ((*pGetWindowRect)(hwnd, lpRect)) hwnd = hwnd_try;
    }
    ret = (*pGetWindowRect)(hwnd, lpRect);
    if(!ret) {
        OutTraceE("%s: GetWindowRect hwnd=%#x error %d at %d\n", ApiRef, hwnd, GetLastError(), __LINE__);
        return ret;
    }
    OutDebugDW("%s: rect=(%d,%d)-(%d,%d)\n", ApiRef, lpRect->left, lpRect->top, lpRect->right, lpRect->bottom);
    // minimized windows behaviour
    if((lpRect->left == -32000) || (lpRect->top == -32000)) return ret;
    if (dxw.IsDesktop(hwnd)) {
        // to avoid keeping track of window frame
        *lpRect = dxw.GetScreenRect();
        OutDebugDW("%s: desktop rect=(%d,%d)-(%d,%d)\n", ApiRef, lpRect->left, lpRect->top, lpRect->right, lpRect->bottom);
    } else if (dxw.IsFullScreen()) {
        *lpRect = dxw.GetWindowRect(*lpRect);
        // Diablo fix: it retrieves coordinates for the explorer window, that are as big as the real desktop!!!
        if(lpRect->left < 0) lpRect->left = 0;
        //		if(lpRect->left > (LONG)dxw.GetScreenWidth()) lpRect->left=dxw.GetScreenWidth();
        //		if(lpRect->right < 0) lpRect->right=0;
        if(lpRect->right > (LONG)dxw.GetScreenWidth()) lpRect->right = dxw.GetScreenWidth();
        if(lpRect->top < 0) lpRect->top = 0;
        //		if(lpRect->top > (LONG)dxw.GetScreenHeight()) lpRect->top=dxw.GetScreenHeight();
        //		if(lpRect->bottom < 0) lpRect->bottom=0;
        if(lpRect->bottom > (LONG)dxw.GetScreenHeight()) lpRect->bottom = dxw.GetScreenHeight();
        OutDebugDW("%s: fixed rect=(%d,%d)-(%d,%d)\n", ApiRef, lpRect->left, lpRect->top, lpRect->right, lpRect->bottom);
    }
    return ret;
}


int WINAPI extMapWindowPoints(HWND hWndFrom, HWND hWndTo, LPPOINT lpPoints, UINT cPoints) {
    UINT pi;
    int ret;
    ApiName("MapWindowPoints");
    // a rarely used API, but responsible for a painful headache: needs hooking for "Commandos 2", "Alien Nations".
    // used also in "Full Pipe" activemovie
    // used also in "NBA Live 99" menu screen
    OutTraceSYS("%s: hWndFrom=%#x%s hWndTo=%#x%s cPoints=%d FullScreen=%#x\n",
                ApiRef, hWndFrom, dxw.IsDesktop(hWndFrom) ? "(DESKTOP)" : "",
                hWndTo, dxw.IsDesktop(hWndTo) ? "(DESKTOP)" : "",
                cPoints, dxw.IsFullScreen());
#ifndef DXW_NOTRACES
    if(IsDebugSYS) {
        OutTrace("Points: ");
        for(pi = 0; pi < cPoints; pi++) OutTrace("(%d,%d)", lpPoints[pi].x, lpPoints[pi].y);
        OutTrace("\n");
    }
#endif
    if(dxw.IsFullScreen()) {
        if(dxw.IsRealDesktop(hWndTo)) hWndTo = dxw.GethWnd();
        if(dxw.IsRealDesktop(hWndFrom)) hWndFrom = dxw.GethWnd();
    }
    ret = (*pMapWindowPoints)(hWndFrom, hWndTo, lpPoints, cPoints);
    // v2.03.16: now must scale every point (fixes "NBA Live 99")
    // v2.03.18: in some cases it should not! "New Your Race"...
    // v2.03.56: scale only on scaled modes
    switch(dxw.GDIEmulationMode) {
    case GDIMODE_SHAREDDC:
    case GDIMODE_EMULATED:
    default:
        break;
    case GDIMODE_STRETCHED:
        for(pi = 0; pi < cPoints; pi++)
            dxw.UnmapClient(&lpPoints[pi]);
#ifndef DXW_NOTRACES
        if(IsDebugDW) {
            OutTrace("Mapped points: ");
            for(pi = 0; pi < cPoints; pi++) OutTrace("(%d,%d)", lpPoints[pi].x, lpPoints[pi].y);
            OutTrace("\n");
        }
#endif
        break;
    }
    // If the function succeeds, the low-order word of the return value is the number of pixels
    // added to the horizontal coordinate of each source point in order to compute the horizontal
    // coordinate of each destination point. (In addition to that, if precisely one of hWndFrom
    // and hWndTo is mirrored, then each resulting horizontal coordinate is multiplied by -1.)
    // The high-order word is the number of pixels added to the vertical coordinate of each source
    // point in order to compute the vertical coordinate of each destination point.
    OutTraceDW("%s: ret=%#x (%d,%d)\n",
               ApiRef, ret, (short)((ret & 0xFFFF0000) >> 16), (short)(ret & 0x0000FFFF));
    return ret;
}

HWND WINAPI extGetDesktopWindow(void) {
    HWND res;
    ApiName("GetDesktopWindow");
    if((!dxw.Windowize) || (dxw.dwFlags5 & DIABLOTWEAK)) {
        HWND ret;
        ret = (*pGetDesktopWindow)();
        OutTraceDW("%s: BYPASS ret=%#x\n", ApiRef, ret);
        return ret;
    }
    OutTraceDW("%s: FullScreen=%#x\n", ApiRef, dxw.IsFullScreen());
    // v2.04.01.fx4: do not return the main window if we still don't have one (dxw.GethWnd() == NULL)
    if (dxw.IsFullScreen() && dxw.GethWnd()) {
        OutTraceDW("%s: returning main window hwnd=%#x\n", ApiRef, dxw.GethWnd());
        return dxw.GethWnd();
    } else {
        res = (*pGetDesktopWindow)();
        OutTraceDW("%s: returning desktop window hwnd=%#x\n", ApiRef, res);
        return res;
    }
}

int WINAPI extGetSystemMetrics(int nindex) {
    HRESULT res;
    ApiName("GetSystemMetrics");
    res = (*pGetSystemMetrics)(nindex);
    OutTraceDW("%s: index=%#x(%s), res=%d\n", ApiRef, nindex, ExplainsSystemMetrics(nindex), res);
    if(!dxw.Windowize) {
        // v2.02.95: if not in window mode, just implement the HIDEMULTIMONITOR flag
        if( (nindex == SM_CMONITORS) &&
                (dxw.dwFlags2 & HIDEMULTIMONITOR) &&
                res > 1) {
            res = 1;
            OutTraceDW("%s: fix SM_CMONITORS=%d\n", ApiRef, res);
        }
        return res;
    }
    switch(nindex) {
    case SM_CXFULLSCREEN:
    case SM_CXSCREEN:
    case SM_CXVIRTUALSCREEN: // v2.02.31
        res = dxw.GetScreenWidth();
        OutTraceDW("%s: fix %s=%d\n", ApiRef, ExplainsSystemMetrics(nindex), res);
        break;
    case SM_CYFULLSCREEN:
    case SM_CYSCREEN:
    case SM_CYVIRTUALSCREEN: // v2.02.31
        res = dxw.GetScreenHeight();
        OutTraceDW("%s: fix %s=%d\n", ApiRef, ExplainsSystemMetrics(nindex), res);
        break;
    case SM_CMONITORS:
        if((dxw.dwFlags2 & HIDEMULTIMONITOR) && res > 1) {
            res = 1;
            OutTraceDW("%s: fix SM_CMONITORS=%d\n", ApiRef, res);
        }
        break;
    }
    return res;
}

ATOM WINAPI extRegisterClassExA(WNDCLASSEXA *lpwcx) {
    ATOM ret;
    ApiName("RegisterClassExA");
    OutTraceDW("%s: PROXED ClassName=\"%s\" style=%#x(%s) WndProc=%#x cbClsExtra=%d cbWndExtra=%d hInstance=%#x\n",
               ApiRef, lpwcx->lpszClassName, lpwcx->style, ExplainStyle(lpwcx->style), lpwcx->lpfnWndProc, lpwcx->cbClsExtra, lpwcx->cbWndExtra, lpwcx->hInstance);
    if ((dxw.dwFlags11 & CUSTOMLOCALE) &&
            (dxw.dwFlags12 & CLASSLOCALE) &&
            pRegisterClassExW) {
        LPWSTR lpClassNameW;
        WNDCLASSEXW wcw;
        memcpy(&wcw, lpwcx, sizeof(wcw));
        BOOL IsStringClass = ((DWORD)lpwcx->lpszClassName & WM_CLASSMASK);
        lpClassNameW = IsStringClass ? (LPWSTR)malloc((strlen(lpwcx->lpszClassName) + 1) * 2) : (LPWSTR)lpwcx->lpszClassName;
        if(IsStringClass) {
            int size = strlen(lpwcx->lpszClassName);
            int n = (*pMultiByteToWideChar)(dxw.CodePage, 0, lpwcx->lpszClassName, size, lpClassNameW, size);
            lpClassNameW[n] = L'\0'; // make tail !
            OutTraceLOC("%s: WIDE class=\"%ls\"\n", ApiRef, lpClassNameW);
        }
        wcw.lpszClassName = lpClassNameW;
        ret = (*pRegisterClassExW)(&wcw);
        if(IsStringClass) free(lpClassNameW);
    } else
        ret = (*pRegisterClassExA)(lpwcx);
    if(ret)
        OutTraceDW("%s: atom=%#x\n", ApiRef, ret);
    else
        OutTraceE("%s: ERROR atom=NULL err=%d\n", ApiRef, GetLastError());
    return ret;
}

ATOM WINAPI extRegisterClassA(WNDCLASSA *lpwcx) {
    ATOM ret;
    ApiName("RegisterClassA");
    // referenced by Syberia, together with RegisterClassExA
    OutTraceDW("%s: PROXED ClassName=\"%s\" style=%#x(%s) WndProc=%#x cbClsExtra=%d cbWndExtra=%d hInstance=%#x\n",
               ApiRef, lpwcx->lpszClassName, lpwcx->style, ExplainStyle(lpwcx->style), lpwcx->lpfnWndProc, lpwcx->cbClsExtra, lpwcx->cbWndExtra, lpwcx->hInstance);
    if ((dxw.dwFlags11 & CUSTOMLOCALE) &&
            (dxw.dwFlags12 & CLASSLOCALE) &&
            pRegisterClassW) {
        LPWSTR lpClassNameW;
        WNDCLASSW wcw;
        memcpy(&wcw, lpwcx, sizeof(wcw));
        BOOL IsStringClass = ((DWORD)lpwcx->lpszClassName & WM_CLASSMASK);
        lpClassNameW = IsStringClass ? (LPWSTR)malloc((strlen(lpwcx->lpszClassName) + 1) * 2) : (LPWSTR)lpwcx->lpszClassName;
        if(IsStringClass) {
            int size = strlen(lpwcx->lpszClassName);
            int n = (*pMultiByteToWideChar)(dxw.CodePage, 0, lpwcx->lpszClassName, size, lpClassNameW, size);
            lpClassNameW[n] = L'\0'; // make tail !
            OutTraceLOC("%s: WIDE class=\"%ls\"\n", ApiRef, lpClassNameW);
        }
        wcw.lpszClassName = lpClassNameW;
        ret = (*pRegisterClassW)(&wcw);
        if(IsStringClass) free(lpClassNameW);
    } else
        ret = (*pRegisterClassA)(lpwcx);
    if(ret)
        OutTraceDW("%s: atom=%#x\n", ApiRef, ret);
    else
        OutTraceE("%s: ERROR atom=NULL err=%d\n", ApiRef, GetLastError());
    return ret;
}

ATOM WINAPI extRegisterClassExW(WNDCLASSEXW *lpwcx) {
    ATOM ret;
    ApiName("RegisterClassExW");
    OutTraceDW("%s: PROXED ClassName=\"%ls\" style=%#x(%s) WndProc=%#x cbClsExtra=%d cbWndExtra=%d hInstance=%#x\n",
               ApiRef, lpwcx->lpszClassName, lpwcx->style, ExplainStyle(lpwcx->style), lpwcx->lpfnWndProc, lpwcx->cbClsExtra, lpwcx->cbWndExtra, lpwcx->hInstance);
    ret = (*pRegisterClassExW)(lpwcx);
    if(ret)
        OutTraceDW("%s: atom=%#x\n", ApiRef, ret);
    else
        OutTraceE("%s: ERROR atom=NULL err=%d\n", ApiRef, GetLastError());
    return ret;
}

ATOM WINAPI extRegisterClassW(WNDCLASSW *lpwcx) {
    ATOM ret;
    ApiName("RegisterClassW");
    OutTraceDW("%s: PROXED ClassName=\"%ls\" style=%#x(%s) WndProc=%#x cbClsExtra=%d cbWndExtra=%d hInstance=%#x\n",
               ApiRef, lpwcx->lpszClassName, lpwcx->style, ExplainStyle(lpwcx->style), lpwcx->lpfnWndProc, lpwcx->cbClsExtra, lpwcx->cbWndExtra, lpwcx->hInstance);
    ret = (*pRegisterClassW)(lpwcx);
    if(ret)
        OutTraceDW("%s: atom=%#x\n", ApiRef, ret);
    else
        OutTraceE("%s: ERROR atom=NULL err=%d\n", ApiRef, GetLastError());
    return ret;
}

int WINAPI extGetClassNameA(HWND hwnd, LPSTR lpClassName, int MaxCount) {
    int ret;
    ApiName("GetClassNameA");
    OutTraceSYS("%s: hwnd=%#x max=%d\n", ApiRef, hwnd, MaxCount);
    if ((dxw.dwFlags11 & CUSTOMLOCALE) &&
            (dxw.dwFlags12 & CLASSLOCALE)) {
        // try ANSI first, it could be an ANSI system class
        ret = (*pGetClassNameA)(hwnd, lpClassName, MaxCount);
        if(ret)
            OutTraceLOC("%s: ANSI class=\"%s\"\n", ApiRef, lpClassName);
        else {
            LPWSTR lpClassNameW = (LPWSTR)malloc((MaxCount + 1) * 2);
            ret = GetClassNameW(hwnd, lpClassNameW, MaxCount);
            OutTraceLOC("%s: WIDE class=\"%ls\"\n", lpClassNameW);
            (*pWideCharToMultiByte)(dxw.CodePage, 0, lpClassNameW, MaxCount, lpClassName, MaxCount, NULL, FALSE);
            free(lpClassNameW);
        }
    } else
        ret = (*pGetClassNameA)(hwnd, lpClassName, MaxCount);
    if(ret)
        OutTraceSYS("%s: class=\"%s\" ret-len=%d\n", ApiRef, lpClassName, ret);
    else
        OutTraceE("%s: err=%d\n", ApiRef, GetLastError());
    return ret;
}

BOOL WINAPI extGetClassInfoA(HINSTANCE hInst, LPCSTR lpClassName, LPWNDCLASSA lpWndClass) {
    BOOL ret;
    ApiName("GetClassInfoA");
    if((DWORD)lpClassName & WM_CLASSMASK)
        OutTraceSYS("%s: hinst=%#x class=\"%s\"\n", ApiRef, hInst, lpClassName);
    else
        OutTraceSYS("%s: hinst=%#x class=%#x\n", ApiRef, hInst, lpClassName);
    if ((dxw.dwFlags11 & CUSTOMLOCALE) &&
            (dxw.dwFlags12 & CLASSLOCALE) &&
            (hInst != 0)) {
        // try ANSI first, it could be an ANSI system class
        ret = (*pGetClassInfoA)(hInst, lpClassName, lpWndClass);
        if(ret)
            OutTraceLOC("%s: ANSI class=\"%s\"\n", ApiRef, lpClassName);
        else {
            WNDCLASSW WndClassW;
            LPWSTR lpClassNameW;
            BOOL IsStringClass = ((DWORD)lpClassName & WM_CLASSMASK);
            lpClassNameW = IsStringClass ? (LPWSTR)malloc((strlen(lpClassName) + 1) * 2) : (LPWSTR)lpClassName;
            if(IsStringClass) {
                int size = strlen(lpClassName);
                int n = (*pMultiByteToWideChar)(dxw.CodePage, 0, lpClassName, size, lpClassNameW, size);
                lpClassNameW[n] = L'\0'; // make tail !
                OutTraceLOC("%s: WIDE class=\"%ls\"\n", ApiRef, lpClassNameW);
            }
            ret = GetClassInfoW(hInst, lpClassNameW, &WndClassW);
            if(ret) {
                memcpy(lpWndClass, &WndClassW, sizeof(WNDCLASSA));
                // todo: convert WIDE results to ANSI
                //IsStringClass = ((DWORD)WndClassW.lpszClassName & WM_CLASSMASK);
                //lpWndClass->lpszClassName = IsStringClass ? (LPCSTR)malloc((wcslen(WndClassW.lpszClassName) * 2) + 1) : (LPCSTR)WndClassW.lpszClassName;
                //if(IsStringClass) {
                //	int size = wcslen(WndClassW.lpszClassName);
                //	(*pWideCharToMultiByte)(dxw.CodePage, 0, WndClassW.lpszClassName, size+1, (LPSTR)(lpWndClass->lpszClassName), size+1, NULL, NULL);
                //}
                //lpWndClass->lpszClassName = (LPCSTR)malloc(strlen(lpClassName)+1); // no good
                //strcpy((LPSTR)(lpWndClass->lpszClassName), lpClassName);
                //lpWndClass->lpszClassName = lpClassName;
                //lpWndClass->lpszMenuName = NULL; // no good
                //if(lpWndClass->lpfnWndProc == DefWindowProcW) lpWndClass->lpfnWndProc = pDefWindowProcA;
                //lpWndClass->lpfnWndProc = pDefWindowProcA;
            }
            if(IsStringClass) free(lpClassNameW);
        }
    } else
        ret = (*pGetClassInfoA)(hInst, lpClassName, lpWndClass);
    if(ret) {
        // beware: "Piskworky 2001" makes a comparison on the returned lpfnWndProc field!!
        OutTraceSYS("wndproc> %#x\n", lpWndClass->lpfnWndProc);
        OutTraceSYS("style> %#x\n", lpWndClass->style);
        OutTraceSYS("hicon> %#x\n", lpWndClass->hIcon);
        OutTraceSYS("hcurs> %#x\n", lpWndClass->hCursor);
        OutTraceSYS("hback> %#x\n", lpWndClass->hbrBackground);
        OutTraceSYS("class> %s\n", lpWndClass->lpszClassName);
        OutTraceSYS("menu> %s\n", lpWndClass->lpszMenuName);
        OutTraceSYS("%s: ret=%#x\n", ApiRef, ret);
    } else
        OutTraceE("%s: err=%d\n", ApiRef, GetLastError());
    return ret;
}

BOOL WINAPI extUnregisterClassA(LPCSTR lpClassName, HINSTANCE hInst) {
    BOOL ret;
    ApiName("UnregisterClassA");
    OutTraceSYS("%s: hinst=%#x class=\"%s\"\n", ApiRef, hInst, lpClassName);
    if ((dxw.dwFlags11 & CUSTOMLOCALE) &&
            (dxw.dwFlags12 & CLASSLOCALE) &&
            (hInst != 0)) {
        // try ANSI first, it could be an ANSI system class
        ret = (*pUnregisterClassA)(lpClassName, hInst);
        if(ret)
            OutTraceLOC("%s: ANSI class=\"%s\"\n", ApiRef, lpClassName);
        else {
            LPWSTR lpClassNameW;
            BOOL IsStringClass = ((DWORD)lpClassName & WM_CLASSMASK);
            lpClassNameW = IsStringClass ? (LPWSTR)malloc((strlen(lpClassName) + 1) * 2) : (LPWSTR)lpClassName;
            if(IsStringClass) {
                int size = strlen(lpClassName);
                int n = (*pMultiByteToWideChar)(dxw.CodePage, 0, lpClassName, size, lpClassNameW, size);
                lpClassNameW[n] = L'\0'; // make tail !
                OutTraceLOC("%s: WIDE class=\"%ls\"\n", ApiRef, lpClassNameW);
            }
            ret = UnregisterClassW(lpClassNameW, hInst);
        }
    } else
        ret = (*pUnregisterClassA)(lpClassName, hInst);
    if(ret)
        OutTraceSYS("%s: OK\n", ApiRef);
    else
        OutTraceE("%s: err=%d\n", ApiRef, GetLastError());
    return ret;
}

static void HookChildWndProc(HWND hwnd, DWORD dwStyle, LPCTSTR ApiName) {
    // child window inherit the father's windproc, so if it's redirected to
    // a hooker (either extWindowProc or extChildWindowProc) you have to retrieve
    // the correct value (dxwws.GetProc) before saving it (dxwws.PutProc).
    long res;
    WNDPROC pWindowProc;
    if(dxw.dwDFlags2 & NOWINDOWHOOKS) return;
    pWindowProc = (WNDPROC)(*pGetWindowLong)(hwnd, GWL_WNDPROC);
    extern LRESULT CALLBACK dw_Hider_Message_Handler(HWND, UINT, WPARAM, LPARAM);
    if(pWindowProc == dw_Hider_Message_Handler) return;
    if((pWindowProc == extWindowProc) ||
            (pWindowProc == extChildWindowProc) ||
            (pWindowProc == extDialogWindowProc)) { // avoid recursions
        HWND Father;
        WNDPROC pFatherProc;
        Father = GetParent(hwnd);
        pFatherProc = dxwws.GetProc(Father);
        OutTraceDW("%s: WndProc=%s father=%#x WndProc=%#x\n", ApiName,
                   (pWindowProc == extWindowProc) ? "extWindowProc" : ((pWindowProc == extChildWindowProc) ? "extChildWindowProc" : "extDialogWindowProc"),
                   Father, pFatherProc);
        pWindowProc = pFatherProc;
    }
    dxwws.PutProc(hwnd, pWindowProc);
    if(dwStyle & WS_CHILD) {
        OutTraceDW("%s: Hooking CHILD hwnd=%#x father WindowProc %#x->%#x\n", ApiName, hwnd, pWindowProc, extChildWindowProc);
        res = (*pSetWindowLong)(hwnd, GWL_WNDPROC, (LONG)extChildWindowProc);
    } else { // must be dwStyle & WS_DLGFRAME
        OutTraceDW("%s: Hooking DLGFRAME hwnd=%#x father WindowProc %#x->%#x\n", ApiName, hwnd, pWindowProc, extDialogWindowProc);
        res = (*pSetWindowLong)(hwnd, GWL_WNDPROC, (LONG)extDialogWindowProc);
    }
#ifndef DXW_NOTRACES
    if(!res) OutTraceE("%s: SetWindowLong ERROR %#x\n", ApiName, GetLastError());
#endif // DXW_NOTRACES
}

HWND hControlParentWnd = NULL;

static HWND hLastFullScrWin = 0;
static DDPIXELFORMAT ddpLastPixelFormat;
#define SAFEWINDOWCREATION TRUE

typedef HWND (WINAPI *CreateWindow_Type)(DWORD, LPVOID, LPVOID, DWORD, int, int, int, int, HWND, HMENU, HINSTANCE, LPVOID);

static HWND WINAPI CreateWindowCommon(
    LPCTSTR api,
    CreateWindow_Type pCreateWindow,
    BOOL bActiveMovie,
    DWORD dwExStyle,
    void *lpClassName,
    void *lpWindowName,
    DWORD dwStyle,
    int x,
    int y,
    int nWidth,
    int nHeight,
    HWND hWndParent,
    HMENU hMenu,
    HINSTANCE hInstance,
    LPVOID lpParam) {
    HWND hwnd;
    BOOL isValidHandle = TRUE;
    BOOL isNewDesktop;
    int origx, origy, origw, origh;
    DWORD origstyle, origexstyle;
    BOOL bShowSpec = FALSE;
    int CmdShow;
    origx = x;
    origy = y;
    origw = nWidth;
    origh = nHeight;
    origstyle = dwStyle;
    origexstyle = dwExStyle;
    if(!dxw.Windowize || (hWndParent == HWND_MESSAGE)) { // v2.02.87: don't process message windows (hWndParent == HWND_MESSAGE)
        // v2.04.73: also in fullscreen mode add WS_CLIPCHILDREN if asked to. Fixes "Psychotoxic".
        if((dxw.dwFlags4 & FORCECLIPCHILDREN) && dxw.IsRealDesktop(hWndParent)) {
            OutTraceDW("%s: fixed style +WS_CLIPCHILDREN\n", ApiRef);
            dwStyle |= WS_CLIPCHILDREN;
        }
        hwnd = (*pCreateWindow)(dwExStyle, lpClassName, lpWindowName, dwStyle, x, y, nWidth, nHeight, hWndParent, hMenu, hInstance, lpParam);
        if ((dxw.dwFlags1 & HOOKCHILDWIN) && (dwStyle & (WS_CHILD | WS_DLGFRAME)))
            HookChildWndProc(hwnd, dwStyle, ApiRef);
        OutTraceDW("%s: ret=%#x\n", ApiRef, hwnd);
        return hwnd;
    }
    OutDebugDW("%s: ActiveMovie=%#x\n", ApiRef, bActiveMovie);
    if(bActiveMovie && dxw.Windowize) {
        if(dxw.IsRealDesktop(hWndParent)) {
            if(dxw.GethWnd()) {
                OutTraceDW("%s: on ActiveMovie Window FIX parent=%#x->%#x\n", ApiRef, hWndParent, dxw.GethWnd());
                hWndParent = dxw.GethWnd();
                // v2.04.90: beware: turning WS_DLGFRAME off seems to cause deafness on listening the ESC key to
                // interrupt the movie in "War Times".
                //dwStyle &= ~(WS_BORDER|WS_CAPTION|WS_DLGFRAME|WS_MAXIMIZEBOX|WS_MINIMIZEBOX|WS_SIZEBOX|WS_THICKFRAME);
                dwStyle &= ~(WS_BORDER | WS_CAPTION | WS_SYSMENU | WS_MAXIMIZEBOX | WS_MINIMIZEBOX | WS_SIZEBOX | WS_THICKFRAME);
                origstyle = dwStyle;
                OutTraceDW("%s: STRETCHMOVIES on ActiveMovie Window style=%#x(%s)\n", ApiRef, dwStyle, ExplainStyle(dwStyle));
            } else
                OutTraceDW("%s: on ActiveMovie Window no parent!!\n", ApiRef);
        }
        if(dxw.dwFlags6 & STRETCHMOVIES) {
            nWidth = dxw.GetScreenWidth();
            nHeight = dxw.GetScreenHeight();
            x = 0;
            y = 0;
            OutTraceDW("%s: STRETCHMOVIES on ActiveMovie Window unmapped size=(%dx%d)\n", ApiRef, nWidth, nHeight);
        }
    }
    // good for all modes
    if(dxw.dwFlags5 & UNLOCKZORDER)  dwExStyle &= ~WS_EX_TOPMOST ;
    if(dxw.dwFlags9 & LOCKTOPZORDER) dwExStyle |=  WS_EX_TOPMOST ;
    // no maximized windows in any case
    if (dxw.dwFlags1 & PREVENTMAXIMIZE) {
        OutTraceDW("%s: handling PREVENTMAXIMIZE mode\n", ApiRef);
        dwStyle &= ~WS_MAXIMIZE;
    }
    // v2.1.92: fixes size & position for auxiliary big window, often used
    // for intro movies etc. : needed for ......
    // evidently, this was supposed to be a fullscreen window....
    // v2.1.100: fixes for "The Grinch": this game creates a new main window for OpenGL
    // rendering using CW_USEDEFAULT placement and 800x600 size while the previous
    // main win was 640x480 only!
    // v2.02.13: if it's a WS_CHILD window, don't reposition the x,y, placement for BIG win.
    // v2.02.30: fix (Fable - lost chapters) Fable creates a bigger win with negative x,y coordinates.
    // v2.03.53: revised code, logic moved to IsFullscreenWindow
    if(isNewDesktop = IsFullscreenWindow(dwStyle, dwExStyle, hWndParent, x, y, nWidth, nHeight)) {
        OutDebugDW("%s: ASSERT IsFullscreenWindow==TRUE\n", ApiRef);
        // if already in fullscreen mode, save previous settings
        if(dxw.IsFullScreen() && dxw.GethWnd()) {
            hLastFullScrWin = dxw.GethWnd();
            ddpLastPixelFormat = dxw.VirtualPixelFormat;
        }
        // inserted some checks here, since the main window could be destroyed
        // or minimized (see "Jedi Outcast") so that you may get a dangerous
        // zero size. In this case, better renew the hWnd assignement and its coordinates.
        isValidHandle = dxw.IsValidMainWindow();
        // v2.03.58 fix: don't consider CW_USEDEFAULT as a big unsigned integer!! Fixes "Imperialism".
        // v2.04.87 fix: use boolean AND to detect default size, not strict EQUAL operator
        // fix CW_USEDEFAULT coordinates. We haven't scaled yet, so coordinates must be logical
        // from MSDN:
        // X: The initial horizontal position of the window. For an overlapped or pop-up window, the x parameter
        // is the initial x-coordinate of the window's upper-left corner, in screen coordinates. For a child window,
        // x is the x-coordinate of the upper-left corner of the window relative to the upper-left corner of the
        // parent window's client area. If x is set to CW_USEDEFAULT, the system selects the default position for
        // the window's upper-left corner and ignores the y parameter. CW_USEDEFAULT is valid only for overlapped
        // windows; if it is specified for a pop-up or child window, the x and y parameters are set to zero.
        //
        // Y: The initial vertical position of the window. For an overlapped or pop-up window, the y parameter is
        // the initial y-coordinate of the window's upper-left corner, in screen coordinates. For a child window,
        // y is the initial y-coordinate of the upper-left corner of the child window relative to the upper-left
        // corner of the parent window's client area. For a list box y is the initial y-coordinate of the upper-left
        // corner of the list box's client area relative to the upper-left corner of the parent window's client area.
        //
        // If an overlapped window is created with the WS_VISIBLE style bit set and the x parameter is set to
        // CW_USEDEFAULT, then the y parameter determines how the window is shown. If the y parameter is CW_USEDEFAULT,
        // then the window manager calls ShowWindow with the SW_SHOW flag after the window has been created.
        // If the y parameter is some other value, then the window manager calls ShowWindow with that value as the
        // nCmdShow parameter.
        if(nWidth & CW_USEDEFAULT) nWidth = dxw.GetScreenWidth();
        if(nHeight & CW_USEDEFAULT) nHeight = dxw.GetScreenHeight();
        if(x & CW_USEDEFAULT) {
            if(dwStyle & WS_VISIBLE) {
                bShowSpec = TRUE;
                if (y & CW_USEDEFAULT) CmdShow = SW_SHOW;
                else CmdShow = y;
                if(CmdShow == SW_SHOWMAXIMIZED) CmdShow = SW_SHOWNORMAL;
                OutTraceDW("%s: VISIBLE win CmdShow=%#x(%s)\n", ApiRef, CmdShow, ExplainShowCmd(CmdShow));
            }
            x = 0;
            y = 0; // y value is ignored
        }
        if(y & CW_USEDEFAULT) y = 0;
        OutTraceDW("%s: unmapped win pos=(%d,%d) size=(%d,%d) valid=%#x\n",
                   ApiRef, x, y, nWidth, nHeight, isValidHandle);
        // declare fullscreen mode
        dxw.SetFullScreen(TRUE);
        // update virtual screen size if it has grown
        // v2.04.96: do that ONLY on outer window creations!!
        if(!InMainWinCreation) dxw.SetScreenSize(nWidth, nHeight);
    }
    if(!dxw.IsFullScreen()) { // v2.1.63: needed for "Monster Truck Madness"
        hwnd = (*pCreateWindow)(dwExStyle, lpClassName, lpWindowName, dwStyle, x, y, nWidth, nHeight, hWndParent, hMenu, hInstance, lpParam);
        if(hwnd)
            OutTraceDW("%s: windowed mode ret=%#x\n", ApiRef, hwnd);
        else
            OutTraceDW("%s: windowed mode ERROR err=%d\n", ApiRef, GetLastError());
        return hwnd;
    }
    // from here on, fullscreen is garanteed
    if (IsRelativePosition(dwStyle, dwExStyle, hWndParent)) {
        dxw.MapClient(&x, &y, &nWidth, &nHeight);
        OutTraceDW("%s: fixed RELATIVE pos=(%d,%d) size=(%d,%d)\n", ApiRef, x, y, nWidth, nHeight);
    } else {
        dxw.MapWindow(&x, &y, &nWidth, &nHeight);
        OutTraceDW("%s: fixed ABSOLUTE pos=(%d,%d) size=(%d,%d)\n", ApiRef, x, y, nWidth, nHeight);
    }
    if(dxw.dwFlags4 & FORCECLIPCHILDREN) {
        // v2.04.73: in window mode add WS_CLIPCHILDREN if asked to. Fixes "Psychotoxic".
        OutTraceDW("%s: fixed style +WS_CLIPCHILDREN\n", ApiRef);
        dwStyle |= WS_CLIPCHILDREN;
        origstyle |= WS_CLIPCHILDREN;
    }
    // if it is a new main window, give it proper style, unless
    // a) it is a child window
    // b) it is a movie window
    // n.b. the WSTYLE_DEFAULT mode is handles internally by FixWinStyle and FixWinExStyle
    if((!isValidHandle) && isNewDesktop && !(
                (dwStyle & WS_CHILD) ||						// a)
                bActiveMovie)								// b)
      ) {
        OutTraceDW("%s: set main win style\n", ApiRef);
        dwStyle = dxw.FixWinStyle(dwStyle);
        dwExStyle = dxw.FixWinExStyle(dwExStyle);
    }
    OutDebugDW("%s: CREATION pos=(%d,%d) size=(%d,%d) Style=%#x(%s) ExStyle=%#x(%s)\n",
               ApiRef, origx, origy, origw, origh, origstyle, ExplainStyle(origstyle), origexstyle, ExplainExStyle(origexstyle));
    // v2.04.02: InMainWinCreation semaphore, signals to the CreateWin callback that the window to be created will be a main window,
    // so rules about LOCKWINPOS etc. must be applied. Fixes "Civil War 2 Generals" main window displacement.
    // v2.04.05: the semaphore must be a counter, since within the CreateWin callback there could be other CreateWin calls.
    // happens in "Warhammer: Shadow of the Horned Rat" !
    // SAFEWINDOWCREATION mode: fixes problems of "Warhammer shadow of the Horned rat", but also allows "Diablo" to run in fake fullscreen high-res mode.
    // this way, any creation callback routine invoked within the window creation will receive only the original call parameters, while the new scaled
    // values and adjusted styles will be applied only after the creation.
    InMainWinCreation++;
    hwnd = (*pCreateWindow)(origexstyle, lpClassName, lpWindowName, origstyle, origx, origy, origw, origh, hWndParent, hMenu, hInstance, lpParam);
    InMainWinCreation--;
    if (hwnd == (HWND)NULL) {
        OutTraceE("%s: ERROR err=%d Style=%#x(%s) ExStyle=%#x\n",
                  ApiRef, GetLastError(), dwStyle, ExplainStyle(dwStyle), dwExStyle);
        return hwnd;
    }
    // if we have no current valid main window handle, then set it now
    if ((!isValidHandle) && isNewDesktop) dxw.SethWnd(hwnd);
    // if is a control parent, update the current control parent
    if (dwExStyle & WS_EX_CONTROLPARENT) hControlParentWnd = hwnd;
    OutDebugDW("%s: FIXED pos=(%d,%d) size=(%d,%d) Style=%#x(%s) ExStyle=%#x(%s)\n",
               ApiRef, x, y, nWidth, nHeight, dwStyle, ExplainStyle(dwStyle), dwExStyle, ExplainExStyle(dwExStyle));
    // assign final position & style
    dxw.FixWindow(hwnd, dwStyle, dwExStyle, x, y, nWidth, nHeight);
    if(bShowSpec)
        (*pShowWindow)(hwnd, CmdShow);
    else {
        // force window showing only for main win - fixes "Diablo" regression
        // v2.04.94: possibly unnecessary ... commented out. Check ...
        if(dxw.IsDesktop(hwnd))(*pShowWindow)(hwnd, SW_SHOWNORMAL);
    }
    BOOL bWPHooked = FALSE;
    if ((dxw.dwFlags1 & HOOKCHILDWIN) && (dwStyle & WS_CHILD))  {
        HookChildWndProc(hwnd, dwStyle, ApiRef);
        bWPHooked = TRUE;
    }
    if ((dxw.dwFlags8 & HOOKDLGWIN) && (dwStyle & WS_DLGFRAME)) {
        HookChildWndProc(hwnd, dwStyle, ApiRef);;
        bWPHooked = TRUE;
    }
    if(!bWPHooked && isNewDesktop) {
        // v2.05.40 fix: this was simply forgotten, it is unneeded for ddraw, d3d windows
        // but absolutely necessary for pure GDI. Fixes "Lode Runner Online" !!!
        // fixing windows message handling procedure
        dxw.HookWindowProc(hwnd);
    }
    // "Hoyle Casino Empire" needs to be in a maximized state to continue after the intro movie.
    // Sending a SW_MAXIMIZE message intercepted by the PREVENTMAXIMIZE handling fixes the problem.
    //if (dxw.IsFullScreen() && (dxw.dwFlags1 & PREVENTMAXIMIZE)){
    if ((hwnd == dxw.GethWnd()) && dxw.IsFullScreen() && (dxw.dwFlags1 & PREVENTMAXIMIZE)) {
        OutTraceDW("%s: entering maximized state\n", ApiRef);
        dxw.IsVisible = TRUE;
        (*pShowWindow)(hwnd, SW_MAXIMIZE);
    }
    if(dxw.dwFlags1 & CLIPCURSOR) dxw.SetClipCursor();
    if(dxw.dwFlags4 & HIDEDESKTOP) dxw.HideDesktop(hwnd);
    if((dxw.dwFlags5 & PUSHACTIVEMOVIE) && bActiveMovie) (*pSetWindowPos)(hwnd, HWND_BOTTOM, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOREDRAW | SWP_NOSIZE);
    if(bActiveMovie) CurrentActiveMovieWin = hwnd;
    OutTraceDW("%s: created hwnd ret=%#x\n", ApiRef, hwnd);
    return hwnd;
}


#ifndef DXW_NOTRACES
static LPCSTR ClassToStr(LPCSTR Class) {
    static char AtomBuf[20 + 1];
    if(((DWORD)Class & 0xFFFF0000) == 0) {
        sprintf(AtomBuf, "ATOM(%X)", (DWORD)Class);
        return AtomBuf;
    }
    return Class;
}

static LPCWSTR ClassToWStr(LPCWSTR Class) {
    static WCHAR AtomBuf[20 + 1];
    if(((DWORD)Class & 0xFFFF0000) == 0) {
        swprintf(AtomBuf, L"ATOM(%X)", (DWORD)Class);
        return AtomBuf;
    }
    return Class;
}
#endif // DXW_NOTRACES
// to do: implement and use ClassToWStr() for widechar call

HWND WINAPI extCreateWindowExW(
    DWORD dwExStyle,
    LPCWSTR lpClassName,
    LPCWSTR lpWindowName,
    DWORD dwStyle,
    int x,
    int y,
    int nWidth,
    int nHeight,
    HWND hWndParent,
    HMENU hMenu,
    HINSTANCE hInstance,
    LPVOID lpParam) {
    BOOL bActiveMovie;
    ApiName("CreateWindowExW");
#ifndef DXW_NOTRACES
    if(IsTraceSYS) {
        char xString[20], yString[20], wString[20], hString[20];
        if (x == CW_USEDEFAULT) strcpy(xString, "CW_USEDEFAULT");
        else sprintf(xString, "%d", x);
        if (y == CW_USEDEFAULT) strcpy(yString, "CW_USEDEFAULT");
        else sprintf(yString, "%d", y);
        if (nWidth == CW_USEDEFAULT) strcpy(wString, "CW_USEDEFAULT");
        else sprintf(wString, "%d", nWidth);
        if (nHeight == CW_USEDEFAULT) strcpy(hString, "CW_USEDEFAULT");
        else sprintf(hString, "%d", nHeight);
        OutTrace("%s: class=\"%ls\" wname=\"%ls\" pos=(%s,%s) size=(%s,%s) Style=%#x(%s) ExStyle=%#x(%s) hWndParent=%#x%s hMenu=%#x depth=%d\n",
                 ApiRef, ClassToWStr(lpClassName), lpWindowName, xString, yString, wString, hString,
                 dwStyle, ExplainStyle(dwStyle), dwExStyle, ExplainExStyle(dwExStyle),
                 hWndParent, hWndParent == HWND_MESSAGE ? "(HWND_MESSAGE)" : "", hMenu, InMainWinCreation);
    }
    OutDebugSYS("%s: DEBUG fullscreen=%#x mainwin=%#x screen=(%d,%d)\n",
                api, dxw.IsFullScreen(), dxw.GethWnd(), dxw.GetScreenWidth(), dxw.GetScreenHeight());
#endif
    // v2.05.41 fix: don't wcscmp atoms!
    if((dxw.dwFlags9 & KILLBLACKWIN) && ((DWORD)lpClassName & 0xFFFF0000) && (
                !wcscmp(lpClassName, L"Curtain")						// on "Clear-it"
            )) {
        OutTraceDW("%s: KILL \"%ls\" window class ret=%#x\n", api, lpClassName, FAKEKILLEDWIN);
        return (HWND)FAKEKILLEDWIN;
    }
    bActiveMovie = lpWindowName && (
                       !wcscmp(lpWindowName, L"ActiveMovie Window") ||
                       !wcscmp(lpWindowName, L"MSCTFIME UI") // v2.04.82: "Extreme Boards Blades"
                   );
    return CreateWindowCommon(
               ApiRef,
               (CreateWindow_Type)pCreateWindowExW,
               bActiveMovie, dwExStyle,
               (void *)lpClassName,
               (void *)lpWindowName,
               dwStyle,
               x, y, nWidth, nHeight,
               hWndParent,
               hMenu,
               hInstance,
               lpParam);
}

#define WM_CLASSMASK 0xFFFF0000

// GHO: pro Diablo
HWND WINAPI extCreateWindowExA(
    DWORD dwExStyle,
    LPCTSTR lpClassName,
    LPCTSTR lpWindowName,
    DWORD dwStyle,
    int x,
    int y,
    int nWidth,
    int nHeight,
    HWND hWndParent,
    HMENU hMenu,
    HINSTANCE hInstance,
    LPVOID lpParam) {
    BOOL bActiveMovie;
    ApiName("CreateWindowExA");
#ifndef DXW_NOTRACES
    if(IsTraceSYS) {
        char xString[20], yString[20], wString[20], hString[20];
        if (x == CW_USEDEFAULT) strcpy(xString, "CW_USEDEFAULT");
        else sprintf(xString, "%d", x);
        if (y == CW_USEDEFAULT) strcpy(yString, "CW_USEDEFAULT");
        else sprintf(yString, "%d", y);
        if (nWidth == CW_USEDEFAULT) strcpy(wString, "CW_USEDEFAULT");
        else sprintf(wString, "%d", nWidth);
        if (nHeight == CW_USEDEFAULT) strcpy(hString, "CW_USEDEFAULT");
        else sprintf(hString, "%d", nHeight);
        OutTrace("%s: class=\"%s\" wname=\"%s\" pos=(%s,%s) size=(%s,%s) Style=%#x(%s) ExStyle=%#x(%s) hWndParent=%#x%s hMenu=%#x depth=%d\n",
                 ApiRef, ClassToStr(lpClassName), lpWindowName, xString, yString, wString, hString,
                 dwStyle, ExplainStyle(dwStyle), dwExStyle, ExplainExStyle(dwExStyle),
                 hWndParent, hWndParent == HWND_MESSAGE ? "(HWND_MESSAGE)" : "", hMenu, InMainWinCreation);
    }
    OutDebugSYS("%s: DEBUG fullscreen=%#x mainwin=%#x screen=(%d,%d)\n",
                ApiRef, dxw.IsFullScreen(), dxw.GethWnd(), dxw.GetScreenWidth(), dxw.GetScreenHeight());
#endif
    bActiveMovie = lpWindowName && (
                       !strcmp(lpWindowName, "ActiveMovie Window") ||
                       !strcmp(lpWindowName, "MSCTFIME UI") // v2.04.82: "Extreme Boards Blades"
                   );
    // v2.05.41 fix: don't strcmp atoms!
    if((dxw.dwFlags9 & KILLBLACKWIN) && ((DWORD)lpClassName & 0xFFFF0000) && (
                !strcmp(lpClassName, "DDFullBck") ||				// on "Three Dirty Dwarves" ...
                !strcmp(lpClassName, "Kane & Lynch 2 window 2") ||	// on "Kane & Lynch 2", obviously ...
                !strcmp(lpClassName, "Curtain")						// on "Tennis Critters" demo
            )) {
        OutTraceDW("%s: KILL \"%s\" window class ret=%#x\n", ApiRef, lpClassName, FAKEKILLEDWIN);
        return (HWND)FAKEKILLEDWIN;
    }
    if((dxw.dwFlags11 & CUSTOMLOCALE) && pCreateWindowExW) {
        OutTraceDW("%s: using WIDECHAR call\n", ApiRef);
        extern __inline NTLEA_TLS_DATA *GetTlsValueInternal(void);
        extern void InstallCbtHook(NTLEA_TLS_DATA * ptls);
        extern void UninstallCbtHook(NTLEA_TLS_DATA * ptls);
        NTLEA_TLS_DATA *p = GetTlsValueInternal();
        DWORD PrevCallType = p->CurrentCallType;
        // COMMENTED OUT - NO GOOD, DON'T KNOW WHY ...
        //p->CurrentCallType = CT_CREATE_WINDOW;
        LPWSTR lpClassNameW;
        LPWSTR lpWindowNameW;
        BOOL IsStringClass = ((DWORD)lpClassName & WM_CLASSMASK);
        HWND ret;
        lpClassNameW = IsStringClass ? (LPWSTR)malloc((strlen(lpClassName) + 1) * 2) : (LPWSTR)lpClassName;
        lpWindowNameW = lpWindowName ? (LPWSTR)malloc((strlen(lpWindowName) + 1) * 2) : NULL;
        if(IsStringClass) {
            int size = strlen(lpClassName);
            int n = (*pMultiByteToWideChar)(dxw.CodePage, 0, lpClassName, size, lpClassNameW, size);
            lpClassNameW[n] = L'\0'; // make tail !
            OutTraceLOC("%s: WIDE class=\"%ls\"\n", ApiRef, lpClassNameW);
        }
        if(lpWindowNameW) {
            int size = strlen(lpWindowName);
            int n = (*pMultiByteToWideChar)(dxw.CodePage, 0, lpWindowName, size, lpWindowNameW, size);
            lpWindowNameW[n] = L'\0'; // make tail !
            OutTraceLOC("%s: WIDE wname=\"%ls\"\n", ApiRef, lpWindowNameW);
        }
        InstallCbtHook(p);
        ret = CreateWindowCommon(
                  ApiRef,
                  (CreateWindow_Type)pCreateWindowExW,
                  bActiveMovie, dwExStyle,
                  (void *)lpClassNameW,
                  (void *)lpWindowNameW,
                  dwStyle,
                  x, y, nWidth, nHeight,
                  hWndParent,
                  hMenu,
                  hInstance,
                  lpParam);
        UninstallCbtHook(p);
        p->CurrentCallType = PrevCallType;
        if(IsStringClass) free(lpClassNameW);
        if(lpWindowNameW) free(lpWindowNameW);
        return ret;
    }
    return CreateWindowCommon(
               ApiRef,
               (CreateWindow_Type)pCreateWindowExA,
               bActiveMovie, dwExStyle,
               (void *)lpClassName,
               (void *)lpWindowName,
               dwStyle,
               x, y, nWidth, nHeight,
               hWndParent,
               hMenu,
               hInstance,
               lpParam);
}

#ifndef DXW_NOTRACES
extern void ExplainMsg(char *, HWND, UINT, WPARAM, LPARAM);
#endif

extern LRESULT CALLBACK DefConversionProc(LPVOID, HWND, HWND, BOOL, INT, WPARAM, LPARAM);

LRESULT WINAPI extCallWindowProcA(WNDPROC lpPrevWndFunc, HWND hwnd, UINT Msg, WPARAM wParam, LPARAM lParam) {
    // v2.02.30: fix (Imperialism II): apply to main window only !!!
    HRESULT res;
    ApiName("CallWindowProcA");
#ifndef DXW_NOTRACES
    if(IsTraceW) ExplainMsg(ApiRef, hwnd, Msg, wParam, lParam);
#endif
    res = -1;
    if(hwnd == dxw.GethWnd()) res = FixWindowProc(ApiRef, hwnd, Msg, wParam, &lParam);
    if((dxw.dwFlags1 & FIXTEXTOUT) && (Msg == WM_SETFONT)) {
        wParam = (WPARAM)fontdb.GetScaledFont((HFONT)wParam);
        OutTraceDW("%s: replaced scaled font hfnt=%#x\n", ApiRef, wParam);
    }
    if (res == (HRESULT) - 1) {
        // tofix !!!
        // lines commmented out block "Tang Poetry II" ...
        //if((dxw.dwFlags11 & CUSTOMLOCALE) && pCallWindowProcW){
        //	return DefConversionProc((LPVOID)(DWORD_PTR)pCallWindowProcW, hwnd, NULL, FALSE, Msg, wParam, lParam);
        //}
        //else
        return (*pCallWindowProcA)(lpPrevWndFunc, hwnd, Msg, wParam, lParam);
    } else
        return res;
}

LRESULT WINAPI extCallWindowProcW(WNDPROC lpPrevWndFunc, HWND hwnd, UINT Msg, WPARAM wParam, LPARAM lParam) {
    // v2.02.30: fix (Imperialism II): apply to main window only !!!
    HRESULT res;
    ApiName("CallWindowProcW");
#ifndef DXW_NOTRACES
    if(IsTraceW) ExplainMsg(ApiRef, hwnd, Msg, wParam, lParam);
#endif
    res = -1;
    if(hwnd == dxw.GethWnd()) res = FixWindowProc(ApiRef, hwnd, Msg, wParam, &lParam);
    if((dxw.dwFlags1 & FIXTEXTOUT) && (Msg == WM_SETFONT)) {
        wParam = (WPARAM)fontdb.GetScaledFont((HFONT)wParam);
        OutTraceDW("%s: replaced scaled font hfnt=%#x\n", ApiRef, wParam);
    }
    if (res == (HRESULT) - 1)
        return (*pCallWindowProcW)(lpPrevWndFunc, hwnd, Msg, wParam, lParam);
    else
        return res;
}

static LRESULT WINAPI DefWindowProcCommon(char *api, DefWindowProc_Type pDefWindowProc, HWND hwnd, UINT Msg, WPARAM wParam, LPARAM lParam) {
    // v2.02.30: fix (Imperialism II): apply to main window only !!!
    // v2.03.50: fix - do clip cursor only after the window has got focus
    // v2.04.14: fix - erase clip cursor when window loses focus !!!
    HRESULT res;
    res = (HRESULT) - 1;
#ifndef DXW_NOTRACES
    if(IsTraceW) ExplainMsg(api, hwnd, Msg, wParam, lParam);
#endif
    if(hwnd == dxw.GethWnd()) res = FixWindowProc(api, hwnd, Msg, wParam, &lParam);
    if (res == (HRESULT) - 1) {
        // v2.05.14: added integration with ntlea hooks
        if((dxw.dwFlags11 & CUSTOMLOCALE) && (pDefWindowProc == pDefWindowProcA) && pDefWindowProcW) {
            // to be fixed: integration with DefConversionProc here strangely makes the window titles
            // to be wrong, as if we called pDefWindowProcA. Strange ...
            //res = DefConversionProc((LPVOID)(DWORD_PTR)pDefWindowProcW, hwnd, NULL, FALSE, Msg, wParam, lParam);
            res = (*pDefWindowProcW)(hwnd, Msg, wParam, lParam);
        } else
            res = (*pDefWindowProc)(hwnd, Msg, wParam, lParam);
    }
    switch(Msg) {
    case WM_ACTIVATE:
        // turn DirectInput bActive flag on & off .....
        dxw.bActive = (LOWORD(wParam) == WA_ACTIVE || LOWORD(wParam) == WA_CLICKACTIVE) ? 1 : 0;
    case WM_NCACTIVATE:
        // turn DirectInput bActive flag on & off .....
        if(Msg == WM_NCACTIVATE) dxw.bActive = wParam;
        OutTraceW("%s: bActive=%d\n", api, dxw.bActive);
        break;
    case WM_SETFOCUS:
        if(dxw.dwFlags1 & CLIPCURSOR) dxw.SetClipCursor();
        break;
    case WM_KILLFOCUS:
        if(dxw.dwFlags1 & CLIPCURSOR) dxw.EraseClipCursor();
        break; // v2.04.14: forgotten case ....
    }
    return res;
}

LRESULT WINAPI extDefWindowProcW(HWND hwnd, UINT Msg, WPARAM wParam, LPARAM lParam) {
    return DefWindowProcCommon("DefWindowProcW", pDefWindowProcW, hwnd, Msg, wParam, lParam);
}
LRESULT WINAPI extDefWindowProcA(HWND hwnd, UINT Msg, WPARAM wParam, LPARAM lParam) {
    return DefWindowProcCommon("DefWindowProcA", pDefWindowProcA, hwnd, Msg, wParam, lParam);
}

LRESULT WINAPI extDefFrameProcA(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam) {
    LRESULT res;
    OutDebugSYS("DefFrameProcA: hWnd=%x Msg=%x wParam=%x lParam=%x\n",
                hWnd, Msg, wParam, lParam);
    if(dxw.dwFlags11 & CUSTOMLOCALE)
        res = DefConversionProc((LPVOID)(DWORD_PTR)DefFrameProcW, hWnd, NULL, FALSE, Msg, wParam, lParam);
    else
        res = (*pDefFrameProcA)(hWnd, Msg, wParam, lParam);
    return res;
}

LRESULT WINAPI extDefMDIChildProcA(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    return DefConversionProc((LPVOID)(DWORD_PTR)(DWORD_PTR)DefMDIChildProcW, hWnd, NULL, FALSE, uMsg, wParam, lParam);
}
LRESULT WINAPI extDefDlgProcA(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    return DefConversionProc((LPVOID)(DWORD_PTR)DefDlgProcW, hWnd, NULL, FALSE, uMsg, wParam, lParam);
}

static int HandleRect(char *api, void *pFun, HDC hdc, const RECT *lprc, HBRUSH hbr) {
    // used for both FillRect and FrameRect calls
    int res;
    RECT rc;
    OutTraceSYS("%s: hdc=%#x hbrush=%#x rect=(%d,%d)-(%d,%d)\n", api, hdc, hbr, lprc->left, lprc->top, lprc->right, lprc->bottom);
    if(dxw.dwFlags4 & NOFILLRECT) {
        OutTraceDW("%s: SUPPRESS\n", api, hdc, hbr, lprc->left, lprc->top, lprc->right, lprc->bottom);
        return TRUE;
    }
    // if no emulation, just proxy
    if(dxw.GDIEmulationMode == GDIMODE_NONE)
        return (*(FillRect_Type)pFun)(hdc, &rc, hbr);
    memcpy(&rc, lprc, sizeof(rc));
    // Be careful: when you call CreateCompatibleDC with NULL DC, it is created a memory DC
    // with same characteristics as desktop. That would return true from the call to
    // dxw.IsRealDesktop(WindowFromDC(hdc)) because WindowFromDC(hdc) is null.
    // So, it's fundamental to check also the hdc type (OBJ_DC is a window's DC)
    if((dxw.IsRealDesktop(WindowFromDC(hdc)) && (OBJ_DC == (*pGetObjectType)(hdc)))) {
        HWND VirtualDesktop;
        VirtualDesktop = dxw.GethWnd();
        if(VirtualDesktop == NULL) {
            OutTraceDW("%s: no virtual desktop\n", api);
            return TRUE;
        }
        OutTraceDW("%s: remapped hdc to virtual desktop hwnd=%#x\n", api, dxw.GethWnd());
        hdc = (*pGDIGetDC)(dxw.GethWnd());
    }
    if(dxw.IsToRemap(hdc)) {
        if(rc.left < 0) rc.left = 0;
        if(rc.top < 0) rc.top = 0;
        if((DWORD)rc.right > dxw.GetScreenWidth()) rc.right = dxw.GetScreenWidth();
        if((DWORD)rc.bottom > dxw.GetScreenHeight()) rc.bottom = dxw.GetScreenHeight();
        switch(dxw.GDIEmulationMode) {
        case GDIMODE_SHAREDDC:
            sdc.GetPrimaryDC(hdc);
            res = (*(FillRect_Type)pFun)(sdc.GetHdc(), &rc, hbr);
            sdc.PutPrimaryDC(hdc, TRUE, rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top);
            return res;
            break;
        case GDIMODE_STRETCHED:
            dxw.MapClient(&rc);
            OutTraceDW("%s: fixed rect=(%d,%d)-(%d,%d)\n", api, rc.left, rc.top, rc.right, rc.bottom);
            break;
        default:
            break;
        }
    } else {
        // when not in fullscreen mode, just proxy the call
        // but check coordinates: some games may use excessive coordinates: see "Premier Manager 98"
        RECT client;
        HWND hwnd;
        hwnd = WindowFromDC(hdc);
        // v2.03.76 fix: sometimes WindowFromDC returns NULL with unpredictable results
        // if NULL, try to bount within the main window rect
        if(!hwnd) hwnd = dxw.GethWnd();
        // if still NULL, avoid doing changes
        if(hwnd) {
            (*pGetClientRect)(hwnd, &client);
            if(rc.left < client.left) rc.left = client.left;
            if(rc.top < client.top) rc.top = client.top;
            if(rc.right > client.right) rc.right = client.right;
            if(rc.bottom > client.bottom) rc.bottom = client.bottom;
            OutTraceDW("%s: remapped hdc from hwnd=%#x to rect=(%d,%d)-(%d,%d)\n", api, hwnd, rc.left, rc.top, rc.right, rc.bottom);
        }
    }
    res = (*(FillRect_Type)pFun)(hdc, &rc, hbr);
    return res;
}

int WINAPI extFillRect(HDC hdc, const RECT *lprc, HBRUSH hbr) {
    return HandleRect("FillRect", (void *)pFillRect, hdc, lprc, hbr);
}
int WINAPI extFrameRect(HDC hdc, const RECT *lprc, HBRUSH hbr) {
    return HandleRect("FrameRect", (void *)pFrameRect, hdc, lprc, hbr);
}

BOOL WINAPI extInvertRect(HDC hdc, const RECT *lprc) {
    int res;
    ApiName("InvertRect");
    RECT rc;
    OutTraceDW("%s: hdc=%#x rect=(%d,%d)-(%d,%d)\n", ApiRef, hdc, lprc->left, lprc->top, lprc->right, lprc->bottom);
    memcpy(&rc, lprc, sizeof(rc));
    if(dxw.IsToRemap(hdc)) {
        switch(dxw.GDIEmulationMode) {
        case GDIMODE_SHAREDDC:
            sdc.GetPrimaryDC(hdc);
            res = (*pInvertRect)(sdc.GetHdc(), &rc);
            sdc.PutPrimaryDC(hdc, TRUE, rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top);
            return res;
            break;
        case GDIMODE_STRETCHED:
            dxw.MapClient(&rc);
            OutTraceDW("%s: fixed rect=(%d,%d)-(%d,%d)\n", ApiRef, rc.left, rc.top, rc.right, rc.bottom);
            break;
        default:
            break;
        }
    }
    res = (*pInvertRect)(hdc, &rc);
    return res;
}

int WINAPI extValidateRect(HWND hwnd, const RECT *lprc) {
    // v2.03.91: manages the possibility of a NULL lprc value
    int res;
    ApiName("ValidateRect");
    RECT rc;
#ifndef DXW_NOTRACES
    if(IsTraceSYS) {
        if (lprc)
            OutTrace("%s: hwnd=%#x rect=(%d,%d)-(%d,%d)\n",
                     ApiRef, hwnd, lprc->left, lprc->top, lprc->right, lprc->bottom);
        else
            OutTrace("%s: hwnd=%#x rect=(NULL)\n", ApiRef, hwnd);
    }
#endif
    if(lprc) {
        // avoid changing the pointed RECT structure!!
        memcpy(&rc, lprc, sizeof(rc));
        lprc = &rc;
    }
    if(dxw.IsFullScreen()) {
        if(dxw.IsRealDesktop(hwnd)) hwnd = dxw.GethWnd();
        if(lprc) dxw.MapClient((LPRECT)lprc);
        if (lprc) {
            OutTraceDW("%s: fixed rect=(%d,%d)-(%d,%d)\n",
                       ApiRef, lprc->left, lprc->top, lprc->right, lprc->bottom);
        }
    }
    res = (*pValidateRect)(hwnd, lprc);
    return res;
}

BOOL WINAPI extClipCursor(RECT *lpRectArg) {
    // reference: hooking and setting ClipCursor is mandatori in "Emergency: Fighters for Life"
    // where the application expects the cursor to be moved just in a inner rect within the
    // main window surface.
    BOOL res;
    ApiName("ClipCursor");
    RECT *lpRect;
    RECT Rect;
#ifndef DXW_NOTRACES
    if(IsTraceC) {
        if (lpRectArg)
            OutTrace("%s: rect=(%d,%d)-(%d,%d)\n",
                     ApiRef, lpRectArg->left, lpRectArg->top, lpRectArg->right, lpRectArg->bottom);
        else
            OutTrace("%s: rect=(NULL)\n", ApiRef);
    }
#endif
    if (!(dxw.dwFlags1 & DISABLECLIPPING)) return TRUE;
    if ((dxw.dwFlags8 & CLIPLOCKED) && (lpRectArg == NULL)) return TRUE;
    if(lpRectArg) {
        Rect = *lpRectArg;
        lpRect = &Rect;
    } else
        lpRect = NULL;
    if(dxw.dwFlags1 & CLIENTREMAPPING) { //v2.03.61
        // save desired clip region
        // v2.02.39: fix - do not attempt to write to NULL lpRect
        if (lpRect) {
            ClipRegion = *lpRectArg;
            lpClipRegion = &ClipRegion;
            *lpRect = dxw.MapWindowRect(lpRect);
        } else
            lpClipRegion = NULL;
    }
    if (pClipCursor) res = (*pClipCursor)(lpRect);
#ifndef DXW_NOTRACES
    if (lpRect) OutTraceDW("%s: REMAPPED rect=(%d,%d)-(%d,%d) res=%#x\n",
                               ApiRef, lpRect->left, lpRect->top, lpRect->right, lpRect->bottom, res);
#endif // DXW_NOTRACES
    return TRUE;
}

BOOL WINAPI extGetClipCursor(LPRECT lpRect) {
    // v2.1.93: if DISABLECLIPPING, return the saved clip rect coordinates
    BOOL ret;
    ApiName("GetClipCursor");
    // proxy....
    if (!(dxw.dwFlags1 & DISABLECLIPPING)) {
        ret = (*pGetClipCursor)(lpRect);
        // v2.03.11: fix for "SubCulture" mouse movement
        if(lpRect && dxw.Windowize)	*lpRect = dxw.GetScreenRect();
#ifndef DXW_NOTRACES
        if(IsTraceSYS) {
            if (lpRect)
                OutTrace("%s: rect=(%d,%d)-(%d,%d) ret=%d\n",
                         ApiRef, lpRect->left, lpRect->top, lpRect->right, lpRect->bottom, ret);
            else
                OutTrace("%s: rect=(NULL) ret=%d\n", ApiRef, ret);
        }
#endif
        return ret;
    }
    if(lpRect) {
        if(lpClipRegion)
            *lpRect = ClipRegion;
        else
            *lpRect = dxw.GetScreenRect();
        OutTraceDW("%s: rect=(%d,%d)-(%d,%d) ret=%d\n",
                   ApiRef, lpRect->left, lpRect->top, lpRect->right, lpRect->bottom, TRUE);
    }
    return TRUE;
}

LONG WINAPI extEnumDisplaySettings(LPCTSTR lpszDeviceName, DWORD iModeNum, DEVMODE *lpDevMode) {
    LONG res;
    ApiName("EnumDisplaySettings");
    OSVERSIONINFO osinfo;
    osinfo.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
    (*pGetVersionExA)(&osinfo);
    OutTraceDW("%s: Devicename=%s ModeNum=%#x OS=%d.%d\n",
               ApiRef, lpszDeviceName, iModeNum, osinfo.dwMajorVersion, osinfo.dwMinorVersion);
    if(dxw.dwFlags4 & NATIVERES) {
        // lists video card native resolutions, though faking emulated color resolutions
        if((osinfo.dwMajorVersion >= 6) && (dxw.IsEmulated)) {
            switch(iModeNum) {
            case ENUM_CURRENT_SETTINGS:
            case ENUM_REGISTRY_SETTINGS: // lie ...
                res = (*pEnumDisplaySettings)(lpszDeviceName, iModeNum, lpDevMode);
                if(dxw.dwFlags2 & INIT8BPP) lpDevMode->dmBitsPerPel = 8;
                if(dxw.dwFlags2 & INIT16BPP) lpDevMode->dmBitsPerPel = 16;
                if(dxw.dwFlags3 & FORCE16BPP) lpDevMode->dmBitsPerPel = 16;
                break;
            default:
                res = (*pEnumDisplaySettings)(lpszDeviceName, iModeNum / SUPPORTED_DEPTHS_NUMBER, lpDevMode);
                lpDevMode->dmBitsPerPel = (DWORD)SupportedDepths[iModeNum % SUPPORTED_DEPTHS_NUMBER];
                break;
            }
        } else
            res = (*pEnumDisplaySettings)(lpszDeviceName, iModeNum, lpDevMode);
    } else { // simulated modes: VGA or HDTV
        //int SupportedDepths[5]={8,16,24,32,0};
        DWORD nRes, nDepths, nEntries;
        res = (*pEnumDisplaySettings)(lpszDeviceName, ENUM_CURRENT_SETTINGS, lpDevMode);
        switch(iModeNum) {
        case ENUM_CURRENT_SETTINGS:
        case ENUM_REGISTRY_SETTINGS: // lie ...
            // v2.04.47: set current virtual resolution values
            lpDevMode->dmPelsHeight = dxw.GetScreenHeight();
            lpDevMode->dmPelsWidth = dxw.GetScreenWidth();
            if(dxw.dwFlags2 & INIT8BPP) lpDevMode->dmBitsPerPel = 8;
            if(dxw.dwFlags2 & INIT16BPP) lpDevMode->dmBitsPerPel = 16;
            if(dxw.dwFlags3 & FORCE16BPP) lpDevMode->dmBitsPerPel = 16;
            break;
        default:
            // v2.04.77: check how many entries we should emulate
            for(nRes = 0; SupportedRes[nRes].h; nRes++);
            for(nDepths = 0; SupportedDepths[nDepths]; nDepths++);
            nEntries = nRes * nDepths;
            // v2.04.77: added bounds check to avoid random errors when querying high indexes
            if(iModeNum >= nEntries) {
                OutTraceDW("%s: index overflow max=%d - returning 0\n", ApiRef, nEntries);
                return 0;
            }
            lpDevMode->dmPelsHeight = SupportedRes[iModeNum / 4].h;
            lpDevMode->dmPelsWidth  = SupportedRes[iModeNum / 4].w;
            lpDevMode->dmBitsPerPel = SupportedDepths[iModeNum % 4];
            if(lpDevMode->dmPelsHeight == 0) res = 0; // end of list
            break;
        }
    }
    if(dxw.dwFlags4 & LIMITSCREENRES) {
#define HUGE 100000
        DWORD maxw, maxh;
        maxw = maxh = HUGE;
        switch(dxw.MaxScreenRes) {
        case DXW_NO_LIMIT:
            maxw = HUGE;
            maxh = HUGE;
            break;
        case DXW_LIMIT_320x200:
            maxw = 320;
            maxh = 200;
            break;
        case DXW_LIMIT_400x300:
            maxw = 400;
            maxh = 300;
            break; // v2.04.78 fix
        case DXW_LIMIT_640x480:
            maxw = 640;
            maxh = 480;
            break;
        case DXW_LIMIT_800x600:
            maxw = 800;
            maxh = 600;
            break;
        case DXW_LIMIT_1024x768:
            maxw = 1024;
            maxh = 768;
            break;
        case DXW_LIMIT_1280x960:
            maxw = 1280;
            maxh = 960;
            break;
        }
        if((lpDevMode->dmPelsWidth > maxw) || (lpDevMode->dmPelsHeight > maxh)) {
            OutTraceDW("%s: limit device size=(%d,%d)\n", ApiRef, maxw, maxh);
            lpDevMode->dmPelsWidth = maxw;
            lpDevMode->dmPelsHeight = maxh;
        }
    }
    if(dxw.dwFlags7 & MAXIMUMRES) {
        if((lpDevMode->dmPelsWidth > (DWORD)dxw.iMaxW) || (lpDevMode->dmPelsHeight > (DWORD)dxw.iMaxH)) {
            OutTraceDW("%s: limit device size=(%d,%d)\n", ApiRef, dxw.iMaxW, dxw.iMaxH);
            lpDevMode->dmPelsWidth = dxw.iMaxW;
            lpDevMode->dmPelsHeight = dxw.iMaxH;
        }
    }
    OutTraceDW("%s: color=%dBPP size=(%dx%d) refresh=%dHz\n",
               ApiRef, lpDevMode->dmBitsPerPel, lpDevMode->dmPelsWidth, lpDevMode->dmPelsHeight, lpDevMode->dmDisplayFrequency);
    return res;
}

LONG WINAPI extChangeDisplaySettingsA(DEVMODEA *lpDevMode, DWORD dwflags) {
    ApiName("ChangeDisplaySettingsA");
#ifndef DXW_NOTRACES
    if(IsTraceSYS) {
        char sInfo[1024];
        strcpy(sInfo, "");
        // v2.04.04: dmDeviceName not printed since it could be not initialized (Warhammer SOTHR)
        if (lpDevMode) sprintf(sInfo, " fields=%#x(%s) size=(%d x %d) bpp=%d freq=%d",
                                   lpDevMode->dmFields, ExplainDevModeFields(lpDevMode->dmFields),
                                   lpDevMode->dmPelsWidth, lpDevMode->dmPelsHeight,
                                   lpDevMode->dmBitsPerPel, lpDevMode->dmDisplayFrequency);
        OutTrace("%s: lpDevMode=%#x flags=%#x(%s)%s\n",
                 ApiRef, lpDevMode, dwflags, ExplainChangeDisplaySettingsFlags(dwflags), sInfo);
    }
#endif
    // do not trust dxw.Windowize because it gets falsed within dialogs creation!!
    if(dxw.LockedRes)
        return MyChangeDisplaySettings(ApiRef, FALSE, lpDevMode, dwflags);
    else
        return (*pChangeDisplaySettingsExA)(NULL, lpDevMode, NULL, dwflags, NULL);
}

LONG WINAPI extChangeDisplaySettingsW(DEVMODEW *lpDevMode, DWORD dwflags) {
    ApiName("ChangeDisplaySettingsW");
#ifndef DXW_NOTRACES
    if(IsTraceSYS) {
        char sInfo[1024];
        strcpy(sInfo, "");
        // v2.04.04: dmDeviceName not printed since it could be not initialized (Warhammer SOTHR)
        if (lpDevMode) sprintf(sInfo, "fields=%#x(%s) size=(%d x %d) bpp=%d",
                                   lpDevMode->dmFields, ExplainDevModeFields(lpDevMode->dmFields),
                                   lpDevMode->dmPelsWidth, lpDevMode->dmPelsHeight, lpDevMode->dmBitsPerPel);
        OutTrace("%s: lpDevMode=%#x flags=%#x(%s)%s\n",
                 ApiRef, lpDevMode, dwflags, ExplainChangeDisplaySettingsFlags(dwflags), sInfo);
    }
#endif
    // do not trust dxw.Windowize because it gets falsed within dialogs creation!!
    if(dxw.LockedRes)
        return MyChangeDisplaySettings(ApiRef, TRUE, lpDevMode, dwflags);
    else
        return (*pChangeDisplaySettingsW)(lpDevMode, dwflags);
}

LONG WINAPI extChangeDisplaySettingsExA(LPCTSTR lpszDeviceName, DEVMODEA *lpDevMode, HWND hwnd, DWORD dwflags, LPVOID lParam) {
    ApiName("ChangeDisplaySettingsExA");
#ifndef DXW_NOTRACES
    if(IsTraceSYS) {
        char sInfo[1024];
        strcpy(sInfo, "");
        if (lpDevMode) sprintf(sInfo, " DeviceName=%s fields=%#x(%s) size=(%d x %d) bpp=%d",
                                   lpDevMode->dmDeviceName, lpDevMode->dmFields, ExplainDevModeFields(lpDevMode->dmFields),
                                   lpDevMode->dmPelsWidth, lpDevMode->dmPelsHeight, lpDevMode->dmBitsPerPel);
        OutTrace("%s: DeviceName=%s lpDevMode=%#x flags=%#x(%s)%s\n",
                 ApiRef, lpszDeviceName, lpDevMode, dwflags, ExplainChangeDisplaySettingsFlags(dwflags), sInfo);
    }
#endif
    // do not trust dxw.Windowize because it gets falsed within dialogs creation!!
    if(dxw.LockedRes)
        return MyChangeDisplaySettings(ApiRef, FALSE, lpDevMode, dwflags);
    else
        return (*pChangeDisplaySettingsExA)(lpszDeviceName, lpDevMode, hwnd, dwflags, lParam);
}

LONG WINAPI extChangeDisplaySettingsExW(LPCTSTR lpszDeviceName, DEVMODEW *lpDevMode, HWND hwnd, DWORD dwflags, LPVOID lParam) {
    ApiName("ChangeDisplaySettingsExW");
#ifndef DXW_NOTRACES
    if(IsTraceSYS) {
        char sInfo[1024];
        strcpy(sInfo, "");
        if (lpDevMode) sprintf(sInfo, " DeviceName=%ls fields=%#x(%s) size=(%d x %d) bpp=%d",
                                   lpDevMode->dmDeviceName, lpDevMode->dmFields, ExplainDevModeFields(lpDevMode->dmFields),
                                   lpDevMode->dmPelsWidth, lpDevMode->dmPelsHeight, lpDevMode->dmBitsPerPel);
        OutTrace("%s: DeviceName=%ls lpDevMode=%#x flags=%#x(%s)%s\n",
                 ApiRef, lpszDeviceName, lpDevMode, dwflags, ExplainChangeDisplaySettingsFlags(dwflags), sInfo);
    }
#endif
    // do not trust dxw.Windowize because it gets falsed within dialogs creation!!
    if(dxw.LockedRes)
        return MyChangeDisplaySettings(ApiRef, TRUE, lpDevMode, dwflags);
    else
        return (*pChangeDisplaySettingsExW)(lpszDeviceName, lpDevMode, hwnd, dwflags, lParam);
}

static HDC WINAPI sGetDC(HWND hwnd, char *ApiName) {
    // to do: add parameter and reference to pGDIGetDCEx to merge properly GetDC and GetDCEx
    HDC ret;
    HWND lochwnd;
    if(!dxw.IsFullScreen()) return(*pGDIGetDC)(hwnd);
    lochwnd = hwnd;
    if (dxw.IsRealDesktop(hwnd)) {
        OutTraceDW("%s: desktop remapping hwnd=%#x->%#x\n", ApiName, hwnd, dxw.GethWnd());
        lochwnd = dxw.GethWnd();
    }
    switch(dxw.GDIEmulationMode) {
    case GDIMODE_EMULATED:
        ret = dxw.AcquireEmulatedDC(lochwnd);
        break;
    case GDIMODE_SHAREDDC:
    case GDIMODE_STRETCHED:
    default:
        ret = (*pGDIGetDC)(lochwnd);
        break;
    }
    if(ret)
        OutTraceDW("%s: hwnd=%#x ret=%#x\n", ApiName, lochwnd, ret);
    else {
        int err;
        err = GetLastError();
        OutTraceE("%s: ERROR hwnd=%#x err=%d at %d\n", ApiName, lochwnd, err, __LINE__);
        if((err == ERROR_INVALID_WINDOW_HANDLE) && (lochwnd != hwnd)) {
            ret = (*pGDIGetDC)(hwnd);
            if(ret)
                OutTraceDW("%s: hwnd=%#x ret=%#x\n", ApiName, hwnd, ret);
            else
                OutTraceE("%s: ERROR hwnd=%#x err=%d at %d\n", ApiName, hwnd, GetLastError(), __LINE__);
        }
    }
    return ret;
}

HDC WINAPI extGDIGetDC(HWND hwnd) {
    OutTraceDW("GDI.GetDC: hwnd=%#x\n", hwnd);
    return sGetDC(hwnd, "GDI.GetDC");
}

HDC WINAPI extGDIGetDCEx(HWND hwnd, HRGN hrgnClip, DWORD flags) {
    // used by Star Wars Shadow of the Empire
    OutTraceDW("GDI.GetDCEx: hwnd=%#x hrgnClip=%#x flags=%#x(%s)\n", hwnd, hrgnClip, flags, ExplainGetDCExFlags(flags));
    return sGetDC(hwnd, "GDI.GetDCEx");
}

HDC WINAPI extGDIGetWindowDC(HWND hwnd) {
    OutTraceDW("GDI.GetWindowDC: hwnd=%#x\n", hwnd);
    // if not fullscreen or not desktop win, just proxy the call
    if(!dxw.IsFullScreen() || !dxw.IsDesktop(hwnd)) {
        HDC ret;
        ret = (*pGDIGetWindowDC)(hwnd);
        OutTraceDW("GDI.GetWindowDC: hwnd=%#x hdc=%#x\n", hwnd, ret);
        return ret;
    }
    return sGetDC(hwnd, "GDI.GetWindowDC");
}

int WINAPI extGDIReleaseDC(HWND hwnd, HDC hDC) {
    int res;
    OutTraceDW("GDI.ReleaseDC: hwnd=%#x hdc=%#x\n", hwnd, hDC);
    if (dxw.IsRealDesktop(hwnd)) hwnd = dxw.GethWnd();
    if(hwnd == 0) return(TRUE);
    switch(dxw.GDIEmulationMode) {
    case GDIMODE_EMULATED:
        res = dxw.ReleaseEmulatedDC(hwnd);
        break;
    case GDIMODE_SHAREDDC:
    case GDIMODE_STRETCHED:
    default:
        res = (*pGDIReleaseDC)(hwnd, hDC);
        break;
    }
#ifndef DXW_NOTRACES
    if (!res) OutTraceE("GDI.ReleaseDC ERROR: err=%d at %d\n", GetLastError(), __LINE__);
#endif // DXW_NOTRACES
    return(res);
}

HDC WINAPI extBeginPaint(HWND hwnd, LPPAINTSTRUCT lpPaint) {
    HDC hdc;
    ApiName("BeginPaint");
    OutTraceDW("%s: hwnd=%#x lpPaint=%#x FullScreen=%#x\n", ApiRef, hwnd, lpPaint, dxw.IsFullScreen());
    // avoid access to real desktop
    if(dxw.IsRealDesktop(hwnd)) hwnd = dxw.GethWnd();
    if(dxw.dwFlags3 & FULLPAINTRECT) {
        static PAINTSTRUCT Paint;
        memcpy(&Paint, lpPaint, sizeof(PAINTSTRUCT));
        Paint.rcPaint = dxw.GetScreenRect();
        hdc = (*pBeginPaint)(hwnd, &Paint);
    } else
        hdc = (*pBeginPaint)(hwnd, lpPaint);
    // if not in fullscreen mode, that's all!
    if(!dxw.IsFullScreen()) return hdc;
    switch(dxw.GDIEmulationMode) {
    case GDIMODE_STRETCHED:
        // v2.05.34 fix: "(LPRECT)&(lpPaint->rcPaint)" is ok, "&(RECT)(lpPaint->rcPaint)" is bad!!
        if(dxw.dwFlags1 & CLIENTREMAPPING) dxw.UnmapClient((LPRECT) & (lpPaint->rcPaint));
        break;
    case GDIMODE_EMULATED:
        HDC EmuHDC;
        EmuHDC = dxw.AcquireEmulatedDC(hwnd);
#ifndef DXW_NOTRACES
        if(!DeleteObject(lpPaint->hdc))
            OutTraceE("%s: DeleteObject ERROR hdc=%#x err=%d at %d\n", ApiRef, lpPaint->hdc, GetLastError(), __LINE__);
#endif // DXW_NOTRACES
        lpPaint->hdc = EmuHDC;
        hdc = EmuHDC;
        break;
    case GDIMODE_SHAREDDC:
#if 0
        sdc.GetPrimaryDC(hdc);
        lpPaint->hdc = sdc.GetHdc();
        (*pBeginPaint)(hwnd, lpPaint);
        lpPaint->hdc = hdc;
        sdc.PutPrimaryDC(hdc, FALSE);
#endif
        break;
    default:
        break;
    }
    OutTraceDW("%s: hdc=%#x rcPaint=(%d,%d)-(%d,%d)\n",
               ApiRef, hdc,
               lpPaint->rcPaint.left, lpPaint->rcPaint.top, lpPaint->rcPaint.right, lpPaint->rcPaint.bottom);
    return hdc;
}

BOOL WINAPI extEndPaint(HWND hwnd, const PAINTSTRUCT *lpPaint) {
    BOOL ret;
    ApiName("EndPaint");
    OutTraceDW("%s: hwnd=%#x lpPaint=%#x lpPaint.hdc=%#x lpPaint.rcpaint=(%d,%d)-(%d,%d)\n",
               ApiRef, hwnd, lpPaint, lpPaint->hdc,
               lpPaint->rcPaint.left, lpPaint->rcPaint.top, lpPaint->rcPaint.right, lpPaint->rcPaint.bottom);
    // if not fullscreen or not desktop win, just proxy the call
    if(!dxw.IsFullScreen()) {
        ret = (*pEndPaint)(hwnd, lpPaint);
        return ret;
    }
    // avoid access to real desktop
    if(dxw.IsRealDesktop(hwnd)) hwnd = dxw.GethWnd();
    dxw.HandleFPS(); // handle refresh delays
    switch(dxw.GDIEmulationMode) {
    case GDIMODE_EMULATED:
        ret = dxw.ReleaseEmulatedDC(hwnd);
        break;
    case GDIMODE_SHAREDDC:
#if 1
        if(lpPaint) dxw.MapClient((LPRECT) & (lpPaint->rcPaint));
        ret = (*pEndPaint)(hwnd, lpPaint);
#else
        PAINTSTRUCT Paint;
        Paint = *lpPaint;
        Paint.hdc = sdc.GetHdc();
        (*pEndPaint)(hwnd, &Paint);
        if(lpPaint) dxw.MapClient((LPRECT) & (lpPaint->rcPaint));
        ret = (*pEndPaint)(hwnd, lpPaint);
#endif
        break;
    default:
        ret = (*pEndPaint)(hwnd, lpPaint);
        break;
    }
    if(ret)
        OutTraceDW("%s: hwnd=%#x ret=%#x\n", ApiRef, hwnd, ret);
    else
        OutTraceE("%s: ERROR err=%d at %d\n", ApiRef, GetLastError(), __LINE__);
    return ret;
}

// v2.04.68: Riitaoja revised fix - it is wise to eliminate theeffects of HOOKDLGWIN flag for all dialog's child windows
// makes Red Alert 2 games working (and maybe the HOOKDLGWIN flag a little less dangerous)

HWND WINAPI extCreateDialogIndirectParamA(HINSTANCE hInstance, LPCDLGTEMPLATE lpTemplate, HWND hWndParent, DLGPROC lpDialogFunc, LPARAM lParamInit) {
    HWND RetHWND;
    ApiName("CreateDialogIndirectParam");
    DWORD dwFlags8;
    OutTraceDW("%s: hInstance=%#x lpTemplate=%#x hWndParent=%#x lpDialogFunc=%#x lParamInit=%#x\n",
               ApiRef,
               hInstance,
               lpTemplate,
               hWndParent, lpDialogFunc, lParamInit);
    if(dxw.Windowize && (dxw.dwFlags10 & REPLACEDIALOGS)) {
        HWND hwnd;
        hwnd = dxw.CreateVirtualDesktop();
        OutTraceDW("%s: REPLACEDIALOG hwnd=%#x\n", ApiRef, hwnd);
        dxw.SethWnd(hwnd);
        return hwnd;
    }
    if(dxw.IsFullScreen() && dxw.IsRealDesktop(hWndParent)) hWndParent = dxw.GethWnd();
    // v2.04.68: no child dialog hooking
    dwFlags8 = dxw.dwFlags8;
    dxw.dwFlags8 &= ~HOOKDLGWIN;
    if(dxw.dwFlags10 & STRETCHDIALOGS) {
        DWORD dwSize;
        LPVOID lpScaledRes;
        dwSize = dxwStretchDialog((LPVOID)lpTemplate, NULL); // just calculate the dialog size
#ifndef DXW_NOTRACES
        OutHexDW((LPBYTE)lpTemplate, dwSize);
#endif //DXW_NOTRACES
        lpScaledRes = malloc(dwSize);
        memcpy(lpScaledRes, lpTemplate, dwSize);
        dxwStretchDialog(lpScaledRes, DXW_DIALOGFLAG_DUMP | DXW_DIALOGFLAG_STRETCH | (dxw.dwFlags1 & FIXTEXTOUT ? DXW_DIALOGFLAG_STRETCHFONT : 0));
        RetHWND = (*pCreateDialogIndirectParamA)(hInstance, (LPCDLGTEMPLATE)lpScaledRes, hWndParent, lpDialogFunc, lParamInit);
        free(lpScaledRes);
    } else
        RetHWND = (*pCreateDialogIndirectParamA)(hInstance, lpTemplate, hWndParent, lpDialogFunc, lParamInit);
    // recover dialog hooking
    dxw.dwFlags8 = dwFlags8;
    // v2.02.73: redirect lpDialogFunc only when it is nor NULL: fix for "LEGO Stunt Rally"
    if(lpDialogFunc && (dxw.dwFlags8 & HOOKDLGWIN)) {	// v2.03.41 - debug option
        dxwws.PutProc(RetHWND, (WNDPROC)lpDialogFunc);
#ifndef DXW_NOTRACES
        if(!(*pSetWindowLong)(RetHWND, DWL_DLGPROC, (LONG)extDialogWindowProc))
            OutTraceE("%s: SetWindowLong ERROR err=%d at %d\n", ApiRef, GetLastError(), __LINE__);
#else
        (*pSetWindowLong)(RetHWND, DWL_DLGPROC, (LONG)extDialogWindowProc);
#endif // DXW_NOTRACES
    }
    OutTraceDW("%s: hwnd=%#x\n", ApiRef, RetHWND);
    return RetHWND;
}

HWND WINAPI extCreateDialogParamA(HINSTANCE hInstance, LPCTSTR lpTemplateName, HWND hWndParent, DLGPROC lpDialogFunc, LPARAM lParamInit) {
    HWND RetHWND;
    DWORD dwFlags8;
    ApiName("CreateDialogParamA");
    OutTraceDW("%s: hInstance=%#x lpTemplateName=%s hWndParent=%#x lpDialogFunc=%#x lParamInit=%#x\n",
               ApiRef, hInstance, sTemplateName(lpTemplateName), hWndParent, lpDialogFunc, lParamInit);
    if(dxw.IsFullScreen() && dxw.IsRealDesktop(hWndParent)) {
        OutTraceDW("%s: set hwndparent=%#x->%#x\n", ApiRef, hWndParent, dxw.GethWnd());
        hWndParent = dxw.GethWnd();
    }
    // v2.04.68: no child dialog hooking
    dwFlags8 = dxw.dwFlags8;
    dxw.dwFlags8 &= ~HOOKDLGWIN;
    if(dxw.dwFlags10 & STRETCHDIALOGS) {
        HRSRC hRes;
        HGLOBAL hgRes;
        LPVOID lpRes;
        LPVOID lpScaledRes;
        DWORD dwSize;
        hRes = FindResource(NULL, lpTemplateName, RT_DIALOG);
        if(!hRes)
            OutTraceE("%s: FindResource ERROR err=%d at %d\n", ApiRef, GetLastError(), __LINE__);
        hgRes = LoadResource(NULL, hRes);
        if(!hgRes)
            OutTraceE("%s: LoadResource ERROR err=%d at %d\n", ApiRef, GetLastError(), __LINE__);
        lpRes = LockResource(hgRes);
        if(!lpRes)
            OutTraceE("%s: LockResource ERROR err=%d at %d\n", ApiRef, GetLastError(), __LINE__);
        dwSize = SizeofResource(NULL, hRes);
        if(!dwSize)
            OutTraceE("%s: SizeofResource ERROR err=%d at %d\n", ApiRef, GetLastError(), __LINE__);
        lpScaledRes = malloc(dwSize);
        memcpy(lpScaledRes, lpRes, dwSize);
        UnlockResource(lpRes);
#ifndef DXW_NOTRACES
        OutHexDW((LPBYTE)lpScaledRes, dwSize);
#endif //DXW_NOTRACES
        dxwStretchDialog(lpScaledRes, DXW_DIALOGFLAG_DUMP | DXW_DIALOGFLAG_STRETCH);
        RetHWND = (*pCreateDialogIndirectParamA)(hInstance, (LPCDLGTEMPLATE)lpScaledRes, hWndParent, lpDialogFunc, lParamInit);
        free(lpScaledRes);
    } else
        RetHWND = (*pCreateDialogParam)(hInstance, lpTemplateName, hWndParent, lpDialogFunc, lParamInit);
    dxw.dwFlags8 = dwFlags8;
    if(!RetHWND)
        OutTraceDW("%s: ERROR err=%d\n", ApiRef, GetLastError());
    // v2.02.73: redirect lpDialogFunc only when it is nor NULL: fix for "LEGO Stunt Rally"
    // v2.04.18: HOOKDLGWIN (not to be checked to fix "PBA Bowling 2")
    if(lpDialogFunc && (dxw.dwFlags8 & HOOKDLGWIN)) {	// v2.03.41 - debug option
        dxwws.PutProc(RetHWND, (WNDPROC)lpDialogFunc);
#ifndef DXW_NOTRACES
        if(!(*pSetWindowLong)(RetHWND, DWL_DLGPROC, (LONG)extDialogWindowProc))
            OutTraceE("%s: SetWindowLong ERROR err=%d at %d\n", ApiRef, GetLastError(), __LINE__);
#else
        (*pSetWindowLong)(RetHWND, DWL_DLGPROC, (LONG)extDialogWindowProc);
#endif // DXW_NOTRACES
    }
    OutTraceDW("%s: hwnd=%#x\n", ApiRef, RetHWND);
    return RetHWND;
}

BOOL WINAPI extMoveWindow(HWND hwnd, int X, int Y, int nWidth, int nHeight, BOOL bRepaint) {
    BOOL ret;
    DWORD dwStyle, dwExStyle;
    char *ApiName = "MoveWindow";
#ifdef DXWHIDEWINDOWUPDATES
    int origx, origy, origw, origh;
#endif
    OutTraceSYS("%s: hwnd=%#x xy=(%d,%d) size=(%d,%d) repaint=%#x\n",
                ApiName, hwnd, X, Y, nWidth, nHeight, bRepaint);
    OutDebugDW("> fullscreen=%d InMainWinCreation=%d\n", dxw.IsFullScreen(), InMainWinCreation);
#if DXWHIDEWINDOWUPDATES
    origx = X;
    origy = Y;
    origw = nWidth;
    origh = nHeight;
#endif
    //if(dxw.Windowize && !InMainWinCreation){
    if(dxw.Windowize) { // v2.04.97 - "Fallen Haven"
        if(dxw.IsDesktop(hwnd)) {
            // v2.1.93: happens in "Emergency Fighters for Life" ...
            // what is the meaning of this? is it related to video stretching?
            OutTraceDW("%s: prevent moving desktop win\n", ApiName);
            return TRUE;
        }
        if((hwnd == dxw.GethWnd()) || (hwnd == dxw.hParentWnd)) {
            OutTraceDW("%s: prevent moving main win\n", ApiName);
            return TRUE;
        }
        // v2.04.32: trim dimensions - useful for "Mig Alley"
        // v2.05.01: greater or equal!
        if (((DWORD)nWidth >= dxw.GetScreenWidth()) && ((DWORD)nHeight >= dxw.GetScreenHeight())) {
            if(dxw.GethWnd() == 0) {
                OutTraceDW("%s: MAIN win hwnd=%#x\n", ApiName, hwnd);
                // v2.05.01: promote to main win
                dxw.SethWnd(hwnd);
                dxw.SetFullScreen(TRUE);
            } else
                OutTraceDW("%s: BIG win hwnd=%#x\n", ApiName, hwnd);
            X = Y = 0;
            nWidth = dxw.GetScreenWidth();
            nHeight = dxw.GetScreenHeight();
        }
        dwStyle = (*pGetWindowLong)(hwnd, GWL_STYLE);
        dwExStyle = (*pGetWindowLong)(hwnd, GWL_EXSTYLE);
        if (dxw.IsFullScreen() && (dxw.dwFlags1 & CLIENTREMAPPING)) {
            POINT upleft = {0, 0};
            if (IsRelativePosition(dwStyle, dwExStyle, (HWND) - 1)) { // parent = unknown here, but not essential?
                dxw.MapClient(&X, &Y, &nWidth, &nHeight);
                OutTraceDW("%s: fixed RELATIVE pos=(%d,%d) size=(%d,%d)\n", ApiName, X, Y, nWidth, nHeight);
            } else {
                dxw.MapWindow(&X, &Y, &nWidth, &nHeight);
                OutTraceDW("%s: fixed ABSOLUTE pos=(%d,%d) size=(%d,%d)\n", ApiName, X, Y, nWidth, nHeight);
            }
        } else {
            if((X == 0) && (Y == 0) && (nWidth == (int)dxw.GetScreenWidth()) && (nHeight == (int)dxw.GetScreenHeight())) {
                // evidently, this was supposed to be a fullscreen window....
                RECT screen;
                POINT upleft = {0, 0};
                char *sStyle;
                (*pGetClientRect)(dxw.GethWnd(), &screen);
                (*pClientToScreen)(dxw.GethWnd(), &upleft);
                if (IsRelativePosition(dwStyle, dwExStyle, (HWND) - 1)) { // parent = unknown here, but not essential?
                    // Big main child window: see "Reah"
                    X = Y = 0;
                    sStyle = "(relative) ";
                } else {
                    // Regular big main window, usual case.
                    X = upleft.x;
                    Y = upleft.y;
                    sStyle = "(absolute) ";
                }
                nWidth = screen.right;
                nHeight = screen.bottom;
                if (dxw.dwFlags7 & ANCHORED) {
                    WINDOWPOS MaxPos;
                    dxw.CalculateWindowPos(hwnd, dxw.iSizX, dxw.iSizY, &MaxPos);
                    nWidth = MaxPos.cx;
                    nHeight = MaxPos.cy;
                    X  = MaxPos.x;
                    Y  = MaxPos.y;
                }
                OutTraceDW("%s: fixed BIG %swin pos=(%d,%d) size=(%d,%d)\n", ApiName, sStyle, X, Y, nWidth, nHeight);
            }
        }
    }
#ifdef DXWHIDEWINDOWUPDATES
    if(dxw.dwFlags10 & HIDEWINDOWCHANGES) {
        DWORD lpWinCB;
        ret = (*pMoveWindow)(hwnd, origx, origy, origw, origh, FALSE);
        lpWinCB = (*pGetWindowLong)(hwnd, GWL_WNDPROC);
        (*pSetWindowLong)(hwnd, GWL_WNDPROC, NULL);
        ret = (*pMoveWindow)(hwnd, X, Y, nWidth, nHeight, bRepaint);
        (*pSetWindowLong)(hwnd, GWL_WNDPROC, lpWinCB);
    } else
        ret = (*pMoveWindow)(hwnd, X, Y, nWidth, nHeight, bRepaint);
#else
    ret = (*pMoveWindow)(hwnd, X, Y, nWidth, nHeight, bRepaint);
#endif
#ifndef DXW_NOTRACES
    if(!ret) OutTraceE("%s: ERROR err=%d at %d\n", ApiName, GetLastError(), __LINE__);
#endif // DXW_NOTRACES
    if(dxw.bAutoScale) dxw.AutoScale();
    return ret;
}

int WINAPI extShowCursor(BOOL bShow) {
    static int iFakeCounter = 0;
    ApiName("ShowCursor");
    int ret;
    if((dxw.dwFlags9 & ZERODISPLAYCOUNTER) && !bShow) {
        ret = (*pShowCursor)(bShow);
        OutTraceDW("%s: bShow=%#x ret=%#x FIXED ret=0\n", ApiRef, bShow, ret);
        return 0;
    }
    OutTraceC("%s: bShow=%#x\n", ApiRef, bShow);
    if (bShow) {
        if (dxw.dwFlags1 & HIDEHWCURSOR) {
            iFakeCounter++;
            OutTraceC("%s: HIDEHWCURSOR ret=%#x\n", ApiRef, iFakeCounter);
            return iFakeCounter;
        }
    } else {
        if (dxw.dwFlags2 & SHOWHWCURSOR) {
            iFakeCounter--;
            OutTraceC("%s: SHOWHWCURSOR ret=%#x\n", ApiRef, iFakeCounter);
            return iFakeCounter;
        }
    }
    ret = (*pShowCursor)(bShow);
    OutTraceC("%s: ret=%#x\n", ApiRef, ret);
    return ret;
}

BOOL WINAPI extDrawFocusRect(HDC hDC, const RECT *lprc) {
    return TRUE;
}

BOOL WINAPI extScrollDC(HDC hdc, int dx, int dy, const RECT *lprcScroll, const RECT *lprcClip, HRGN hrgnUpdate, LPRECT lprcUpdate) {
    BOOL res;
    ApiName("ScrollDC");
    if(IsTraceSYS) {
        char sRect[81];
        if(lprcScroll) sprintf(sRect, "(%d,%d)-(%d,%d)", lprcScroll->left, lprcScroll->top, lprcScroll->right, lprcScroll->bottom);
        else strcpy(sRect, "NULL");
        char sClip[81];
        if(lprcClip) sprintf(sClip, "(%d,%d)-(%d,%d)", lprcClip->left, lprcClip->top, lprcClip->right, lprcClip->bottom);
        else strcpy(sClip, "NULL");
        OutTraceSYS("%s: hdc=%#x(%s) dxy=(%d,%d) scrollrect=%s cliprect=%s hrgn=%#x\n",
                    ApiRef, hdc, ExplainDCType((*pGetObjectType)(hdc)), dx, dy, sRect, sClip, hrgnUpdate);
    }
    if(dxw.IsToRemap(hdc)) {
        switch(dxw.GDIEmulationMode) {
        case GDIMODE_STRETCHED:
            RECT rcSaveScroll, rcSaveClip;
            dxw.MapClient(&dx, &dy);
            OutTraceDW("%s: FIXED dxy=(%d,%d)\n", ApiRef, dx, dy);
            if(lprcScroll) {
                rcSaveScroll = *lprcScroll;
                dxw.MapClient((LPRECT)lprcScroll);
                OutTraceDW("%s: FIXED scrollrect=(%d,%d)-(%d,%d)\n",
                           ApiRef, lprcScroll->left, lprcScroll->top, lprcScroll->right, lprcScroll->bottom);
            }
            if(lprcClip) {
                rcSaveClip = *lprcClip;
                dxw.MapClient((LPRECT)lprcClip);
                OutTraceDW("%s: FIXED cliplrect=(%d,%d)-(%d,%d)\n",
                           ApiRef, lprcClip->left, lprcClip->top, lprcClip->right, lprcClip->bottom);
            }
            res = (*pScrollDC)(hdc, dx, dy, lprcScroll, lprcClip, hrgnUpdate, lprcUpdate);
            if(lprcScroll) *(LPRECT)lprcScroll = rcSaveScroll;
            if(lprcClip) *(LPRECT)lprcClip = rcSaveClip;
            if(res && lprcUpdate) {
                OutTraceDW("%s: updaterect=(%d,%d)-(%d,%d)\n",
                           ApiRef, lprcUpdate->left, lprcUpdate->top, lprcUpdate->right, lprcUpdate->bottom);
                dxw.UnmapClient(lprcUpdate);
                OutTraceDW("%s: FIXED updaterect=(%d,%d)-(%d,%d)\n",
                           ApiRef, lprcUpdate->left, lprcUpdate->top, lprcUpdate->right, lprcUpdate->bottom);
            }
            return res;
            break;
        case GDIMODE_SHAREDDC:
            sdc.GetPrimaryDC(hdc);
            res = (*pScrollDC)(sdc.GetHdc(), dx, dy, lprcScroll, lprcClip, hrgnUpdate, lprcUpdate);
            sdc.PutPrimaryDC(hdc, TRUE, lprcUpdate->left, lprcUpdate->top, lprcUpdate->right - lprcUpdate->left, lprcUpdate->bottom - lprcUpdate->top);
            return res;
            break;
        case GDIMODE_EMULATED:
#if 0
            // not working with 688(I) sonar !!!
            if(dxw.IsVirtual(hdc)) {
                RECT rcScroll, rcClip;
                if(lprcScroll) {
                    rcScroll = *lprcScroll;
                    OffsetRect(&rcScroll, dxw.VirtualOffsetX, dxw.VirtualOffsetY);
                }
                if(lprcClip) {
                    rcClip = *lprcClip;
                    OffsetRect(&rcClip, dxw.VirtualOffsetX, dxw.VirtualOffsetY);
                }
                res = (*pScrollDC)(hdc, dx, dy, &rcScroll, &rcClip, hrgnUpdate, lprcUpdate);
                return res;
            }
#endif
            break;
        default:
            break;
        }
    }
    res = (*pScrollDC)(hdc, dx, dy, lprcScroll, lprcClip, hrgnUpdate, lprcUpdate);
    if(res) {
        if(lprcUpdate) {
            OutTraceSYS("%s: updaterect=(%d,%d)-(%d,%d)\n",
                        ApiRef, lprcUpdate->left, lprcUpdate->top, lprcUpdate->right, lprcUpdate->bottom);
        } else
            OutTraceSYS("%s: updaterect=NULL\n", ApiRef);
    } else OutTraceE("%s: ERROR err=%d\n", ApiRef, GetLastError());
    return res;
}

HWND WINAPI extGetTopWindow(HWND hwnd) {
    HWND ret;
    ApiName("GetTopWindow");
    OutTraceDW("%s: hwnd=%#x fullscreen=%#x\n", ApiRef, hwnd, dxw.IsFullScreen());
    // a fullscreen program is supposed to be always top Z-order on the desktop!
    ret = (dxw.IsFullScreen() && dxw.IsDesktop(hwnd) && (dxw.dwFlags8 & PRETENDVISIBLE)) ? dxw.GethWnd() : (*pGetTopWindow)(hwnd);
    OutTraceDW("%s: ret=%#x\n", ApiRef, ret);
    return ret;
}

LONG WINAPI extTabbedTextOutA(HDC hdc, int X, int Y, LPCTSTR lpString, int nCount, int nTabPositions, const LPINT lpnTabStopPositions, int nTabOrigin) {
    BOOL res;
    ApiName("TabbedTextOutA");
    LPINT lpnScaledTabStopPositions;
#ifndef DXW_NOTRACES
    OutTraceSYS("%s: hdc=%#x xy=(%d,%d) nCount=%d nTP=%d nTOS=%d str=(%d)\"%*.*s\"\n",
                ApiRef, hdc, X, Y, nCount, nTabPositions, nTabOrigin, nCount, nCount, nCount, lpString);
    for(int iTab = 0; iTab < nTabPositions; iTab++)
        OutTraceSYS("> tab[%d]=%d\n", iTab, lpnTabStopPositions[iTab]);
#endif // DXW_NOTRACES
    lpnScaledTabStopPositions = lpnTabStopPositions;
    if(dxw.IsToRemap(hdc)) {
        switch(dxw.GDIEmulationMode) {
        case GDIMODE_SHAREDDC:
            sdc.GetPrimaryDC(hdc);
            res = (*pTabbedTextOutA)(sdc.GetHdc(), X, Y, lpString, nCount, nTabPositions, lpnTabStopPositions, nTabOrigin);
            sdc.PutPrimaryDC(hdc, TRUE);
            return res;
            break;
        case GDIMODE_STRETCHED: {
            // scale all value args, copy & scale ref args.
            nTabOrigin = (nTabOrigin * dxw.iSizX) / dxw.GetScreenWidth();
            lpnScaledTabStopPositions = (LPINT)malloc(sizeof(int) * nTabPositions);
            for(int iTab = 0; iTab < nTabPositions; iTab++) {
                lpnScaledTabStopPositions[iTab] = (lpnTabStopPositions[iTab] * dxw.iSizX) / dxw.GetScreenWidth();
                OutTraceDW("> tab[%d]: pos=%d->%d\n", iTab, lpnTabStopPositions[iTab], lpnScaledTabStopPositions[iTab]);
            }
            dxw.MapClient(&X, &Y);
        }
        break;
        case GDIMODE_EMULATED:
            if(dxw.IsVirtual(hdc)) {
                X += dxw.VirtualOffsetX;
                Y += dxw.VirtualOffsetY;
            }
            break;
        default:
            break;
        }
        OutTraceDW("%s: fixed dest=(%d,%d)\n", ApiRef, X, Y);
    }
    res = (*pTabbedTextOutA)(hdc, X, Y, lpString, nCount, nTabPositions, lpnScaledTabStopPositions, nTabOrigin);
    if(lpnScaledTabStopPositions != lpnTabStopPositions) free(lpnScaledTabStopPositions);
    return res;
}

LONG WINAPI extTabbedTextOutW(HDC hdc, int X, int Y, LPCWSTR lpString, int nCount, int nTabPositions, const LPINT lpnTabStopPositions, int nTabOrigin) {
    BOOL res;
    ApiName("TabbedTextOutW");
    OutTraceDW("%s: hdc=%#x xy=(%d,%d) nCount=%d nTP=%d nTOS=%d str=(%d)\"%ls\"\n",
               ApiRef, hdc, X, Y, nCount, nTabPositions, nTabOrigin, lpString);
    if(dxw.IsToRemap(hdc)) {
        switch(dxw.GDIEmulationMode) {
        case GDIMODE_SHAREDDC:
            sdc.GetPrimaryDC(hdc);
            res = (*pTabbedTextOutW)(sdc.GetHdc(), X, Y, lpString, nCount, nTabPositions, lpnTabStopPositions, nTabOrigin);
            sdc.PutPrimaryDC(hdc, TRUE);
            return res;
            break;
        case GDIMODE_STRETCHED:
            dxw.MapClient(&X, &Y);
            break;
        case GDIMODE_EMULATED:
            if(dxw.IsVirtual(hdc)) {
                X += dxw.VirtualOffsetX;
                Y += dxw.VirtualOffsetY;
            }
            break;
        default:
            break;
        }
        OutTraceDW("%s: fixed dest=(%d,%d)\n", ApiRef, X, Y);
    }
    res = (*pTabbedTextOutW)(hdc, X, Y, lpString, nCount, nTabPositions, lpnTabStopPositions, nTabOrigin);
    return res;
}

BOOL WINAPI extDestroyWindow(HWND hWnd) {
    // v2.02.43: "Empire Earth" builds test surfaces that must be destroyed!
    // v2.03.20: "Prince of Persia 3D" destroys the main window that must be preserved!
    BOOL res;
    ApiName("DestroyWindow");
    OutDebugDW("%s: hwnd=%#x\n", ApiRef, hWnd);
    if (hWnd == dxw.GethWnd()) {
        OutTraceDW("%s: destroy main hwnd=%#x\n", ApiRef, hWnd);
        if(hLastFullScrWin) {
            OutTraceDW("%s: revert to main hwnd=%#x bpp=%d\n",
                       ApiRef, hWnd, ddpLastPixelFormat.dwRGBBitCount);
            dxw.SethWnd(hLastFullScrWin);
            hLastFullScrWin = NULL;
            dxw.VirtualPixelFormat = ddpLastPixelFormat;
            extern int iPrimarySurfaceVersion;
            SetBltTransformations();
        } else {
            OutTraceDW("%s: destroy main hwnd=%#x\n", ApiRef, hWnd);
            dxw.SethWnd(NULL);
        }
        if(dxw.dwFlags6 & NODESTROYWINDOW) {
            OutTraceDW("%s: do NOT destroy main hwnd=%#x\n", ApiRef, hWnd);
            return TRUE;
        }
    }
    // useless ...
    //if(hWnd == wHider){
    //	OutTraceDW("DestroyWindow: do NOT destroy hider hwnd=%#x\n", hWnd);
    //	return TRUE;
    //}
    if (hControlParentWnd && (hWnd == hControlParentWnd)) {
        OutTraceDW("%s: destroy control parent hwnd=%#x\n", ApiRef, hWnd);
        hControlParentWnd = NULL;
    }
    res = (*pDestroyWindow)(hWnd);
    IfTraceError();
    if(CurrentActiveMovieWin == hWnd) CurrentActiveMovieWin = NULL;
    if(dxw.dwFlags7 & NOWINERRORS) return TRUE; // v2.03.69: suppress unessential errors
    return res;
}

#ifndef DXW_NOTRACES
static char *ExplainTAAlign(UINT c) {
    static char eb[256];
    unsigned int l;
    strcpy(eb, "TA_");
    strcat(eb, (c & TA_UPDATECP) ? "UPDATECP+" : "NOUPDATECP+");
    strcat(eb, (c & TA_RIGHT) ? (((c & TA_CENTER) == TA_CENTER) ? "CENTER+" : "RIGHT+") : "LEFT+");
    strcat(eb, (c & TA_BOTTOM) ? "BOTTOM+" : "TOP+");
    if ((c & TA_BASELINE) == TA_BASELINE) strcat(eb, "BASELINE+");
    if (c & TA_RTLREADING) strcat(eb, "RTLREADING+");
    l = strlen(eb);
    eb[l - 1] = 0;
    return(eb);
}

static char *ExplainDTFormat(UINT c) {
    static char eb[256];
    unsigned int l;
    strcpy(eb, "DT_");
    if(!(c & (DT_CENTER | DT_RIGHT))) strcat(eb, "LEFT+");
    if(c & DT_CENTER) strcat(eb, "CENTER+");
    if(c & DT_RIGHT) strcat(eb, "RIGHT+");
    if(!(c & (DT_VCENTER | DT_BOTTOM))) strcat(eb, "TOP+");
    if(c & DT_VCENTER) strcat(eb, "VCENTER+");
    if(c & DT_BOTTOM) strcat(eb, "BOTTOM+");
    if(c & DT_WORDBREAK) strcat(eb, "WORDBREAK+");
    if(c & DT_SINGLELINE) strcat(eb, "SINGLELINE+");
    if(c & DT_EXPANDTABS) strcat(eb, "EXPANDTABS+");
    if(c & DT_TABSTOP) strcat(eb, "TABSTOP+");
    if(c & DT_NOCLIP) strcat(eb, "NOCLIP+");
    if(c & DT_EXTERNALLEADING) strcat(eb, "EXTERNALLEADING+");
    if(c & DT_CALCRECT) strcat(eb, "CALCRECT+");
    if(c & DT_NOPREFIX) strcat(eb, "NOPREFIX+");
    if(c & DT_INTERNAL) strcat(eb, "INTERNAL+");
    l = strlen(eb);
    eb[l - 1] = 0;
    return(eb);
}
#endif // DXW_NOTRACES

BOOL gFixed;

int WINAPI extDrawTextA(HDC hdc, LPCTSTR lpchText, int nCount, LPRECT lpRect, UINT uFormat) {
    int ret;
    ApiName("DrawTextA");
    OutTraceDW("%s: hdc=%#x rect=(%d,%d)-(%d,%d) Format=%#x(%s) Text=(%d)\"%s\"\n",
               ApiRef, hdc, lpRect->left, lpRect->top, lpRect->right, lpRect->bottom,
               uFormat, ExplainDTFormat(uFormat), nCount, lpchText);
    gFixed = TRUE; // semaphore to avoid multiple scaling with HOT patching
    if(dxw.IsToRemap(hdc)) {
        switch(dxw.GDIEmulationMode) {
        case GDIMODE_SHAREDDC:
            sdc.GetPrimaryDC(hdc);
            ret = (*pDrawTextA)(sdc.GetHdc(), lpchText, nCount, lpRect, uFormat);
            if(nCount)
                sdc.PutPrimaryDC(hdc, TRUE, lpRect->left, lpRect->top, lpRect->right - lpRect->left, lpRect->bottom - lpRect->top);
            else {
                sdc.PutPrimaryDC(hdc, FALSE); // Diablo makes a DrawText of null string in the intro ...
            }
            return ret;
            break;
        case GDIMODE_STRETCHED:
            dxw.MapClient(lpRect);
            OutTraceDW("%s: fixed rect=(%d,%d)-(%d,%d)\n", ApiRef, lpRect->left, lpRect->top, lpRect->right, lpRect->bottom);
            ret = (*pDrawTextA)(hdc, lpchText, nCount, lpRect, uFormat);
            dxw.UnmapClient((RECT *)lpRect);
            OutTraceDW("%s: fixed output rect=(%d,%d)-(%d,%d)\n", ApiRef, lpRect->left, lpRect->top, lpRect->right, lpRect->bottom);
            break;
        default:
            ret = (*pDrawTextA)(hdc, lpchText, nCount, lpRect, uFormat);
            break;
        }
    } else
        ret = (*pDrawTextA)(hdc, lpchText, nCount, lpRect, uFormat);
    gFixed = FALSE;
    // if nCount is zero, DrawRect returns 0 as text heigth, but this is not an error! (ref. "Imperialism II")
#ifndef DXW_NOTRACES
    if(nCount && !ret) OutTraceE("%s: ERROR ret=%#x err=%d\n", ApiRef, ret, GetLastError());
#endif // DXW_NOTRACES
    return ret;
}

int WINAPI extDrawTextExA(HDC hdc, LPTSTR lpchText, int nCount, LPRECT lpRect, UINT dwDTFormat, LPDRAWTEXTPARAMS lpDTParams) {
    int ret;
    ApiName("DrawTextExA");
#ifndef DXW_NOTRACES
    OutTraceSYS("%s: hdc=%#x rect=(%d,%d)-(%d,%d) DTFormat=%#x Text=(%d)\"%s\"\n",
                ApiRef, hdc, lpRect->left, lpRect->top, lpRect->right, lpRect->bottom, dwDTFormat, nCount, lpchText);
    if (IsDebugSYS) {
        if(lpDTParams)
            OutTrace("> DTParams: size=%d (L,R)margins=(%d,%d) TabLength=%d lDrawn=%d\n",
                     lpDTParams->cbSize, lpDTParams->iLeftMargin, lpDTParams->iRightMargin,
                     lpDTParams->iTabLength, lpDTParams->uiLengthDrawn);
        else
            OutTrace("> DTParams: NULL\n");
    }
#endif
    gFixed = TRUE; // semaphore to avoid multiple scaling with HOT patching
    if(dxw.IsToRemap(hdc)) {
        switch(dxw.GDIEmulationMode) {
        case GDIMODE_SHAREDDC:
            sdc.GetPrimaryDC(hdc);
            ret = (*pDrawTextExA)(sdc.GetHdc(), lpchText, nCount, lpRect, dwDTFormat, lpDTParams);
            if(nCount)
                sdc.PutPrimaryDC(hdc, TRUE, lpRect->left, lpRect->top, lpRect->right - lpRect->left, lpRect->bottom - lpRect->top);
            else
                sdc.PutPrimaryDC(hdc, FALSE); // in cases like Diablo that makes a DrawText of null string in the intro ...
            return ret;
            break;
        case GDIMODE_STRETCHED:
            dxw.MapClient((RECT *)lpRect);
            OutTraceDW("%s: fixed rect=(%d,%d)-(%d,%d)\n", ApiRef, lpRect->left, lpRect->top, lpRect->right, lpRect->bottom);
            ret = (*pDrawTextExA)(hdc, lpchText, nCount, lpRect, dwDTFormat, lpDTParams);
            dxw.UnmapClient((RECT *)lpRect);
            OutTraceDW("%s: fixed output rect=(%d,%d)-(%d,%d)\n", ApiRef, lpRect->left, lpRect->top, lpRect->right, lpRect->bottom);
            break;
        default:
            ret = (*pDrawTextExA)(hdc, lpchText, nCount, lpRect, dwDTFormat, lpDTParams);
            break;
        }
    } else
        ret = (*pDrawTextExA)(hdc, lpchText, nCount, lpRect, dwDTFormat, lpDTParams);
    gFixed = FALSE;
    // if nCount is zero, DrawRect returns 0 as text heigth, but this is not an error! (ref. "Imperialism II")
#ifndef DXW_NOTRACES
    if(nCount && !ret) OutTraceE("%s: ERROR ret=%#x err=%d\n", ApiRef, ret, GetLastError());
#endif // DXW_NOTRACES
    return ret;
}

int WINAPI extDrawTextW(HDC hdc, LPCWSTR lpchText, int nCount, LPRECT lpRect, UINT uFormat) {
    int ret;
    ApiName("DrawTextW");
    OutTraceDW("%s: hdc=%#x rect=(%d,%d)-(%d,%d) Format=%#x(%s) Text=(%d)\"%ls\"\n",
               ApiRef, hdc, lpRect->left, lpRect->top, lpRect->right, lpRect->bottom,
               uFormat, ExplainDTFormat(uFormat), nCount, lpchText);
    gFixed = TRUE; // semaphore to avoid multiple scaling with HOT patching
    if(dxw.IsToRemap(hdc)) {
        switch(dxw.GDIEmulationMode) {
        case GDIMODE_SHAREDDC:
            sdc.GetPrimaryDC(hdc);
            ret = (*pDrawTextW)(sdc.GetHdc(), lpchText, nCount, lpRect, uFormat);
            sdc.PutPrimaryDC(hdc, TRUE, lpRect->left, lpRect->top, lpRect->right - lpRect->left, lpRect->bottom - lpRect->top);
            return ret;
            break;
        case GDIMODE_STRETCHED:
            dxw.MapClient((RECT *)lpRect);
            OutTraceDW("%s: fixed rect=(%d,%d)-(%d,%d)\n", ApiRef, lpRect->left, lpRect->top, lpRect->right, lpRect->bottom);
            ret = (*pDrawTextW)(hdc, lpchText, nCount, lpRect, uFormat);
            dxw.UnmapClient((RECT *)lpRect);
            OutTraceDW("%s: fixed output rect=(%d,%d)-(%d,%d)\n", ApiRef, lpRect->left, lpRect->top, lpRect->right, lpRect->bottom);
            break;
        default:
            ret = (*pDrawTextW)(hdc, lpchText, nCount, lpRect, uFormat);
            break;
        }
    } else
        ret = (*pDrawTextW)(hdc, lpchText, nCount, lpRect, uFormat);
    gFixed = FALSE;
    // if nCount is zero, DrawRect returns 0 as text heigth, but this is not an error! (ref. "Imperialism II")
#ifndef DXW_NOTRACES
    if(nCount && !ret) OutTraceE("%s: ERROR ret=%#x err=%d\n", ApiRef, ret, GetLastError());
#endif // DXW_NOTRACES
    return ret;
}

int WINAPI extDrawTextExW(HDC hdc, LPCWSTR lpchText, int nCount, LPRECT lpRect, UINT dwDTFormat, LPDRAWTEXTPARAMS lpDTParams) {
    int ret;
    ApiName("DrawTextExW");
#ifndef DXW_NOTRACES
    OutTraceSYS("%s: hdc=%#x rect=(%d,%d)-(%d,%d) DTFormat=%#x Text=(%d)\"%ls\"\n",
                ApiRef, hdc, lpRect->left, lpRect->top, lpRect->right, lpRect->bottom, dwDTFormat, nCount, lpchText);
    if (IsDebugSYS) {
        if(lpDTParams)
            OutTrace("DTParams: size=%d (L,R)margins=(%d,%d) TabLength=%d lDrawn=%d\n",
                     lpDTParams->cbSize, lpDTParams->iLeftMargin, lpDTParams->iRightMargin,
                     lpDTParams->iTabLength, lpDTParams->uiLengthDrawn);
        else
            OutTrace("DTParams: NULL\n");
    }
#endif
    gFixed = TRUE; // semaphore to avoid multiple scaling with HOT patching
    if(dxw.IsToRemap(hdc)) {
        switch(dxw.GDIEmulationMode) {
        case GDIMODE_SHAREDDC:
            sdc.GetPrimaryDC(hdc);
            ret = (*pDrawTextExW)(sdc.GetHdc(), lpchText, nCount, lpRect, dwDTFormat, lpDTParams);
            sdc.PutPrimaryDC(hdc, TRUE, lpRect->left, lpRect->top, lpRect->right - lpRect->left, lpRect->bottom - lpRect->top);
            return ret;
            break;
        case GDIMODE_STRETCHED:
            dxw.MapClient((RECT *)lpRect);
            OutTraceDW("%s: fixed rect=(%d,%d)-(%d,%d)\n", ApiRef, lpRect->left, lpRect->top, lpRect->right, lpRect->bottom);
            ret = (*pDrawTextExW)(hdc, lpchText, nCount, lpRect, dwDTFormat, lpDTParams);
            dxw.UnmapClient((RECT *)lpRect);
            OutTraceDW("%s: fixed output rect=(%d,%d)-(%d,%d)\n", ApiRef, lpRect->left, lpRect->top, lpRect->right, lpRect->bottom);
            break;
        default:
            ret = (*pDrawTextExW)(hdc, lpchText, nCount, lpRect, dwDTFormat, lpDTParams);
            break;
        }
    } else
        ret = (*pDrawTextExW)(hdc, lpchText, nCount, lpRect, dwDTFormat, lpDTParams);
    gFixed = FALSE;
    // if nCount is zero, DrawRect returns 0 as text heigth, but this is not an error! (ref. "Imperialism II")
#ifndef DXW_NOTRACES
    if(nCount && !ret) OutTraceE("%s: ERROR ret=%#x err=%d\n", ApiRef, ret, GetLastError());
#endif // DXW_NOTRACES
    return ret;
}

BOOL WINAPI extCloseWindow(HWND hWnd) {
    // from MSDN: Minimizes (but does not destroy) the specified window.
    BOOL res;
    ApiName("CloseWindow");
    OutDebugDW("%s: hwnd=%#x\n", ApiRef, hWnd);
    if (hWnd == dxw.GethWnd()) {
        OutTraceDW("%s: close main hwnd=%#x\n", ApiRef, hWnd);
        // do not delete the reference to main hWnd.
    }
    res = (*pCloseWindow)(hWnd);
    IfTraceError();
    return res;
}

BOOL WINAPI extSetSysColors(int cElements, const INT *lpaElements, const COLORREF *lpaRgbValues) {
    // v2.02.32: added to avoid SysColors changes by "Western Front"
    BOOL res;
    ApiName("SetSysColors");
    OutTraceDW("%s: Elements=%d\n", ApiRef, cElements);
    if(dxw.dwFlags3 & LOCKSYSCOLORS) return TRUE;
    res = (*pSetSysColors)(cElements, lpaElements, lpaRgbValues);
    IfTraceError();
    return res;
}

BOOL WINAPI extUpdateWindow(HWND hwnd) {
    BOOL res;
    ApiName("UpdateWindow");
    OutDebugDW("%s: hwnd=%#x\n", ApiRef, hwnd);
    if(dxw.Windowize && dxw.IsRealDesktop(hwnd)) {
        OutTraceDW("%s: remapping hwnd=%#x->%#x\n", ApiRef, hwnd, dxw.GethWnd());
        hwnd = dxw.GethWnd();
    }
    res = (*pUpdateWindow)(hwnd);
    IfTraceError();
    return res;
}

#ifndef DXW_NOTRACES
static char *sRedrawFlags(UINT flags) {
    static char s[256];
    strcpy(s, "RDW_");
    if(flags & RDW_ERASE) strcat(s, "ERASE+");
    if(flags & RDW_FRAME) strcat(s, "FRAME+");
    if(flags & RDW_INTERNALPAINT) strcat(s, "INTERNALPAINT+");
    if(flags & RDW_INVALIDATE) strcat(s, "INVALIDATE+");
    if(flags & RDW_NOERASE) strcat(s, "NOERASE+");
    if(flags & RDW_NOFRAME) strcat(s, "NOFRAME+");
    if(flags & RDW_NOINTERNALPAINT) strcat(s, "NOINTERNALPAINT+");
    if(flags & RDW_VALIDATE) strcat(s, "VALIDATE+");
    if(flags & RDW_ERASENOW) strcat(s, "ERASENOW+");
    if(flags & RDW_UPDATENOW) strcat(s, "UPDATENOW+");
    if(flags & RDW_ALLCHILDREN) strcat(s, "ALLCHILDREN+");
    if(flags & RDW_NOCHILDREN) strcat(s, "NOCHILDREN+");
    if(strlen(s) > strlen("RDW_")) s[strlen(s) - 1] = 0;
    else s[0] = 0;
    return s;
}
#endif // DXW_NOTRACES

BOOL WINAPI extRedrawWindow(HWND hWnd, const RECT *lprcUpdate, HRGN hrgnUpdate, UINT flags) {
    RECT rcUpdate;
    BOOL res;
    ApiName("RedrawWindow");
#ifndef DXW_NOTRACES
    if(IsTraceSYS) {
        char sRect[81];
        if(lprcUpdate) sprintf(sRect, "(%d,%d)-(%d,%d)", lprcUpdate->left, lprcUpdate->top, lprcUpdate->right, lprcUpdate->bottom);
        else strcpy(sRect, "NULL");
        OutTrace("%s: hwnd=%#x rcupdate=%s hrgn=%#x flags=%#x(%s)\n", ApiRef, hWnd, sRect, hrgnUpdate, flags, sRedrawFlags(flags));
    }
#endif
    // v2.03.64 fix: if hrgnUpdate is set, lprcUpdate is ignored, so it can't be scaled
    // beware: they both could be null, and that means the whole window
    if (!hrgnUpdate && lprcUpdate) rcUpdate = *lprcUpdate;
    // avoid redrawing the whole desktop
    if(dxw.Windowize && dxw.IsRealDesktop(hWnd)) hWnd = dxw.GethWnd();
    if(dxw.IsFullScreen()) {
        // v2.03.64 fix: if hrgnUpdate is set, lprcUpdate is ignored, so it can't be scaled
        if (!hrgnUpdate && lprcUpdate) rcUpdate = dxw.MapClientRect((LPRECT)lprcUpdate);
    }
    res = (*pRedrawWindow)(hWnd, lprcUpdate ? &rcUpdate : NULL, hrgnUpdate, flags);
    // v2.04.41: fix error condition is 0 return value
    IfTraceError();
    return res;
}

#ifdef BYPASSEDAPI
BOOL WINAPI extGetWindowPlacement(HWND hwnd, WINDOWPLACEMENT *lpwndpl) {
    BOOL ret;
    OutTraceDW("GetWindowPlacement: hwnd=%#x\n", hwnd);
    if(dxw.IsRealDesktop(hwnd)) {
        OutTraceDW("GetWindowPlacement: remapping hwnd=%#x->%#x\n", hwnd, dxw.GethWnd());
        hwnd = dxw.GethWnd();
    }
    ret = (*pGetWindowPlacement)(hwnd, lpwndpl);
    OutTraceDW("GetWindowPlacement: flags=%#x showCmd=%#x MinPosition=(%d,%d) MaxPosition=(%d,%d) NormalPosition=(%d,%d)-(%d,%d)\n",
               lpwndpl->flags, lpwndpl->showCmd,
               lpwndpl->ptMinPosition.x, lpwndpl->ptMinPosition.y,
               lpwndpl->ptMaxPosition.x, lpwndpl->ptMaxPosition.y,
               lpwndpl->rcNormalPosition.left, lpwndpl->rcNormalPosition.top, lpwndpl->rcNormalPosition.right, lpwndpl->rcNormalPosition.bottom);
    if (ret && dxw.Windowize && dxw.IsFullScreen()) {
        lpwndpl->showCmd = SW_SHOWNORMAL;
        lpwndpl->ptMinPosition.x = -1;
        lpwndpl->ptMinPosition.y = -1;
        lpwndpl->ptMaxPosition.x = -1;
        lpwndpl->ptMaxPosition.y = -1;
        OutTraceDW("GetWindowPlacement: FIXED showCmd=%#x MinPosition=(%d,%d) MaxPosition=(%d,%d) NormalPosition=(%d,%d)-(%d,%d)\n",
                   lpwndpl->showCmd,
                   lpwndpl->ptMinPosition.x, lpwndpl->ptMinPosition.y,
                   lpwndpl->ptMaxPosition.x, lpwndpl->ptMaxPosition.y,
                   lpwndpl->rcNormalPosition.left, lpwndpl->rcNormalPosition.top,
                   lpwndpl->rcNormalPosition.right, lpwndpl->rcNormalPosition.bottom);
    }
    if(!ret) OutTraceE("GetWindowPlacement: ERROR er=%d\n", GetLastError());
    return ret;
}

BOOL WINAPI extSetWindowPlacement(HWND hwnd, WINDOWPLACEMENT *lpwndpl) {
    BOOL ret;
    OutTraceDW("SetWindowPlacement: hwnd=%#x\n", hwnd);
    if(dxw.IsRealDesktop(hwnd)) {
        OutTraceDW("SetWindowPlacement: remapping hwnd=%#x->%#x\n", hwnd, dxw.GethWnd());
        hwnd = dxw.GethWnd();
    }
    OutTraceDW("SetWindowPlacement: flags=%#x showCmd=%#x MinPosition=(%d,%d) MaxPosition=(%d,%d) NormalPosition=(%d,%d)-(%d,%d)\n",
               lpwndpl->flags, lpwndpl->showCmd,
               lpwndpl->ptMinPosition.x, lpwndpl->ptMinPosition.y,
               lpwndpl->ptMaxPosition.x, lpwndpl->ptMaxPosition.y,
               lpwndpl->rcNormalPosition.left, lpwndpl->rcNormalPosition.top, lpwndpl->rcNormalPosition.right, lpwndpl->rcNormalPosition.bottom);
    switch (lpwndpl->showCmd) {
    case SW_MAXIMIZE:
        if (dxw.IsFullScreen()) {
            lpwndpl->showCmd = SW_SHOW;
            OutTraceDW("SetWindowPlacement: forcing SW_SHOW state\n");
        }
        break;
    }
    ret = (*pSetWindowPlacement)(hwnd, lpwndpl);
    if(!ret) OutTraceE("SetWindowPlacement: ERROR er=%d\n", GetLastError());
    return ret;
}
#endif // BYPASSEDAPI

HWND WINAPI extGetActiveWindow(void) {
    HWND ret;
    ApiName("GetActiveWindow");
    ret = (*pGetActiveWindow)();
    OutDebugSYS("%s: ret=%#x\n", ApiRef, ret);
    if(dxw.dwFlags10 & SLOWWINPOLLING) dxw.DoSlow(2);
#ifdef KEEPWINDOWSTATES
    if (KEEPWINDOWSTATES && hActiveWindow) {
        OutTraceDW("GetActiveWindow: ret=%#x->%#x\n", ret, hActiveWindow);
        return hActiveWindow;
    }
#endif
    if((dxw.dwFlags8 & WININSULATION) && dxw.Windowize && dxw.IsFullScreen()) {
        OutTraceDW("%s: ret=%#x->%#x\n", ApiRef, ret, dxw.GethWnd());
        return dxw.GethWnd();
    }
    return ret;
}

HWND WINAPI extGetForegroundWindow(void) {
    HWND ret;
    ApiName("GetForegroundWindow");
    ret = (*pGetForegroundWindow)();
    OutDebugDW("%s: ret=%#x\n", ApiRef, ret);
    if(dxw.dwFlags10 & SLOWWINPOLLING) dxw.DoSlow(2);
#ifdef KEEPWINDOWSTATES
    if (KEEPWINDOWSTATES && hForegroundWindow) {
        OutTraceDW("GetForegroundWindow: ret=%#x->%#x\n", ret, hForegroundWindow);
        return hForegroundWindow;
    }
#endif
    if((dxw.dwFlags8 & WININSULATION) && dxw.Windowize && dxw.IsFullScreen()) {
        OutTraceDW("%s: ret=%#x->%#x\n", ApiRef, ret, dxw.GethWnd());
        if (dxw.GethWnd()) return dxw.GethWnd(); // v2.04.80: avoid setting a NULL main window
    }
    return ret;
}

#ifdef TRACESYSCALLS

HWND WINAPI extSetCapture(HWND hwnd) {
    HWND ret;
    ApiName("SetCapture");
    OutTraceSYS("%s: hwnd=%#x\n", ApiRef, hwnd);
    ret = (*pSetCapture)(hwnd);
    OutTraceSYS("%s: ret=%#x\n", ApiRef, ret);
    return ret;
}

BOOL WINAPI extReleaseCapture(void) {
    BOOL ret;
    ApiName("ReleaseCapture");
    OutTraceSYS("%s\n", ApiRef);
    ret = (*pReleaseCapture)();
    OutTraceSYS("%s: ret=%#x\n", ApiRef, ret);
    return ret;
}

#ifndef DXW_NOTRACES
static char *ExplainGAFlags(UINT f) {
    char *s;
    switch(f) {
    case GA_PARENT:
        s = "PARENT";
        break;
    case GA_ROOT:
        s = "ROOT";
        break;
    case GA_ROOTOWNER:
        s = "ROOTOWNER";
        break;
    default:
        s = "???";
    }
    return s;
}
#endif // DXW_NOTRACES

HWND WINAPI extGetAncestor(HWND hwnd, UINT gaFlags) {
    HWND ret;
    ApiName("GetAncestor");
    OutTraceDW("%s: hwnd=%#x flags=%#x(%s)\n", ApiRef, hwnd, ExplainGAFlags(gaFlags));
    ret = (*pGetAncestor)(hwnd, gaFlags);
    OutTraceDW("%s: ret=%#x\n", ApiRef, ret);
    return ret;
}

#ifdef TRACEWINDOWS
HWND WINAPI extGetFocus(void) {
    HWND ret;
    ApiName("GetFocus");
    ret = (*pGetFocus)();
    OutTraceDW("%s: ret=%#x\n", ApiRef, ret);
    return ret;
}
#endif // TRACEWINDOWS

#ifdef TRACEWINDOWS
HWND WINAPI extSetFocus(HWND hWnd) {
    HWND ret;
    ApiName("SetFocus");
    ret = (*pSetFocus)(hWnd);
    OutTraceDW("%s: hwnd=%#x ret=%#x\n", ApiRef, hWnd, ret);
    return ret;
}
#endif // TRACEWINDOWS

HWND WINAPI extGetWindow(HWND hWnd, UINT uCmd) {
    HWND ret;
    ApiName("GetWindow");
    ret = (*pGetWindow)(hWnd, uCmd);
    OutTraceDW("%s: hwnd=%#x cmd=%#x ret=%#x\n", ApiRef, hWnd, uCmd, ret);
    return ret;
}

DWORD WINAPI extGetWindowThreadProcessId(HWND hWnd, LPDWORD lpdwProcessId) {
    DWORD ret;
    ApiName("GetWindowThreadProcessId");
    ret = (*pGetWindowThreadProcessId)(hWnd, lpdwProcessId);
    if(lpdwProcessId) OutTraceDW("%s: pid=%#x\n", ApiRef, *lpdwProcessId);
    OutTraceDW("%s: hwnd=%#x ret=%#x\n", ApiRef, hWnd, ret);
    return ret;
}

BOOL WINAPI extIsWindow(HWND hWnd) {
    BOOL ret;
    ApiName("IsWindow");
    ret = (*pIsWindow)(hWnd);
    OutTraceDW("%s: hwnd=%#x ret=%#x\n", ApiRef, hWnd, ret);
    return ret;
}

BOOL WINAPI extIsWindowEnabled(HWND hWnd) {
    BOOL ret;
    ApiName("IsWindowEnabled");
    ret = (*pIsWindowEnabled)(hWnd);
    OutTraceDW("%s: hwnd=%#x ret=%#x\n", ApiRef, hWnd, ret);
    return ret;
}

BOOL WINAPI extIsZoomed(HWND hWnd) {
    BOOL ret;
    ApiName("IsZoomed");
    ret = (*pIsZoomed)(hWnd);
    OutTraceDW("%s: hwnd=%#x ret=%#x\n", ApiRef, hWnd, ret);
    return ret;
}

BOOL WINAPI extIsIconic(HWND hWnd) {
    BOOL ret;
    ApiName("IsIconic");
    ret = (*pIsIconic)(hWnd);
    OutTraceDW("%s: hwnd=%#x ret=%#x\n", ApiRef, hWnd, ret);
    return ret;
}

HWND WINAPI extSetActiveWindow(HWND hwnd) {
    HWND ret;
    ApiName("SetActiveWindow");
    OutTraceDW("%s: hwnd=%#x\n", ApiRef, hwnd);
    ret = (*pSetActiveWindow)(hwnd);
#ifdef KEEPWINDOWSTATES
    if (KEEPWINDOWSTATES) hActiveWindow = hwnd;
#endif
    OutTraceDW("%s: ret=%#x\n", ApiRef, ret);
    return ret;
}
#endif // TRACESYSCALLS

BOOL WINAPI extIsWindowVisible(HWND hwnd) {
    BOOL ret;
    ApiName("IsWindowVisible");
    ret = (*pIsWindowVisible)(hwnd);
    OutDebugDW("%s: hwnd=%#x ret=%#x\n", ApiRef, hwnd, ret);
    while(!ret) {
        // v2.04.32
        if(dxw.dwFlags9 & MAKEWINVISIBLE) {
            OutTraceDW("%s: MAKEWINVISIBLE ret=TRUE\n", ApiRef);
            //(*pShowWindow)(hwnd, SW_SHOWNORMAL);
            SetWindowPos(hwnd, HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
            ret = TRUE;
            break;
        }
        if(dxw.IsDesktop(hwnd) && (dxw.dwFlags8 & PRETENDVISIBLE) && !ret) {
            OutTraceDW("%s: PRETENDVISIBLE ret=TRUE\n", ApiRef);
            ret = TRUE;
            break;
        }
        break;
    }
    return ret;
}

BOOL WINAPI intSystemParametersInfo(char *api, SystemParametersInfo_Type pSystemParametersInfo,
                                    UINT uiAction, UINT uiParam, PVOID pvParam, UINT fWinIni) {
    BOOL res;
    OutTraceDW("%s: Action=0x%04.4X Param=0x%X WinIni=0x%X\n", api, uiAction, uiParam, fWinIni);
    switch(uiAction) {
    case SPI_SETKEYBOARDDELAY:
    case SPI_SETKEYBOARDSPEED:
    case SPI_SETSCREENSAVERRUNNING: // v2.03.75 used by Dethkarz, but not really necessary
    case SPI_SETWORKAREA: // v2.05.05 very nasty effect, used by "Jetboat Superchamps 2"
        OutTraceDW("%s: bypass action=0x%04.4X\n", api, uiAction);
        return TRUE;
        break;
    }
    // some SPI_SETxx cases are commented out since already excluded above
    if(dxw.dwFlags11 & LOCKSYSSETTINGS) {
        switch (uiAction) {
        case SPI_SETBEEP:
        case SPI_SETMOUSE:
        case SPI_SETBORDER:
        //case SPI_SETKEYBOARDSPEED:
        case SPI_SETSCREENSAVETIMEOUT:
        case SPI_SETSCREENSAVEACTIVE:
        case SPI_SETGRIDGRANULARITY:
        case SPI_SETDESKWALLPAPER:
        case SPI_SETDESKPATTERN:
        //case SPI_SETKEYBOARDDELAY:
        case SPI_SETICONTITLEWRAP:
        case SPI_SETMENUDROPALIGNMENT:
        case SPI_SETDOUBLECLKWIDTH:
        case SPI_SETDOUBLECLKHEIGHT:
        case SPI_SETDOUBLECLICKTIME:
        case SPI_SETMOUSEBUTTONSWAP:
        case SPI_SETICONTITLELOGFONT:
        case SPI_SETFASTTASKSWITCH:
        case SPI_SETDRAGFULLWINDOWS:
        case SPI_SETNONCLIENTMETRICS:
        case SPI_SETMINIMIZEDMETRICS:
        case SPI_SETICONMETRICS:
        //case SPI_SETWORKAREA:
        case SPI_SETPENWINDOWS:
        case SPI_SETHIGHCONTRAST:
        case SPI_SETKEYBOARDPREF:
        case SPI_SETSCREENREADER:
        case SPI_SETANIMATION:
        case SPI_SETFONTSMOOTHING:
        case SPI_SETDRAGWIDTH:
        case SPI_SETDRAGHEIGHT:
        case SPI_SETHANDHELD:
        case SPI_SETLOWPOWERTIMEOUT:
        case SPI_SETPOWEROFFTIMEOUT:
        case SPI_SETLOWPOWERACTIVE:
        case SPI_SETPOWEROFFACTIVE:
        case SPI_SETCURSORS:
        case SPI_SETICONS:
        case SPI_SETDEFAULTINPUTLANG:
        case SPI_SETLANGTOGGLE:
        case SPI_SETMOUSETRAILS:
        //case SPI_SETSCREENSAVERRUNNING:
        case SPI_SETFILTERKEYS:
        case SPI_SETTOGGLEKEYS:
        case SPI_SETMOUSEKEYS:
        case SPI_SETSHOWSOUNDS:
        case SPI_SETSTICKYKEYS:
        case SPI_SETACCESSTIMEOUT:
        case SPI_SETSERIALKEYS:
        case SPI_SETSOUNDSENTRY:
        case SPI_SETSNAPTODEFBUTTON:
        case SPI_SETMOUSEHOVERWIDTH:
        case SPI_SETMOUSEHOVERHEIGHT:
        case SPI_SETMOUSEHOVERTIME:
        case SPI_SETWHEELSCROLLLINES:
        case SPI_SETMENUSHOWDELAY:
        case SPI_SETWHEELSCROLLCHARS:
        case SPI_SETSHOWIMEUI:
        case SPI_SETMOUSESPEED:
        case SPI_SETAUDIODESCRIPTION:
        case SPI_SETSCREENSAVESECURE:
            // there are more for WINVER >= 0x0500
            OutTraceDW("%s: lock action=0x%04.4X\n", api, uiAction);
            return TRUE;
            break;
        default:
            break;
        }
    }
    res = (*pSystemParametersInfo)(uiAction, uiParam, pvParam, fWinIni);
    if(uiAction == SPI_GETWORKAREA) {
        LPRECT cli = (LPRECT)pvParam;
        *cli = dxw.GetScreenRect();
        OutTraceDW("%s: resized client workarea rect=(%d,%d)-(%d,%d)\n", api, cli->left, cli->top, cli->right, cli->bottom);
    }
    IfTraceError();
    return res;
}

BOOL WINAPI extSystemParametersInfoA(UINT uiAction, UINT uiParam, PVOID pvParam, UINT fWinIni) {
    return intSystemParametersInfo("SystemParametersInfoA", pSystemParametersInfoA, uiAction, uiParam, pvParam, fWinIni);
}
BOOL WINAPI extSystemParametersInfoW(UINT uiAction, UINT uiParam, PVOID pvParam, UINT fWinIni) {
    return intSystemParametersInfo("SystemParametersInfoW", pSystemParametersInfoW, uiAction, uiParam, pvParam, fWinIni);
}

UINT_PTR WINAPI extSetTimer(HWND hWnd, UINT_PTR nIDEvent, UINT uElapse, TIMERPROC lpTimerFunc) {
    UINT uShiftedElapse;
    UINT_PTR ret;
    ApiName("SetTimer");
    // beware: the quicker the time flows, the more the time clicks are incremented,
    // and the lesser the pauses must be lasting! Shift operations are reverted in
    // GetSystemTime vs. Sleep or SetTimer
    uShiftedElapse = dxw.StretchTime(uElapse);
    OutTraceT("%s: hwnd=%#x TimerFunc=%#x elapse=%d->%d timeshift=%d\n", ApiRef, hWnd, lpTimerFunc, uElapse, uShiftedElapse, dxw.TimeShift);
    ret = (*pSetTimer)(hWnd, nIDEvent, uShiftedElapse, lpTimerFunc);
    if(ret) dxw.PushTimer(hWnd, ret, uElapse, lpTimerFunc);
    OutTraceT("%s: IDEvent=%#x ret=%#x\n", ApiRef, nIDEvent, ret);
    return ret;
}

BOOL WINAPI extKillTimer(HWND hWnd, UINT_PTR uIDEvent) {
    BOOL ret;
    ApiName("KillTimer");
    OutTraceT("%s: hwnd=%#x IDEvent=%#x\n", ApiRef, hWnd, uIDEvent);
    ret = (*pKillTimer)(hWnd, uIDEvent);
    OutTraceT("%s: ret=%#x\n", ApiRef, ret);
    if(ret) dxw.PopTimer(hWnd, uIDEvent);
    return ret;
}

BOOL WINAPI extGetUpdateRect(HWND hWnd, LPRECT lpRect, BOOL bErase) {
    BOOL ret;
    ApiName("GetUpdateRect");
    OutTraceDW("%s: hwnd=%#x lprect=%#x Erase=%#x\n", ApiRef, hWnd, lpRect, bErase);
    ret = (*pGetUpdateRect)(hWnd, lpRect, bErase);
    // v2.05.50: fix to handle possibly NULL lpRect value, used to test the existence of an updated rect
    // ref. https://docs.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-getupdaterect
    // fixes Silent Storm 2 & 3
    if(ret && lpRect) {
        OutTraceDW("%s: rect=(%d,%d)-(%d,%d)\n", ApiRef, lpRect->left, lpRect->top, lpRect->right, lpRect->bottom);
        if(dxw.IsFullScreen()) {
            dxw.UnmapClient(lpRect);
            OutTraceDW("%s: FIXED rect=(%d,%d)-(%d,%d)\n", ApiRef, lpRect->left, lpRect->top, lpRect->right, lpRect->bottom);
        }
    } else
        OutTraceE("%s: EMPTY\n", ApiRef);
    return ret;
}

BOOL WINAPI extGetCursorInfo(PCURSORINFO pci) {
    BOOL ret;
    ApiName("GetCursorInfo");
    OutTraceDW("%s\n", ApiRef);
    ret = (*pGetCursorInfo)(pci);
    if(ret) {
        OutTraceDW("%s: flags=%#x hcursor=%#x pos=(%d,%d)\n",
                   ApiRef, pci->flags, pci->hCursor, pci->ptScreenPos.x, pci->ptScreenPos.y);
        if(dxw.IsFullScreen()) {
            dxw.UnmapClient(&(pci->ptScreenPos));
            OutTraceDW("%s: FIXED pos=(%d,%d)\n", ApiRef, pci->ptScreenPos.x, pci->ptScreenPos.y);
        }
    } else
        OutTraceE("%s: ERROR err=%d\n", ApiRef, GetLastError());
    return ret;
}

HWND WINAPI extWindowFromPoint(POINT Point) {
    HWND ret;
    ApiName("WindowFromPoint");
    OutTraceDW("%s: point=(%d,%d)\n", ApiRef, Point.x, Point.y);
    if(dxw.IsFullScreen()) {
        dxw.MapWindow(&Point); // v2.03.69 fix
        OutTraceDW("%s: FIXED point=(%d,%d)\n", ApiRef, Point.x, Point.y);
    }
    ret = (*pWindowFromPoint)(Point);
    OutTraceDW("%s: hwnd=%#x\n", ApiRef, ret);
    return ret;
}

HWND WINAPI extChildWindowFromPoint(HWND hWndParent, POINT Point) {
    HWND ret;
    ApiName("ChildWindowFromPoint");
    OutTraceDW("%s: hWndParent=%#x point=(%d,%d)\n", ApiRef, hWndParent, Point.x, Point.y);
    if(dxw.IsDesktop(hWndParent) && dxw.IsFullScreen() && dxw.Windowize) {
        dxw.MapClient(&Point);
        OutTraceDW("%s: FIXED point=(%d,%d)\n", ApiRef, Point.x, Point.y);
    }
    ret = (*pChildWindowFromPoint)(hWndParent, Point);
    OutTraceDW("%s: hwnd=%#x\n", ApiRef, ret);
    return ret;
}

HWND WINAPI extChildWindowFromPointEx(HWND hWndParent, POINT Point, UINT uFlags) {
    HWND ret;
    ApiName("ChildWindowFromPointEx");
    OutTraceDW("%s: hWndParent=%#x point=(%d,%d) flags=%#x\n", ApiRef, hWndParent, Point.x, Point.y, uFlags);
    if(dxw.IsDesktop(hWndParent) && dxw.IsFullScreen() && dxw.Windowize) {
        dxw.UnmapClient(&Point);
        OutTraceDW("%s: FIXED point=(%d,%d)\n", ApiRef, Point.x, Point.y);
    }
    ret = (*pChildWindowFromPointEx)(hWndParent, Point, uFlags);
    OutTraceDW("%s: hwnd=%#x\n", ApiRef, ret);
    return ret;
}

BOOL extGetMonitorInfo(char *api, GetMonitorInfo_Type pGetMonitorInfo, HMONITOR hMonitor, LPMONITORINFO lpmi) {
    BOOL res, realres;
    OutTraceDW("%s: hMonitor=%#x mi=MONITORINFO%s\n", api, hMonitor, lpmi->cbSize == sizeof(MONITORINFO) ? "" : "EX");
    res = realres = (*pGetMonitorInfo)(hMonitor, lpmi);
    //v2.03.15 - must fix the coordinates also in case of error: that may depend on the windowed mode.
    if(dxw.Windowize) {
        if(res) {
            OutTraceDW("%s: FIX Work=(%d,%d)-(%d,%d) Monitor=(%d,%d)-(%d,%d) -> (%d,%d)-(%d,%d)\n",
                       api, lpmi->rcWork.left, lpmi->rcWork.top, lpmi->rcWork.right, lpmi->rcWork.bottom,
                       lpmi->rcMonitor.left, lpmi->rcMonitor.top, lpmi->rcMonitor.right, lpmi->rcMonitor.bottom,
                       0, 0, dxw.GetScreenWidth(), dxw.GetScreenHeight());
        } else {
            OutTraceDW("%s: ERROR err=%d FIX Work&Monitor -> (%d,%d)-(%d,%d) res=OK\n",
                       api, GetLastError(), 0, 0, dxw.GetScreenWidth(), dxw.GetScreenHeight());
        }
        res = TRUE;
        lpmi->rcWork = dxw.GetScreenRect();
        lpmi->rcMonitor = dxw.GetScreenRect();
    }
    if(realres) {
        LPMONITORINFOEXA lpmia;
        LPMONITORINFOEXW lpmiw;
        OutTraceDW("%s: Work=(%d,%d)-(%d,%d) Monitor=(%d,%d)-(%d,%d)\n", api,
                   lpmi->rcWork.left, lpmi->rcWork.top, lpmi->rcWork.right, lpmi->rcWork.bottom,
                   lpmi->rcMonitor.left, lpmi->rcMonitor.top, lpmi->rcMonitor.right, lpmi->rcMonitor.bottom);
        switch(lpmi->cbSize) {
        case sizeof(MONITORINFOEXA):
            lpmia = (LPMONITORINFOEXA)lpmi;
            OutTraceDW("%s: Monitor=\"%s\"\n", api, lpmia->szDevice);
            break;
        case sizeof(MONITORINFOEXW):
            lpmiw = (LPMONITORINFOEXW)lpmi;
            OutTraceDW("%s: Monitor=\"%ls\"\n", api, lpmiw->szDevice);
            break;
        }
    } else
        OutTraceE("%s: ERROR err=%d\n", api, GetLastError());
    return res;
}

BOOL WINAPI extGetMonitorInfoA(HMONITOR hMonitor, LPMONITORINFO lpmi) {
    return extGetMonitorInfo("GetMonitorInfoA", pGetMonitorInfoA, hMonitor, lpmi);
}
BOOL WINAPI extGetMonitorInfoW(HMONITOR hMonitor, LPMONITORINFO lpmi) {
    return extGetMonitorInfo("GetMonitorInfoW", pGetMonitorInfoW, hMonitor, lpmi);
}

int WINAPI extGetUpdateRgn(HWND hWnd, HRGN hRgn, BOOL bErase) {
    int regionType;
    ApiName("GetUpdateRgn");
    regionType = (*pGetUpdateRgn)(hWnd, hRgn, bErase);
    OutTraceDW("%s: hwnd=%#x hrgn=%#x erase=%#x regionType=%#x(%s)\n",
               ApiRef, hWnd, hRgn, bErase, regionType, ExplainRegionType(regionType));
    if(dxw.IsFullScreen() && (dxw.GDIEmulationMode != GDIMODE_NONE)) {
        HRGN hrgnScaled = CreateRectRgn(0, 0, 0, 0);
        hrgnScaled = dxw.UnmapRegion(ApiRef, hRgn);
        CombineRgn(hRgn, hrgnScaled, NULL, RGN_COPY);
        (*pDeleteObject)(hrgnScaled);
    }
    return regionType;
}

#ifdef NOUNHOOKED
BOOL WINAPI extValidateRect(HWND hWnd, const RECT *lpRect) {
    BOOL ret;
    if(IsTraceSYS) {
        if(lpRect)
            OutTrace("ValidateRect: hwnd=%#x rect=(%d,%d)-(%d,%d)\n",
                     hWnd, lpRect->left, lpRect->top, lpRect->right, lpRect->bottom);
        else
            OutTrace("ValidateRect: hwnd=%#x rect=NULL\n", hWnd);
    }
    ret = (*pValidateRect)(hWnd, lpRect);
    return ret;
}

int WINAPI extGetWindowTextA(HWND hWnd, LPTSTR lpString, int nMaxCount) {
    // purpose of this wrapped call is to clear the FPS indicator (format " ~ (%d FPS)")
    // from the window title, if present. It crashes games such as "Panzer General 3 Scorched Earth"
    // when FPS on window title is activated.
    int ret;
    OutTraceDW("GetWindowTextA: hwnd=%#x MaxCount=%d\n", hWnd, nMaxCount);
    ret = (*pGetWindowTextA)(hWnd, lpString, nMaxCount);
    if(ret) OutTraceDW("GetWindowTextA: ret=%d String=\"%s\"\n", ret, lpString);
    if (ret && (dxw.dwFlags2 & SHOWFPS) && dxw.ishWndFPS(hWnd)) {
        char *p;
        p = strstr(lpString, " ~ (");
        if(p) {
            *p = NULL;
            ret = strlen(lpString);
            OutTraceDW("GetWindowTextA: FIXED ret=%d String=\"%s\"\n", ret, lpString);
        }
    }
    return ret;
}
#endif

BOOL WINAPI extBringWindowToTop(HWND hwnd) {
    BOOL res;
    ApiName("BringWindowToTop");
    OutTraceDW("%s: hwnd=%#x\n", ApiRef, hwnd);
    if(dxw.dwFlags5 & UNLOCKZORDER) return TRUE;
    res = (*pBringWindowToTop)(hwnd);
    return res;
}

BOOL WINAPI extSetForegroundWindow(HWND hwnd) {
    BOOL res;
    ApiName("SetForegroundWindow");
    OutTraceDW("%s: hwnd=%#x\n", ApiRef, hwnd);
#ifdef KEEPWINDOWSTATES
    hForegroundWindow = hwnd;
#endif
    if(dxw.dwFlags5 & UNLOCKZORDER) return TRUE;
    res = (*pSetForegroundWindow)(hwnd);
    return res;
}

/*
    HOOKPROC glpMouseHookProcessFunction;
    LRESULT CALLBACK extMouseHookProc(int code, WPARAM wParam, LPARAM lParam)
    {
	LRESULT ret;
	OutTrace("HookProc intercepted: code=%#x wParam=%#x lParam=%#x\n", code, wParam, lParam);
    MOUSEHOOKSTRUCT * pMouseStruct = (MOUSEHOOKSTRUCT *)lParam;
    if (pMouseStruct != NULL){
		dxw.UnmapWindow(&(pMouseStruct->pt));
    }
	ret= (*glpMouseHookProcessFunction)(code, wParam, lParam);
	return ret;
    }
*/

#ifndef DXW_NOTRACES
#ifndef WH_HARDWARE
#define WH_HARDWARE 8
#endif

static char *sWinHookLabel(int id) {
    char *s;
    switch(id) {
    case WH_MSGFILTER:
        s = "MSGFILTER";
        break;
    case WH_JOURNALRECORD:
        s = "JOURNALRECORD";
        break;
    case WH_JOURNALPLAYBACK:
        s = "WJOURNALPLAYBACK";
        break;
    case WH_KEYBOARD:
        s = "WKEYBOARD";
        break;
    case WH_GETMESSAGE:
        s = "GETMESSAGE";
        break;
    case WH_CALLWNDPROC:
        s = "CALLWNDPROC";
        break;
    case WH_CBT:
        s = "CBT";
        break;
    case WH_SYSMSGFILTER:
        s = "SYSMSGFILTER";
        break;
    case WH_MOUSE:
        s = "MOUSE";
        break;
    case WH_HARDWARE:
        s = "HARDWARE";
        break;
    case WH_DEBUG:
        s = "DEBUG";
        break;
    case WH_SHELL:
        s = "SHELL";
        break;
    case WH_FOREGROUNDIDLE:
        s = "FOREGROUNDIDLE";
        break;
    case WH_CALLWNDPROCRET:
        s = "CALLWNDPROCRET";
        break;
    case WH_KEYBOARD_LL:
        s = "KEYBOARD_LL";
        break;
    case WH_MOUSE_LL:
        s = "MOUSE_LL";
        break;
    default:
        s = "unknown";
        break;
    }
    return s;
}
#endif // DXW_NOTRACES

HOOKPROC glpMessageHookProcessFunction;
HOOKPROC glpMouseHookProcessFunction;
HOOKPROC glpMouseHookProcessFunctionLL;
HOOKPROC glpCBTHookProcessFunction;
HOOKPROC glpKeyboardHookProcessFunctionLL;
HOOKPROC glpKeyboardHookProcessFunction;
HOOKPROC glpMsgFilterHookProcessFunction;
#ifdef HOOKWINDOWSHOOKPROCS
HOOKPROC glpWindowsHookProcessFunction;
HOOKPROC glpWindowsHookProcessRetFunction;
#endif // HOOKWINDOWSHOOKPROCS

#if 0
LRESULT CALLBACK extMsgFilterHookProc(int code, WPARAM wParam, LPARAM lParam) {
    LRESULT ret;
    //OutTraceDW("MsgFilterHookProc: code=%#x wParam=%#x lParam=%#x\n", code, wParam, lParam);
    OutTrace("MsgFilterHookProc: code=%#x wParam=%#x lParam=%#x\n", code, wParam, lParam);
    MSG *pMessage = (MSG *)lParam;
    ret = NULL;
    if(pMessage) {
        UINT message = pMessage->message;
        if ((message >= WM_MOUSEFIRST) && (message <= WM_MOUSELAST)) {		// mouse messages
            //OutTrace("MsgFilterHookProc: MOUSE message\n");
            //lParam = 0;
        }
    }
    ret = (*glpMsgFilterHookProcessFunction)(code, wParam, lParam);
    return ret;
}
#endif

LRESULT CALLBACK extEASportsMessageHookProc(int code, WPARAM wParam, LPARAM lParam) {
    LRESULT ret;
    OutTraceDW("MessageHookProc: code=%#x wParam=%#x lParam=%#x\n", code, wParam, lParam);
    MSG *pMessage = (MSG *)lParam;
    ret = NULL;
    if(pMessage) {
        UINT message = pMessage->message;
        if ((message >= 0x600) ||											// custom messages
                ((message >= WM_KEYFIRST) && (message <= WM_KEYLAST)) ||		// keyboard messages
                ((message >= WM_MOUSEFIRST) && (message <= WM_MOUSELAST))		// mouse messages
           )
            ret = (*glpMessageHookProcessFunction)(code, wParam, lParam);
    }
    return ret;
}

LRESULT CALLBACK extMessageHookProc(int code, WPARAM wParam, LPARAM lParam) {
    LRESULT ret;
    OutTraceDW("MessageHookProc: code=%#x wParam=%#x lParam=%#x\n", code, wParam, lParam);
    if(code < 0) return CallNextHookEx(0, code, wParam, lParam);
    MSG *pMessage = (MSG *)lParam;
    ret = NULL;
    if(pMessage && dxw.IsFullScreen()) {
        UINT message = pMessage->message;
        POINT pt;
        (*pGetCursorPos)(&pt);
        // pt.x -= dxw.iPosX;
        // pt.y -= dxw.iPosY;
        pMessage->pt = pt;
        if ((message >= WM_MOUSEFIRST) && (message <= WM_MOUSELAST)) 		// mouse messages
            pMessage->lParam = MAKELPARAM(pt.x, pt.y);
        ret = (*glpMessageHookProcessFunction)(code, wParam, lParam);
    }
    return ret;
}

static POINT FixMousePoint(POINT pt) {
    dxw.UnmapWindow(&pt);
    if(pt.x < 0) pt.x = 0;
    if(pt.x >= (LONG)dxw.GetScreenWidth()) pt.x = dxw.GetScreenWidth() - 1;
    if(pt.y < 0) pt.y = 0;
    if(pt.y >= (LONG)dxw.GetScreenHeight()) pt.y = dxw.GetScreenHeight() - 1;
    return pt;
}

LRESULT CALLBACK extMouseHookProc(int code, WPARAM wParam, LPARAM lParam) {
    OutTraceC("MouseHookProc: code=%#x wParam=%#x lParam=%#x\n", code, wParam, lParam);
    if(code < 0) return CallNextHookEx(0, code, wParam, lParam);
    if(lParam) {
        MOUSEHOOKSTRUCT MouseStruct = *(MOUSEHOOKSTRUCT *)lParam;
        MouseStruct.pt = FixMousePoint(MouseStruct.pt);
        OutTraceC("MouseHookProc: event=%s pos=(%d,%d)->(%d,%d)\n",
                  ExplainWinMessage(wParam),
                  ((MOUSEHOOKSTRUCT *)lParam)->pt.x,  ((MOUSEHOOKSTRUCT *)lParam)->pt.y,
                  MouseStruct.pt.x, MouseStruct.pt.y);
        return (*glpMouseHookProcessFunction)(code, wParam, (LPARAM)&MouseStruct);
    }
    return (*glpMouseHookProcessFunction)(code, wParam, lParam);
}

LRESULT CALLBACK extMouseHookProcLL(int code, WPARAM wParam, LPARAM lParam) {
    OutTraceC("MouseHookProcLL: code=%#x wParam=%#x lParam=%#x\n", code, wParam, lParam);
    if(code < 0) return CallNextHookEx(0, code, wParam, lParam);
    if(lParam) {
        MSLLHOOKSTRUCT MouseStruct = *(MSLLHOOKSTRUCT *)lParam;
        MouseStruct.pt = FixMousePoint(MouseStruct.pt);
        OutTraceC("MouseHookProcLL: event=%s pos=(%d,%d)->(%d,%d)\n",
                  ExplainWinMessage(wParam),
                  ((MSLLHOOKSTRUCT *)lParam)->pt.x,  ((MSLLHOOKSTRUCT *)lParam)->pt.y,
                  MouseStruct.pt.x, MouseStruct.pt.y);
        return (*glpMouseHookProcessFunctionLL)(code, wParam, (LPARAM)&MouseStruct);
    }
    return (*glpMouseHookProcessFunctionLL)(code, wParam, lParam);
}

LRESULT CALLBACK KeyboardHookProcessFunctionLL(int code, WPARAM wParam, LPARAM lParam) {
    OutDebugC("KeyboardHookProcessFunctionLL: code=%#x wParam=%#x lParam=%#x\n", code, wParam, lParam);
    if(code < 0) return CallNextHookEx(0, code, wParam, lParam);
    switch(wParam) {
    case WM_SYSKEYDOWN:
    case WM_SYSKEYUP:
        // v2.03.54: disable the disable Alt-Tab fix
        // process the WM_SYSKEYDOWN/UP events only - needed by "Recoil".
        // v2.05.06: moved inside DxWnd hook
        if(dxw.dwFlags7 & NODISABLEALTTAB) {
            OutTraceDW("KeyboardHookProcessFunctionLL: NODISABLEALTTAB skip SYSKEY\n");
            return 0;
        }
        break;
    case VK_PRINT:
    case VK_SNAPSHOT:
        if(dxw.dwFlags11 & NODISABLEPRINT) {
            OutTraceDW("KeyboardHookProcessFunction: skip PRINT key\n");
            return 0;
        }
        break;
    }
    return (*glpKeyboardHookProcessFunctionLL)(code, wParam, lParam);
}

LRESULT CALLBACK KeyboardHookProcessFunction(int code, WPARAM wParam, LPARAM lParam) {
    OutDebugC("KeyboardHookProcessFunction: code=%#x wParam=%#x lParam=%#x\n", code, wParam, lParam);
    if(code < 0) return CallNextHookEx(0, code, wParam, lParam);
    switch(wParam) {
    case WM_SYSKEYDOWN:
    case WM_SYSKEYUP:
        if(dxw.dwFlags7 & NODISABLEALTTAB) {
            OutTraceDW("KeyboardHookProcessFunction: NODISABLEALTTAB skip SYSKEY\n");
            return 0;
        }
        break;
    case VK_PRINT:
    case VK_SNAPSHOT:
        if(dxw.dwFlags11 & NODISABLEPRINT) {
            OutTraceDW("KeyboardHookProcessFunction: skip PRINT key\n");
            return 0;
        }
        break;
    }
    return (*glpKeyboardHookProcessFunction)(code, wParam, lParam);
}

#ifndef DXW_NOTRACES
static char *ExplainCBTCode(int code) {
    char *sCodes[] = {
        "MOVESIZE",
        "MINMAX",
        "QS",
        "CREATEWND",
        "DESTROYWND",
        "ACTIVATE",
        "CLICKSKIPPED",
        "KEYSKIPPED",
        "SYSCOMMAND",
        "SETFOCUS",
        "unknown"
    };
    if((code < 0) || (code > HCBT_SETFOCUS)) return sCodes[HCBT_SETFOCUS + 1];
    return sCodes[code];
}
#endif // DXW_NOTRACES

LRESULT CALLBACK extCBTHookProcessFunction(int code, WPARAM wParam, LPARAM lParam) {
    LRESULT ret;
    LPCBT_CREATEWNDA lpCBWndA;
    POINT pt;
    OutTraceSYS("CBTHookProcess: code=%#x(%s) wParam=%#x lParam=%#x\n", code, ExplainCBTCode(code), wParam, lParam);
    if(code < 0) return CallNextHookEx(0, code, wParam, lParam);
    switch(code) {
    case HCBT_CREATEWND:
        lpCBWndA = (LPCBT_CREATEWNDA) lParam;
        OutDebugSYS("CBTHookProcess: CREATE pt=(%d,%d) sz=(%d,%d)\n",
                    lpCBWndA->lpcs->x, lpCBWndA->lpcs->y, lpCBWndA->lpcs->cx, lpCBWndA->lpcs->cy);
        pt.x = lpCBWndA->lpcs->x;
        pt.y = lpCBWndA->lpcs->y;
        dxw.MapClient(&pt);
        lpCBWndA->lpcs->x = pt.x;
        lpCBWndA->lpcs->y = pt.y;
        pt.x = lpCBWndA->lpcs->cx;
        pt.y = lpCBWndA->lpcs->cy;
        dxw.MapClient(&pt);
        lpCBWndA->lpcs->cx = pt.x;
        lpCBWndA->lpcs->cy = pt.y;
        OutDebugSYS("CBTHookProcess: FIXED pt=(%d,%d) sz=(%d,%d)\n",
                    lpCBWndA->lpcs->x, lpCBWndA->lpcs->y, lpCBWndA->lpcs->cx, lpCBWndA->lpcs->cy);
        break;
    default:
        break;
    }
    ret = (*glpCBTHookProcessFunction)(code, wParam, lParam);
    return 0;
}

#ifdef HOOKWINDOWSHOOKPROCS

LRESULT CALLBACK extWindowsHookProc(int code, WPARAM wParam, LPARAM lParam) {
    OutTraceSYS("HookWndProc: code=%#x wParam=%#x lParam=%#x\n", code, wParam, lParam);
    if(code < 0) return CallNextHookEx(0, code, wParam, lParam);
    if(lParam) {
        CWPSTRUCT *cwp;
        WINDOWPOS *lpwp;
        CREATESTRUCTA *lpcs;
        int width, height;
        cwp = (LPCWPSTRUCT)lParam;
        OutTraceSYS("HookWndProc: msg=%#x(%s) wParam=%#x lParam=%#x hwnd=%#x\n",
                    cwp->message, ExplainWinMessage(cwp->message), cwp->wParam, cwp->lParam, cwp->hwnd);
        switch(cwp->message) {
        case WM_SIZE:
            width = LOWORD(cwp->lParam);
            height = HIWORD(cwp->lParam);
            OutTraceDW("HookWndProc: original WM_SIZE size=(%dx%d)\n", width, height);
            dxw.UnmapClient(&width, &height);
            cwp->lParam = MAKELPARAM(width, height);
            OutTraceDW("HookWndProc: remapped WM_SIZE size=(%dx%d)\n", width, height);
            break;
        case WM_WINDOWPOSCHANGING:
        case WM_WINDOWPOSCHANGED:
            lpwp = (LPWINDOWPOS)cwp->lParam;
            OutTraceDW("HookWndProc: original WM_WINDOWPOSCHANGx flags=%#x pos=(%d,%d) size=(%dx%d)\n",
                       lpwp->flags, lpwp->x, lpwp->y, lpwp->cx, lpwp->cy);
            if(!(lpwp->flags & (SWP_NOSIZE | SWP_NOMOVE))) {
                if(dxw.IsDesktop(cwp->hwnd)) {
                    lpwp->x = 0;
                    lpwp->y = 0;
                    lpwp->cx = dxw.GetScreenWidth();
                    lpwp->cy = dxw.GetScreenHeight();
                } else
                    dxw.UnmapWindow(&(lpwp->x), &(lpwp->y), &(lpwp->cx), &(lpwp->cy));
            }
            OutTraceDW("HookWndProc: remapped WM_WINDOWPOSCHANGx flags=%#x pos=(%d,%d) size=(%dx%d)\n",
                       lpwp->flags, lpwp->x, lpwp->y, lpwp->cx, lpwp->cy);
            break;
        case WM_CREATE:
        case WM_NCCREATE:
            lpcs = (LPCREATESTRUCTA)cwp->lParam;
            OutTraceDW("HookWndProc: original pos=(%d,%d) size=(%dx%d)\n",
                       lpcs->x, lpcs->y, lpcs->cx, lpcs->cy);
            dxw.UnmapWindow(&(lpcs->x), &(lpcs->y), &(lpcs->cx), &(lpcs->cy));
            OutTraceDW("HookWndProc: remapped pos=(%d,%d) size=(%dx%d)\n",
                       lpcs->x, lpcs->y, lpcs->cx, lpcs->cy);
        case WM_NCCALCSIZE:
            if(wParam) {
            } else {
                LPRECT lprect = (LPRECT)cwp->lParam;
                OutTraceDW("HookWndProc: original WM_NCCALCSIZE rect=(%d,%d)-(%d,%d)\n",
                           lprect->left, lprect->top, lprect->right, lprect->bottom);
                dxw.UnmapWindow(lprect);
                OutTraceDW("HookWndProc: remapped WM_NCCALCSIZE rect=(%d,%d)-(%d,%d)\n",
                           lprect->left, lprect->top, lprect->right, lprect->bottom);
            }
            // tbd: useful or harmful? Should fix points only inside client area ??
            //case WM_NCHITTEST:
            //	width = LOWORD(cwp->lParam);
            //	height = HIWORD(cwp->lParam);
            //	OutTraceDW("HookWndProc: original WM_NCHITTEST pos=(%d,%d)\n", width, height);
            //	dxw.UnmapClient(&width, &height);
            //	cwp->lParam = MAKELPARAM(width, height);
            //	OutTraceDW("HookWndProc: remapped WM_NCHITTEST pos=(%d,%d)\n", width, height);
            //	break;
        }
    }
    return (*glpWindowsHookProcessFunction)(code, wParam, lParam);
}

LRESULT CALLBACK extWindowsHookProcRet(int code, WPARAM wParam, LPARAM lParam) {
    OutTraceSYS("HookWndProcRet: code=%#x wParam=%#x lParam=%#x\n", code, wParam, lParam);
    if(code < 0) return CallNextHookEx(0, code, wParam, lParam);
    if(lParam) {
        CWPRETSTRUCT *cwp;
        WINDOWPOS *lpwp;
        CREATESTRUCTA *lpcs;
        int width, height;
        cwp = (LPCWPRETSTRUCT)lParam;
        OutTraceSYS("HookWndProcRet: res=%#x msg=%#x(%s) wParam=%#x lParam=%#x hwnd=%#x\n",
                    cwp->lResult, cwp->message, ExplainWinMessage(cwp->message), cwp->wParam, cwp->lParam, cwp->hwnd);
        switch(cwp->message) {
        case WM_SIZE:
            width = LOWORD(cwp->lParam);
            height = HIWORD(cwp->lParam);
            OutTraceDW("HookWndProcRet: original WM_SIZE size=(%dx%d)\n", width, height);
            dxw.UnmapClient(&width, &height);
            cwp->lParam = MAKELPARAM(width, height);
            OutTraceDW("HookWndProcRet: remapped WM_SIZE size=(%dx%d)\n", width, height);
            break;
        case WM_WINDOWPOSCHANGING:
        case WM_WINDOWPOSCHANGED:
            lpwp = (LPWINDOWPOS)cwp->lParam;
            OutTraceDW("HookWndProcRet: original WM_WINDOWPOSCHANGx flags=%#x pos=(%d,%d) size=(%dx%d)\n",
                       lpwp->flags, lpwp->x, lpwp->y, lpwp->cx, lpwp->cy);
            if(!(lpwp->flags & (SWP_NOSIZE | SWP_NOMOVE))) {
                if(dxw.IsDesktop(cwp->hwnd)) {
                    lpwp->x = 0;
                    lpwp->y = 0;
                    lpwp->cx = dxw.GetScreenWidth();
                    lpwp->cy = dxw.GetScreenHeight();
                } else
                    dxw.UnmapWindow(&(lpwp->x), &(lpwp->y), &(lpwp->cx), &(lpwp->cy));
            }
            OutTraceDW("HookWndProcRet: remapped WM_WINDOWPOSCHANGx flags=%#x pos=(%d,%d) size=(%dx%d)\n",
                       lpwp->flags, lpwp->x, lpwp->y, lpwp->cx, lpwp->cy);
            break;
        case WM_CREATE:
        case WM_NCCREATE:
            lpcs = (LPCREATESTRUCTA)cwp->lParam;
            OutTraceDW("HookWndProcRet: original pos=(%d,%d) size=(%dx%d)\n",
                       lpcs->x, lpcs->y, lpcs->cx, lpcs->cy);
            dxw.UnmapWindow(&(lpcs->x), &(lpcs->y), &(lpcs->cx), &(lpcs->cy));
            OutTraceDW("HookWndProc: remapped pos=(%d,%d) size=(%dx%d)\n",
                       lpcs->x, lpcs->y, lpcs->cx, lpcs->cy);
            break;
        case WM_NCCALCSIZE:
            if(wParam) {
            } else {
                LPRECT lprect = (LPRECT)cwp->lParam;
                OutTraceDW("HookWndProcRet: original WM_NCCALCSIZE rect=(%d,%d)-(%d,%d)\n",
                           lprect->left, lprect->top, lprect->right, lprect->bottom);
                dxw.UnmapWindow(lprect);
                OutTraceDW("HookWndProcRet: remapped WM_NCCALCSIZE rect=(%d,%d)-(%d,%d)\n",
                           lprect->left, lprect->top, lprect->right, lprect->bottom);
            }
            break;
        }
    }
    return (*glpWindowsHookProcessRetFunction)(code, wParam, lParam);
}
#endif // HOOKWINDOWSHOOKPROCS

LRESULT CALLBACK DummyHookProc(int code, WPARAM wParam, LPARAM lParam) {
    return CallNextHookEx((HHOOK)0, code, wParam, lParam);
}

static HHOOK WINAPI extSetWindowsHookEx(char *api, SetWindowsHookEx_Type pSetWindowsHookEx,
                                        int idHook, HOOKPROC lpfn, HINSTANCE hMod, DWORD dwThreadId) {
    HHOOK ret;
    OutTraceSYS("%s: id=%#x(%s) lpfn=%#x hmod=%#x threadid=%#x\n", api, idHook, sWinHookLabel(idHook), lpfn, hMod, dwThreadId);
    if(dxw.dwDFlags2 & DISABLEWINHOOKS) {
        ret = (*pSetWindowsHookEx)(idHook, DummyHookProc, hMod, dwThreadId);
        OutTraceDW("%s: DISABLEWINHOOKS hhook=%#x\n", ApiRef, ret);
    }
    if(dxw.dwFlags5 & EASPORTSHACK) {
        OutTraceDW("%s: EASPORTSHACK bypass active\n", api);
        if(idHook == WH_MOUSE) return NULL;
        if(idHook == WH_GETMESSAGE) {
            glpMessageHookProcessFunction = lpfn;
            lpfn = extEASportsMessageHookProc;
        }
    }
    if((dxw.dwFlags11 & FIXMESSAGEHOOK) && (idHook == WH_GETMESSAGE)) {
        OutTraceDW("%s: MESSAGEHOOK filter active\n", api);
        glpMessageHookProcessFunction = lpfn;
        lpfn = extMessageHookProc;
    }
    if(idHook == WH_KEYBOARD) {
        // v2.03.39: "One Must Fall Battlegrounds" keyboard fix
        if(dwThreadId == NULL) {
            dwThreadId = GetCurrentThreadId();
            OutTraceDW("%s: fixing WH_KEYBOARD thread=0->%#x\n", api, dwThreadId);
        }
        OutTraceDW("%s: WH_KEYBOARD bypass active\n", api);
        glpKeyboardHookProcessFunction = lpfn;
        lpfn = KeyboardHookProcessFunction;
    }
    if(idHook == WH_KEYBOARD_LL) {
        OutTraceDW("%s: WH_KEYBOARD_LL bypass active\n", api);
        glpKeyboardHookProcessFunctionLL = lpfn;
        lpfn = KeyboardHookProcessFunctionLL;
    }
    // v2.04.13: "Starsiege" mouse control fix
    if((idHook == WH_CBT) && (dwThreadId == NULL)) {
        dwThreadId = GetCurrentThreadId();
        OutTraceDW("%s: fixing WH_CBT thread=0->%#x\n", api, dwThreadId);
    }
    if((idHook == WH_CBT) && (dxw.dwFlags12 & SCALECBTHOOK)) {
        OutTraceDW("%s: WH_CBT bypass active\n", api);
        glpCBTHookProcessFunction = lpfn;
        lpfn = extCBTHookProcessFunction;
    }
    if(dxw.dwFlags8 & FIXMOUSEHOOK) {
        if(dwThreadId == 0) dwThreadId = GetCurrentThreadId(); // GameStation mouse input fix
        if(idHook == WH_MOUSE) {
            OutTraceDW("%s: FIXMOUSEHOOK filter active on WH_MOUSE\n", api);
            glpMouseHookProcessFunction = lpfn;
            lpfn = extMouseHookProc;
        }
        if (idHook == WH_MOUSE_LL) {
            OutTraceDW("%s: FIXMOUSEHOOK filter active on WH_MOUSE_LL\n", api);
            glpMouseHookProcessFunctionLL = lpfn;
            lpfn = extMouseHookProcLL;
        }
    }
    if(dxw.dwDFlags2 & EXPERIMENTAL) {
        if(idHook == WH_MSGFILTER) return (HHOOK)0xDEADBEEF;
        if(idHook == WH_CBT) return (HHOOK)0xDEADBEEF;
        //if(idHook == WH_MSGFILTER){
        //	OutTraceDW("%s: FIXMOUSEHOOK filter active on WH_MSGFILTER\n", api);
        //	glpMsgFilterHookProcessFunction = lpfn;
        //	lpfn=extMsgFilterHookProc;
        //}
    }
#ifdef HOOKWINDOWSHOOKPROCS
    if(HOOKWINDOWSHOOKPROCS) {
        if(idHook == WH_CALLWNDPROC) {
            OutTraceDW("%s: FIXWINDOWSHOOK filter active on WH_CALLWNDPROC\n", api);
            glpWindowsHookProcessFunction = lpfn;
            lpfn = extWindowsHookProc;
        }
        if(idHook == WH_CALLWNDPROCRET) {
            OutTraceDW("%s: FIXWINDOWSHOOK filter active on WH_CALLWNDPROCRET\n", api);
            glpWindowsHookProcessRetFunction = lpfn;
            lpfn = extWindowsHookProcRet;
        }
    }
#endif // HOOKWINDOWSHOOKPROCS
    ret = (*pSetWindowsHookEx)(idHook, lpfn, hMod, dwThreadId);
    OutTraceSYS("%s: hhk=%#x\n", api, ret);
    return ret;
}

HHOOK WINAPI extSetWindowsHookExA(int idHook, HOOKPROC lpfn, HINSTANCE hMod, DWORD dwThreadId) {
    return extSetWindowsHookEx("SetWindowsHookExA", pSetWindowsHookExA, idHook, lpfn, hMod, dwThreadId);
}
HHOOK WINAPI extSetWindowsHookExW(int idHook, HOOKPROC lpfn, HINSTANCE hMod, DWORD dwThreadId) {
    return extSetWindowsHookEx("SetWindowsHookExW", pSetWindowsHookExW, idHook, lpfn, hMod, dwThreadId);
}

BOOL WINAPI extUnhookWindowsHookEx(HHOOK hhk) {
    BOOL ret;
    ApiName("UnhookWindowsHookEx");
    OutTraceSYS("%s: hhk=%#x\n", ApiRef, hhk);
    ret = (*pUnhookWindowsHookEx)(hhk);
    if(!ret)
        OutTraceE("%s: ERROR err=%d\n", ApiRef, GetLastError());
    return ret;
}

HRESULT WINAPI extMessageBoxA(HWND hWnd, LPCSTR lpText, LPCSTR lpCaption, UINT uType) {
    HRESULT res;
    OutTraceDW("MessageBoxA: hwnd=%#x text=\"%s\" caption=\"%s\" type=%d\n", hWnd, lpText, lpCaption, uType);
    if(dxw.dwFlags9 & NODIALOGS) {
        switch (uType & 0x7) {
        case MB_OK:
            res = IDOK;
            break;
        case MB_OKCANCEL:
            res = IDCANCEL;
            break;
        case MB_ABORTRETRYIGNORE:
            res = IDIGNORE;
            break;
        case MB_YESNOCANCEL:
            res = IDCANCEL;
            break;
        case MB_YESNO:
            res = IDYES;
            break;
        case MB_RETRYCANCEL:
            res = IDCANCEL;
            break;
        case MB_CANCELTRYCONTINUE:
            res = IDCONTINUE;
            break;
        default:
            res = IDOK;
            break;
        }
        return res;
    }
    if(dxw.dwFlags11 & CUSTOMLOCALE) {
        LPWSTR wstr = NULL;
        LPWSTR wcaption = NULL;
        int n;
        if (lpText) {
            int size = lstrlenA(lpText);
            wstr = (LPWSTR)malloc((size + 1) << 1);
            n = MultiByteToWideChar(dxw.CodePage, 0, lpText, size, wstr, size);
            wstr[n] = L'\0'; // make tail !
        }
        if (lpCaption) {
            int size = lstrlenA(lpCaption);
            wcaption = (LPWSTR)malloc((size + 1) << 1);
            n = MultiByteToWideChar(dxw.CodePage, 0, lpCaption, size, wcaption, size);
            wcaption[n] = L'\0'; // make tail !
        }
        if(!pMessageBoxW) {
            HMODULE hinst = (*pLoadLibraryA)("user32.dll");
            pMessageBoxW = (MessageBoxW_Type)(*pGetProcAddress)(hinst, "MessageBoxW");
            if(!pMessageBoxW) return FALSE;
        }
        res = (*pMessageBoxW)(hWnd, wstr, wcaption, uType);
        if (wcaption) free((LPVOID)wcaption);
        if (wstr) free((LPVOID)wstr);
        return res;
    }
    res = (*pMessageBoxA)(hWnd, lpText, lpCaption, uType);
    return res;
}

HRESULT WINAPI extMessageBoxTimeoutA(HWND hWnd, LPCSTR lpText, LPCSTR lpCaption, UINT uType, WORD wLanguageId, DWORD dwMilliseconds) {
    HRESULT res;
    OutTraceDW("MessageBoxTimeoutA: hwnd=%#x text=\"%s\" caption=\"%s\" type=%d lang=%#x msec=%d\n", hWnd, lpText, lpCaption, uType, wLanguageId, dwMilliseconds);
    if(dxw.dwFlags9 & NODIALOGS) {
        switch (uType & 0x7) {
        case MB_OK:
            res = IDOK;
            break;
        case MB_OKCANCEL:
            res = IDCANCEL;
            break;
        case MB_ABORTRETRYIGNORE:
            res = IDIGNORE;
            break;
        case MB_YESNOCANCEL:
            res = IDCANCEL;
            break;
        case MB_YESNO:
            res = IDYES;
            break;
        case MB_RETRYCANCEL:
            res = IDCANCEL;
            break;
        case MB_CANCELTRYCONTINUE:
            res = IDCONTINUE;
            break;
        default:
            res = IDOK;
            break;
        }
        return res;
    }
    if(dxw.dwFlags11 & CUSTOMLOCALE) {
        LPWSTR wstr = NULL;
        LPWSTR wcaption = NULL;
        int n;
        if (lpText) {
            int size = lstrlenA(lpText);
            wstr = (LPWSTR)malloc((size + 1) << 1);
            //n = MultiByteToWideChar(CP_ACP, 0, lpString, size, wstr, size);
            n = MultiByteToWideChar(dxw.CodePage, 0, lpText, size, wstr, size);
            wstr[n] = L'\0'; // make tail !
        }
        if (lpCaption) {
            int size = lstrlenA(lpCaption);
            wcaption = (LPWSTR)malloc((size + 1) << 1);
            //n = MultiByteToWideChar(CP_ACP, 0, lpString, size, wstr, size);
            n = MultiByteToWideChar(dxw.CodePage, 0, lpCaption, size, wcaption, size);
            wcaption[n] = L'\0'; // make tail !
        }
        if(!pMessageBoxTimeoutW) {
            HMODULE hinst = (*pLoadLibraryA)("user32.dll");
            pMessageBoxTimeoutW = (MessageBoxTimeoutW_Type)(*pGetProcAddress)(hinst, "MessageBoxTimeoutW");
            if(!pMessageBoxTimeoutW) return FALSE;
        }
        res = (*pMessageBoxTimeoutW)(hWnd, wstr, wcaption, uType, dxw.Language, dwMilliseconds);
        if (wcaption) free((LPVOID)wcaption);
        if (wstr) free((LPVOID)wstr);
        return res;
    }
    res = (*pMessageBoxTimeoutA)(hWnd, lpText, lpCaption, uType, wLanguageId, dwMilliseconds);
    return res;
}

HRESULT WINAPI extMessageBoxTimeoutW(HWND hWnd, LPCWSTR lpText, LPCWSTR lpCaption, UINT uType, WORD wLanguageId, DWORD dwMilliseconds) {
    HRESULT res;
    OutTraceDW("MessageBoxTimeoutW: hwnd=%#x text=\"%ls\" caption=\"%ls\" type=%d lang=%#x msec=%d\n", hWnd, lpText, lpCaption, uType, wLanguageId, dwMilliseconds);
    if(dxw.dwFlags9 & NODIALOGS) return 1;
    res = (*pMessageBoxTimeoutW)(hWnd, lpText, lpCaption, uType, wLanguageId, dwMilliseconds);
    return res;
}

HDESK WINAPI extCreateDesktop( LPCTSTR lpszDesktop, LPCTSTR lpszDevice, DEVMODE *pDevmode, DWORD dwFlags, ACCESS_MASK dwDesiredAccess, LPSECURITY_ATTRIBUTES lpsa) {
    OutTraceDW("CreateDesktop: SUPPRESS flags=%#x access=%#x\n", dwFlags, dwDesiredAccess);
    return (HDESK)0xDEADBEEF; // fake handle
}

BOOL WINAPI extSwitchDesktop(HDESK hDesktop) {
    OutTraceDW("SwitchDesktop: SUPPRESS hDesktop=%#x\n", hDesktop);
    return TRUE;
}

HDESK WINAPI extOpenDesktop(LPTSTR lpszDesktop, DWORD dwFlags, BOOL fInherit, ACCESS_MASK dwDesiredAccess) {
    OutTraceDW("OpenDesktop: SUPPRESS flags=%#x access=%#x\n", dwFlags, dwDesiredAccess);
    return (HDESK)0xDEADBEEF; // fake handle
    //return (HDESK)NULL; // fake handle
}

BOOL WINAPI extCloseDesktop(HDESK hDesktop) {
    OutTraceDW("CloseDesktop: SUPPRESS hDesktop=%#x\n", hDesktop);
    return TRUE;
}

INT_PTR WINAPI extDialogBoxParamA(HINSTANCE hInstance, LPCTSTR lpTemplateName, HWND hWndParent, DLGPROC lpDialogFunc, LPARAM dwInitParam) {
    BOOL ret;
    char *fname = "DialogBoxParamA";
    OutTraceDW("%s: FullScreen=%#x TemplateName=\"%s\" WndParent=%#x\n",
               fname, dxw.IsFullScreen(), sTemplateName(lpTemplateName), hWndParent);
    // v2.04.78: "Sentinel Returns" creates a full-sized top game window, initially invisible, and then a launcher dialog through DialogBoxParamA
    // appended to parent 0. In order to make everything work in window mode, two actions are necessary
    // 1) append the dialog to the virtual desktop window instead of 0 to avoid the launcer to be overlapped by the game window
    // 2) handle the dialog as if it were not in fullscreen mode
    if(dxw.Windowize && (hWndParent == 0)) {
        hWndParent = dxw.GethWnd();
        OutTraceDW("%s: remap hWndParent 0->%#x\n", fname, hWndParent);
    }
    BOOL WasWindowized = dxw.Windowize;
    if (WasWindowized) {
        dxw.Windowize = FALSE;
        OutTraceDW("%s: set no windowize\n", fname);
    }
    // attempt to fix "Colonial Project 2" dialog. Doesn't work, but it could be ok.....
    //if(FullScreen && dxw.IsRealDesktop(hWndParent)){
    //	OutTraceDW("DialogBoxParamA: remap WndParent=%#x->%#x\n", hWndParent, dxw.GethWnd());
    //	hWndParent = dxw.GethWnd();
    //}
    InMainWinCreation++;
    if(dxw.dwFlags10 & STRETCHDIALOGS) {
        // this used by "Aaron vs. Ruth"
        HRSRC hRes;
        HGLOBAL hgRes;
        LPVOID lpRes;
        LPVOID lpScaledRes;
        DWORD dwSize;
        hRes = FindResource(NULL, lpTemplateName, RT_DIALOG);
        if(!hRes)
            OutTraceE("%s: FindResource ERROR err=%d at %d\n", fname, GetLastError(), __LINE__);
        hgRes = LoadResource(NULL, hRes);
        if(!hgRes)
            OutTraceE("%s: LoadResource ERROR err=%d at %d\n", fname, GetLastError(), __LINE__);
        lpRes = LockResource(hgRes);
        if(!lpRes)
            OutTraceE("%s: LockResource ERROR err=%d at %d\n", fname, GetLastError(), __LINE__);
        dwSize = SizeofResource(NULL, hRes);
        if(!dwSize)
            OutTraceE("%s: SizeofResource ERROR err=%d at %d\n", fname, GetLastError(), __LINE__);
        lpScaledRes = malloc(dwSize);
        memcpy(lpScaledRes, lpRes, dwSize);
        UnlockResource(lpRes);
#ifndef DXW_NOTRACES
        OutHexDW((LPBYTE)lpScaledRes, dwSize);
#endif //DXW_NOTRACES
        dxwStretchDialog(lpScaledRes, DXW_DIALOGFLAG_DUMP | DXW_DIALOGFLAG_STRETCH | (dxw.dwFlags1 & FIXTEXTOUT ? DXW_DIALOGFLAG_STRETCHFONT : 0));
        ret = (*pDialogBoxIndirectParamA)(hInstance, (LPCDLGTEMPLATE)lpScaledRes, hWndParent, lpDialogFunc, dwInitParam);
        free(lpScaledRes);
    } else
        ret = (*pDialogBoxParamA)(hInstance, lpTemplateName, hWndParent, lpDialogFunc, dwInitParam);
    InMainWinCreation--;
    dxw.Windowize = WasWindowized;
    if((ret == (INT_PTR)0) || (ret == (INT_PTR) - 1))
        OutTraceDW("%s: ERROR ret=%#x err=%d\n", fname, ret, GetLastError());
    else
        OutTraceDW("%s: ret=%#x\n", fname, ret);
    return ret;
}

BOOL extScrollWindow(HWND hWnd, int XAmount, int YAmount, const RECT *lpRect, const RECT *lpClipRect) {
    RECT Rect, ClipRect;
    BOOL res;
    ApiName("ScrollWindow");
#ifndef DXW_NOTRACES
    if(IsTraceSYS) {
        char sRect[81];
        char sClipRect[81];
        if(lpRect) sprintf(sRect, "(%d,%d)-(%d,%d)", lpRect->left, lpRect->top, lpRect->right, lpRect->bottom);
        else strcpy(sRect, "(NULL)");
        if(lpClipRect) sprintf(sClipRect, "(%d,%d)-(%d,%d)", lpClipRect->left, lpClipRect->top, lpClipRect->right, lpClipRect->bottom);
        else strcpy(sClipRect, "(NULL)");
        OutTrace("%s: hwnd=%#x amount=(%d,%d) rect=%s clip=%s\n", ApiRef, hWnd, XAmount, YAmount, sRect, sClipRect);
    }
#endif
    if(dxw.Windowize && dxw.IsFullScreen()) {
        dxw.MapClient(&XAmount, &YAmount);
        if(lpRect) {
            Rect = *lpRect;
            dxw.MapClient(&Rect);
            lpRect = &Rect;
        }
        if(lpClipRect) {
            ClipRect = *lpClipRect;
            dxw.MapClient(&ClipRect);
            lpClipRect = &ClipRect;
        }
    }
    res = (*pScrollWindow)(hWnd, XAmount, YAmount, lpRect, lpClipRect);
    IfTraceError();
    return res;
}

// commented out, too dangerous. Known side effects:
// 1) Recursion on HOT PATCH mode (or forever loop?)
// 2) blanked dialog boxes in Galapagos
// In any case, if useful somehow, it should not be hooked on GDImode != NONE condition
// P.s.so far, GetParent wrapping is useless and is eliminated.

#if 0
HWND WINAPI extGetParent(HWND hWnd) {
    // Beware: can cause recursion on HOT PATCH mode
    HWND ret;
    ret = (*pGetParent)(hWnd);
    OutDebugDW("GetParent: hwnd=%#x ret=%#x\n", hWnd, ret);
    if(dxw.IsFullScreen()) {
        if(ret == dxw.GethWnd()) {
            OutDebugDW("GetParent: setting desktop reached\n");
            ret = 0; // simulate reaching the desktop
        }
    }
    return ret;
}
#endif

BOOL WINAPI extInvalidateRgn(HWND hWnd, HRGN hRgn, BOOL bErase) {
    OutTraceDW("InvalidateRgn: hwnd=%#x hrgn=%#x erase=%#x\n", hWnd, hRgn, bErase);
    if(dxw.IsFullScreen()) {
        if (dxw.IsRealDesktop(hWnd) && bErase) return true;
    }
    return (*pInvalidateRgn)(hWnd, hRgn, bErase);
}

BOOL WINAPI extDrawIcon(HDC hdc, int X, int Y, HICON hIcon) {
    BOOL res;
    ApiName("DrawIcon");
    OutTraceDW("%s: hdcdest=%#x pos=(%d,%d) hicon=%#x\n", ApiRef, hdc, X, Y, hIcon);
    if(dxw.IsToRemap(hdc)) {
        switch(dxw.GDIEmulationMode) {
        case GDIMODE_STRETCHED:
            dxw.MapClient(&X, &Y);
            OutTraceDW("%s: fixed STRETCHED pos=(%d,%d)\n", ApiRef, X, Y);
            break;
        case GDIMODE_SHAREDDC:
            sdc.GetPrimaryDC(hdc);
            res = (*pDrawIcon)(sdc.GetHdc(),  X, Y, hIcon);
            sdc.PutPrimaryDC(hdc, TRUE);
            return res;
            break;
        default:
            break;
        }
    }
    res = (*pDrawIcon)(hdc, X, Y, hIcon);
    IfTraceError();
    return res;
}

// not working in HOT PATCH mode
BOOL WINAPI extDrawIconEx( HDC hdc, int xLeft, int yTop, HICON hIcon, int cxWidth, int cyWidth, UINT istepIfAniCur, HBRUSH hbrFlickerFreeDraw, UINT diFlags) {
    BOOL res;
    ApiName("DrawIconEx");
    OutTraceDW("%s: hdc=%#x pos=(%d,%d) hicon=%#x size=(%d,%d) istep=%#x flags=%#x\n",
               ApiRef, hdc, xLeft, yTop, hIcon, cxWidth, cyWidth, istepIfAniCur, diFlags);
    if(dxw.IsToRemap(hdc)) {
        switch(dxw.GDIEmulationMode) {
        case GDIMODE_STRETCHED:
            dxw.MapClient(&xLeft, &yTop, &cxWidth, &cyWidth);
            OutTraceDW("%s: fixed STRETCHED pos=(%d,%d) size=(%d,%d)\n", ApiRef, xLeft, yTop, cxWidth, cyWidth);
            break;
        case GDIMODE_SHAREDDC:
            sdc.GetPrimaryDC(hdc);
            res = (*pDrawIconEx)(sdc.GetHdc(), xLeft, yTop, hIcon, cxWidth, cyWidth, istepIfAniCur, hbrFlickerFreeDraw, diFlags);
            sdc.PutPrimaryDC(hdc, TRUE, xLeft, yTop, cxWidth, cyWidth);
            return res;
            break;
        default:
            break;
        }
    }
    res = (*pDrawIconEx)(hdc, xLeft, yTop, hIcon, cxWidth, cyWidth, istepIfAniCur, hbrFlickerFreeDraw, diFlags);
    IfTraceError();
    return res;
}

BOOL WINAPI extDrawCaption(HWND hwnd, HDC hdc, LPCRECT lprc, UINT uFlags) {
    BOOL res;
    ApiName("DrawCaption");
    OutTraceDW("%s: hwnd=%#x hdc=%#x rect=(%d,%d)-(%d,%d) flags=%#x\n",
               ApiRef, hwnd, hdc, lprc->left, lprc->top, lprc->right, lprc->bottom, uFlags);
    if(dxw.IsToRemap(hdc)) {
        switch(dxw.GDIEmulationMode) {
        case GDIMODE_STRETCHED:
            dxw.MapClient((LPRECT)lprc);
            OutTraceDW("%s: fixed STRETCHED rect=(%d,%d)-(%d,%d)\n",
                       ApiRef, lprc->left, lprc->top, lprc->right, lprc->bottom);
            break;
        case GDIMODE_SHAREDDC:
            sdc.GetPrimaryDC(hdc);
            res = (*pDrawCaption)(hwnd, sdc.GetHdc(), lprc, uFlags);
            sdc.PutPrimaryDC(hdc, TRUE, lprc->left, lprc->top, lprc->right, lprc->bottom);
            return res;
            break;
        default:
            break;
        }
    }
    res = (*pDrawCaption)(hwnd, hdc, lprc, uFlags);
    IfTraceError();
    return res;
}

BOOL WINAPI extPaintDesktop(HDC hdc) {
    BOOL res;
    ApiName("PaintDesktop");
    OutTraceDW("%s: hdc=%#x\n", ApiRef, hdc);
    if(dxw.IsToRemap(hdc)) {
        switch(dxw.GDIEmulationMode) {
        case GDIMODE_SHAREDDC:
            sdc.GetPrimaryDC(hdc);
            res = (*pPaintDesktop)(sdc.GetHdc());
            sdc.PutPrimaryDC(hdc, TRUE);
            return res;
            break;
        default:
            break;
        }
    }
    res = (*pPaintDesktop)(hdc);
    IfTraceError();
    return res;
}

#ifndef DXW_NOTRACES
char *ExplainMouseMoveFlags(DWORD c) {
    static char eb[256];
    unsigned int l;
    strcpy(eb, "MOUSEEVENTF_");
    if (c & MOUSEEVENTF_MOVE) strcat(eb, "MOVE+");
    if (c & MOUSEEVENTF_LEFTDOWN) strcat(eb, "LEFTDOWN+");
    if (c & MOUSEEVENTF_LEFTUP) strcat(eb, "LEFTUP+");
    if (c & MOUSEEVENTF_RIGHTDOWN) strcat(eb, "RIGHTDOWN+");
    if (c & MOUSEEVENTF_RIGHTUP) strcat(eb, "RIGHTUP+");
    if (c & MOUSEEVENTF_MIDDLEDOWN) strcat(eb, "MIDDLEDOWN+");
    if (c & MOUSEEVENTF_MIDDLEUP) strcat(eb, "MIDDLEUP+");
    if (c & MOUSEEVENTF_XDOWN) strcat(eb, "XDOWN+");
    if (c & MOUSEEVENTF_XUP) strcat(eb, "XUP+");
    if (c & MOUSEEVENTF_WHEEL) strcat(eb, "WHEEL+");
    if (c & MOUSEEVENTF_HWHEEL) strcat(eb, "HWHEEL+");
    if (c & MOUSEEVENTF_ABSOLUTE) strcat(eb, "ABSOLUTE+");
    l = strlen(eb);
    if (l > strlen("MOUSEEVENTF_")) eb[l - 1] = 0; // delete last '+' if any
    else eb[0] = 0;
    return(eb);
}
#endif

VOID WINAPI extmouse_event(DWORD dwFlags, DWORD dx, DWORD dy, DWORD dwData, ULONG_PTR dwExtraInfo) {
    ApiName("mouse_event");
    OutTraceC("%s: flags=%#x(%s) xy=(%d,%d) data=%#x, extrainfo=%lx\n",
              ApiRef, dwFlags, ExplainMouseMoveFlags(dwFlags), dx, dy, dwData, dwExtraInfo);
    if(dxw.dwFlags9 & NOMOUSEEVENTS) {
        OutTraceDW("%s: SUPPRESS mouse event\n", ApiRef);
        return;
    }
    if((dwFlags & MOUSEEVENTF_MOVE) && (dxw.dwFlags2 & KEEPCURSORFIXED)) {
        OutTraceDW("%s: SUPPRESS mouse move\n", ApiRef);
        return;
    }
    if(dxw.Windowize && (dxw.dwFlags1 & MODIFYMOUSE)) {
        POINT cursor;
        cursor.x = dx;
        cursor.y = dy;
        if(dwFlags & MOUSEEVENTF_ABSOLUTE) {
            // ???? untested ......
            //dxw.MapClient((int *)&dx, (int *)&dy);
            cursor = dxw.FixCursorPos(cursor);
        } else
            dxw.MapClient(&cursor);
        OutTraceDW("%s: FIX MOUSEEVENTF_ABSOLUTE (%d,%d) -> (%d,%d)\n", ApiRef, dx, dy, cursor.x, cursor.y);
        dx = cursor.x;
        dy = cursor.y;
    }
    return (*pmouse_event)(dwFlags, dx, dy, dwData, dwExtraInfo);
}

BOOL WINAPI extShowScrollBar(HWND hWnd, int wBar, BOOL bShow) {
    BOOL res;
    ApiName("ShowScrollBar");
    OutTraceDW("%s: hwnd=%#x wBar=%#x show=%#x\n", ApiRef, hWnd, wBar, bShow);
    if(dxw.Windowize && dxw.IsRealDesktop(hWnd)) hWnd = dxw.GethWnd();
    res = (*pShowScrollBar)(hWnd, wBar, bShow);
    IfTraceError();
    return res;
}

BOOL WINAPI extDrawMenuBar(HWND hWnd) {
    BOOL res;
    ApiName("DrawMenuBar");
    OutTraceDW("%s: hwnd=%#x\n", ApiRef, hWnd);
    if(dxw.Windowize && dxw.IsRealDesktop(hWnd)) hWnd = dxw.GethWnd();
    res = (*pDrawMenuBar)(hWnd);
    IfTraceError();
    return res;
}

#ifdef TRACESYSCALLS
BOOL WINAPI extTranslateMessage(MSG *pMsg) {
    BOOL ret;
    ApiName("TranslateMessage");
    OutTraceSYS("%s: type=%#x pos=(%d,%d)\n", ApiRef, pMsg->message, pMsg->pt.x, pMsg->pt.y);
    ret = (*pTranslateMessage)(pMsg);
    OutTraceSYS("%s: ret=%#x(%s)\n", ApiRef, ret, ret ? "translated" : "not translated");
    return ret;
}
#endif

BOOL WINAPI extEnumDisplayDevicesA(LPCSTR lpDevice, DWORD iDevNum, PDISPLAY_DEVICE lpDisplayDevice, DWORD dwFlags) {
    BOOL ret;
    ApiName("EnumDisplayDevicesA");
    OutTraceDW("%s: device=%s devnum=%i flags=%#x\n", ApiRef, lpDevice, iDevNum, dwFlags);
    if((dxw.dwFlags2 & HIDEMULTIMONITOR) && (iDevNum > 0)) {
        OutTraceDW("%s: HIDEMULTIMONITOR devnum=%i\n", ApiRef, iDevNum);
        return FALSE;
    }
    ret = (*pEnumDisplayDevicesA)(lpDevice, iDevNum, lpDisplayDevice, dwFlags);
    if(ret) {
        OutTraceDW("%s: cb=%#x devname=%s devstring=%s stateflags=%#x\n",
                   ApiRef, lpDisplayDevice->cb, lpDisplayDevice->DeviceName, lpDisplayDevice->DeviceString, lpDisplayDevice->StateFlags);
    } else
        OutTraceE("%s: ERROR err=%d\n", ApiRef, GetLastError());
    return ret;
}

BOOL WINAPI extEnumDisplayDevicesW(LPCWSTR lpDevice, DWORD iDevNum, PDISPLAY_DEVICEW lpDisplayDevice, DWORD dwFlags) {
    BOOL ret;
    ApiName("EnumDisplayDevicesW");
    OutTraceDW("%s: device=%ls devnum=%i flags=%#x\n", ApiRef, lpDevice, iDevNum, dwFlags);
    if((dxw.dwFlags2 & HIDEMULTIMONITOR) && (iDevNum > 0)) {
        OutTraceDW("%s: HIDEMULTIMONITOR devnum=%i\n", ApiRef, iDevNum);
        return FALSE;
    }
    ret = (*pEnumDisplayDevicesW)(lpDevice, iDevNum, lpDisplayDevice, dwFlags);
    if(ret) {
        OutTraceDW("%s: cb=%#x devname=%ls devstring=%ls stateflags=%#x\n",
                   ApiRef, lpDisplayDevice->cb, lpDisplayDevice->DeviceName, lpDisplayDevice->DeviceString, lpDisplayDevice->StateFlags);
    } else
        OutTraceE("%s ERROR: err=%d\n", ApiRef, GetLastError());
    return ret;
}

INT_PTR WINAPI extDialogBoxIndirectParamA(HINSTANCE hInstance, LPCDLGTEMPLATE hDialogTemplate, HWND hWndParent, DLGPROC lpDialogFunc, LPARAM dwInitParam) {
    ApiName("DialogBoxIndirectParamA");
    OutTraceDW("%s: hInstance=%#x pos=(%d,%d) size=(%dx%d) hWndParent=%#x, lpDialogFunc=%#x dwInitParam=%#x\n",
               ApiRef, hInstance,
               hDialogTemplate->x, hDialogTemplate->y, hDialogTemplate->cx, hDialogTemplate->cy,
               hWndParent, lpDialogFunc, dwInitParam);
    return (*pDialogBoxIndirectParamA)(hInstance, hDialogTemplate, hWndParent, lpDialogFunc, dwInitParam);
}

BOOL WINAPI extEnumWindows(WNDENUMPROC lpEnumFunc, LPARAM lParam) {
    ApiName("EnumWindows");
    OutTraceDW("%s\n", ApiRef);
    if(dxw.dwFlags8 & WININSULATION) {
        OutTraceDW("%s: BYPASS\n", ApiRef);
        // if(dxw.GethWnd()) (*lpEnumFunc)(dxw.GethWnd(), lParam); // to do ....
        return TRUE;
    }
    return (*pEnumWindows)(lpEnumFunc, lParam);
}

static void RedirectCoordinates(char *api, LPRECT lpRect) {
    WINDOWPOS wp;
    dxw.CalculateWindowPos(NULL, dxw.GetScreenWidth(), dxw.GetScreenHeight(), &wp);
    lpRect->left = wp.x;
    lpRect->right = wp.x + wp.cx;
    lpRect->top = wp.y;
    lpRect->bottom = wp.y + wp.cy;
    OutTraceDW("%s: FIX rect=(%d,%d)-(%d,%d)\n",
               api, lpRect->left, lpRect->top, lpRect->right, lpRect->bottom);
}

BOOL WINAPI extAdjustWindowRect(LPRECT lpRect, DWORD dwStyle, BOOL bMenu) {
    BOOL ret;
    ApiName("AdjustWindowRect");
    OutTraceDW("%s: IN rect=(%d,%d)-(%d,%d) style=%#x(%s) menu=%#x\n",
               ApiRef, lpRect->left, lpRect->top, lpRect->right, lpRect->bottom,
               dwStyle, ExplainStyle(dwStyle), bMenu);
    if(dxw.Windowize && (dxw.dwFlags8 & FIXADJUSTWINRECT)) RedirectCoordinates(ApiRef, lpRect);
    ret = pAdjustWindowRect(lpRect, dwStyle, bMenu);
    if(ret) {
        OutTraceDW("%s: OUT rect=(%d,%d)-(%d,%d)\n",
                   ApiRef, lpRect->left, lpRect->top, lpRect->right, lpRect->bottom);
    } else
        OutTraceE("%s: ERROR err=%d\n", ApiRef, GetLastError());
    return ret;
}

BOOL WINAPI extAdjustWindowRectEx(LPRECT lpRect, DWORD dwStyle, BOOL bMenu, DWORD dwExStyle) {
    BOOL ret;
    ApiName("AdjustWindowRectEx");
    OutTraceDW("%s: IN rect=(%d,%d)-(%d,%d) style=%#x(%s) menu=%#x exstyle=%#x(%s)\n",
               ApiRef, lpRect->left, lpRect->top, lpRect->right, lpRect->bottom,
               dwStyle, ExplainStyle(dwStyle), bMenu, dwExStyle, ExplainExStyle(dwExStyle));
    if(dxw.Windowize && (dxw.dwFlags8 & FIXADJUSTWINRECT)) RedirectCoordinates(ApiRef, lpRect);
    ret = pAdjustWindowRectEx(lpRect, dwStyle, bMenu, dwExStyle);
    if(ret) {
        OutTraceDW("%s: OUT rect=(%d,%d)-(%d,%d)\n",
                   ApiRef, lpRect->left, lpRect->top, lpRect->right, lpRect->bottom);
    } else
        OutTraceE("%s ERROR: err=%d\n", ApiRef, GetLastError());
    return ret;
}

BOOL WINAPI extValidateRgn(HWND hwnd, HRGN hrgn) {
    BOOL ret;
    ApiName("ValidateRgn");
    OutTraceDW("%s: hwnd=%#x hrgn=%#x\n", ApiRef, hwnd, hrgn);
    ret = (*pValidateRgn)(hwnd, hrgn);
    return ret;
}

UINT WINAPI extSendInput(UINT nInputs, LPINPUT pInputs, int cbSize) {
    UINT ret;
    ApiName("SendInput");
    OutTraceDW("%s: nInputs=%d cdSize=%d\n", ApiRef, nInputs, cbSize);
#ifndef DXW_NOTRACES
    if(IsDebugIN) {
        for(UINT i = 0; i < nInputs; i++) {
            PINPUT pi = &pInputs[i];
            switch(pi->type) {
            case INPUT_MOUSE:
                OutTrace("%s: input=%d [Mouse:(%d,%d) data=%#x flags=%#x]\n", ApiRef, i, pi->mi.dx, pi->mi.dy, pi->mi.mouseData, pi->mi.dwFlags);
                break;
            case INPUT_KEYBOARD:
                OutTrace("%s: input=%d [Keybd:scan=%#x vk=%#x flags=%#x]\n", ApiRef, pi->ki.wScan, pi->ki.wVk, pi->ki.dwFlags);
                break;
            case INPUT_HARDWARE:
                OutTrace("%s: input=%d [Hardw:param h=%#x l=%#x]\n", ApiRef, pi->hi.wParamH, pi->hi.wParamL);
                break;
            default:
                OutTrace("%s: input=%d [Ubknown type=%#x]\n", ApiRef, pi->type);
                break;
            }
        }
    }
#endif
    if(dxw.IsFullScreen() & dxw.Windowize) {
        for(UINT i = 0; i < nInputs; i++) {
            PINPUT pi = &pInputs[i];
            if(pi->type == INPUT_MOUSE) {
                POINT pt;
                pt.x = pi->mi.dx;
                pt.y = pi->mi.dy;
                dxw.MapWindow(&pt);
                OutTraceDW("%s: fixed mouse pt=(%d,%d) -> (%d,%d)\n", ApiRef, pi->mi.dx, pi->mi.dy, pt.x, pt.y);
                pi->mi.dx = pt.x;
                pi->mi.dy = pt.y;
            }
        }
    }
    ret = (*pSendInput)(nInputs, pInputs, cbSize);
    if(!ret)
        OutTraceE("%s: ERROR err=%d\n", ApiRef, GetLastError());
    return ret;
}

#ifdef TRACESYSCALLS
HCURSOR WINAPI extSetCursor(HCURSOR hCursor) {
    HCURSOR ret;
    ApiName("SetCursor");
    ret = (*pSetCursor)(hCursor);
    OutDebugSYS("%s: Cursor=%#x, ret=%#x\n", ApiRef, hCursor, ret);
    return ret;
}

HWND WINAPI extFindWindowA(LPCTSTR lpClassName, LPCTSTR lpWindowName) {
    HWND ret;
    ApiName("FindWindowA");
    OutTraceSYS("%s: cname=%s wname=%s\n", ApiRef,
                lpClassName ? lpClassName : "(NULL)",
                lpWindowName ? lpWindowName : "(NULL)");
    ret = (*pFindWindowA)(lpClassName, lpWindowName);
    OutTraceSYS("%s: ret=%#x\n", ApiRef, ret);
    return ret;
}
#endif TRACESYSCALLS
// To do:
// GrayStringA
// GrayStringW

#ifndef DXW_NOTRACES
static char *sRawDevFlags(DWORD c) {
    static char eb[257];
    unsigned int l;
    strcpy(eb, "RIDEV_");
    if (c & RIDEV_REMOVE) strcat(eb, "REMOVE+");
    if (c & RIDEV_EXCLUDE) strcat(eb, "EXCLUDE+");
    if (c & RIDEV_PAGEONLY) strcat(eb, "PAGEONLY+");
    if (c & RIDEV_NOLEGACY) strcat(eb, "NOLEGACY+");
    if (c & RIDEV_INPUTSINK) strcat(eb, "INPUTSINK+");
    if (c & RIDEV_CAPTUREMOUSE) strcat(eb, "CAPTUREMOUSE+");
    if (c & RIDEV_NOHOTKEYS) strcat(eb, "NOHOTKEYS+");
    if (c & RIDEV_APPKEYS) strcat(eb, "APPKEYS+");
    if (c & RIDEV_EXINPUTSINK) strcat(eb, "EXINPUTSINK+");
    if (c & RIDEV_DEVNOTIFY) strcat(eb, "DEVNOTIFY+");
    l = strlen(eb);
    if (l > strlen("RIDEV_")) eb[l - 1] = 0; // delete last '+' if any
    else eb[0] = 0;
    return(eb);
}

static char *sRIMType(DWORD type) {
#define RIM_TYPEBAD (RIM_TYPEHID+1)
    if(type > RIM_TYPEBAD) type = RIM_TYPEBAD;
    static char *sRIMTypes[] = {"MOUSE", "KEYBOARD", "HID", "invalid"};
    return sRIMTypes[type];
}

static char *sRawMouseFlags(USHORT c) {
    static char eb[81];
    unsigned int l;
    strcpy(eb, "MOUSE_");
    if (c & MOUSE_MOVE_ABSOLUTE) strcat(eb, "MOVE_ABSOLUTE+");
    else strcat(eb, "MOVE_RELATIVE+");
    if (c & MOUSE_VIRTUAL_DESKTOP) strcat(eb, "VIRTUAL_DESKTOP+");
    if (c & MOUSE_ATTRIBUTES_CHANGED) strcat(eb, "ATTRIBUTES_CHANGED+");
    if (c & MOUSE_MOVE_NOCOALESCE) strcat(eb, "MOVE_NOCOALESCE+");
    l = strlen(eb);
    if (l > strlen("MOUSE_")) eb[l - 1] = 0; // delete last '+' if any
    else eb[0] = 0;
    return(eb);
}
#endif // DXW_NOTRACES

BOOL WINAPI extRegisterRawInputDevices(PCRAWINPUTDEVICE pRawInputDevices, UINT uiNumDevices, UINT cbSize) {
    BOOL res;
    ApiName("RegisterRawInputDevices");
#ifndef DXW_NOTRACES
    OutTraceC("%s: numdevs=%d size=%d\n", ApiRef, uiNumDevices, cbSize);
    for(UINT i = 0; i < uiNumDevices; i++) {
        OutTraceC("> dev[%d]: upage=%#x usage=%#x flags=%#x(%s) hwnd=%#x\n", i,
                  pRawInputDevices[i].usUsagePage,
                  pRawInputDevices[i].usUsage,
                  pRawInputDevices[i].dwFlags, sRawDevFlags(pRawInputDevices[i].dwFlags),
                  pRawInputDevices[i].hwndTarget
                 );
    }
#endif // DXW_NOTRACES
    if(dxw.Windowize) {
        for(UINT i = 0; i < uiNumDevices; i++) (DWORD)(pRawInputDevices[i].dwFlags) &= ~RIDEV_CAPTUREMOUSE;
    }
    res = (*pRegisterRawInputDevices)(pRawInputDevices, uiNumDevices, cbSize);
    IfTraceError();
    return res;
}

UINT WINAPI extGetRawInputData(HRAWINPUT hRawInput, UINT uiCommand, LPVOID pData, PUINT pcbSize, UINT cbSizeHeader) {
    UINT ret;
    ApiName("GetRawInputData");
    OutTraceC("%s: hri=%#x cmd=%#x(%s) data=%#x sizehdr=%#x\n",
              ApiRef, hRawInput, uiCommand,
              (uiCommand == RID_INPUT) ? "INPUT" : (uiCommand == RID_HEADER) ? "HEADER" : "unknown",
              pData, cbSizeHeader);
    ret = (*pGetRawInputData)(hRawInput, uiCommand, pData, pcbSize, cbSizeHeader);
    if(ret == -1) {
        OutTraceE("%s: ERROR err=%d\n", ApiRef, GetLastError());
        return ret;
    }
#ifndef DXW_NOTRACES
    if(IsTraceC) {
        OutTrace("%s: size=%d\n", ApiRef, *pcbSize);
        if(pData) {
            RAWINPUTHEADER *phdr;
            RAWMOUSE *pmouse;
            phdr = (RAWINPUTHEADER *)pData;
            // this info retrieved in any case
            OutTrace("%s: size=%d type=%#x(%s) hdev=%#x wparam=%#x\n",
                     ApiRef,
                     phdr->dwSize,
                     phdr->dwType, sRIMType(phdr->dwType),
                     phdr->hDevice,
                     phdr->wParam);
            if(uiCommand == RID_INPUT) {
                // this info retrieved only with RID_INPUT command
                phdr = (RAWINPUTHEADER *)pData;
                OutTrace("%s: size=%d type=%#x(%s) hdev=%#x wparam=%#x\n",
                         ApiRef,
                         phdr->dwSize,
                         phdr->dwType, sRIMType(phdr->dwType),
                         phdr->hDevice,
                         phdr->wParam);
                switch (phdr->dwType) {
                case RIM_TYPEMOUSE:
                    pmouse = &((LPRAWINPUT)pData)->data.mouse;
                    OutTrace("%s: MOUSE flags=%#x(%s) xy=(%d,%d) bflags=%#x bdata=%#x extra=%#x\n",
                             ApiRef,
                             pmouse->usFlags, sRawMouseFlags(pmouse->usFlags),
                             pmouse->lLastX, pmouse->lLastY,
                             pmouse->usButtonFlags, pmouse->usButtonData, pmouse->ulExtraInformation);
                    break;
                case RIM_TYPEKEYBOARD:
                    OutTrace("%s: KEYBOARD\n", ApiRef);
                    break;
                case RIM_TYPEHID:
                    OutTrace("%s: HID\n", ApiRef);
                    break;
                }
            }
        }
    }
#endif // DXW_NOTRACES
    // mouse coordinate scaling
    if((dxw.dwFlags10 & FIXMOUSERAWINPUT) && pData && (uiCommand == RID_INPUT)) {
        RAWINPUTHEADER *phdr;
        RAWMOUSE *pmouse;
        phdr = (RAWINPUTHEADER *)pData;
        if(phdr->dwType == RIM_TYPEMOUSE) {
            pmouse = &((LPRAWINPUT)pData)->data.mouse;
            POINT point, prev;
            point.x = pmouse->lLastX;
            point.y = pmouse->lLastY;
            prev = point;
            if(pmouse->usFlags & MOUSE_MOVE_ABSOLUTE)
                dxw.UnmapClient(&point);
            else {
                // todo: reminders handling can't be done with a single static variable, since
                // you're supposed to be here because there are more than 1 single mouse device!
                dxw.ScaleRelMouse(ApiRef, &point);
            }
            pmouse->lLastX = point.x;
            pmouse->lLastY = point.y;
            OutTraceC("%s: FIXED pos=(%d,%d)->(%d,%d)\n", ApiRef, prev.x, prev.y, point.x, point.y);
        }
    }
    return ret;
}

UINT WINAPI extGetRawInputBuffer(PRAWINPUT pData, PUINT pcbSize, UINT cbSizeHeader) {
    UINT ret;
    ApiName("GetRawInputBuffer");
    OutTraceC("%s: data=%#x psize=%#x sizehdr=%#x\n", ApiRef, pData, pcbSize, cbSizeHeader);
    ret = (*pGetRawInputBuffer)(pData, pcbSize, cbSizeHeader);
#ifndef DXW_NOTRACES
    if(ret)
        OutTraceE("%s: ERROR err=%d\n", ApiRef, GetLastError());
    else
        OutTraceC("%s: size=%d\n", ApiRef, *pcbSize);
#endif // DXW_NOTRACES
    return ret;
}

static char *sRawInfoCmd(UINT c) {
    char *p = "invalid";
    switch(c) {
    case RIDI_DEVICENAME:
        p = "DEVICENAME";
        break;
    case RIDI_PREPARSEDDATA:
        p = "PREPARSEDDATA";
        break;
    case RIDI_DEVICEINFO:
        p = "DEVICEINFO";
        break;
    }
    return p;
}

static UINT WINAPI extGetRawInputDeviceInfo(GetRawInputDeviceInfo_Type pGetRawInputDeviceInfo, BOOL isWide,
        HANDLE hDevice, UINT uiCommand, LPVOID pData, PUINT pcbSize) {
    UINT ret;
#ifndef DXW_NOTRACES
    char *ApiName = isWide ? "GetRawInputDeviceInfoW" : "GetRawInputDeviceInfoA";
    if(pData)
        OutTraceSYS("%s: hdev=%#x cmd=%#x(%s) pdata=%#x size=%d\n", ApiName, hDevice, uiCommand, sRawInfoCmd(uiCommand), pData, *pcbSize);
    else
        OutTraceSYS("%s: hdev=%#x cmd=%#x(%s) pdata=NULL\n", ApiName, hDevice, uiCommand, sRawInfoCmd(uiCommand));
#endif // DXW_NOTRACES
    ret = (*pGetRawInputDeviceInfo)(hDevice, uiCommand, pData, pcbSize);
    if(ret == -1) {
        OutTraceE("%s: ERROR err=%d\n", ApiName, GetLastError());
        return ret;
    }
#ifndef DXW_NOTRACES
    OutTraceSYS("%s: ret=%d size=%d\n", ApiName, ret, *pcbSize);
    if(pData && IsTraceSYS) {
        RID_DEVICE_INFO *info;
        switch(uiCommand) {
        case RIDI_DEVICENAME:
            if(isWide)
                OutTrace("%s: NAME name=%ls\n", ApiName, (PTCHAR)pData);
            else
                OutTrace("%s: NAME name=%s\n", ApiName, (PCHAR)pData);
            break;
        case RIDI_DEVICEINFO:
            info = (RID_DEVICE_INFO *)pData;
            RID_DEVICE_INFO_HID *pHid;
            RID_DEVICE_INFO_KEYBOARD *pKbd;
            RID_DEVICE_INFO_MOUSE *pMouse;
            OutTraceSYS("%s: INFO size=%d type=%#x(%s)\n", ApiName, info->cbSize, info->dwType, sRIMType(info->dwType));
            switch (info->dwType) {
            case RIM_TYPEMOUSE:
                pMouse = &(info->mouse);
                OutTrace("> id=%#x\n", pMouse->dwId);
                OutTrace("> nbuttons=%d\n", pMouse->dwNumberOfButtons);
                OutTrace("> samplerate=%d\n", pMouse->dwSampleRate);
                OutTrace("> haswheel=%s\n", pMouse->fHasHorizontalWheel ? "yes" : "no");
                break;
            case RIM_TYPEKEYBOARD:
                pKbd = &(info->keyboard);
                OutTrace("> type=%#x\n", pKbd->dwType);
                OutTrace("> subtype=%#x\n", pKbd->dwSubType);
                OutTrace("> mode=%#x\n", pKbd->dwKeyboardMode);
                OutTrace("> fkeys=%d\n", pKbd->dwNumberOfFunctionKeys);
                OutTrace("> indicators=%d\n", pKbd->dwNumberOfIndicators);
                OutTrace("> ktotal=%d\n", pKbd->dwNumberOfKeysTotal);
                break;
            case RIM_TYPEHID:
                pHid = &(info->hid);
                OutTrace("> vendor=%#x\n", pHid->dwVendorId);
                OutTrace("> product=%#x\n", pHid->dwProductId);
                OutTrace("> version=%#x\n", pHid->dwVersionNumber);
                OutTrace("> usagepage=%#x\n", pHid->usUsagePage);
                OutTrace("> usage=%#x\n", pHid->usUsage);
                break;
            }
            break;
        }
    }
#endif // DXW_NOTRACES
    return ret;
}

UINT WINAPI extGetRawInputDeviceInfoA(HANDLE hDevice, UINT uiCommand, LPVOID pData, PUINT pcbSize) {
    return extGetRawInputDeviceInfo(pGetRawInputDeviceInfoA, FALSE, hDevice, uiCommand, pData, pcbSize);
}
UINT WINAPI extGetRawInputDeviceInfoW(HANDLE hDevice, UINT uiCommand, LPVOID pData, PUINT pcbSize) {
    return extGetRawInputDeviceInfo(pGetRawInputDeviceInfoW, TRUE, hDevice, uiCommand, pData, pcbSize);
}

UINT WINAPI extGetRawInputDeviceList(PRAWINPUTDEVICELIST pRawInputDeviceList, PUINT puiNumDevices, UINT cbSize) {
    UINT ret;
    ApiName("GetRawInputDeviceList");
    if(pRawInputDeviceList == NULL)
        OutTraceSYS("%s: list=NULL cbSize=%d\n", ApiRef, cbSize);
    else
        OutTraceSYS("%s: list=%#x puiNumDevices=%d cbSize=%d\n", ApiRef, pRawInputDeviceList, *puiNumDevices, cbSize);
    ret = (*pGetRawInputDeviceList)(pRawInputDeviceList, puiNumDevices, cbSize);
    if(ret == -1) {
        OutTraceE("%s: ERROR err=%d\n", ApiRef, GetLastError());
        return ret;
    }
    OutTraceC("%s: ret=%d numdevices=%d size=%d\n", ApiRef, ret, *puiNumDevices, cbSize);
    for(UINT i = 0; i < ret; i++) {
        OutTraceSYS("> dev[%d]: hid=%#x type=%#x(%s)\n", i,
                    pRawInputDeviceList[i].hDevice,
                    pRawInputDeviceList[i].dwType, sRIMType(pRawInputDeviceList[i].dwType));
    }
    return ret;
}

#ifdef TRACESYSCALLS
/*
    ScrollWindowEx
*/

int WINAPI extSetScrollInfo(HWND hwnd, int nBar, LPSCROLLINFO lpsi, BOOL redraw) {
    int ret;
    ApiName("SetScrollInfo");
    OutTraceSYS("%s: hwnd=%#x nbar=%#x redraw=%#x\n", ApiRef, hwnd, nBar, redraw);
    OutTraceSYS("%s: lpsi={size=%d mask=%#x scroll(min,max)=(%d,%d) page=%d pos=%d trackpos=%d}\n",
                ApiRef,
                lpsi->cbSize,
                lpsi->fMask,
                lpsi->nMin, lpsi->nMax,
                lpsi->nPage,
                lpsi->nPos,
                lpsi->nTrackPos);
    ret = (*pSetScrollInfo)(hwnd, nBar, lpsi, redraw);
    OutTraceSYS("%s: ret(current pos)=%d\n", ApiRef, ret);
    return ret;
}

BOOL WINAPI extGetScrollInfo(HWND hwnd, int nBar, LPSCROLLINFO lpsi) {
    BOOL ret;
    ApiName("GetScrollInfo");
    OutTraceSYS("%s: hwnd=%#x nbar=%#x\n", ApiRef, hwnd, nBar);
    ret = (*pGetScrollInfo)(hwnd, nBar, lpsi);
    if(ret) {
        OutTraceSYS("%s: lpsi={size=%d mask=%#x scroll(min,max)=(%d,%d) page=%d pos=%d trackpos=%d}\n",
                    ApiRef,
                    lpsi->cbSize,
                    lpsi->fMask,
                    lpsi->nMin, lpsi->nMax,
                    lpsi->nPage,
                    lpsi->nPos,
                    lpsi->nTrackPos);
    } else
        OutTraceE("%s: ERROR err=%d\n", ApiRef, GetLastError());
    return ret;
}

int WINAPI extSetScrollPos(HWND hwnd, int nBar, int nPos, BOOL redraw) {
    int ret;
    ApiName("SetScrollPos");
    OutTraceSYS("%s: hwnd=%#x nbar=%#x pos=%d redraw=%#x\n", ApiRef, hwnd, nBar, redraw);
    ret = (*pSetScrollPos)(hwnd, nBar, nPos, redraw);
    OutTraceSYS("%s: ret(current pos)=%d\n", ApiRef, ret);
    return ret;
}

BOOL WINAPI extGetScrollPos(HWND hwnd, int nBar) {
    BOOL ret;
    ApiName("GetScrollPos");
    OutTraceSYS("%s: hwnd=%#x nbar=%#x\n", ApiRef, hwnd, nBar);
    ret = (*pGetScrollPos)(hwnd, nBar);
    OutTraceSYS("%s: pos=%d\n", ApiRef, ret);
    return ret;
}

BOOL WINAPI extGetScrollRange(HWND hwnd, int nBar, LPINT lpMinPos, LPINT lpMaxPos) {
    BOOL ret;
    ApiName("GetScrollRange");
    OutTraceSYS("%s: hwnd=%#x nbar=%#x\n", ApiRef, hwnd, nBar);
    ret = (*pGetScrollRange)(hwnd, nBar, lpMinPos, lpMaxPos);
    if(ret)
        OutTraceSYS("%s: range(min,max)=(%d,%d)\n", ApiRef, *lpMinPos, *lpMaxPos);
    else
        OutTraceE("%s: ERROR err=%d\n", ApiRef, GetLastError());
    return ret;
}

BOOL WINAPI extSetScrollRange(HWND hwnd, int nBar, int nMinPos, int nMaxPos, BOOL bRedraw) {
    BOOL ret;
    ApiName("SetScrollRange");
    OutTraceSYS("%s: hwnd=%#x nbar=%#x range(min,max)=(%d,%d) redraw=%#x\n", ApiRef, hwnd, nBar, nMinPos, nMaxPos, bRedraw);
    ret = (*pSetScrollRange)(hwnd, nBar, nMinPos, nMaxPos, bRedraw);
    if(ret)
        OutTraceSYS("%s: ok\n", ApiRef);
    else
        OutTraceE("%s: ERROR err=%d\n", ApiRef, GetLastError());
    return ret;
}

BOOL WINAPI extIsDialogMessageA(HWND hDlg, LPMSG lpMsg) {
    BOOL ret;
    ApiName("IsDialogMessageA");
#ifndef DXW_NOTRACES
    OutTrace("%s: hdlg=%#x lpmsg=%#x\n", ApiRef, hDlg, lpMsg);
    ExplainMsg(ApiRef, lpMsg->hwnd, lpMsg->message, lpMsg->wParam, lpMsg->lParam);
#endif // DXW_NOTRACES
    ret = (*pIsDialogMessageA)(hDlg, lpMsg);
    return ret;
}
#endif // TRACESYSCALLS

#define GET_X_PARAM(v) ((short)((DWORD)v & 0xFFFF))
#define GET_Y_PARAM(v) ((short)(((DWORD)v & 0xFFFF0000) >> 16))
#define MAKELPOINT(p)	((DWORD)(((short)p.x) | ((DWORD)(short)p.y << 16)))

DWORD WINAPI extGetMessagePos(void) {
    DWORD ret;
    ApiName("GetMessagePos");
    ret = (*pGetMessagePos)();
    OutTraceC("%s: pos=(%d,%d)\n", ApiRef, GET_X_PARAM(ret), GET_Y_PARAM(ret));
    if(dxw.dwFlags1 & MODIFYMOUSE) {
        POINT pt;
        pt.x = GET_X_PARAM(ret);
        pt.y = GET_Y_PARAM(ret);
        dxw.UnmapWindow(&pt);
        ret = MAKELPOINT(pt);
        OutTraceC("%s: FIXED pos=(%d,%d)\n", ApiRef, GET_X_PARAM(ret), GET_Y_PARAM(ret));
    }
    GetHookInfo()->MessageX = GET_X_PARAM(ret);
    GetHookInfo()->MessageY = GET_Y_PARAM(ret);
    return ret;
}

#ifdef TRACEWAITOBJECTS
#ifndef DXW_NOTRACES
char *ExplainWakeMask(DWORD c) {
    static char eb[256];
    unsigned int l;
    strcpy(eb, "QS_");
    if (c & QS_KEY) strcat(eb, "KEY+");
    if (c & QS_MOUSEMOVE) strcat(eb, "MOUSEMOVE+");
    if (c & QS_MOUSEBUTTON) strcat(eb, "MOUSEBUTTON+");
    if (c & QS_POSTMESSAGE) strcat(eb, "POSTMESSAGE+");
    if (c & QS_TIMER) strcat(eb, "TIMER+");
    if (c & QS_PAINT) strcat(eb, "PAINT+");
    if (c & QS_SENDMESSAGE) strcat(eb, "SENDMESSAGE+");
    if (c & QS_HOTKEY) strcat(eb, "HOTKEY+");
    if (c & QS_ALLPOSTMESSAGE) strcat(eb, "ALLPOSTMESSAGE+");
    if (c & QS_RAWINPUT) strcat(eb, "RAWINPUT+");
    l = strlen(eb);
    if (l > strlen("QS_")) eb[l - 1] = 0; // delete last '+' if any
    else eb[0] = 0;
    return(eb);
}

char *ExplainMsgWaitRetcode(DWORD ret) {
    static char buf[26];
    if (ret == WAIT_FAILED) return "WAIT_FAILED";
    if (ret == WAIT_TIMEOUT) return "WAIT_TIMEOUT";
    if (ret > WAIT_ABANDONED_0) {
        sprintf(buf, "WAIT_ABANDONED_%d", (ret - WAIT_ABANDONED_0));
        return buf;
    }
    sprintf(buf, "WAIT_OBJECT_%d", (ret - WAIT_OBJECT_0 ));
    return buf;
}
#endif // DXW_NOTRACES

DWORD WINAPI extMsgWaitForMultipleObjects(DWORD nCount, const HANDLE *pHandles, BOOL fWaitAll, DWORD dwMilliseconds, DWORD dwWakeMask) {
    DWORD ret;
    ApiName("MsgWaitForMultipleObjects");
#ifndef DXW_NOTRACES
    OutTraceSYS("%s: count=%d waitall=%#x msec=%d wakemask=%#x(%s)\n", ApiRef, nCount, fWaitAll, dwMilliseconds, dwWakeMask, ExplainWakeMask(dwWakeMask));
    for (DWORD i = 0; i < nCount; i++)
        OutTraceSYS("> handle[%d]=%#x\n", i, pHandles[i]);
#endif
    ret = (*pMsgWaitForMultipleObjects)(nCount, pHandles, fWaitAll, dwMilliseconds, dwWakeMask);
    OutTraceSYS("%s: ret=%#x(%s)\n", ApiRef, ret, ExplainMsgWaitRetcode(ret));
    return ret;
}
#endif // TRACEWAITOBJECTS

#ifdef HACKVIRTUALKEYS
#define VKMAPTONUMPAD TRUE
UINT WINAPI extMapVirtualKeyA(UINT uCode, UINT uMapType) {
    UINT res;
    ApiName("MapVirtualKeyA");
    OutTraceSYS("%s: code=%#x maptype=%#x\n", ApiRef, uCode, uMapType);
    res = (*pMapVirtualKeyA)(uCode, uMapType);
    OutTraceSYS("%s: ret=%#x\n", ApiRef, res);
    return res;
}
#endif // HACKVIRTUALKEYS

SHORT WINAPI extGetAsyncKeyState(int vKey) {
    SHORT res;
    ApiName("GetAsyncKeyState");
    // beware: this call can be used quite frequently, it can easily flood the logfile.
    OutDebugIN("%s: vkey=%#x\n", ApiRef, vKey);
    res = (*pGetAsyncKeyState)(vKey);
    if((dxw.dwFlags11 & FIXASYNCKEYSTATE) && (GetForegroundWindow() != dxw.GethWnd())) {
#ifndef DXW_NOTRACES
        if(res) OutDebugDW("%s: suppressed vkey=%#x\n", ApiRef, res);
#endif // DXW_NOTRACES
        res = 0;
    }
    OutDebugIN("%s: vkey=%#x res=%#x\n", ApiRef, vKey, res);
    return res;
}

// --- NLS

LPSTR WINAPI extCharPrev(LPCSTR lpStart, LPCSTR lpCurrentChar) {
    OutTraceSYS("CharPrev\n");
    return CharPrevExA((WORD)dxw.CodePage, lpStart, lpCurrentChar, 0);
}

LPSTR WINAPI extCharNext(LPCSTR lpCurrentChar) {
    OutTraceSYS("CharNext\n");
    return CharNextExA((WORD)dxw.CodePage, lpCurrentChar, 0);
}

BOOL WINAPI extSetWindowTextA(HWND hWnd, LPCSTR lpString) {
    BOOL res;
    ApiName("SetWindowTextA");
    OutTraceSYS("%s: hwnd=%#x(%s) str=\"%s\"\n",
                ApiRef, hWnd, IsWindowUnicode(hWnd) ? "WIDE" : "ANSI", lpString);
    if(dxw.dwFlags11 & CUSTOMLOCALE) {
        LPWSTR wstr = NULL;
        int n;
        if (lpString) {
            int size = lstrlenA(lpString);
            wstr = (LPWSTR)malloc((size + 1) << 1);
            n = MultiByteToWideChar(dxw.CodePage, 0, lpString, size, wstr, size);
            wstr[n] = L'\0'; // make tail !
        }
        //res = SetWindowTextW(hWnd, wstr); <- no good!!
        // enlightment from https://stackoverflow.com/questions/9410681/setwindowtextw-in-an-ansi-project
        // SetWindowText()/SetWindowTextA() and SetWindowTextW() are all really WM_SETTEXT which is one of
        // the few messages subject to code-page translation when you create a multibyte/Ansi window.
        // This means there is no W and A versions of the message.
        // Even so, it is easy to display Unicode in a Vista/Win7 titlebar built as an Ansi/Multibyte
        // application. All you need to do is intercept the WM_SETTEXT message in your window and pass the
        // arguments to DefWindowProcW() instead of the usual DefWindowProcA/DefWindowProc().
        // This works because internally all windows are actually unicode.
        // Note that if you simply pass the arguments to DefWindowProcW() then you must be absolutely sure
        // that the argument really does point to a wchar_t string.
        // n.b. both methods here below are working!
        //#define METHOD1
#ifdef METHOD1
        res = DefWindowProcW(hWnd, WM_SETTEXT, NULL, (LPARAM)wstr);
#else
        LONG_PTR OrigWndP = GetWindowLongPtrW(hWnd, GWLP_WNDPROC);
        SetWindowLongPtrW(hWnd, GWLP_WNDPROC, (LONG_PTR)DefWindowProcW);
        res = SetWindowTextW(hWnd, wstr);
        SetWindowLongPtrW(hWnd, GWLP_WNDPROC, (LONG_PTR)OrigWndP);
#endif
        OutTraceSYS("%s: [W] wstr=\"%ls\" res=%#x\n", ApiRef, wstr, res);
        if (wstr) free((LPVOID)wstr);
    } else
        res = (*pSetWindowTextA)(hWnd, lpString);
    IfTraceError();
    return res;
}

int WINAPI extGetWindowTextA(HWND hWnd, LPSTR lpString, int nMaxCount) {
    int ret;
    OutTraceSYS("GetWindowTextA: hwnd=%#x count=%d\n", hWnd, nMaxCount);
    int len = (int)(*pSendMessageW)(hWnd, WM_GETTEXTLENGTH, 0, 0) + 1;
    LPWSTR lpStringW = (LPWSTR)malloc(len * sizeof(wchar_t));
    if(dxw.dwFlags11 & CUSTOMLOCALE) {
        // see comments above for SetWindowTextA
#ifdef METHOD1
        ret = DefWindowProcW(hWnd, WM_GETTEXT, (WPARAM)len, (LPARAM)lpStringW);
#else
        LONG_PTR OrigWndP = GetWindowLongPtrW(hWnd, GWLP_WNDPROC);
        SetWindowLongPtrW(hWnd, GWLP_WNDPROC, (LONG_PTR)DefWindowProcW);
        ret = GetWindowTextW(hWnd, lpStringW, len);
        SetWindowLongPtrW(hWnd, GWLP_WNDPROC, (LONG_PTR)OrigWndP);
#endif
        if (ret > 0) {
            int size = WideCharToMultiByte(CP_ACP, 0, lpStringW, -1, lpString, nMaxCount, NULL, NULL);
            if (size > 0) ret = size - 1;
            else {
                if (GetLastError() == ERROR_INSUFFICIENT_BUFFER) {
                    lpString[nMaxCount - 1] = '\0';
                    ret = nMaxCount - 1;
                } else {
                    lpString[0] = '\0';
                    ret = 0;
                }
            }
        } else {
            lpString[0] = '\0';
            ret = 0;
        }
        free(lpStringW);
    } else
        ret = GetWindowTextA(hWnd, lpString, len);
    OutTraceSYS("GetWindowTextA: lpstring=\"%s\" ret=%#x\n", lpString, ret);
    return ret;
}

BOOL WINAPI extOemToCharA(LPCSTR pSrc, LPSTR pDest) {
    BOOL ret;
    ret = (*pOemToCharA)(pSrc, pDest);
    OutTraceLOC("OemToCharA: src=\"%s\" dst=\"%s\" ret=%#x\n", pSrc, pDest, ret);
    return ret;
}

BOOL WINAPI extCharToOemA(LPCSTR pSrc, LPSTR pDest) {
    BOOL ret;
    ret = (*pCharToOemA)(pSrc, pDest);
    OutTraceLOC("CharToOemA: src=\"%s\" dst=\"%s\" ret=%#x\n", pSrc, pDest, ret);
    return ret;
}

extern char *ExplainRgnType(int);

int WINAPI extSetWindowRgn(HWND hWnd, HRGN hRgn, BOOL bRedraw) {
    int ret;
    ApiName("SetWindowRgn");
    OutTraceDW("%s: hwnd=%#x hrgn=%#x redraw=%#x\n", ApiRef, hWnd, hRgn, bRedraw);
    if(dxw.IsFullScreen() && (dxw.GDIEmulationMode != GDIMODE_NONE)) {
        //(*pDeleteObject)(hrgnScaled); -- https://docs.microsoft.com/it-it/windows/win32/api/winuser/nf-winuser-setwindowrgn
        // After a successful call to SetWindowRgn, the system owns the region specified by the region handle hRgn.
        // The system does not make a copy of the region.
        // Thus, you should not make any further function calls with this region handle.
        // In particular, do not delete this region handle.
        HRGN hrgnScaled;
        hrgnScaled = dxw.MapRegion(ApiRef, hRgn);
        CombineRgn(hRgn, hrgnScaled, NULL, RGN_COPY);
        (*pDeleteObject)(hrgnScaled);
    }
    ret = (*pSetWindowRgn)(hWnd, hRgn, bRedraw);
    OutTraceSYS("%s: ret=%d(%s)\n", ApiRef, ret, ExplainRgnType(ret));
    return ret;
}

int WINAPI extGetWindowRgn(HWND hWnd, HRGN hRgn) {
    int ret;
    ApiName("GetWindowRgn");
    OutTraceDW("%s: hwnd=%#x hrgn=%#x\n", ApiRef, hWnd, hRgn);
    ret = (*pGetWindowRgn)(hWnd, hRgn);
    OutTraceSYS("%s: ret=%d(%s)\n", ApiRef, ret, ExplainRgnType(ret));
    // to do: leave owned hRgn unscaled !!!
    if(dxw.IsFullScreen() && (dxw.GDIEmulationMode != GDIMODE_NONE)) {
        HRGN hrgnScaled;
        hrgnScaled = dxw.UnmapRegion(ApiRef, hRgn);
        CombineRgn(hRgn, hrgnScaled, NULL, RGN_COPY);
        (*pDeleteObject)(hrgnScaled);
    }
    OutTraceSYS("%s: ret=%d(%s)\n", ApiRef, ret, ExplainRgnType(ret));
    return ret;
}

// --------------------------------------------------------------------
// point trace wrappers
// --------------------------------------------------------------------

#ifdef TRACEPOINTS
BOOL WINAPI extPtInRect(CONST RECT *lprc, POINT pt) {
    BOOL ret;
    ApiName("PtInRect");
    ret = (*pPtInRect)(lprc, pt);
    OutTrace("%s: rect=(%d,%d)-(%d,%d) pt=(%d,%d) ret=%#x(%s)\n",
             ApiRef,
             lprc->left, lprc->top, lprc->right, lprc->bottom,
             pt.x, pt.y,
             ret, ret ? "IN" : "OUT");
    return ret;
}

BOOL WINAPI extSetRect(LPRECT lprc, int xLeft, int yTop, int xRight, int yBottom) {
    BOOL ret;
    ApiName("SetRect");
    ret = (*pSetRect)(lprc, xLeft, yTop, xRight, yBottom);
    if(ret) {
        OutTrace("%s: rect=(%d,%d)-(%d,%d)\n",
                 ApiRef,
                 lprc->left, lprc->top, lprc->right, lprc->bottom);
    } else
        OutTrace("%s: ERROR err=%d\n", ApiRef, GetLastError());
    return ret;
}

BOOL WINAPI extSetRectEmpty(LPRECT lprc) {
    BOOL ret;
    ApiName("SetRectEmpty");
    ret = (*pSetRectEmpty)(lprc);
    if(ret) {
        OutTrace("%s: rect=(%d,%d)-(%d,%d)\n",
                 ApiRef,
                 lprc->left, lprc->top, lprc->right, lprc->bottom);
    } else
        OutTrace("%s: ERROR err=%d\n", ApiRef, GetLastError());
    return ret;
}

BOOL WINAPI extOffsetRect(LPRECT lprc, int dx, int dy) {
    BOOL ret;
    ApiName("OffsetRect");
    ret = (*pOffsetRect)(lprc, dx, dy);
    if(ret) {
        OutTrace("%s: delta=(%d,%d) rect=(%d,%d)-(%d,%d)\n",
                 ApiRef,
                 dx, dy,
                 lprc->left, lprc->top, lprc->right, lprc->bottom);
    } else
        OutTrace("%s: ERROR err=%d\n", ApiRef, GetLastError());
    return ret;
}

BOOL WINAPI extInflateRect(LPRECT lprc, int dx, int dy) {
    BOOL ret;
    ApiName("InflateRect");
    ret = (*pInflateRect)(lprc, dx, dy);
    if(ret) {
        OutTrace("%s: delta=(%d,%d) rect=(%d,%d)-(%d,%d)\n",
                 ApiRef,
                 dx, dy,
                 lprc->left, lprc->top, lprc->right, lprc->bottom);
    } else
        OutTrace("%s: ERROR err=%d\n", ApiRef, GetLastError());
    return ret;
}

BOOL WINAPI extSubtractRect(LPRECT lprcDst, CONST RECT *lprcSrc1, CONST RECT *lprcSrc2) {
    BOOL ret;
    ApiName("SubtractRect");
    OutTrace("%s: rect1=(%d,%d)-(%d,%d)\n",
             ApiRef, lprcSrc1->left, lprcSrc1->top, lprcSrc1->right, lprcSrc1->bottom);
    OutTrace("%s: rect2=(%d,%d)-(%d,%d)\n",
             ApiRef, lprcSrc2->left, lprcSrc2->top, lprcSrc2->right, lprcSrc2->bottom);
    ret = (*pSubtractRect)(lprcDst, lprcSrc1, lprcSrc2);
    if(ret) {
        OutTrace("%s: dest=(%d,%d)-(%d,%d)\n",
                 ApiRef, lprcDst->left, lprcDst->top, lprcDst->right, lprcDst->bottom);
    } else
        OutTrace("%s: ERROR err=%d\n", ApiRef, GetLastError());
    return ret;
}

BOOL WINAPI extUnionRect(LPRECT lprcDst, CONST RECT *lprcSrc1, CONST RECT *lprcSrc2) {
    BOOL ret;
    ApiName("UnionRect");
    OutTrace("%s: rect1=(%d,%d)-(%d,%d)\n",
             ApiRef, lprcSrc1->left, lprcSrc1->top, lprcSrc1->right, lprcSrc1->bottom);
    OutTrace("%s: rect2=(%d,%d)-(%d,%d)\n",
             ApiRef, lprcSrc2->left, lprcSrc2->top, lprcSrc2->right, lprcSrc2->bottom);
    ret = (*pUnionRect)(lprcDst, lprcSrc1, lprcSrc2);
    if(ret) {
        OutTrace("%s: dest=(%d,%d)-(%d,%d)\n",
                 ApiRef, lprcDst->left, lprcDst->top, lprcDst->right, lprcDst->bottom);
    } else
        OutTrace("%s: ERROR err=%d\n", ApiRef, GetLastError());
    return ret;
}

BOOL WINAPI extIntersectRect(LPRECT lprcDst, CONST RECT *lprcSrc1, CONST RECT *lprcSrc2) {
    BOOL ret;
    ApiName("IntersectRect");
    OutTrace("%s: rect1=(%d,%d)-(%d,%d)\n",
             ApiRef, lprcSrc1->left, lprcSrc1->top, lprcSrc1->right, lprcSrc1->bottom);
    OutTrace("%s: rect2=(%d,%d)-(%d,%d)\n",
             ApiRef, lprcSrc2->left, lprcSrc2->top, lprcSrc2->right, lprcSrc2->bottom);
    ret = (*pIntersectRect)(lprcDst, lprcSrc1, lprcSrc2);
    if(ret) {
        OutTrace("%s: dest=(%d,%d)-(%d,%d)\n",
                 ApiRef, lprcDst->left, lprcDst->top, lprcDst->right, lprcDst->bottom);
    } else
        OutTrace("%s: ERROR err=%d\n", ApiRef, GetLastError());
    return ret;
}

BOOL WINAPI extIsRectEmpty(CONST RECT *lprc) {
    BOOL ret;
    ApiName("IsRectEmpty");
    ret = (*pIsRectEmpty)(lprc);
    OutTrace("%s: rect=(%d,%d)-(%d,%d) ret=%#x(%s)\n",
             ApiRef,
             lprc->left, lprc->top, lprc->right, lprc->bottom,
             ret, ret ? "YES" : "NO");
    return ret;
}

BOOL WINAPI extCopyRect(LPRECT lprcDst, CONST RECT *lprcSrc) {
    BOOL ret;
    ApiName("CopyRect");
    ret = (*pCopyRect)(lprcDst, lprcSrc);
    OutTrace("%s: rect=(%d,%d)-(%d,%d) ret=%#x(%s)\n",
             ApiRef,
             lprcSrc->left, lprcSrc->top, lprcSrc->right, lprcSrc->bottom,
             ret, ret ? "YES" : "NO");
    return ret;
}
#endif // TRACEPOINTS

#ifdef TRACEWINDOWS
BOOL WINAPI extEnableWindow(HWND hWnd, BOOL bEnable) {
    BOOL ret;
    ApiName("EnableWindow");
    ret = (*pEnableWindow)(hWnd, bEnable);
    OutTraceSYS("%s: hwnd=%#x enable=%#x ret(prev state)=%#x\n",
                ApiRef, hWnd, bEnable, ret);
    return ret;
}
#endif // TRACEWINDOWS

#ifdef TRACEWINDOWS
BOOL WINAPI extEnumChildWindows(HWND hWndParent, WNDENUMPROC lpEnumFunc, LPARAM lParam) {
    BOOL ret;
    ApiName("EnumChildWindows");
    OutTraceSYS("%s: hwndparent=%#x lparam=%#x\n", ApiRef, lParam);
    ret = (*pEnumChildWindows)(hWndParent, lpEnumFunc, lParam);
    OutTraceSYS("%s: ret=%#x\n", ApiRef, ret);
    return ret;
}
#endif // TRACEWINDOWS

#ifdef TRACEIMAGES
HBITMAP WINAPI extLoadBitmapA(HINSTANCE hInstance, LPCSTR lpBitmapName) {
    HBITMAP ret;
    ApiName("LoadBitmapA");
    OutTraceDW("%s: hinst=%#x name=%s\n", ApiRef, hInstance, lpBitmapName);
    ret = (*pLoadBitmapA)(hInstance, lpBitmapName);
    if(ret) {
        if(dxw.dwDFlags2 & DUMPBITMAPS) DumpBitmap(ApiRef, (HBITMAP)ret);
        OutTraceDW("%s: hbitmap=%#x\n", ApiRef, ret);
    } else
        OutTraceE("%s: ERROR err=%d\n", ApiRef, GetLastError());
    return ret;
}
#endif // TRACEIMAGES

HANDLE WINAPI extLoadImageA(HINSTANCE hInst, LPCSTR name, UINT type, int cx, int cy, UINT fuLoad) {
    HANDLE res;
    ApiName("LoadImageA");
    OutTraceSYS("%s: hinst=%#x name=\"%s\" type=%d xy=(%d,%d) load=%#x\n",
                ApiRef, hInst, ClassToStr(name), type, cx, cy, fuLoad);
    if(dxw.dwFlags10 & (FAKECDDRIVE | FAKEHDDRIVE)) {
        if((fuLoad & LR_LOADFROMFILE) && name) {
            extern LPCSTR dxwTranslatePathA(LPCSTR, DWORD *);
            name = dxwTranslatePathA(name, NULL);
            OutTraceDW("%s: mapping path=\"%s\"\n", ApiRef, name);
        }
    }
    res = (*pLoadImageA)(hInst, name, type, cx, cy, fuLoad);
#ifndef DXW_NOTRACES
    if(!res)
        OutTraceE("%s: ERROR err=%d\n", ApiRef, GetLastError());
    else {
        if((dxw.dwDFlags2 & DUMPBITMAPS) && (type == IMAGE_BITMAP)) DumpBitmap(ApiRef, (HBITMAP)res);
        OutTraceSYS("%s: ret=%#x\n", ApiRef, res);
    }
#endif // DXW_NOTRACES
    return res;
}

BOOL WINAPI extDrawEdge(HDC hdc, LPRECT qrc, UINT edge, UINT grfFlags) {
    BOOL res;
    ApiName("DrawEdge");
    // MessageBox(0, ApiRef, "DxWnd", 0);
    OutTraceDW("%s: hdc=%#x qrc=(%d,%d)-(%d,%d) edge=%d flags=%#x\n",
               ApiRef, hdc,
               qrc->left, qrc->top, qrc->right, qrc->bottom,
               edge, grfFlags);
    if(dxw.IsToRemap(hdc)) {
        RECT scaledqrc;
        switch(dxw.GDIEmulationMode) {
        case GDIMODE_SHAREDDC:
            sdc.GetPrimaryDC(hdc);
            res = (*pDrawEdge)(sdc.GetHdc(), qrc, edge, grfFlags);
            sdc.PutPrimaryDC(hdc, TRUE);
            return res;
            break;
        case GDIMODE_STRETCHED:
            scaledqrc = *qrc;
            dxw.MapClient(&scaledqrc);
            qrc = &scaledqrc;
            break;
        default:
            break;
        }
    }
    res = (*pDrawEdge)(hdc, qrc, edge, grfFlags);
    IfTraceError();
    return res;
}

HMENU WINAPI extGetMenu(HWND hwnd) {
    HMENU hmenu;
    ApiName("GetMenu");
    hmenu = (*pGetMenu)(hwnd);
    if(hmenu && (dxw.dwFlags12 & SUPPRESSMENUS)) {
        OutTraceDW("%s: SUPPRESS hmenu=%#x whnd=%#x\n", ApiRef, hmenu, hwnd);
        if (!(*pSetMenu)(hwnd, NULL)) OutTraceE("SetMenu ERROR: err=%d\n", GetLastError());
        if (!DestroyMenu(hmenu)) OutTraceE("DestroyMenu ERROR: err=%d\n", GetLastError());
        hmenu = NULL;
    }
    OutTraceSYS("%s: hwnd=%#x hmenu=%#x\n", ApiRef, hwnd, hmenu);
    return hmenu;
}

BOOL WINAPI extSetMenu(HWND hwnd, HMENU hmenu) {
    BOOL res;
    ApiName("SetMenu");
    OutTraceSYS("%s: hwnd=%#x hmenu=%#x\n", ApiRef, hwnd, hmenu);
    if(dxw.dwFlags12 & SUPPRESSMENUS) {
        OutTraceDW("%s: SUPPRESS hmenu=%#x whnd=%#x\n", ApiRef, hmenu, hwnd);
        hmenu = NULL;
    }
    res = (*pSetMenu)(hwnd, hmenu);
    _if(!res) OutTraceE("%s: ERROR err=%d\n", ApiRef, GetLastError());
    return res;
}

#ifdef TRACEMENUS
HMENU WINAPI extCreateMenu(void) {
    HMENU res;
    ApiName("CreateMenu");
    res = (*pCreateMenu)();
    OutTraceSYS("%s: hmenu=%#x\n", ApiRef, res);
    return res;
}

HMENU WINAPI extCreatePopupMenu(void) {
    HMENU res;
    ApiName("CreatePopupMenu");
    res = (*pCreatePopupMenu)();
    OutTraceSYS("%s: hmenu=%#x\n", ApiRef, res);
    return res;
}

UINT WINAPI extGetMenuState(HMENU hMenu, UINT uId, UINT uFlags) {
    UINT res;
    ApiName("GetMenuState");
    res = (*pGetMenuState)(hMenu, uId, uFlags);
    OutTraceSYS("%s: hmenu=%#x id=%d flags=%#x res=%#x\n", ApiRef, hMenu, uId, uFlags, res);
    return res;
}

BOOL WINAPI extGetMenuItemInfoA(HMENU hmenu, UINT item, BOOL fByPosition, LPMENUITEMINFOA lpmii) {
    UINT res;
    ApiName("GetMenuItemInfoA");
    OutTraceSYS("%s: hmenu=%#x item=%d bypos=%d\n", ApiRef, hmenu, item, fByPosition);
    res = (*pGetMenuItemInfoA)(hmenu, item, fByPosition, lpmii);
    if(res) {
        if(IsTraceSYS) {
            OutTrace("> size=%d\n", lpmii->cbSize);
            OutTrace("> fmask=%d\n", lpmii->fMask);
            OutTrace("> ftype=%d\n", lpmii->fType);
            OutTrace("> fstate=%d\n", lpmii->fState);
            OutTrace("> wid=%#x\n", lpmii->wID);
            OutTrace("> submenu=%#x\n", lpmii->hSubMenu);
            /* ... */
        }
    } else
        OutTraceE("%s: ERROR err=%d\n", ApiRef, GetLastError());
    return res;
}

BOOL WINAPI extGetMenuItemInfoW(HMENU hmenu, UINT item, BOOL fByPosition, LPMENUITEMINFOW lpmii) {
    UINT res;
    ApiName("GetMenuItemInfoW");
    OutTraceSYS("%s: hmenu=%#x item=%d bypos=%d\n", ApiRef, hmenu, item, fByPosition);
    res = (*pGetMenuItemInfoW)(hmenu, item, fByPosition, lpmii);
    if(res) {
        if(IsTraceSYS) {
            OutTrace("> size=%d\n", lpmii->cbSize);
            OutTrace("> fmask=%d\n", lpmii->fMask);
            OutTrace("> ftype=%d\n", lpmii->fType);
            OutTrace("> fstate=%d\n", lpmii->fState);
            OutTrace("> wid=%#x\n", lpmii->wID);
            OutTrace("> submenu=%#x\n", lpmii->hSubMenu);
            /* ... */
        }
    } else
        OutTraceE("%s: ERROR err=%d\n", ApiRef, GetLastError());
    return res;
}

BOOL WINAPI extSetMenuItemInfoA(HMENU hmenu, UINT item, BOOL fByPosition, LPCMENUITEMINFOA lpmii) {
    UINT res;
    ApiName("setMenuItemInfoA");
    OutTraceSYS("%s: hmenu=%#x item=%d bypos=%d\n", ApiRef, hmenu, item, fByPosition);
    if(IsTraceSYS) {
        OutTrace("> size=%d\n", lpmii->cbSize);
        OutTrace("> fmask=%d\n", lpmii->fMask);
        OutTrace("> ftype=%d\n", lpmii->fType);
        OutTrace("> fstate=%d\n", lpmii->fState);
        OutTrace("> wid=%#x\n", lpmii->wID);
        OutTrace("> submenu=%#x\n", lpmii->hSubMenu);
        /* ... */
    }
    return TRUE;
    res = (*pSetMenuItemInfoA)(hmenu, item, fByPosition, lpmii);
    if(!res)
        OutTraceE("%s: ERROR err=%d\n", ApiRef, GetLastError());
    return res;
}
#endif // #ifdef TRACEMENUS
