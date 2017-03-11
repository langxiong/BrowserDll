/******************************************************************************
 *  版权所有（C）2010-2015													*
 *  保留所有权利。                                                            *
 *****************************************************************************
 *  作者 : 熊浪
 *  版本 : 1.0
 *****************************************************************************/
/*  修改记录: 
      日期       版本    修改人             修改内容
    --------------------------------------------------------------------------
******************************************************************************/
#pragma once

namespace DuiLib
{
	class CCrossStarUI : public CContainerUI
	{
	public:
		CCrossStarUI();

		~CCrossStarUI();

		void SetHorXLinePos(const RECT& rcHorXLinePos);

		void SetVerYLinePos(const RECT& rcVerYLinePos);

		virtual void DoPaint(HDC hDC, const RECT& rcPaint);

		void SetXLabelText(const CDuiString& strText);

		void SetYLabelText(const CDuiString& strText);

		virtual void SetPos(RECT rc);

	private:
		CLabelUI* m_pLblHor;
		CLabelUI* m_pLblVer;

		DWORD m_dwLineColor;
		RECT m_rcHorXLinePos;
		RECT m_rcVerYLinePos;
	};
}

