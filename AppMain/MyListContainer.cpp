#include "StdAfx.h"
#include "MyListContainer.h"
#include "MyListElementItem.h"
#include "MyListHeaderItem.h"
#include "MyListSubContainer.h"

using namespace DuiLib;

namespace MyWeb {
    namespace UI {
        CMyListContainer::CMyListContainer():
            m_pCtrLeader(NULL)
        {
            this->SetFixedHeight(27);
            SetTopBorderSize(1);
        }

        CMyListContainer::~CMyListContainer(void)
        {
        }

        LPCTSTR CMyListContainer::GetClass() const
        {
            return _T("MyListContainer");
        }

        LPVOID CMyListContainer::GetInterface(LPCTSTR pstrName)
        {
            if (DUI_CTR_MY_LIST_CONTAINER == pstrName)
            {
                return this;
            }
            return CListContainerElementUI::GetInterface(pstrName);
        }

        void CMyListContainer::DoInit()
        {
            TListInfoUI* pListInfo = m_pOwner ? m_pOwner->GetListInfo() : nullptr;
            if (!pListInfo)
            {
                return;
            }

            SetBkColor(pListInfo->dwBkColor);
            SetBorderColor(pListInfo->dwLineColor);
            {
                m_pCtrLeader = new CLabelUI;
                Add(m_pCtrLeader);
                m_pCtrLeader->SetTextPadding({ 6, 0, 8, 0 });
                m_pCtrLeader->SetTextColor(pListInfo->dwTextColor);
                m_pCtrLeader->SetTextStyle(DT_RIGHT | DT_VCENTER);
                m_pCtrLeader->SetFixedWidth(120);
                m_pCtrLeader->SetFont(pListInfo->nFont);
                m_pCtrLeader->SetBorderColor(pListInfo->dwLineColor);
                m_pCtrLeader->SetRightBorderSize(1);
                m_pCtrLeader->SetLeftBorderSize(1);
                m_pCtrLeader->SetEnabled(false);
            }
            for (int i = 0; i < 5; ++i)
            {
                auto pSubContainer = new CMyListSubContainer;
                Add(pSubContainer);
                if (pListInfo)
                {
                    pSubContainer->UpdateListInfo(pListInfo);
                }
            }
        }

        void CMyListContainer::SetLeaderText(const std::wstring& strLeaderText)
        {
            m_strLeaderText = strLeaderText;
            if (m_pCtrLeader)
            {
                m_pCtrLeader->SetText(m_strLeaderText.c_str());
            }
        }

        const std::wstring& CMyListContainer::GetLeaderText() const
        {
            return m_strLeaderText;
        }

        void CMyListContainer::DoPaint(HDC hDC, const RECT& rcPaint)
        {
            CListContainerElementUI::DoPaint(hDC, rcPaint);
        }

        void CMyListContainer::UpdateFollowNums(std::vector<TItemData>& itemDatas)
        {
            assert(itemDatas.size() == 5);
            for (int i = 5; i > (5 - itemDatas.size()); --i)
            {
                auto pSubContainer = static_cast<CMyListSubContainer*>(GetItemAt(i));
                itemDatas[i - 1].m_pItem = pSubContainer->UpdateSelectIndex(itemDatas[i - 1].m_nNum);
            }
        }

        void CMyListContainer::DrawItemBk(HDC hDC, const RECT& rcItem)
        {
            CListContainerElementUI::DrawItemBk(hDC, rcItem);
        }

        const std::wstring CMyListContainer::DUI_CTR_MY_LIST_CONTAINER(_T("MyListContainer"));
    }
}