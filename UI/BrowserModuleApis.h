#pragma once

#include <functional>
#include <tchar.h>
#include <WinDef.h>

namespace MyWeb
{
    namespace UI
    {
        struct TBrowserModuleApis
        {
            std::function<int (HWND hBindWnd)>  _createBrowserCtrl;
            std::function<void (int)>           _destroyBrowserCtrl;

            std::function<void(int, RECT)>     _setBrowserCtrlPos;

            std::function<void (int, const TCHAR*)>  _navigateUrl;
            std::function<void (int, const TCHAR*)>  _executeJSCode;

            std::function<bool (int)>  _registerBrowserCallback;
            std::function<bool (int)>  _unRegisterBrowserCallback;
        };
    }
}
