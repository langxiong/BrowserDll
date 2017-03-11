/******************************************************************************
 *  版权所有（C）2012-2016													*
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

#include <map>

namespace DuiLib
{
	class CCandlestickChartUI;
	class CMovingAverageUI : public CHorizontalLayoutUI
	{
	public:
		CMovingAverageUI();
		~CMovingAverageUI();

		void SetOwner(CCandlestickChartUI* pOwner);
		void UpdateData(size_t nIndex);
		virtual void DoPaint(HDC hDC, const RECT& rcPaint);
	private:
		CCandlestickChartUI* m_pOwner;
	};
}

