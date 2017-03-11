#include "StdAfx.h"
#include <algorithm>
#include "UICandlestickVolume.h"
#include "UICandlestickChart.h"

namespace DuiLib
{
	CCandlestickVolumeUI::CCandlestickVolumeUI():
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

	CCandlestickVolumeUI::~CCandlestickVolumeUI()
	{

	}

	void CCandlestickVolumeUI::SetOwner(CCandlestickChartUI* pOwner)
	{
		m_pOwner = pOwner;
	}

	void CCandlestickVolumeUI::SetTickLimis(double minTick,double maxTick)
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

	double CCandlestickVolumeUI::GetMinYTick()
	{
		return m_iMinTick;
	}

	double CCandlestickVolumeUI::GetMaxYTick()
	{
		return m_iMaxTick;
	}

	double CCandlestickVolumeUI::GetSepYTick()
	{
		return m_uStepTick;
	}

	void CCandlestickVolumeUI::CalAxisDataLimits()
	{
		CalMinMaxDrawValue();
		SetTickLimis(m_minDrawValue, m_maxDrawValue);
	}

	CDuiPoint CCandlestickVolumeUI::GetOriginCoordinate() const
	{
		return CDuiPoint(m_rcChartGraph.left, m_rcChartGraph.bottom);
	}

	double CCandlestickVolumeUI::GetXScale() const
	{
		return m_dxScale;
	}

	double CCandlestickVolumeUI::GetYScale() const
	{
		return m_dyScale;
	}

	void CCandlestickVolumeUI::Invalidate()
	{
		if (m_pOwner)
			m_pOwner->Invalidate();
	}

	void CCandlestickVolumeUI::DoPaint(HDC hDC, const RECT& rcPaint)
	{
		if (!m_pOwner)
		{
			return;
		}
		RECT rcItem = m_rcItem;
		rcItem.left		+= m_rcPadding.left;
		rcItem.top		+= m_rcPadding.top;
		rcItem.right	-= m_rcPadding.right;
		rcItem.bottom	-= m_rcPadding.bottom;

		m_rcChartGraph = rcItem;
		m_rcChartGraph.left += CCandlestickChartUI::sm_lblWidth;
		m_rcChartGraph.top += CCandlestickChartUI::sm_lblHeight;
		m_rcChartGraph.bottom -= CCandlestickChartUI::sm_lblHeight / 2;

		CDuiPoint ptOriCoorinate(m_rcChartGraph.left, m_rcChartGraph.bottom);

		const LONG nMaxGraphWidth = m_rcChartGraph.right - m_rcChartGraph.left;
		const LONG nMaxGraphHeight = m_rcChartGraph.bottom - m_rcChartGraph.top;

		auto& datas = m_pOwner->GetDatas();
		size_t nMaxDrawCount = (size_t)m_pOwner->GetMaxDrawCounts();
		const size_t firstCount = nMaxDrawCount < datas.size() ? (datas.size() - nMaxDrawCount) : 0;
		nMaxDrawCount = std::min<std::size_t>((size_t)nMaxDrawCount, datas.size());

		m_dyScale = nMaxGraphHeight / (GetMaxYTick() - GetMinYTick());
		m_dxScale = (double)nMaxGraphWidth / nMaxDrawCount;
		DrawXAxis(hDC, m_rcChartGraph, m_dxScale);
		DrawYAxis(hDC, m_rcChartGraph, m_dyScale);

		if (datas.size() < 2)
		{
			return;
		}

		Graphics graphics(hDC);
		std::vector<Gdiplus::RectF> upRects;
		std::vector<Gdiplus::RectF> downRects;
		const Color red(0xff, 0x00, 0x00);
		const Color green(0xfe, 0x00, 0x8c, 0x00);
		SolidBrush greenBrush(green);

		Pen redPen(red);
		Pen greenPen(green);
		const double nRectWidth = CCandlestickChartUI::GetRectWidth(m_dxScale);;
		for (size_t i = 0; i < nMaxDrawCount; ++i)
		{
			auto& itemData = datas[i + firstCount];
			REAL height = (itemData.m_transVolume - GetMinYTick()) * m_dyScale;
			if (itemData.IsRose())
			{
				Gdiplus::RectF rc(ptOriCoorinate.x + i * m_dxScale, 
					ptOriCoorinate.y - height,
					nRectWidth, height);
				if (rc.Height < 1.0f)
				{
					rc.Height = 1.0f;
				}
				upRects.emplace_back(rc);
			}
			else
			{
				Gdiplus::RectF rc(ptOriCoorinate.x + i * m_dxScale, 
					ptOriCoorinate.y - height,
					nRectWidth, height);
				if (rc.Height < 1.0f)
				{
					rc.Height = 1.0f;
				}
				downRects.emplace_back(rc);
			}
		}
		if (!upRects.empty())
		{
			graphics.DrawRectangles(&redPen, &upRects[0], upRects.size());
		}

		if (!downRects.empty())
		{
			graphics.FillRectangles(&greenBrush, &downRects[0], downRects.size());
		}
	}

