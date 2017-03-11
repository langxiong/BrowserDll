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

namespace DuiLib
{
	class CLineChartUI;
	class CCrossStarUI;
	class CLineVolumeUI;

	struct UILIB_API TLineChartData
	{
		double m_nValue;
		long m_volumeVal;
	};

	class UILIB_API CLineChartAxis : public CLabelUI
	{
	public:
		CLineChartAxis();
		~CLineChartAxis();

	public:
		void SetChartOwner(CLineChartUI* pOwner);
		void SetTickLimis(double minTick,double maxTick);
		double GetMinYTick();
		double GetMaxYTick();
		double GetSepYTick();
		void ResetTick();

		void CalAxisDataLimits();
		POINT GetOriginCoordinate() const;

		double GetXScale() const;
		double GetYScale() const;

		void SetVisibleBaseLine(bool _Visible);
		bool GetVisibleBaseLine();

		void Invalidate();
		void DrawAxis(HDC hDC, const RECT& rcPaint,RECT& rcItem);
		void CalMinMaxDrawValue();

		static CDuiString GetLblText(double nValue);

	private:

		void DrawYAxis(HDC hDC, const RECT& rcGraph, double dyScale);
		void DrawXAxis(HDC hDC, const RECT& rcGraph, double dyScale);

	private:
		double			m_minDrawValue;
		double          m_maxDrawValue;
		double			m_iMinTick;
		double			m_iMaxTick;
		double			m_uStepTick;
		DWORD			m_dwLineColor;
		double			m_dxScale;
		double			m_dyScale;
		std::map<long, CDuiString> m_sepXTicks;
		std::vector<double> m_sepYTicks;
		CDuiPoint		m_ptOriCoorinate;
		CLineChartUI*	m_pOwner;
	};

	class UILIB_API CLineChartUI : public CContainerUI
	{
	public:
		enum
		{
			FLAG_CAN_ZOOM_IN = 0x01,
			FLAG_CAN_ZOOM_OUT = 0x02,
		};

		enum
		{
			CHARTS_VOLUME = 0x01,
			CHARTS_AVERAGE_LINE = 0x02,
		};
	public:
		CLineChartUI(void);
		~CLineChartUI(void);

		LPCTSTR GetClass() const;
		LPVOID  GetInterface(LPCTSTR pstrName);
		void DoPaint(HDC hDC, const RECT& rcPaint);
		void DoEvent(TEventUI& event);
	public:

		void SetDatas(const std::vector<TLineChartData>& datas);
		const std::vector<TLineChartData>& GetDatas() const;

		void RemoveAllData();

		void SetZoomValue(double nZoomValue);
		double GetZoomValue() const;
		DWORD GetZoomFlag() const;

		bool CanZoomIn() const;
		bool CanZoomOut() const;

		virtual CControlUI* FindControl(FINDCONTROLPROC Proc, LPVOID pData, UINT uFlags);

		int GetMaxDrawCounts() const;
	public:
		static const LONG sm_lblWidth;
		static const LONG sm_lblHeight;

		static const double sm_nMaxZoomVal;
		static const double sm_nMinZoomVal;

		static const double sm_nDistancesBetweenItems;
	private:
		void OnMouseEnter(TEventUI& event);
		void OnMouseMove(TEventUI& event);
		void OnMouseLeave(TEventUI& event);

	private:
		int				m_nMaxDrawItems;
		double			m_nZoomValue;

		CLineChartAxis* m_pAxis;
		CLineVolumeUI* m_pVolume;
		CCrossStarUI* m_pCrossStar;
		std::vector<TLineChartData> m_datas;
	};
}