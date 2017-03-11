#pragma once

#include <WinUser.h>
#include <tchar.h>
#include <string>
#include <array>

namespace MyWeb {

enum UI_MESSAGES
{
    MY_USER_MSG_BEGIN = WM_USER + 0x008,
    WM_THREAD_MAIN_CREATE_FRAME, // 主线程创建Frame窗口
    WM_THREAD_MAIN_CREATE_RENDER_PARENT_WND, // 主线程创建Render的父窗口
    WM_THREAD_FRAME_QUIT, // Frame窗口退出
};

namespace Constants
{
    static const std::string key_nMainThreadId = "key_nMainThreadId";
    static const std::string key_nTimerUpdateUserData = "key_nTimerUpdateUserData";

	static const std::array<std::wstring, 5> sm_UserNums = {
		L"万位：",
		L"千位：",
		L"百位：",
		L"十位：",
		L"个位：",
	};

} // Constants

namespace UI_MSGS
{
    static const TCHAR* UI_MSG_HIGHT_LIGHT_ITEM = L"ui_msg_hight_light_item";
    static const TCHAR* UI_MSG_UNHIGHT_LIGHT_ITEM = L"ui_msg_unhight_light_item";
}

namespace UI
{
    class CMyListElementItem;
    struct TItemData
    {
        int m_nNum;
        CMyListElementItem* m_pItem;

        TItemData(int nNum) :
            m_nNum(nNum),
            m_pItem(nullptr)
        {}
    };

    struct TGapItem
    {
        int m_nNum;
        std::wstring m_str;
    };

}
} // MyWeb
