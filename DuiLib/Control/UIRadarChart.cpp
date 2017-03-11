#include "StdAfx.h"
#include <cmath>
#include "UIRadarChart.h"

namespace DuiLib
{
    const double pi = 4 * std::atan(1);

    CRadarChartUI::CRadarChartUI():
        m_dwOutterBorderColor(0xfffdcb43),
        m_dwOutterBrushBgColor(0xff242536),
        m_dwMiddleBorderColor(0xff388279),
        m_dwMiddleBrushBgColor(0xff2b414b),
        m_dwInnerBorderColor(0xff208aa4),
        m_dwInnerBrushBgColor(0xff2d4f66),
        m_dwCornerTextColor(Gdiplus::Color::Gray),
        m_nAngleNumber(7),
        m_nLadderCircleNumber(5),
        m_cornerPos(7),
        m_radius(0)
    {
        SetPadding({8, 20, 8, 8});
    }

    CRadarChartUI::~CRadarChartUI()
    {

    }

    void CRadarChartUI::DoPaint(HDC hDC, const RECT& rcPaint)
    {
        if (!::IntersectRect(&m_rcPaint, &rcPaint, &m_rcItem))
        {
            return;
        }

        Gdiplus::Pen pen(Gdiplus::Color(GetBValue(m_dwOutterBorderColor),
            GetGValue(m_dwOutterBorderColor), GetRValue(m_dwOutterBorderColor)), 2.0f);
        Gdiplus::Graphics graphics(hDC);

		graphics.SetSmoothingMode(SmoothingModeAntiAlias);
        RECT rcItem = GetPos();
        rcItem.left += m_rcPadding.left;
        rcItem.top += m_rcPadding.top;
        rcItem.right -= m_rcPadding.right;
        rcItem.bottom -= m_rcPadding.bottom;

        Gdiplus::PointF oriCoorinate(
            (rcItem.left + (rcItem.right - rcItem.left) / 2),
            (rcItem.top + (rcItem.bottom - rcItem.top) / 2));

        m_radius = (rcItem.right - rcItem.left) / 2;

        std::vector<Gdiplus::PointF> pts(m_nAngleNumber);
        std::vector<Gdiplus::PointF> ptLineBrushPts(m_nAngleNumber);
        std::vector<Gdiplus::PointF> ptMiddles(m_nAngleNumber);
        std::vector<Gdiplus::PointF> ptInners(m_nAngleNumber);
        std::vector<Gdiplus::PointF> ptConners(m_nAngleNumber);
        const double flag = 2 * pi / 4;
        for (size_t i = 0; i < m_nAngleNumber; ++i)
        {
            // 向左旋转90度
            const double angle = pi / 2 + 2 * pi * i / m_nAngleNumber;
            Gdiplus::PointF pt;
            Gdiplus::PointF middlePt;
            Gdiplus::PointF innerPt;
            Gdiplus::PointF cornerPt;
            Gdiplus::PointF ptLineBrushPt;

            Gdiplus::REAL dx = std::cos(angle) * m_radius;
            Gdiplus::REAL dy = std::sin(angle) * m_radius;
            pt.X = oriCoorinate.X + dx;
            pt.Y = oriCoorinate.Y - dy;

            middlePt.X = oriCoorinate.X + dx * 0.7;
            middlePt.Y = oriCoorinate.Y - dy * 0.7;

            cornerPt.X = oriCoorinate.X + dx * 1.1;
            cornerPt.Y = oriCoorinate.Y - dy * 1.1;

            ptLineBrushPt.X = oriCoorinate.X + dx * 1.01;
            ptLineBrushPt.Y = oriCoorinate.Y - dy * 1.01;

            innerPt.X = oriCoorinate.X + dx * m_cornerPos[i];
            innerPt.Y = oriCoorinate.Y - dy * m_cornerPos[i];
          
            ptConners[i] = cornerPt;
            pts[i] = pt;
            ptLineBrushPts[i] = ptLineBrushPt;
            ptMiddles[i] = middlePt;
            ptInners[i] = innerPt;
        }

        // 背景
        // Outter
        {
            Gdiplus::SolidBrush brush(Gdiplus::Color(GetBValue(m_dwOutterBrushBgColor),
                GetGValue(m_dwOutterBrushBgColor), GetRValue(m_dwOutterBrushBgColor)));
            graphics.FillPolygon(&brush, &pts[0], pts.size());
        }

        // Middle
        {
            Gdiplus::SolidBrush brush(Gdiplus::Color(GetBValue(m_dwMiddleBrushBgColor),
                GetGValue(m_dwMiddleBrushBgColor), GetRValue(m_dwMiddleBrushBgColor)));
            graphics.FillPolygon(&brush, &ptMiddles[0], ptMiddles.size());
        }

        // 外边
        // Outter
        {
            // graphics.DrawPolygon(&pen, &pts[0], pts.size());

            std::vector<std::pair<Gdiplus::Color, Gdiplus::Color>> colors(pts.size());
            std::vector<std::pair<Gdiplus::PointF, Gdiplus::PointF>> tmpPts(pts.size());
            std::vector<std::vector<Gdiplus::PointF>> tmpBrushPts(pts.size());
            size_t i = 0;
            for (; i < m_cornerColors.size() - 1; ++i)
            {
                colors[i] = std::make_pair(
                    Gdiplus::Color(GetBValue(m_cornerColors[i]), GetGValue(m_cornerColors[i]), GetRValue(m_cornerColors[i])),
                    Gdiplus::Color(GetBValue(m_cornerColors[i+1]), GetGValue(m_cornerColors[i+1]), GetRValue(m_cornerColors[i+1])));

                tmpBrushPts[i].push_back(pts[i]);
                tmpBrushPts[i].push_back(pts[i+1]);
                tmpBrushPts[i].push_back(ptLineBrushPts[i + 1]);
                tmpBrushPts[i].push_back(ptLineBrushPts[i]);
                tmpPts[i] = std::make_pair(pts[i], pts[i + 1]);
            }
            colors[i] = std::make_pair(
                Gdiplus::Color(GetBValue(m_cornerColors[i]), GetGValue(m_cornerColors[i]), GetRValue(m_cornerColors[i])),
                Gdiplus::Color(GetBValue(m_cornerColors[0]), GetGValue(m_cornerColors[0]), GetRValue(m_cornerColors[0])));
            tmpPts[i] = std::make_pair(pts[i], pts[0]);

            tmpBrushPts[i].push_back(pts[i]);
            tmpBrushPts[i].push_back(pts[0]);
            tmpBrushPts[i].push_back(ptLineBrushPts[0]);
            tmpBrushPts[i].push_back(ptLineBrushPts[i]);
            
            for (size_t j = 0; j < pts.size(); ++j)
            {
                Gdiplus::LinearGradientBrush brush(
                    tmpPts[j].first, tmpPts[j].second,
                    colors[j].first, colors[j].second);

                graphics.FillPolygon(&brush, &tmpBrushPts[j][0], tmpBrushPts[j].size(), FillModeWinding);

                Gdiplus::Pen tmpPen(colors[j].first, 1.04);
                graphics.DrawLine(&tmpPen, oriCoorinate, ptLineBrushPts[j]);
            }
        }

        // Inner
        {
            Gdiplus::SolidBrush brush(Gdiplus::Color(GetBValue(m_dwInnerBrushBgColor),
                GetGValue(m_dwInnerBrushBgColor), GetRValue(m_dwInnerBrushBgColor)));
            graphics.FillPolygon(&brush, &ptInners[0], ptInners.size());
        }

        // Middle
        {
            Gdiplus::Pen middlePen(Gdiplus::Color(GetBValue(m_dwMiddleBorderColor),
                GetGValue(m_dwMiddleBorderColor), GetRValue(m_dwMiddleBorderColor)), 2.0f);
            graphics.DrawPolygon(&middlePen, &ptMiddles[0], ptMiddles.size());
        }

        // inner
        {
            Gdiplus::Pen innerPen(Gdiplus::Color(GetBValue(m_dwInnerBorderColor),
                GetGValue(m_dwInnerBorderColor), GetRValue(m_dwInnerBorderColor)), 2.0f);
            graphics.DrawPolygon(&innerPen, &ptInners[0], ptInners.size());
        }

        //// 连接线
        //for (auto pt: pts)
        //{
        //    graphics.DrawLine(&pen, oriCoorinate, pt);
        //}

        //// 阶梯圆
        //for (size_t i = 1; i < m_nLadderCircleNumber; ++i)
        //{
        //    Gdiplus::REAL radius = i * m_radius / m_nLadderCircleNumber;

        //    Gdiplus::RectF rc = {
        //        oriCoorinate.X - radius,
        //        oriCoorinate.Y - radius,
        //        2 * radius,
        //        2 * radius
        //    };
        //    graphics.DrawEllipse(&pen, rc);
        //}

        // 绘制边角顶点
		if (m_cornerColors.size() == ptConners.size())
		{
			Gdiplus::REAL r = 2.5;
			for (size_t i = 0; i < ptConners.size(); ++i)
			{
				Gdiplus::SolidBrush brush(Gdiplus::Color(GetBValue(m_cornerColors[i]),
					GetGValue(m_cornerColors[i]), GetRValue(m_cornerColors[i])));

				Gdiplus::RectF rc = {
					ptConners[i].X - r,
                    ptConners[i].Y - r,
					2 * r,
					2 * r
				};
				graphics.FillEllipse(&brush, rc);
			}
		}

        // 绘制边角文字
        if (m_cornerTexts.size() == ptConners.size() && m_revisedTextPts.size() == ptConners.size())
        {
            Gdiplus::REAL r = 4.5;
            for (size_t i = 0; i < ptConners.size(); ++i)
            {
                LONG x = ptConners[i].X += m_revisedTextPts[i].x;
                LONG y = ptConners[i].Y += m_revisedTextPts[i].y;

                RECT rc = { x, y, x + 50, y + 50 };
                CRenderEngine::DrawText(hDC, m_pManager, rc, m_cornerTexts[i].c_str(), m_dwCornerTextColor, 0, DT_SINGLELINE);
            }
        }
    }

