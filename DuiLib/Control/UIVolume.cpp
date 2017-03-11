#include "StdAfx.h"
#include <algorithm>
#include "UIVolume.h"

namespace DuiLib
{
    const LONG CVolumeUI::sm_lblWidth = 37;

    const LONG CVolumeUI::sm_lblHeight = 27;

    const double CVolumeUI::sm_nMaxZoomVal = 4.1;

    const double CVolumeUI::sm_nMinZoomVal = 0.0626;

    const double CVolumeUI::sm_nMaxItemWidth = 46.0;

    const double CVolumeUI::sm_nMaxItemDistance = 15.0;

    const RECT CVolumeUI::sm_rcTextPadding = { 0,0,8,0 };

    CVolumeUI::CVolumeUI() :
        m_Font(0),
        m_minDrawValue(0),
        m_maxDrawValue(0),
        m_iMinTick(0),
        m_iMaxTick(25),
        m_uStepTick(1.0),
        m_dxScale(0.0),
        m_dyScale(0.0),
        m_dwTextColor(0x333333)
    {
        SetTickLimis(GetMinYTick(),GetMaxYTick());
        EnableScrollBar(false, true);
    }

    CVolumeUI::~CVolumeUI()
    {
    }

    LPCTSTR CVolumeUI::GetClass() const
    {
        return _T("VolumeUI");
    }

    LPVOID CVolumeUI::GetInterface(LPCTSTR pstrName)
    {
        if (_tcscmp(pstrName, DUI_CTR_VOLUME) == 0) return static_cast<CVolumeUI*>(this);
        return CContainerUI::GetInterface(pstrName);
    }

    void CVolumeUI::SetTickLimis(int minTick, int maxTick)
    {
        m_iMinTick    = minTick;
        m_iMaxTick    = maxTick;

        int dy = m_iMaxTick - m_iMinTick;
        int nSepLineNums = dy > 4 ? 4 : dy;
        m_uStepTick = (m_iMaxTick - m_iMinTick) / nSepLineNums;
        m_sepYTicks.clear();
        if(m_uStepTick > 0.0001){
            for(int sepTick = m_iMinTick + m_uStepTick;sepTick <= (int)( maxTick - m_uStepTick) ;sepTick += m_uStepTick){
                m_sepYTicks.push_back(sepTick);
            }
        }
    }

    int CVolumeUI::GetMinYTick()
    {
        return m_iMinTick;
    }

    int CVolumeUI::GetMaxYTick()
    {
        return m_iMaxTick;
    }

    double CVolumeUI::GetSepYTick()
    {
        return m_uStepTick;
    }

    void CVolumeUI::CalAxisDataLimits()
    {
        CalMinMaxDrawValue();
        SetTickLimis(m_minDrawValue, m_maxDrawValue);
    }

    CDuiPoint CVolumeUI::GetOriginCoordinate() const
    {
        return CDuiPoint(m_rcChartGraph.left, m_rcChartGraph.bottom);
    }

    double CVolumeUI::GetXScale() const
    {
        return m_dxScale;
    }

    double CVolumeUI::GetYScale() const
    {
        return m_dyScale;
    }

    double CVolumeUI::GetRectWidth(double dxScale)
    {
        if (dxScale > sm_nMaxItemWidth)
        {
            return sm_nMaxItemWidth;
        }
        if (dxScale < 1)
        {
            return 1.0;
        }
        return dxScale;
    }

    void CVolumeUI::DoPaint(HDC hDC, const RECT& rcPaint)
    {
        CContainerUI::DoPaint(hDC, rcPaint);
        if (m_data.empty())
        {
            return;
        }
        RECT rcItem = m_rcItem;
        rcItem.left        += m_rcPadding.left;
        rcItem.top        += m_rcPadding.top;
        rcItem.right    -= m_rcPadding.right;
        rcItem.bottom    -= m_rcPadding.bottom;

        m_rcChartGraph = rcItem;
        m_rcChartGraph.left += sm_lblWidth;
        m_rcChartGraph.top += sm_lblHeight;
        m_rcChartGraph.bottom -= sm_lblHeight / 2;

        CDuiPoint ptOriCoorinate(m_rcChartGraph.left, m_rcChartGraph.bottom);

        const LONG nMaxGraphWidth = m_rcChartGraph.right - m_rcChartGraph.left;
        const LONG nMaxGraphHeight = m_rcChartGraph.bottom - m_rcChartGraph.top;

        size_t nMaxDrawCount = m_data.size();
        const size_t firstCount = 0;

        m_dyScale = nMaxGraphHeight / (GetMaxYTick() - GetMinYTick());
        if (m_dyScale < 0.01)
        {
            m_dyScale = 1.0;
        }
        m_dxScale = (double)nMaxGraphWidth / nMaxDrawCount;
        const double nRectWidth = GetRectWidth(m_dxScale);

        if (m_dxScale > (nRectWidth + sm_nMaxItemDistance))
        {
            m_dxScale = nRectWidth + sm_nMaxItemDistance;
        }
        DrawXAxis(hDC, m_rcChartGraph);
        DrawYAxis(hDC, m_rcChartGraph, m_dyScale);

        Graphics graphics(hDC);
        std::vector<Gdiplus::RectF> upRects;
        std::vector<std::pair<RECT, int>> textData;

        const Color red(0xff, 0x00, 0x00);
        const Color black(0x3c, 0x3c, 0x3c);

        Pen linePen(black);
        SolidBrush redBrush(red);

        size_t i = 0;
        for (auto v: m_data)
        {
            REAL height = (v.second - GetMinYTick()) * m_dyScale;

            Gdiplus::RectF rc(ptOriCoorinate.x + i++ * m_dxScale,
                ptOriCoorinate.y - height,
                nRectWidth, height);
            if (rc.Height < 1.0f)
            {
                rc.Height = 1.0f;
            }
            upRects.emplace_back(rc);

            {
                RECT rcText = { rc.GetLeft(), rc.GetTop(), rc.GetRight(), rc.GetBottom() };
                rcText.left += (rcText.right - rcText.left) / 2;
                rcText.right = rcText.left + sm_lblWidth;
                rcText.bottom = rcText.top + sm_lblHeight;

                textData.emplace_back(std::make_pair(rcText, v.first));
            }
        }

        if (!upRects.empty())
        {
            graphics.FillRectangles(&redBrush, &upRects[0], upRects.size());
            graphics.DrawRectangles(&linePen, &upRects[0], upRects.size());
        }
        for (auto v: textData)
        {
            CRenderEngine::DrawText(hDC, m_pManager, v.first, std::to_wstring(v.second).c_str(),
                m_dwTextColor, m_Font, DT_LEFT | DT_VCENTER);
        }
    }

