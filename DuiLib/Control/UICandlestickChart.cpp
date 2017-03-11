#include "StdAfx.h"
#include "UICandlestickChart.h"
#include "UICandlestickChartAxis.h"
#include "UICandlestickVolume.h"
#include "UICrossStart.h"
#include "UIMovingAverage.h"
#include "../Algorithm/MAMethod.h"
#include "../Algorithm/RSIMethod.h"

#include <algorithm>
#include <numeric>

using namespace MyAlgorithm;

std::map<size_t, std::vector<double>> CalAverage(const std::vector<DuiLib::TCandlestickChartData>& datas, 
	size_t nStartIndex, const std::vector<size_t>& arr)
{
	std::map<size_t, std::vector<double>> ret;
	if (datas.empty())
	{
		return ret;
	}
	for (auto itArr = arr.cbegin(); itArr != arr.cend(); ++itArr)
	{
		const size_t tmpDistance = *itArr;
		std::vector<DuiLib::TCandlestickChartData> tmpDatas(tmpDistance - 1, datas.front());
		tmpDatas.insert(tmpDatas.end(), datas.begin(), datas.end());
		auto firstIt = tmpDatas.begin();
		if (nStartIndex > tmpDistance)
		{
			std::advance(firstIt, nStartIndex - tmpDistance);
		}

		std::vector<double> initQueue;
		auto endIt = tmpDatas.end();
		if (std::distance(firstIt, tmpDatas.end()) <= tmpDistance)
		{
			for (auto it = firstIt; it != endIt; ++it)
			{
				initQueue.push_back(it->m_endVal);
			}
			CEMAMethod calAverage(initQueue);
			ret[tmpDistance].push_back(calAverage.GetAverageVal());
		}
		else
		{
			for (auto it = firstIt; it != firstIt + tmpDistance; ++it)
			{
				initQueue.push_back(it->m_endVal);
			}
			CEMAMethod calAverage(initQueue);
			ret[tmpDistance].push_back(calAverage.GetAverageVal());
			for (auto it = firstIt + tmpDistance; it != endIt; ++it)
			{
				ret[tmpDistance].push_back(calAverage.Update(it->m_endVal));
			}
		}
	}

	for (size_t szArr = 0; szArr < arr.size(); ++szArr)
	{
		size_t tmpDistance = arr[szArr];
		auto firstIt = datas.begin();
		if (nStartIndex > tmpDistance)
		{
			std::advance(firstIt, nStartIndex - tmpDistance);
		}

		std::vector<double> initQueue;
		auto endIt = datas.end();
		if (std::distance(firstIt, datas.end()) <= tmpDistance)
		{
			for (auto it = firstIt; it != endIt; ++it)
			{
				initQueue.push_back(it->m_endVal);
			}
			CEMAMethod calAverage(initQueue);
			ret[tmpDistance].push_back(calAverage.GetAverageVal());
		}
		else
		{
			for (auto it = firstIt; it != firstIt + tmpDistance; ++it)
			{
				initQueue.push_back(it->m_endVal);
			}
			CEMAMethod calAverage(initQueue);
			ret[tmpDistance].push_back(calAverage.GetAverageVal());
			for (auto it = firstIt + tmpDistance + 1; it != endIt; ++it)
			{
				ret[tmpDistance].push_back(calAverage.Update(it->m_endVal));
			}
		}
	}
	return std::move(ret);
}



using namespace Gdiplus;

namespace DuiLib
{
	const LONG CCandlestickChartUI::sm_lblWidth = 65;

	const LONG CCandlestickChartUI::sm_lblHeight = 17;

	const double CCandlestickChartUI::sm_nMaxZoomVal = 4.1;

	const double CCandlestickChartUI::sm_nMinZoomVal = 0.0626;

	const double CCandlestickChartUI::sm_nMaxDistancesBetweenItems = 5.0;

	CCandlestickChartUI::CCandlestickChartUI(void):
		m_nMaxDrawItems(64),
		m_nZoomValue(1.0),
		m_pAxis(new CCandlestickChartAxis),
		m_pMovingAverage(new CMovingAverageUI),
		m_pVolume(new CCandlestickVolumeUI),
		m_pCrossStar(new CCrossStarUI)
	{
		// m_needDrawLines.insert(std::make_pair(5, Gdiplus::Color::Orange));
		// m_needDrawLines.insert(std::make_pair(10, Gdiplus::Color::Yellow));
		// m_needDrawLines.insert(std::make_pair(20, Gdiplus::Color::Purple));
		// m_needDrawLines.insert(std::make_pair(60, Gdiplus::Color::Green));
		// m_needDrawLines.insert(std::make_pair(120, Gdiplus::Color::LightBlue));

		m_needDrawLines.insert(std::make_pair(12, Gdiplus::Color::Red));
		m_needDrawLines.insert(std::make_pair(26, Gdiplus::Color::Green));

		m_pAxis->SetOwner(this);
		m_pMovingAverage->SetOwner(this);
		m_pVolume->SetOwner(this);
		m_pAxis->SetAttribute(_T("float"), _T("0, 0, 1, 0.66"));
		RECT padding = {0, 0, 5, 0};
		m_pAxis->SetPadding(padding);
		m_pVolume->SetAttribute(_T("float"), _T("0, 0.66, 1, 1"));
		m_pVolume->SetPadding(padding);
		Add(m_pAxis);
		Add(m_pVolume);
		Add(m_pCrossStar);
		Add(m_pMovingAverage);
	}

