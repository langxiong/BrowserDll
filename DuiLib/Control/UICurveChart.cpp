#include "StdAfx.h"
#include "UICurveChart.h"

#include <algorithm>

#include <gdiplus.h>
#pragma comment( lib, "GdiPlus.lib")

namespace DuiLib
{
    const double CCurveChartUI::sm_nMaxZoomVal = 4.1;

    const double CCurveChartUI::sm_nMinZoomVal = 0.0626;

    CCurveChartUI::CCurveChartUI():
        m_nMaxDrawItems(240),
        m_nZoomValue(1.0),
        m_minDrawValue(0.0),
        m_maxDrawValue(0.0),
        m_dwAxisColor(0xdddddd),
        m_dwCurveColor(0xA9A9A9),
		m_isDrawAxis(false)
    {
        SetPadding({ 1,0,0,1 });
    }

    CCurveChartUI::~CCurveChartUI()
    {

    }

    LPCTSTR CCurveChartUI::GetClass() const
    {
        return _T("CurveChartUI");
    }

    LPVOID CCurveChartUI::GetInterface(LPCTSTR pstrName)
    {
        if (_tcscmp(pstrName, _T("CurveChart")) == 0) return static_cast<CCurveChartUI*>(this);
        else return CControlUI::GetInterface(pstrName);
    }

    void CCurveChartUI::DoEvent(TEventUI& event)
    {
        return CControlUI::DoEvent(event);
    }

    void CCurveChartUI::DoPaint(HDC hDC, const RECT& rcPaint)
    {
        if (!::IntersectRect(&m_rcPaint, &rcPaint, &m_rcItem))
        {
            return;
        }
        CControlUI::DoPaint(hDC, rcPaint);
        DrawChart(hDC, rcPaint);
    }

    void CCurveChartUI::SetDatas(const std::vector<double>& datas)
    {
        m_datas = datas;
        CalMinMaxDrawValue();
        Invalidate();
    }

    const std::vector<double>& CCurveChartUI::GetDatas() const
    {
        return m_datas;
    }

    void CCurveChartUI::AddData(double data)
    {
        if (data > 5.0f)
        {
            data = 5.0f;
        }
        m_datas.emplace_back(data);
        if (m_datas.size() > 11000)
        {
            m_datas = std::vector<double>(m_datas.cbegin() + 10000, m_datas.cend());
        }
        CalMinMaxDrawValue();
        Invalidate();
    }