	void CCandlestickVolumeUI::CalMinMaxDrawValue()
	{
		if (!m_pOwner)
		{
			return;
		}

		auto& datas = m_pOwner->GetDatas();

		size_t nMaxDrawCount = (size_t)m_pOwner->GetMaxDrawCounts();
		const size_t firstCount = nMaxDrawCount < datas.size() ? (datas.size() - nMaxDrawCount) : 0;
		nMaxDrawCount = std::min<std::size_t>((size_t)nMaxDrawCount, datas.size());
		auto minMaxIt = std::minmax_element(datas.begin() + firstCount, datas.begin() + firstCount + nMaxDrawCount, 
			[](const TCandlestickChartData& lhs, const TCandlestickChartData& rhs) -> bool
		{
			return lhs.m_transVolume < rhs.m_transVolume;
		});

		if (minMaxIt.first != datas.end())
		{
			m_minDrawValue = 0.0;
		}

		if (minMaxIt.second != datas.end())
		{
			m_maxDrawValue = minMaxIt.second->m_transVolume;
		}
	}

	void CCandlestickVolumeUI::DrawYAxis(HDC hDC, const RECT& rcGraph, double dyScale)
	{
		Graphics graphics(hDC);
		Pen pen(Color(0x33, 0x33, 0x33));
		{
			RECT rcYLine = rcGraph;
			rcYLine.top = rcYLine.bottom;

			graphics.DrawLine(&pen, rcYLine.left, rcYLine.top, rcYLine.right, rcYLine.bottom);

			rcYLine.left -= CCandlestickChartUI::sm_lblWidth;
			rcYLine.right = rcYLine.left + CCandlestickChartUI::sm_lblWidth;
			rcYLine.top	= rcYLine.top - CCandlestickChartUI::sm_lblHeight / 2;
			rcYLine.bottom	= rcYLine.top + CCandlestickChartUI::sm_lblHeight;

			CRenderEngine::DrawText(hDC, m_pManager, rcYLine, GetLblText(GetMinYTick()),
				GetTextColor(), GetFont(), DT_RIGHT | DT_VCENTER);

			rcYLine = rcGraph;
			rcYLine.bottom = rcYLine.top;
			graphics.DrawLine(&pen, rcYLine.left, rcYLine.top, rcYLine.right, rcYLine.bottom);
			rcYLine.left -= CCandlestickChartUI::sm_lblWidth;
			rcYLine.right = rcYLine.left + CCandlestickChartUI::sm_lblWidth;
			rcYLine.top	= rcYLine.top - CCandlestickChartUI::sm_lblHeight / 2;
			rcYLine.bottom	= rcYLine.top + CCandlestickChartUI::sm_lblHeight;
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
			rcYLine.left -= CCandlestickChartUI::sm_lblWidth;
			rcYLine.right = rcYLine.left + CCandlestickChartUI::sm_lblWidth;
			rcYLine.top	= rcYLine.top - CCandlestickChartUI::sm_lblHeight / 2;
			rcYLine.bottom	= rcYLine.top + CCandlestickChartUI::sm_lblHeight;
			CRenderEngine::DrawText(hDC, m_pManager, rcYLine, GetLblText(*it),
				GetTextColor(), GetFont(), DT_RIGHT | DT_VCENTER);
		}
	}

	void CCandlestickVolumeUI::DrawXAxis(HDC hDC, const RECT& rcGraph, double dyScale)
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

	CDuiString CCandlestickVolumeUI::GetLblText(double nValue)
	{
		CDuiString ret;
		if (nValue > 10000)
		{
			ret.Format(_T("%2.2f%s"), nValue / 10000, _T("Íò"));
		}
		else
		{
			if (nValue <= 0.000001)
			{
				return _T("ÊÖ");
			}
			ret.Format(_T("%2.2f%s"), nValue, _T(""));
		}
		return ret;
	}

}