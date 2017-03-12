#pragma once
#include "MyString.h"
#include <functional>

namespace MyWeb
{
    struct TExternalItem
    {
        MyString m_name;

        std::function<void(DISPPARAMS* params, VARIANT* result)>  m_pFnRun;
    };

    enum
    {
        WM_WEB_THREAD_DO_INITIALIZE = WM_USER + 0x100,
        WM_WEB_THREAD_SET_BROWSER_POS
    };
}