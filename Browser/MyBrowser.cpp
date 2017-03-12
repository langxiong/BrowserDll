// MyUI.cpp : Defines the exported functions for the DLL application.
//
    
#include "stdafx.h"
#include "MyBrowser.h"
#include "MyBrowserCtrl.h"

using namespace MyWeb;
extern "C"
{
    int CreateBrowserCtrl(HWND hBindWnd)
    {
        return MyBrowserCtrl::CreateBrowserCtrl(hBindWnd);
    }

    void DestroyBrowserCtrl(int nIndex)
    {
        return MyBrowserCtrl::DestroyBrowserCtrl(nIndex);
    }

    void SetBrowserCtrlPos(int nIndex, RECT rc)
    {
        return MyBrowserCtrl::SetBrowserCtrlPos(nIndex, rc);
    }

    void NavigateUrl(int nIndex, const TCHAR * url)
    {
        return MyBrowserCtrl::BrowserCtrlNavigateUrl(nIndex, url);
    }

    void ExecuteJscode(int nIndex, const TCHAR * jsCode)
    {
        return MyBrowserCtrl::BrowserCtrlExecuteJscode(nIndex, jsCode);
    }

    bool RegisterBrowserCallback(int nIndex)
    {
        return false;
    }

    bool UnRegisterBrowserCallback(int nIndex)
    {
        return false;
    }
}
