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
    /* 新建一个webbrowser控件. 均在一个单独的线程中运行
    @param hBindWnd webbrowser控件需要绑定的父窗口
    @return 创建成功返回一个(>=0)的index, -1为创建失败. 
            后续调用接口，均需要传入该index，做为接口的第一个参数
    */
    int MYBROWSER_API CreateBrowserCtrl(HWND hBindWnd);
    void MYBROWSER_API DestroyBrowserCtrl(int nIndex);

    void MYBROWSER_API SetBrowserCtrlPos(int nIndex, RECT rc);

    void MYBROWSER_API NavigateUrl(int nIndex, const TCHAR* url);
    void MYBROWSER_API ExecuteJscode(int nIndex, const TCHAR* jsCode);

    bool MYBROWSER_API RegisterBrowserCallback(
        int nIndex, 
        const TCHAR* methodName,
        void pFun(DISPPARAMS* params, VARIANT* result));
    bool MYBROWSER_API UnRegisterBrowserCallback(int nIndex);
}
