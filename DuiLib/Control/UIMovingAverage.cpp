#include "StdAfx.h"
#include "UIMovingAverage.h"
#include "UICandlestickChart.h"

namespace DuiLib
{
	CMovingAverageUI::CMovingAverageUI():
		m_pOwner(nullptr)
	{
		SetChildPadding(3);
	}

	CMovingAverageUI::~CMovingAverageUI()
	{

	}

	void CMovingAverageUI::SetOwner(CCandlestickChartUI* pOwner)
	{
		m_pOwner = pOwner;
		auto& needDrawLines = m_pOwner->GetNeedDrawLines();
		RemoveAll();
		for (size_t i = 0; i < needDrawLines.size(); ++i)
		{
			CLabelUI* pLbl = new CLabelUI;
			pLbl->SetTextStyle(DT_LEFT | DT_SINGLELINE | DT_END_ELLIPSIS);
			Add(pLbl);
		}
	}

	void CMovingAverageUI::UpdateData(size_t nIndex)
	{
		ASSERT(m_pOwner);

		RECT rcParent = GetPos();

		auto& needDrawLines = m_pOwner->GetNeedDrawLines();
		auto& movingAverageDatas = m_pOwner->GetMovingAverageDatas();

		size_t i = 0;
		for (auto it = needDrawLines.cbegin(); it != needDrawLines.cend(); ++it)
		{
			auto& movingAverageIt = movingAverageDatas.find(it->first);
			if (movingAverageIt == movingAverageDatas.cend())
			{
				continue;
			}

			auto& itemData = movingAverageIt->second.at(nIndex);

			CLabelUI* pLbl = static_cast<CLabelUI*>(GetItemAt(i++));
			if (!pLbl)
			{
				break;
			}
			CDuiString str;
			str.Format(_T("MA%d:%2.3f"), it->first, itemData);
			RECT rcText = rcParent;
			DWORD dwTextColor = RGB(it->second.GetBlue(), it->second.GetGreen(), it->second.GetRed());
			CRenderEngine::DrawText(m_pManager->GetPaintDC(), m_pManager, rcText, str, dwTextColor, 0, pLbl->GetTextStyle() | DT_CALCRECT);
			rcParent.left = rcText.right + GetChildPadding();
			pLbl->SetTextColor(dwTextColor);
			pLbl->SetText(str);
			pLbl->SetFixedWidth(rcText.right - rcText.left);
		}
		SetPos(GetPos());
	}

	void CMovingAverageUI::DoPaint(HDC hDC, const RECT& rcPaint)
	{
		CHorizontalLayoutUI::DoPaint(hDC, rcPaint);
	}

}