    void CVolumeUI::CalMinMaxDrawValue()
    {
        size_t nMaxDrawCount = m_data.size();
        const size_t firstCount = 0;
        auto minMaxIt = std::minmax_element(m_data.begin(), m_data.end(), 
            [](const std::pair<int, int>& lhs, const std::pair<int, int>& rhs) -> bool
        {
            return lhs.second < rhs.second;
        });

        if (minMaxIt.first != m_data.end())
        {
            m_minDrawValue = 0;
        }

        if (minMaxIt.second != m_data.end())
        {
            m_maxDrawValue = minMaxIt.second->second;
        }
    }

    void CVolumeUI::SetData(const std::map<int, int>& data)
    {
        m_data = data;
        CalAxisDataLimits();
        this->Invalidate();
    }

    void CVolumeUI::DrawYAxis(HDC hDC, const RECT& rcGraph, double dyScale)
    {
        Graphics graphics(hDC);
        Pen pen(Color(0x33, 0x33, 0x33));
        {
            RECT rcYLine = rcGraph;
            rcYLine.top = rcYLine.bottom;

            graphics.DrawLine(&pen, rcYLine.left, rcYLine.top, rcYLine.right, rcYLine.bottom);

            rcYLine.left -= sm_lblWidth;
            rcYLine.right = rcYLine.left + sm_lblWidth - sm_rcTextPadding.right;
            rcYLine.top    = rcYLine.top - sm_lblHeight / 2;
            rcYLine.bottom    = rcYLine.top + sm_lblHeight;

            CRenderEngine::DrawText(hDC, m_pManager, rcYLine, std::to_wstring(GetMinYTick()).c_str(),
                m_dwTextColor, m_Font, DT_RIGHT | DT_VCENTER);

            rcYLine = rcGraph;
            rcYLine.bottom = rcYLine.top;
            graphics.DrawLine(&pen, rcYLine.left, rcYLine.top, rcYLine.right, rcYLine.bottom);
            rcYLine.left -= sm_lblWidth;
            rcYLine.right = rcYLine.left + sm_lblWidth - sm_rcTextPadding.right;
            rcYLine.top    = rcYLine.top - sm_lblHeight / 2;
            rcYLine.bottom    = rcYLine.top + sm_lblHeight;
            CRenderEngine::DrawText(hDC, m_pManager, rcYLine, std::to_wstring(GetMaxYTick()).c_str(),
                m_dwTextColor, m_Font, DT_RIGHT | DT_VCENTER);
        }

        pen.SetDashStyle(DashStyleDash);
        for (auto it = m_sepYTicks.cbegin(); it != m_sepYTicks.cend(); ++it)
        {
            double dy = rcGraph.bottom - ((*it) - GetMinYTick()) * dyScale;
            RECT rcYLine = rcGraph;
            graphics.DrawLine(&pen, rcYLine.left, dy, rcYLine.right, dy);
            rcYLine.top = dy;
            rcYLine.left -= sm_lblWidth;
            rcYLine.right = rcYLine.left + sm_lblWidth - sm_rcTextPadding.right;
            rcYLine.top    = rcYLine.top - sm_lblHeight / 2;
            rcYLine.bottom    = rcYLine.top + sm_lblHeight;
            CRenderEngine::DrawText(hDC, m_pManager, rcYLine, std::to_wstring(*it).c_str(),
                m_dwTextColor, m_Font, DT_RIGHT | DT_VCENTER);
        }
    }

    void CVolumeUI::DrawXAxis(HDC hDC, const RECT& rcGraph)
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
}