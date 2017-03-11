#pragma once
//////////////BEGIN消息映射宏定义////////////////////////////////////////////////////
///

namespace DuiLib
{
    enum
    {
        WM_USER_WEBBROWSER_2W_BEGIN = WM_USER + 0x200,
        WM_USER_WEBBROWSER_SIZE_CHANGED,
        WM_USER_SHOW_WEBBROWSER,
        WM_USER_WEBBROWSER_2W_END,
        WM_THREAD_WEB_BEGIN,
        WM_THREAD_WEB_CREATE_BROWSER,
        WM_THREAD_WEB_QUIT_BROWSER,
        WM_THREAD_WEB_END,
    };
}// namespace DuiLib

