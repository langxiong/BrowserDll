#include "StdAfx.h"
#include "UICrossStart.h"

namespace DuiLib
{
	CCrossStarUI::CCrossStarUI() :
		m_pLblHor(new CLabelUI),
		m_pLblVer(new CLabelUI),
		m_dwLineColor(0XFF00FFFF)
	{
		memset(&m_rcHorXLinePos, 0, sizeof(RECT));
		memset(&m_rcVerYLinePos, 0, sizeof(RECT));
		{
			m_pLblHor->SetFixedWidth(50);
			m_pLblHor->SetFixedHeight(17);
			m_pLblHor->SetTextStyle(DT_RIGHT | DT_VCENTER);
			m_pLblHor->SetTextColor(0xffe1e1e1);
		}

		{
			m_pLblVer->SetFixedWidth(70);
			m_pLblVer->SetFixedHeight(17);
			m_pLblVer->SetTextStyle(DT_RIGHT | DT_VCENTER);
			m_pLblVer->SetTextColor(0xffe1e1e1);
		}
		Add(m_pLblHor);
		Add(m_pLblVer);
	}

	CCrossStarUI::~CCrossStarUI()
	{

	}

	void CCrossStarUI::SetHorXLinePos(const RECT& rcHorXLinePos)
	{
		if (memcmp(&m_rcHorXLinePos, &rcHorXLinePos, sizeof(RECT)) == 0)
		{
			return;
		}

		m_rcHorXLinePos = rcHorXLinePos;

		RECT rcPos = rcHorXLinePos;
		rcPos.left -= m_pLblHor->GetFixedWidth();
		rcPos.right = rcPos.left + m_pLblHor->GetFixedWidth();
		rcPos.bottom = rcPos.top + m_pLblHor->GetFixedHeight();
		m_pLblHor->SetPos(rcPos);
		Invalidate();
	}

	void CCrossStarUI::SetVerYLinePos(const RECT& rcVerYLinePos)
	{
		if (memcmp(&m_rcVerYLinePos, &rcVerYLinePos, sizeof(RECT)) == 0)
		{
			return;
		}

		m_rcVerYLinePos = rcVerYLinePos;

		const RECT& rcLblY = m_pLblVer->GetPos();
		RECT rcPos = m_rcVerYLinePos;
		rcPos.left -= m_pLblVer->GetFixedWidth() + 5;
		rcPos.right = rcPos.left + m_pLblVer->GetFixedWidth();
		rcPos.bottom = rcPos.top + m_pLblVer->GetFixedHeight();
		m_pLblVer->SetPos(rcPos);
		Invalidate();
	}

	void CCrossStarUI::DoPaint(HDC hDC, const RECT& rcPaint)
	{
		Graphics graphics(hDC);
		Pen pen(Color(GetBValue(m_dwLineColor), GetGValue(m_dwLineColor), GetRValue(m_dwLineColor)));

		graphics.DrawLine(&pen, m_rcHorXLinePos.left, m_rcHorXLinePos.top, m_rcHorXLinePos.right, m_rcHorXLinePos.bottom);
		graphics.DrawLine(&pen, m_rcVerYLinePos.left, m_rcVerYLinePos.top, m_rcVerYLinePos.right, m_rcVerYLinePos.bottom);
		CContainerUI::DoPaint(hDC, rcPaint);
	}

	void CCrossStarUI::SetXLabelText(const CDuiString& strText)
	{
		m_pLblVer->SetText(strText);
	}

	void CCrossStarUI::SetYLabelText(const CDuiString& strText)
	{
		m_pLblHor->SetText(strText);
	}

	void CCrossStarUI::SetPos(RECT rc)
	{
		return CControlUI::SetPos(rc);
	}

}