    void CRadarChartUI::SetPos(RECT rc)
    {
        CControlUI::SetPos(rc);
    }

    void CRadarChartUI::SetAngleNumber(int nAngleNumber)
    {
        m_nAngleNumber = nAngleNumber;
    }

	void CRadarChartUI::SetAngleColors(const std::vector<DWORD>& colors)
	{
		m_cornerColors = colors;
	}

    void CRadarChartUI::SetTextColor(DWORD dwTextColor)
    {
        m_dwCornerTextColor = dwTextColor;
    }

    void CRadarChartUI::SetLadderCircleNumber(int nLadderCircleNumber)
    {
        m_nLadderCircleNumber = nLadderCircleNumber;
    }

    int CRadarChartUI::GetAngleNumber() const
    {
        return m_nAngleNumber;
    }

    void CRadarChartUI::SetCornerPos(const std::vector<double>& cornerPos)
    {
        m_cornerPos = cornerPos;

        for (auto& v: m_cornerPos)
        {
            // assert(v > 0.0000f && v < 1.00000f);
            if (v < 0.000001f)
            {
                v = 0.000001f;
            }
            else if (v > 1.00000f)
            {
                v = 1.0f;
            }
        }

        Invalidate();
    }

    void CRadarChartUI::SetRevisedTextPoses(const std::vector<CDuiPoint>& pts)
    {
        m_revisedTextPts = pts;
    }

    void CRadarChartUI::SetCornerTexts(const std::vector<std::wstring>& texts)
    {
        m_cornerTexts = texts;
    }

}