#include "StdAfx.h"
#include "UICameraLayout.h"

namespace DuiLib
{
    CCameraLayoutUI::CCameraLayoutUI() :m_nCurSel(-1)
    {
    }

    CCameraLayoutUI::~CCameraLayoutUI()
    {

    }

    LPCTSTR CCameraLayoutUI::GetClass() const
    {
        return _T("CameraLayoutUI");
    }

    LPVOID CCameraLayoutUI::GetInterface(LPCTSTR pstrName)
    {
        if (_tcscmp(pstrName, DUI_CTR_CAMERALAYOUT) == 0) return static_cast<CCameraLayoutUI*>(this);
        return CTileLayoutUI::GetInterface(pstrName);
    }

    bool CCameraLayoutUI::SelectItem(int nIndex)
    {
        if (nIndex == m_nCurSel) {
            return true;
        };

        if (nIndex >= m_items.GetSize()) {
            return false;
        }

        m_nCurSel = nIndex;
        for (int i = 0; i < m_items.GetSize(); ++i)
        {
            if (i == nIndex) {
                GetItemAt(i)->SetVisible(true);
                SetPos(m_rcItem);
            }
            else {
                GetItemAt(i)->SetVisible(false);
            }
        }
        NeedParentUpdate();
        return true;
    }

    int CCameraLayoutUI::GetCurSel() const
    {
        return m_nCurSel;
    }

    void CCameraLayoutUI::SplicScreen()
    {
        m_nCurSel = -1;

        for (int i = 0; i < m_items.GetSize(); ++i)
        {
            GetItemAt(i)->SetVisible(true);
        }
        SetPos(GetPos());
    }

    void CCameraLayoutUI::SetPos(RECT rc)
    {
        CControlUI::SetPos(rc);
        rc = m_rcItem;

        if (m_nCurSel >= 0)
        {
            GetItemAt(m_nCurSel)->SetPos(rc);
            return;
        }

        int nChildPadding = GetChildPadding();
        int nColumns = GetColumns() > 0 ? GetColumns() : 1;

        int nRows = (GetCount() + 1) / nColumns;
        nRows = nRows > 0 ? nRows : 1;

        int nItemWidth = (rc.right - rc.left - nChildPadding * (nColumns - 1)) / nColumns;
        int nItemHeight = (rc.bottom - rc.top - nChildPadding * (nRows - 1)) / nRows;
        SetItemSize({ nItemWidth, nItemHeight });

        int nCount = GetCount();
        int nIndex = 0;
        int nTop = rc.top;
        for (int r = 0; r < nRows; ++r)
        {
            int nLeft = rc.left;
            for (int c = 0; c < nColumns && nIndex < nCount; ++c, ++nIndex)
            {
                RECT rcItem = rc;
                rcItem.left = nLeft;
                rcItem.right = rcItem.left + nItemWidth;
                rcItem.top = nTop;
                rcItem.bottom = rcItem.top + nItemHeight;

                GetItemAt(nIndex)->SetPos(rcItem);
                nLeft += (nItemWidth + nChildPadding);
            }

            nTop += (nItemHeight + nChildPadding);
        }
    }

}