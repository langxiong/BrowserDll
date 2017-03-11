/******************************************************************************
 *  版权所有（C）2010-2015                                                    *
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
#include <string>

namespace DuiLib
{
    class UILIB_API CVolumeUI : public CContainerUI
    {
    public:
        static const LONG sm_lblWidth;
        static const LONG sm_lblHeight;

        static const double sm_nMaxZoomVal;
        static const double sm_nMinZoomVal;

        static const double sm_nMaxItemWidth;
        static const double sm_nMaxItemDistance;
        static const RECT sm_rcTextPadding;

    public:
        CVolumeUI();
        ~CVolumeUI();

    public:
        void SetTickLimis(int minTick, int maxTick);

        int GetMinYTick();
        int GetMaxYTick();
        double GetSepYTick();

        void CalAxisDataLimits();
        CDuiPoint GetOriginCoordinate() const;

        double GetXScale() const;
        double GetYScale() const;

        double GetRectWidth(double dxScale);

        virtual void DoPaint(HDC hDC, const RECT& rcPaint);
        void CalMinMaxDrawValue();

        void SetData(const std::map<int, int>& data);
    private:

        void DrawYAxis(HDC hDC, const RECT& rcGraph, double dyScale);
        void DrawXAxis(HDC hDC, const RECT& rcGraph);

        virtual LPCTSTR GetClass() const override;

        virtual LPVOID GetInterface(LPCTSTR pstrName) override;

    private:
        int         m_Font;
        int            m_minDrawValue;
        int         m_maxDrawValue;
        int            m_iMinTick;
        int            m_iMaxTick;
        double            m_uStepTick;
        double            m_dxScale;
        double            m_dyScale;
        DWORD            m_dwTextColor;
        CDuiRect        m_rcChartGraph;

        std::map<int, int> m_data;
        std::vector<int> m_sepYTicks;

    };
}