    void CCurveChartUI::DrawChart(HDC hDC, const RECT& rcGraph)
    {
        Gdiplus::Graphics graphics(hDC);
		graphics.SetSmoothingMode(SmoothingModeAntiAlias);
        
        Gdiplus::Pen pen(Gdiplus::Color(GetBValue(m_dwAxisColor),
            GetGValue(m_dwAxisColor), GetRValue(m_dwAxisColor)));

        RECT rcItem = m_rcItem;

        rcItem.left += m_rcPadding.left;
        rcItem.top += m_rcPadding.top;
        rcItem.right -= m_rcPadding.right;
        rcItem.bottom -= m_rcPadding.bottom;

		if (m_isDrawAxis)
       {
            pen.SetDashStyle(DashStyleDashDot);
            RECT rcXLine = rcItem;
            rcXLine.top = rcXLine.bottom;
            graphics.DrawLine(&pen, rcXLine.left, rcXLine.top, rcXLine.right, rcXLine.bottom);

			RECT rcYLine = rcItem;
			rcYLine.right = rcYLine.left;
			graphics.DrawLine(&pen, rcYLine.left, rcYLine.top, rcYLine.right, rcYLine.bottom);
        }

        auto& datas = GetDatas();
        if (datas.empty())
        {
            return;
        }

        size_t nMaxDrawCount = GetMaxDrawCounts();
        nMaxDrawCount = std::min<size_t>(nMaxDrawCount, datas.size());

        double dxScale = 1.0;
        double dyScale = (rcItem.bottom - rcItem.top) / (m_maxDrawValue - m_minDrawValue);

        // 1.0 为去掉Y轴占用的空间
        Gdiplus::PointF oriCoorinate(m_rcItem.left + 1.0, m_rcItem.bottom);

        Gdiplus::Rect brushRc = {
            m_rcItem.left + 1,
            m_rcItem.bottom,
            rcItem.right - rcItem.left,
            rcItem.bottom - rcItem.top
        };

        Gdiplus::Pen curveLinePen(Gdiplus::Color(GetBValue(m_dwCurveColor), GetGValue(m_dwCurveColor), GetRValue(m_dwCurveColor)));
        Gdiplus::LinearGradientBrush brush(brushRc,
            Gdiplus::Color(204, GetBValue(m_dwCurveColor), GetGValue(m_dwCurveColor), GetRValue(m_dwCurveColor)),
            Gdiplus::Color(76, GetBValue(m_dwCurveColor), GetGValue(m_dwCurveColor), GetRValue(m_dwCurveColor)),
            LinearGradientModeVertical);

        std::vector<Gdiplus::PointF> pts(nMaxDrawCount + 2);

        pts[0] = oriCoorinate;
        const size_t nDataSz = datas.size();
        for (size_t i = 0; i < nMaxDrawCount; ++i)
        {
            auto& itemData = datas[nDataSz - i - 1];
            Gdiplus::PointF pt(oriCoorinate.X + i * dxScale,
                oriCoorinate.Y - (itemData * dyScale));

            pts[i+1] = pt;
        }
        auto ptLast = pts[pts.size() - 2];
        ptLast.Y = oriCoorinate.Y;
        pts[pts.size() - 1] = ptLast;

        if (!pts.empty())
        {
            graphics.FillPolygon(&brush, &pts[0], pts.size());
            graphics.DrawCurve(&curveLinePen, &pts[0], pts.size(), 0.3f);
        }
    }
     
    void CCurveChartUI::CalMinMaxDrawValue()
    {
        auto& datas = GetDatas();
        if (datas.empty())
        {
            return;
        }

        // 固定为最大值为5.0
        m_maxDrawValue = 5.0; 
		// auto minMaxIt = std::minmax_element(datas.begin(), datas.begin() + std::min<size_t>(datas.size(), (size_t)nMaxDrawCount));
        /*

        if (minMaxIt.second != datas.end())
        {
            m_maxDrawValue = *minMaxIt.second;
        }*/
    }

    int CCurveChartUI::GetMaxDrawCounts() const
    {
        return static_cast<int>(m_nZoomValue * m_nMaxDrawItems);
    }


    void CCurveChartUI::SetPos(RECT rc)
    {
        RECT rcItem = rc;
        rcItem.right -= m_rcPadding.right;
        rcItem.left += m_rcPadding.left;
        // 1 为去掉Y轴占用的空间
        m_nMaxDrawItems = rcItem.right - rcItem.left - 1;
        CControlUI::SetPos(rc);
    }

    void CCurveChartUI::SetZoomValue(double nZoomValue)
    {
        m_nZoomValue = nZoomValue;
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

    double CCurveChartUI::GetZoomValue() const
    {
        return m_nZoomValue;
    }

    void CCurveChartUI::SetCurveLineColor(DWORD dwCurveLineColor)
    {
        m_dwCurveColor = dwCurveLineColor;
        Invalidate();
    }

    DWORD CCurveChartUI::GetCurveLineColor() const
    {
        return m_dwCurveColor;
    }

    bool CCurveChartUI::CanZoomIn() const
    {
        return m_nZoomValue / 2 >  CCurveChartUI::sm_nMinZoomVal;
    }

    bool CCurveChartUI::CanZoomOut() const
    {
        return m_nZoomValue * 2 < CCurveChartUI::sm_nMaxZoomVal;
    }
}