#include "StdAfx.h"
#include "UILineChart.h"
#include "UILineVolume.h"
#include "UICrossStart.h"
#include <algorithm>
#include <string>

namespace DuiLib
{
	std::wstring GetStockTradeMinByTimePoint(size_t nTimePoint)
	{
		if (nTimePoint > 240)
		{
			return std::wstring();
		}

		std::wstring ret;
		ret.resize(6);
		if (nTimePoint < 30)
		{
			swprintf_s(&ret[0], ret.size(), L"09:%02d", 30 + nTimePoint);
		}
		else if (nTimePoint < 90)
		{
			swprintf_s(&ret[0], ret.size(), L"10:%02d", nTimePoint - 30);
		}
		else if (nTimePoint < 120)
		{
			swprintf_s(&ret[0], ret.size(), L"11:%02d", nTimePoint - 90);
		}
		else if (nTimePoint < 180)
		{
			swprintf_s(&ret[0], ret.size(), L"13:%02d", nTimePoint - 120);
		}
		else if (nTimePoint < 240)
		{
			swprintf_s(&ret[0], ret.size(), L"14:%02d", nTimePoint - 180);
		}
		else if (nTimePoint == 240)
		{
			return std::wstring(L"15:00");
		}
		return ret;
	}

	const LONG CLineChartUI::sm_lblWidth = 50;

	const LONG CLineChartUI::sm_lblHeight = 17;

	const double CLineChartUI::sm_nMaxZoomVal = 4.1;

	const double CLineChartUI::sm_nMinZoomVal = 0.0626;

	const double CLineChartUI::sm_nDistancesBetweenItems = 1.0;

