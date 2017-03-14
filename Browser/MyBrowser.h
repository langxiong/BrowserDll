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
  
  	/* 销毁一个webbrowser控件. 通过postthreadmessage(WM_QUIT)，
   	退出控件内部线程的消息循环，并等待线程退出
    @param nIndex 创建webbrowser返回的index
    */
    void MYBROWSER_API DestroyBrowserCtrl(int nIndex);

    /* 更新webbrowser控件的窗口位置.
    @param nIndex 创建webbrowser返回的index
    @param rc	  webbrowser控件绑定的父窗口的clientrect
    */
    void MYBROWSER_API SetBrowserCtrlPos(int nIndex, RECT rc);

    /* 导航新的url.
    @param nIndex 创建webbrowser返回的index
    @param url	  需要导航的url地址
    */
    void MYBROWSER_API NavigateUrl(int nIndex, const TCHAR* url);
  
    /* 注入js代码
    @param nIndex 创建webbrowser返回的index
    @param jsCode 需要注入的js代码
    */
    void MYBROWSER_API ExecuteJscode(int nIndex, const TCHAR* jsCode);
  
    /* 注册cpp回调，供网页调用.
    IE通过，window.external.${methodName}(${arguments})
    例: window.external.JsCallCppMsgBox('Hello from js')
    @param nIndex 创建webbrowser返回的index
    @param methodName 方法名称
    @pFun  回调函数，(params是js回调时的入参， result用于设定js回调的返回值
    例:
    */
    bool MYBROWSER_API RegisterBrowserCallback(
        int nIndex, 
        const TCHAR* methodName,
        void pFun(DISPPARAMS* params, VARIANT* result));
    bool MYBROWSER_API UnRegisterBrowserCallback(int nIndex);
}
