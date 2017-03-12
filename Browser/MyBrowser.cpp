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

    void NavigateUrl(int nIndex, const TCHAR * url)
    {
    }

    void ExecuteJSCode(int nIndex, const TCHAR * jsCode)
    {
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
