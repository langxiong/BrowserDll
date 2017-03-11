#include "StdAfx.h"
#include "MyListSubContainer.h"
#include "MyListElementItem.h"

using namespace DuiLib;

namespace MyWeb {
    namespace UI {
        CMyListSubContainer::CMyListSubContainer():
            m_nSelectIndex(-1),
            m_pListInfo(nullptr)
        {
            SetLeftBorderSize(1);
        }

        CMyListSubContainer::~CMyListSubContainer(void)
        {
        }

        LPCTSTR CMyListSubContainer::GetClass() const
        {
            return _T("MyListContainer");
        }

        LPVOID CMyListSubContainer::GetInterface(LPCTSTR pstrName)
        {
            if (DUI_CTR_MY_LIST_SUB_CONTAINER == pstrName)
            {
                return this;
            }
            return CHorizontalLayoutUI::GetInterface(pstrName);
        }

        void CMyListSubContainer::DoInit()
        {
            for (int i = 0; i < 10; ++i)
            {
                auto pLbl = new CMyListElementItem;
                Add(pLbl);
                pLbl->SetText(std::to_wstring(i).c_str());
                pLbl->SetEnabled(false);
            }
            CHorizontalLayoutUI::DoInit();
        }

        void CMyListSubContainer::HighLightItem(int nNum)
        {
            for (size_t i = 0; i < GetCount(); i++)
            {
                if (i == nNum)
                {
                    static_cast<CMyListElementItem*>(GetItemAt(nNum))->HighlightItem(true);
                }
                else
                {
                    static_cast<CMyListElementItem*>(GetItemAt(i))->HighlightItem(false);
                }
            }
        }

        void CMyListSubContainer::UpdateListInfo(DuiLib::TListInfoUI* pListInfo)
        {
            m_pListInfo = pListInfo;

            if (!m_pListInfo)
            {
                return;
            }

            SetBorderColor(m_pListInfo->dwLineColor);

            for (auto i = 0; i < GetCount(); ++i)
            {
                auto pLbl = static_cast<CMyListElementItem*>(GetItemAt(i));
                pLbl->SetTextColor(m_pListInfo->dwTextColor);
                pLbl->SetDisabledTextColor(m_pListInfo->dwDisabledTextColor);
                pLbl->SetFont(m_pListInfo->nFont);
                pLbl->SetBorderColor(m_pListInfo->dwLineColor);
                pLbl->SetRightBorderSize(1);
            }
        }

        CMyListElementItem* CMyListSubContainer::UpdateSelectIndex(int nIndex)
        {
            m_nSelectIndex = nIndex;

            auto pLbl = static_cast<CMyListElementItem*>(GetItemAt(m_nSelectIndex));
            pLbl->SetEnabled(true);

            return pLbl;
        }

        const std::wstring CMyListSubContainer::DUI_CTR_MY_LIST_SUB_CONTAINER(_T("MyListContainer"));
    }
}