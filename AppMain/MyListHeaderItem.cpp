#include "StdAfx.h"
#include "MyListHeaderItem.h"

using namespace DuiLib;

namespace MyWeb {
    namespace UI {
		static const int sm_szFixedNumWidth = 98;
        CListHeaderNumItem::CListHeaderNumItem()
        {
            SetFixedWidth(sm_szFixedNumWidth);
        }

        CListHeaderNumItem::~CListHeaderNumItem(void)
        {
            
        }

        LPCTSTR CListHeaderNumItem::GetClass() const
        {
            return _T("CMyListHeaderItem");
        }

        LPVOID CListHeaderNumItem::GetInterface(LPCTSTR pstrName)
        {
            if (DUI_CTR_THUBNAIL_HEADER_ITEM == pstrName)
            {
                return this;
            }
            return CListHeaderItemUI::GetInterface(pstrName);
        }

        void CListHeaderNumItem::DoInit()
        {
            CChildLayoutUI* pChild = new CChildLayoutUI;
            pChild->SetChildLayoutXML(_T("ui_compose_nums.xml"));
            pChild->SetFixedHeight(28);
            Add(pChild);

            auto pEditNum = pChild->FindSubControl(L"edtNum");
			pEditNum->SetName((std::wstring(L"edtNum") + std::to_wstring(sm_nIndex++)).c_str());

			auto lblNum = pChild->FindSubControl(L"lblNum");
			lblNum->SetText(GetUserData());
        }

        SIZE CListHeaderNumItem::EstimateSize(SIZE szAvailable)
        {
            return CListHeaderItemUI::EstimateSize(szAvailable);
        }

        void CListHeaderNumItem::ResetIndex()
        {
            sm_nIndex = 0;
        }

		int CListHeaderNumItem::sm_nIndex = 0;

        const std::wstring CListHeaderNumItem::DUI_CTR_THUBNAIL_HEADER_ITEM(_T("MyListHeaderItem"));


        CListHeaderDateItem::CListHeaderDateItem()
        {
            SetText(L"ÖÐ½±ÆÚÊý");
            SetRightBorderSize(1);
            SetBorderColor(0xc0c0c0);
        }

        CListHeaderDateItem::~CListHeaderDateItem(void)
        {
        }

        LPCTSTR CListHeaderDateItem::GetClass() const
        {
            return _T("CMyListHeaderItem");
        }

        LPVOID CListHeaderDateItem::GetInterface(LPCTSTR pstrName)
        {
            if (DUI_CTR_LEADER_HEADER_ITEM == pstrName)
            {
                return this;
            }
            return CListHeaderItemUI::GetInterface(pstrName);
        }

        SIZE CListHeaderDateItem::EstimateSize(SIZE szAvailable)
        {
            return{ szAvailable.cx / 5 - sm_szFixedNumWidth, szAvailable.cy };
        }

        const std::wstring CListHeaderDateItem::DUI_CTR_LEADER_HEADER_ITEM(_T("MyLeaderListHeaderItem"));
    }
}