	CCandlestickChartUI::~CCandlestickChartUI(void)
	{
	
	}

	LPCTSTR CCandlestickChartUI::GetClass() const
	{
		return _T("ChartLineViewUI");
	}

	LPVOID CCandlestickChartUI::GetInterface( LPCTSTR pstrName )
	{
		if( _tcscmp(pstrName, DUI_CTR_CANDLESTICKCHART) == 0 ) return static_cast<CCandlestickChartUI*>(this);
		else return CContainerUI::GetInterface(pstrName);
	}

	void CCandlestickChartUI::DoPaint( HDC hDC, const RECT& rcPaint )
	{
		return CContainerUI::DoPaint(hDC, rcPaint);
	}

	void CCandlestickChartUI::DoEvent(TEventUI& event)
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

		if (event.Type == UIEVENT_SETCURSOR)
		{
			::SetCursor(::LoadCursor(NULL, MAKEINTRESOURCE(IDC_CROSS)));
			return;
		}


		CContainerUI::DoEvent(event);
	}


	void CCandlestickChartUI::SetDatas(const std::vector<TCandlestickChartData>& datas)
	{
		m_datas = datas;
		std::vector<size_t> arr;
		for (auto it = m_needDrawLines.cbegin(); it != m_needDrawLines.cend(); ++it)
		{
			arr.push_back(it->first);
		}
		/* 计算 均线的方法
		An = {X0, X1, ..., X(n-1)}
		==>(向前填充数据)
		A`n= {D0, D1, ..., D(d-2), An}
		==>(均值项)
		B`n = { {A`0 + A`1 + ... + A`(d-1)} / d, {A`1 + A`2 + ... + A`d} / d, .... , {A`(n-5) + A`(n-4) + ... + A`(n)} / d}
		*/
		m_movingAverageVals = CalAverage(datas, 0, arr);
		m_pAxis->CalAxisDataLimits();
		m_pVolume->CalAxisDataLimits();
		Invalidate();
	}


	const std::vector<TCandlestickChartData>& CCandlestickChartUI::GetDatas() const
	{
		return m_datas;
	}

	void CCandlestickChartUI::SetNeedDrawLines(const std::map<size_t, Gdiplus::Color>& needDrawLines)
	{
		m_needDrawLines = needDrawLines;
	}

	const std::map<size_t, Gdiplus::Color>& CCandlestickChartUI::GetNeedDrawLines() const
	{
		return m_needDrawLines;
	}

	const std::map<size_t, std::vector<double>>& CCandlestickChartUI::GetMovingAverageDatas() const
	{
		return m_movingAverageVals;
	}

	void CCandlestickChartUI::SetZoomValue(double nZoomValue)
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

	double CCandlestickChartUI::GetZoomValue() const
	{
		return m_nZoomValue;
	}

	DWORD CCandlestickChartUI::GetZoomFlag() const
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

	void CCandlestickChartUI::OnMouseEnter(TEventUI& event)
	{
		m_pCrossStar->SetVisible(true);
		m_pMovingAverage->SetVisible(true);
	}

	void CCandlestickChartUI::OnMouseMove(TEventUI& event)
	{
		POINT pt = event.ptMouse;

		POINT ptOriginCoordinate = m_pAxis->GetOriginCoordinate();
		{
			POINT ptOriginVolume = m_pVolume->GetOriginCoordinate();
			if (pt.x <= ptOriginVolume.x || pt.y >= ptOriginVolume.y)
			{
				m_pCrossStar->SetVisible(false);
				m_pMovingAverage->SetVisible(false);
				SetToolTip(_T(""));
				return;
			}
		}

		LONG dx = pt.x - ptOriginCoordinate.x;
		
		const size_t nIndex = dx / m_pAxis->GetXScale();

		size_t nMaxDrawCount = GetMaxDrawCounts();
		const size_t firstCount = nMaxDrawCount < m_datas.size() ? (m_datas.size() - nMaxDrawCount) : 0;
		nMaxDrawCount = std::min<std::size_t>((size_t)nMaxDrawCount, m_datas.size());
		if (nIndex >= nMaxDrawCount)
		{
			m_pCrossStar->SetVisible(false);
			m_pMovingAverage->SetVisible(false);
			SetToolTip(_T(""));
			return;
		}
		TCandlestickChartData& itemData = m_datas[firstCount + nIndex];
		RECT rcChartGraph = m_rcItem;

		rcChartGraph.left += CCandlestickChartUI::sm_lblWidth;
		rcChartGraph.top += CCandlestickChartUI::sm_lblHeight / 2;
		rcChartGraph.bottom -= CCandlestickChartUI::sm_lblHeight / 2;

		{
			RECT rcHorXLine = rcChartGraph;
			rcHorXLine.top = ptOriginCoordinate.y - (LONG)((itemData.m_endVal - m_pAxis->GetMinYTick()) * m_pAxis->GetYScale());
			rcHorXLine.bottom = rcHorXLine.top;
			rcHorXLine.left = ptOriginCoordinate.x;
			m_pCrossStar->SetHorXLinePos(rcHorXLine);
		}

		{
			RECT rcVerYLine = rcChartGraph;
			const double nRectWidth = GetRectWidth(m_pAxis->GetXScale());
			rcVerYLine.left = (LONG)(ptOriginCoordinate.x + nIndex * m_pAxis->GetXScale() + nRectWidth/ 2);
			rcVerYLine.right = rcVerYLine.left;
			rcVerYLine.bottom = rcChartGraph.bottom;
			m_pCrossStar->SetVerYLinePos(rcVerYLine);
		}

		{
			CDuiString strText;
			strText.Format(_T("%2.2f"), itemData.m_endVal);
			m_pCrossStar->SetYLabelText(strText);
		}
		m_pCrossStar->SetVisible(true);
		m_pMovingAverage->SetVisible(true);
		UpdateToolTip(itemData);
		m_pMovingAverage->UpdateData(firstCount + nIndex);
	}

	void CCandlestickChartUI::OnMouseLeave(TEventUI& event)
	{
		m_pCrossStar->SetVisible(false);
		m_pMovingAverage->SetVisible(false);
		SetToolTip(_T(""));
	}

	void CCandlestickChartUI::UpdateToolTip(const TCandlestickChartData& data)
	{
		auto getVal = [](double v) -> CDuiString
		{
			CDuiString ret;
			if (v > (10000 * 10000))
			{
				ret.Format(_T("%2.2f%s"), v / (10000 * 10000), _T("亿"));
			}
			else if (v > (10000))
			{
				ret.Format(_T("%2.2f%s"), v / 10000, _T("万"));
			}
			else
			{
				ret.Format(_T("%2.2f%s"), v, _T(""));
			}
			return ret;
		};

		CDuiString str(data.m_strXText.GetData());
		str += L"\n开 盘\t";
		str += getVal(data.m_beginVal),
		str += L"\n最 高\t";
		str += getVal(data.m_highVal),
		str += L"\n最 低\t";
		str += getVal(data.m_lowVal),
		str += L"\n收 盘\t";
		str += getVal(data.m_endVal),
		str += L"\n成交量\t";
		str += getVal(data.m_transVolume);
		str += L"\n成交额\t";
		str += getVal(data.m_transAmount * 10000);
		str += L"\n涨 幅\t";
		str += getVal(data.m_changeRange);
		str += L"%";
		SetToolTip(str);
	}

	void CCandlestickChartUI::SetPos(RECT rc)
	{
		CContainerUI::SetPos(rc);
		if (m_pMovingAverage->IsVisible())
		{
			RECT rcPos = rc;
			rcPos.bottom = rc.top + sm_lblHeight;
			rcPos.left += CCandlestickChartUI::sm_lblWidth;
			m_pMovingAverage->SetPos(rcPos);
		}
	}

	CControlUI* CCandlestickChartUI::FindControl(FINDCONTROLPROC Proc, LPVOID pData, UINT uFlags)
	{
		return CControlUI::FindControl(Proc, pData, uFlags);
	}


	double CCandlestickChartUI::GetRectWidth(double dxScale)
	{
		if (dxScale < 1.0)
		{
			return 1.0;
		}
		double nDistance = dxScale / 3;
		if (nDistance > sm_nMaxDistancesBetweenItems)
		{
			nDistance = sm_nMaxDistancesBetweenItems;
		}
		return dxScale - nDistance;
	}

	void CCandlestickChartUI::RemoveAllData()
	{
		m_datas.clear();
	}

	int CCandlestickChartUI::GetMaxDrawCounts() const
	{
		return static_cast<int>(m_nZoomValue * m_nMaxDrawItems);
	}

	bool CCandlestickChartUI::CanZoomIn() const
	{
		return m_nZoomValue / 2 > sm_nMinZoomVal;
	}

	bool CCandlestickChartUI::CanZoomOut() const
	{
		return m_nZoomValue * 2 < sm_nMaxZoomVal;
	}

}

