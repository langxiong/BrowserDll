#include "StdAfx.h"
#include "MyListElementItem.h"

using namespace DuiLib;

namespace MyWeb {
    using namespace UI_MSGS;
    namespace UI {
        CMyListElementItem::CMyListElementItem():
            m_dwBackupBorderColor(0),
            m_isHighLight(false)
        {
            SetTextStyle(DT_CENTER | DT_VCENTER);
        }

        CMyListElementItem::~CMyListElementItem(void)
        {
        }

        LPCTSTR CMyListElementItem::GetClass() const
        {
            return _T("MyListElementItem");
        }

        LPVOID CMyListElementItem::GetInterface(LPCTSTR pstrName)
        {
            if (DUI_CTR_MY_LIST_ELEMENT_ITEM == pstrName)
            {
                return this;
            }
            return CLabelUI::GetInterface(pstrName);
        }

        void CMyListElementItem::DoEvent(TEventUI& event)
        {
            CLabelUI::DoEvent(event);

            if (!IsEnabled())
            {
                return;
            }
            if (event.Type == UIEVENT_MOUSEENTER)
            {
                TNotifyUI uMsg = { UI_MSG_HIGHT_LIGHT_ITEM, nullptr, this };
                m_pManager->SendNotify(uMsg);
                return;
            }

            if (event.Type == UIEVENT_MOUSELEAVE)
            {
                TNotifyUI uMsg = { UI_MSG_UNHIGHT_LIGHT_ITEM, nullptr, this };
                m_pManager->SendNotify(uMsg);
                return;
            }
        }

        void CMyListElementItem::HighlightItem(bool isHighLight)
        {
            if (m_isHighLight == isHighLight || !IsEnabled())
            {
                return;
            }
            m_isHighLight = isHighLight;
            if (m_isHighLight)
            {
                SetBkImage(L"bg_hall_hover.png");
            }
            else
            {
                SetBkImage(L"");
            }
        }

        const DWORD CMyListElementItem::dwDHightLightBorderColor(0xff0000);

        const std::wstring CMyListElementItem::DUI_CTR_MY_LIST_ELEMENT_ITEM(_T("MyListElementItem"));
    }
}