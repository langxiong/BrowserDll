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

#include <vector>
#include <map>
#include "UILabel.h"
#include "UIChartDef.h"

namespace DuiLib
{
	class CCandlestickChartAxis;
	class CCandlestickVolumeUI;
	class CCrossStarUI;
	class CMovingAverageUI;

	class UILIB_API CCandlestickChartUI : public CContainerUI
	{
	public:
		enum
		{
			FLAG_CAN_ZOOM_IN = 0x01,
			FLAG_CAN_ZOOM_OUT = 0x02,
		};
	public:
		CCandlestickChartUI(void);
		~CCandlestickChartUI(void);

		LPCTSTR GetClass() const;
		LPVOID  GetInterface(LPCTSTR pstrName);
		void DoPaint(HDC hDC, const RECT& rcPaint);
		void DoEvent(TEventUI& event);
		virtual void SetPos(RECT rc);
	public:

		void SetDatas(const std::vector<TCandlestickChartData>& datas);
		const std::vector<TCandlestickChartData>& GetDatas() const;

		void SetNeedDrawLines(const std::map<size_t, Gdiplus::Color>& needDrawLines);
		const std::map<size_t, Gdiplus::Color>& GetNeedDrawLines() const;

		const std::map<size_t, std::vector<double>>& GetMovingAverageDatas() const;

		void RemoveAllData();

		int GetMaxDrawCounts() const;

		void SetZoomValue(double nZoomValue);
		double GetZoomValue() const;
		DWORD GetZoomFlag() const;

		bool CanZoomIn() const;
		bool CanZoomOut() const;

		virtual CControlUI* FindControl(FINDCONTROLPROC Proc, LPVOID pData, UINT uFlags);

		static double GetRectWidth(double dxScale);
	private:
		void OnMouseEnter(TEventUI& event);
		void OnMouseMove(TEventUI& event);
		void OnMouseLeave(TEventUI& event);

		void UpdateToolTip(const TCandlestickChartData& data);

	public:

		static const LONG sm_lblWidth;
		static const LONG sm_lblHeight;

		static const double sm_nMaxZoomVal;
		static const double sm_nMinZoomVal;

		static const double sm_nMaxDistancesBetweenItems;

	private:
		int				m_nMaxDrawItems;
		double			m_nZoomValue;

		CCandlestickChartAxis* m_pAxis;
		CMovingAverageUI* m_pMovingAverage;
		CCandlestickVolumeUI* m_pVolume;
		CCrossStarUI* m_pCrossStar;
		std::vector<TCandlestickChartData> m_datas;
		std::map<size_t, std::vector<double>> m_movingAverageVals;
		std::map<size_t, Gdiplus::Color> m_needDrawLines;
	};
}