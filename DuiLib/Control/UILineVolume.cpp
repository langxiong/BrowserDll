#include "StdAfx.h"
#include <algorithm>
#include "UILineVolume.h"
#include "UILineChart.h"

namespace DuiLib
{
	CLineVolumeUI::CLineVolumeUI():
		m_minDrawValue(0.0),
		m_maxDrawValue(0.0),
		m_iMinTick(25.0),
		m_iMaxTick(25.0),
		m_uStepTick(1.0),
		m_dxScale(0.0),
		m_dyScale(0.0),
		m_dwLineColor(0x333333),
		m_pOwner(NULL)
	{
		SetTickLimis(GetMinYTick(),GetMaxYTick());
		SetTextColor(m_dwLineColor);
		SetFont(0);
	}

	CLineVolumeUI::~CLineVolumeUI()
	{

	}

	void CLineVolumeUI::SetOwner(CLineChartUI* pOwner)
	{
		m_pOwner = pOwner;
	}

	void CLineVolumeUI::SetTickLimis(double minTick,double maxTick)
	{
		m_iMinTick	= minTick;
		m_iMaxTick	= maxTick;
		m_uStepTick	= (m_iMaxTick - m_iMinTick) / 3;
		m_sepYTicks.clear();
		if(m_uStepTick > 0.00001){
			for(double sepTick = m_iMinTick + m_uStepTick;sepTick < (maxTick - 0.00001) ;sepTick += m_uStepTick){
				m_sepYTicks.push_back(sepTick);
			}
		}
	}

	double CLineVolumeUI::GetMinYTick()
	{
		return m_iMinTick;
	}

	double CLineVolumeUI::GetMaxYTick()
	{
		return m_iMaxTick;
	}

	double CLineVolumeUI::GetSepYTick()
	{
		return m_uStepTick;
	}

	void CLineVolumeUI::CalAxisDataLimits()
	{
		CalMinMaxDrawValue();
		SetTickLimis(m_minDrawValue, m_maxDrawValue);
	}

	POINT CLineVolumeUI::GetOriginCoordinate() const
	{
		return m_ptOriCoorinate;
	}

	double CLineVolumeUI::GetXScale() const
	{
		return m_dxScale;
	}

	double CLineVolumeUI::GetYScale() const
	{
		return m_dyScale;
	}

	void CLineVolumeUI::Invalidate()
	{
		if (m_pOwner)
			m_pOwner->Invalidate();
	}

	void CLineVolumeUI::DrawVolume(HDC hDC, const RECT& rcPaint, RECT& rcItem)
	{
		if (!m_pOwner)
		{
			return;
		}

		rcItem.left		+= m_rcPadding.left;
		rcItem.top		+= m_rcPadding.top;
		rcItem.right	-= m_rcPadding.right;
		rcItem.bottom	-= m_rcPadding.bottom;

		RECT rcChartGraph = rcItem;
		rcChartGraph.left += CLineChartUI::sm_lblWidth;
		rcChartGraph.bottom -= CLineChartUI::sm_lblHeight / 2;

		m_ptOriCoorinate.x = rcChartGraph.left;
		m_ptOriCoorinate.y = rcChartGraph.bottom;

		const LONG nMaxGraphWidth = rcChartGraph.right - rcChartGraph.left;
		const LONG nMaxGraphHeight = rcChartGraph.bottom - rcChartGraph.top;

		auto& datas = m_pOwner->GetDatas();
		size_t nMaxDrawCount = (size_t)m_pOwner->GetMaxDrawCounts();

		m_dyScale = nMaxGraphHeight / (GetMaxYTick() - GetMinYTick());
		m_dxScale = (double)nMaxGraphWidth / nMaxDrawCount;
		DrawXAxis(hDC, rcChartGraph, m_dxScale);
		DrawYAxis(hDC, rcChartGraph, m_dyScale);

		if (datas.size() < 2)
		{
			return;
		}
		nMaxDrawCount = std::min<std::size_t>((size_t)nMaxDrawCount, datas.size());

		Graphics graphics(hDC);
		std::vector<Gdiplus::RectF> rects;
		const Color orange(0xfe, 0xff, 0xc0, 0x00);
		SolidBrush orangeBrush(orange);
		const double nRectWidth = 1.0;
		for (size_t i = 0; i < nMaxDrawCount; ++i)
		{
			auto& itemData = datas[i];
			REAL height = (itemData.m_volumeVal - GetMinYTick()) * m_dyScale;
			Gdiplus::RectF rc(m_ptOriCoorinate.x + i * m_dxScale, 
				m_ptOriCoorinate.y - height,
				nRectWidth, height);
			if (rc.Height < 1.0f)
			{
				rc.Height = 1.0f;
			}
			rects.emplace_back(rc);
		}
		if (!rects.empty())
		{
			graphics.FillRectangles(&orangeBrush, &rects[0], rects.size());
		}

	}

