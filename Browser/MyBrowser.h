// The following ifdef block is the standard way of creating macros which make exporting 
// from a DLL simpler. All files within this DLL are compiled with the MyUI_EXPORTS
// symbol defined on the command line. This symbol should not be defined on any project
// that uses this DLL. This way any other project whose source files include this file see 
// MyUI_API functions as being imported from a DLL, whereas this DLL sees symbols
// defined with this macro as being exported.
#ifdef BROWSER_EXPORTS
#define MYBROWSER_API __declspec(dllexport)
#else
#define MYBROWSER_API __declspec(dllimport)
#endif

#include <tchar.h>
#include <assert.h>
#include <iostream>
#include <string>
#include <atlbase.h>
#include <windows.h>

#include "BrowserDefine.h"

extern "C"
{
    int MYBROWSER_API CreateBrowserCtrl(HWND hBindWnd);
    void MYBROWSER_API DestroyBrowserCtrl(int nIndex);

    void MYBROWSER_API SetBrowserCtrlPos(int nIndex, RECT rc);

    void MYBROWSER_API NavigateUrl(int nIndex, const TCHAR* url);
    void MYBROWSER_API ExecuteJscode(int nIndex, const TCHAR* jsCode);

    bool MYBROWSER_API RegisterBrowserCallback(int nIndex);
    bool MYBROWSER_API UnRegisterBrowserCallback(int nIndex);
}
