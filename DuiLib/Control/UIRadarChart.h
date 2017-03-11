/******************************************************************************
 *  版权所有（C）2010-2017                                                    *
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

namespace DuiLib
{
    class UILIB_API CRadarChartUI : public CControlUI
    {
    public:
        CRadarChartUI();
        ~CRadarChartUI();

        virtual void DoPaint(HDC hDC, const RECT& rcPaint) override;

        virtual void SetPos(RECT rc) override;

        void SetAngleNumber(int nAngleNumber);
		void SetAngleColors(const std::vector<DWORD>& colors);
        void SetTextColor(DWORD dwTextColor);
        void SetLadderCircleNumber(int nLadderCircleNumber);

        int GetAngleNumber() const;

        void SetCornerPos(const std::vector<double>& cornerPos);
        void SetRevisedTextPoses(const std::vector<CDuiPoint>& pts);
        void SetCornerTexts(const std::vector<std::wstring>& texts);
    private:

        DWORD m_dwOutterBorderColor;
        DWORD m_dwOutterBrushBgColor;
        DWORD m_dwMiddleBorderColor;
        DWORD m_dwMiddleBrushBgColor;
        DWORD m_dwInnerBorderColor;
        DWORD m_dwInnerBrushBgColor;
        DWORD m_dwCornerTextColor;

        LONG m_radius;

        size_t m_nAngleNumber;
        size_t m_nLadderCircleNumber;
        std::vector<CDuiPoint> m_revisedTextPts;
		std::vector<double> m_cornerPos;
        std::vector<DWORD> m_cornerColors;
        std::vector<std::wstring> m_cornerTexts;
    };
}