	void CLineVolumeUI::CalMinMaxDrawValue()
	{
		if (!m_pOwner)
		{
			return;
		}

		int nMaxDrawCount = m_pOwner->GetMaxDrawCounts();
		auto& datas = m_pOwner->GetDatas();
		auto minMaxIt = std::minmax_element(datas.begin(), datas.begin() + std::min<size_t>(datas.size(), (size_t)nMaxDrawCount), 
			[](const TLineChartData& lhs, const TLineChartData& rhs) -> bool
		{
			return lhs.m_volumeVal < rhs.m_volumeVal;
		});

		if (minMaxIt.first != datas.end())
		{
			m_minDrawValue = 0.0;
		}

		if (minMaxIt.second != datas.end())
		{
			m_maxDrawValue = minMaxIt.second->m_volumeVal;
		}
	}

	void CLineVolumeUI::DrawYAxis(HDC hDC, const RECT& rcGraph, double dyScale)
	{
		Graphics graphics(hDC);
		Pen pen(Color(0x33, 0x33, 0x33));
		{
			RECT rcYLine = rcGraph;
			rcYLine.top = rcYLine.bottom;

			graphics.DrawLine(&pen, rcYLine.left, rcYLine.top, rcYLine.right, rcYLine.bottom);

			rcYLine.left -= CLineChartUI::sm_lblWidth;
			rcYLine.right = rcYLine.left + CLineChartUI::sm_lblWidth;
			rcYLine.top	= rcYLine.top - CLineChartUI::sm_lblHeight / 2;
			rcYLine.bottom	= rcYLine.top + CLineChartUI::sm_lblHeight;

			CRenderEngine::DrawText(hDC, m_pManager, rcYLine, GetLblText(GetMinYTick()),
				GetTextColor(), GetFont(), DT_RIGHT | DT_VCENTER);

			rcYLine = rcGraph;
			rcYLine.bottom = rcYLine.top;
			graphics.DrawLine(&pen, rcYLine.left, rcYLine.top, rcYLine.right, rcYLine.bottom);
			rcYLine.left -= CLineChartUI::sm_lblWidth;
			rcYLine.right = rcYLine.left + CLineChartUI::sm_lblWidth;
			rcYLine.top	= rcYLine.top - CLineChartUI::sm_lblHeight / 2;
			rcYLine.bottom	= rcYLine.top + CLineChartUI::sm_lblHeight;
			CRenderEngine::DrawText(hDC, m_pManager, rcYLine, GetLblText(GetMaxYTick()),
				GetTextColor(), GetFont(), DT_RIGHT | DT_VCENTER);
		}

		pen.SetDashStyle(DashStyleDash);
		for (auto it = m_sepYTicks.cbegin(); it != m_sepYTicks.cend(); ++it)
		{
			double dy = rcGraph.bottom - ((*it) - GetMinYTick()) * dyScale;
			RECT rcYLine = rcGraph;
			graphics.DrawLine(&pen, rcYLine.left, dy, rcYLine.right, dy);
			rcYLine.top = dy;
			rcYLine.left -= CLineChartUI::sm_lblWidth;
			rcYLine.right = rcYLine.left + CLineChartUI::sm_lblWidth;
			rcYLine.top	= rcYLine.top - CLineChartUI::sm_lblHeight / 2;
			rcYLine.bottom	= rcYLine.top + CLineChartUI::sm_lblHeight;
			CRenderEngine::DrawText(hDC, m_pManager, rcYLine, GetLblText(*it),
				GetTextColor(), GetFont(), DT_RIGHT | DT_VCENTER);
		}
	}

	void CLineVolumeUI::DrawXAxis(HDC hDC, const RECT& rcGraph, double dyScale)
	{
		Graphics graphics(hDC);
		Pen pen(Color(0x33, 0x33, 0x33));
		{
			RECT rcXLine = rcGraph;
			rcXLine.right = rcXLine.left;
			graphics.DrawLine(&pen, rcXLine.left, rcXLine.top, rcXLine.right, rcXLine.bottom);
			rcXLine.left = rcXLine.right = rcGraph.right;
			graphics.DrawLine(&pen, rcXLine.left, rcXLine.top, rcXLine.right, rcXLine.bottom);
		}
	}

	CDuiString CLineVolumeUI::GetLblText(double nValue)
	{
		CDuiString ret;
		if (nValue > 10000)
		{
			ret.Format(_T("%2.2f%s"), nValue / 10000, _T("Íò"));
		}
		else
		{
			long tmpValue = static_cast<long>(nValue);
			ret.Format(_T("%d"), tmpValue);
		}
		return ret;
	}

}