	CLineChartAxis::CLineChartAxis():
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
		// 将分时图化作9:30~11:30 13:00~15:00
		m_sepXTicks[0] = _T("9:30");
		m_sepXTicks[60] = _T("10:30");
		m_sepXTicks[120] = _T("13:00");
		m_sepXTicks[180] = _T("14:00");
		m_sepXTicks[240] = _T("15:00");
		SetTickLimis(GetMinYTick(),GetMaxYTick());
		SetTextColor(m_dwLineColor);
		SetFont(0);
	}

	CLineChartAxis::~CLineChartAxis()
	{

	}

	void CLineChartAxis::SetChartOwner(CLineChartUI* pOwner)
	{
		m_pOwner = pOwner;
	}

	void CLineChartAxis::SetTickLimis(double minTick, double maxTick)
	{
		m_iMinTick	= minTick;
		m_iMaxTick	= maxTick;
		m_uStepTick	= (m_iMaxTick - m_iMinTick) / 5;
		m_sepYTicks.clear();
		if(m_uStepTick > 0.00001){
			for(double sepTick = m_iMinTick + m_uStepTick;sepTick < (m_iMaxTick - 0.00001) ;sepTick += m_uStepTick){
				m_sepYTicks.push_back(sepTick);
			}
		}
	}

	double CLineChartAxis::GetMinYTick()
	{
		return m_iMinTick;
	}

	double CLineChartAxis::GetMaxYTick()
	{
		return m_iMaxTick;
	}

	double CLineChartAxis::GetSepYTick()
	{
		return m_uStepTick;
	}

	void CLineChartAxis::ResetTick()
	{
		m_iMaxTick		= 0.0;
		m_iMinTick		= 0.0;
		m_uStepTick		= 1.0;

		SetTickLimis(GetMinYTick(),GetMaxYTick());
	}

	void CLineChartAxis::Invalidate()
	{
		if (m_pOwner)
			m_pOwner->Invalidate();
	}

	void CLineChartAxis::DrawAxis( HDC hDC, const RECT& rcPaint,RECT& rcItem )
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
		rcChartGraph.top += CLineChartUI::sm_lblHeight / 2;
		rcChartGraph.bottom -= CLineChartUI::sm_lblHeight;

		m_ptOriCoorinate.x = rcChartGraph.left;
		m_ptOriCoorinate.y = rcChartGraph.bottom;

		const LONG nMaxGraphWidth = rcChartGraph.right - rcChartGraph.left;
		const LONG nMaxGraphHeight = rcChartGraph.bottom - rcChartGraph.top;

		auto& datas = m_pOwner->GetDatas();
		size_t nMaxDrawCount = m_pOwner->GetMaxDrawCounts();

		m_dyScale = nMaxGraphHeight / (GetMaxYTick() - GetMinYTick());
		m_dxScale = (double)nMaxGraphWidth / nMaxDrawCount;
		DrawXAxis(hDC, rcChartGraph, m_dxScale);
		DrawYAxis(hDC, rcChartGraph, m_dyScale);
		if (datas.size() < 2)
		{
			return;
		}
		nMaxDrawCount = std::min<std::size_t>((size_t)nMaxDrawCount, datas.size());
		Gdiplus::Graphics graphics(hDC);
		Gdiplus::Pen pen(Gdiplus::Color(0xe1, 0xe1, 0xe1), (Gdiplus::REAL)1.0);
		std::vector<Gdiplus::PointF> pts(nMaxDrawCount);

		Gdiplus::Pen orangePen(Gdiplus::Color(0xff, 0xc0, 0x00), (Gdiplus::REAL)1.0);
		std::vector<Gdiplus::PointF> ptsAverage(nMaxDrawCount);

		long totalAmount = 0;
		double totalPrice = 0.0f;
		{
			for (size_t i = 0; i < nMaxDrawCount; ++i)
			{
				auto& itemData = datas[i];

				Gdiplus::PointF pt(m_ptOriCoorinate.x + i * m_dxScale,
					m_ptOriCoorinate.y - (itemData.m_nValue - GetMinYTick()) * m_dyScale);

				totalPrice += itemData.m_nValue * itemData.m_volumeVal;
				totalAmount += itemData.m_volumeVal;
				pts[i] = pt;
				Gdiplus::PointF ptAverage(pt.X,
					m_ptOriCoorinate.y - (totalPrice / totalAmount - GetMinYTick()) * m_dyScale);
				ptsAverage[i] = ptAverage;
			}
		}

		if (!ptsAverage.empty())
		{
			graphics.DrawLines(&orangePen, &ptsAverage[0], ptsAverage.size());
		}

		if (!pts.empty())
		{
			graphics.DrawLines(&pen, &pts[0], pts.size());
		}
	}

	CDuiString CLineChartAxis::GetLblText(double nValue)
	{
		CDuiString ret;
		if (nValue > 1000)
		{
			ret.Format(_T("%2.2f%s"), nValue / 1000, _T("K"));
		}
		else
		{
			ret.Format(_T("%2.2f%s"), nValue, _T(""));
		}
		return ret;
	}

	void CLineChartAxis::DrawYAxis(HDC hDC, const RECT& rcGraph, double dyScale)
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

	void CLineChartAxis::DrawXAxis(HDC hDC, const RECT& rcGraph, double dxScale)
	{
		Graphics graphics(hDC);

		for (auto it = m_sepXTicks.cbegin(); it != m_sepXTicks.cend(); ++it)
		{
			Pen pen(Color(0x33, 0x33, 0x33));
			if (it->first == 0 || it->first == 240)
			{
			}
			else
			{
				pen.SetDashStyle(DashStyleDash);
			}
			double dx = rcGraph.left + it->first * dxScale;
			RECT rcXLine = rcGraph;
			graphics.DrawLine(&pen, dx, rcXLine.top, dx, rcXLine.bottom);
			rcXLine.left = (LONG)dx - CLineChartUI::sm_lblWidth / 2;
			rcXLine.right = rcXLine.left + CLineChartUI::sm_lblWidth;
			rcXLine.top	= rcXLine.bottom;
			rcXLine.bottom	= rcXLine.top + CLineChartUI::sm_lblHeight;
			CRenderEngine::DrawText(hDC, m_pManager, rcXLine, it->second,
				GetTextColor(), GetFont(), DT_RIGHT | DT_BOTTOM);
		}
	}

	void CLineChartAxis::CalMinMaxDrawValue()
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
			return lhs.m_nValue < rhs.m_nValue;
		});

		if (minMaxIt.first != datas.end())
		{
			m_minDrawValue = minMaxIt.first->m_nValue;
		}

		if (minMaxIt.second != datas.end())
		{
			m_maxDrawValue = minMaxIt.second->m_nValue;
		}

		// 对数据做修复，防止绘制时异常
		if (m_maxDrawValue - m_minDrawValue < 0.05)
		{
			m_minDrawValue = m_maxDrawValue > 0.05 ? m_maxDrawValue - 0.05 : m_minDrawValue;
		}
	}

	void CLineChartAxis::CalAxisDataLimits()
	{
		CalMinMaxDrawValue();
		SetTickLimis(m_minDrawValue, m_maxDrawValue);
	}


	POINT CLineChartAxis::GetOriginCoordinate() const
	{
		return m_ptOriCoorinate;
	}

	double CLineChartAxis::GetXScale() const
	{
		return m_dxScale;
	}


	double CLineChartAxis::GetYScale() const
	{
		return m_dyScale;
	}

	CLineChartUI::CLineChartUI(void):
		m_nMaxDrawItems(240),
		m_nZoomValue(1.0),
		m_pAxis(new CLineChartAxis),
		m_pVolume(new CLineVolumeUI),
		m_pCrossStar(new CCrossStarUI)
	{
		m_pAxis->SetChartOwner(this);
		m_pVolume->SetOwner(this);
		Add(m_pAxis);
		Add(m_pVolume);
		Add(m_pCrossStar);
	}


	CLineChartUI::~CLineChartUI(void)
	{
	
	}

	LPCTSTR CLineChartUI::GetClass() const
	{
		return _T("ChartLineViewUI");
	}

	LPVOID CLineChartUI::GetInterface( LPCTSTR pstrName )
	{
		if( _tcscmp(pstrName, DUI_CTR_LINECHART) == 0 ) return static_cast<CLineChartUI*>(this);
		else return CContainerUI::GetInterface(pstrName);
	}

	void CLineChartUI::DoPaint( HDC hDC, const RECT& rcPaint )
	{
		if( !::IntersectRect(&m_rcPaint, &rcPaint, &m_rcItem) ) return;
		RECT rcItem = m_rcItem;
		LONG height = rcItem.bottom - rcItem.top;
		if (m_pAxis->IsVisible())
		{
			rcItem.bottom = rcItem.top + height * 7 / 10;
			m_pAxis->DrawAxis(hDC,rcPaint,rcItem);
		}
		if (m_pVolume->IsVisible())
		{
			rcItem.top = rcItem.bottom;
			rcItem.bottom = m_rcItem.bottom;
			m_pVolume->DrawVolume(hDC,rcPaint,rcItem);
		}
		if (m_pCrossStar->IsVisible())
		{
			m_pCrossStar->DoPaint(hDC, m_rcItem);
		}
	}


	void CLineChartUI::DoEvent(TEventUI& event)
	{
		if (!IsMouseEnabled()) 
		{
			CContainerUI::DoEvent(event);
			return;
		}

		if (event.Type == UIEVENT_MOUSEENTER)
		{
			OnMouseEnter(event);
			return;
		}

		if (event.Type == UIEVENT_MOUSELEAVE)
		{
			OnMouseLeave(event);
			return;
		}

		if (event.Type == UIEVENT_MOUSEMOVE)
		{
			OnMouseMove(event);
			return;
		}

		CContainerUI::DoEvent(event);
	}


	void CLineChartUI::SetDatas(const std::vector<TLineChartData>& datas)
	{
		m_datas = datas;
		m_pAxis->CalAxisDataLimits();
		m_pVolume->CalAxisDataLimits();
		Invalidate();
	}


	const std::vector<TLineChartData>& CLineChartUI::GetDatas() const
	{
		return m_datas;
	}

	void CLineChartUI::SetZoomValue(double nZoomValue)
	{
		m_nZoomValue = nZoomValue;
		m_pAxis->ResetTick();
		m_pAxis->CalAxisDataLimits();
		m_pVolume->CalAxisDataLimits();
		if (m_pManager)
		{
			WPARAM flag = 0;
			if (CanZoomIn())
			{
				flag |= FLAG_CAN_ZOOM_IN;
			}
			if (CanZoomOut())
			{
				flag |= FLAG_CAN_ZOOM_OUT;
			}
			m_pManager->SendNotify(this, DUI_MSGTYPE_ZOOMSTATUSCHANGED, flag);
		}
		Invalidate();
	}	

	double CLineChartUI::GetZoomValue() const
	{
		return m_nZoomValue;
	}

	int CLineChartUI::GetMaxDrawCounts() const
	{
		return static_cast<int>(m_nZoomValue * m_nMaxDrawItems);
	}

	bool CLineChartUI::CanZoomIn() const
	{
		return m_nZoomValue / 2 > CLineChartUI::sm_nMinZoomVal;
	}

	bool CLineChartUI::CanZoomOut() const
	{
		return m_nZoomValue * 2 < CLineChartUI::sm_nMaxZoomVal;
	}

	void CLineChartUI::OnMouseEnter(TEventUI& event)
	{
		if (!m_pCrossStar)
		{
			return;
		}
		m_pCrossStar->SetVisible(true);
	}

	void CLineChartUI::OnMouseMove(TEventUI& event)
	{
		POINT pt = event.ptMouse;

		POINT ptOriginCoordinate = m_pAxis->GetOriginCoordinate();
		POINT ptVolume = m_pVolume->GetOriginCoordinate();
		if (pt.x <= ptVolume.x || pt.y >= ptVolume.y)
		{
			return;
		}

		LONG dx = pt.x - ptOriginCoordinate.x;
		
		const size_t nIndex = (pt.x - ptOriginCoordinate.x) / m_pAxis->GetXScale();

		size_t nMaxDrawCount = GetMaxDrawCounts();
		nMaxDrawCount = std::min<std::size_t>((size_t)nMaxDrawCount, m_datas.size());

		if (nIndex >= nMaxDrawCount)
		{
			return;
		}

		TLineChartData& itemData = m_datas[nIndex];
		{
			CDuiString strText;
			strText.Format(_T("%2.2f"), itemData.m_nValue);
			m_pCrossStar->SetYLabelText(strText);

			m_pCrossStar->SetXLabelText(GetStockTradeMinByTimePoint(nIndex).c_str());
		}

		{
			RECT rcHorXLine = m_rcItem;
			rcHorXLine.top = ptOriginCoordinate.y - (LONG)((itemData.m_nValue - m_pAxis->GetMinYTick()) * m_pAxis->GetYScale());
			rcHorXLine.bottom = rcHorXLine.top;
			rcHorXLine.left = ptOriginCoordinate.x;
			m_pCrossStar->SetHorXLinePos(rcHorXLine);
		}

		{
			RECT rcVerYLine = m_rcItem;
			rcVerYLine.left = ptOriginCoordinate.x + nIndex * m_pAxis->GetXScale();
			rcVerYLine.right = rcVerYLine.left;
			rcVerYLine.top += CLineChartUI::sm_lblHeight / 2;
			rcVerYLine.bottom = ptVolume.y;
			m_pCrossStar->SetVerYLinePos(rcVerYLine);
		}
	}

	void CLineChartUI::OnMouseLeave(TEventUI& event)
	{
		if (!m_pCrossStar)
		{
			return;
		}
		m_pCrossStar->SetVisible(false);
	}

	CControlUI* CLineChartUI::FindControl(FINDCONTROLPROC Proc, LPVOID pData, UINT uFlags)
	{
		return CControlUI::FindControl(Proc, pData, uFlags);
	}

	void CLineChartUI::RemoveAllData()
	{
		m_datas.clear();
	}

	DWORD CLineChartUI::GetZoomFlag() const
	{
		DWORD flag = 0;
		if (CanZoomIn())
		{
			flag |= FLAG_CAN_ZOOM_IN;
		}
		if (CanZoomOut())
		{
			flag |= FLAG_CAN_ZOOM_OUT;
		}
		return flag;
	}

}

