#include "stdafx.h"
#include "MainFrame.h"
#include "AppCommon.h"
#include "Poco/FileStream.h"
#include "Poco/StreamCopier.h"
#include "Poco/JSON/JSON.h"
#include "Poco/JSON/Parser.h"
#include "Poco/Nullable.h"
#include "Poco/Dynamic/Struct.h"

#include "StringHelper.h"
#include <vector>
#include <unordered_map>
#include <random>
#include <ppltasks.h>

#include "DateTimeUtil.h"

using namespace DuiLib;
using namespace Poco;
using namespace Poco::JSON;
using namespace Poco::Dynamic;


namespace MyWeb {
    namespace UI {

        inline Poco::Logger& logger()
        {
            return Poco::Util::Application::instance().getSubsystem<CUISubsystem>().logger();
        }

        inline Poco::Util::LayeredConfiguration& config()
        {
            return Poco::Util::Application::instance().config();
        }

        CMainFrame::CMainFrame(uint32_t frameID):
            m_frameID(frameID)
        {
        }
        CMainFrame::~CMainFrame()
        {
        }

        void CMainFrame::InitWindow()
        {
            InitControls();
            CenterWindow();
        }

        void CMainFrame::OnFinalMessage(HWND hWnd)
        {
            for (auto id: m_dwWorkThreadIds)
            {
                ::PostThreadMessage(id, WM_QUIT, 0, 0);
            }
            if (!m_hWorkthreads.empty())
            {
                ::WaitForMultipleObjectsEx(m_hWorkthreads.size(), &m_hWorkthreads[0], TRUE, INFINITE, FALSE);
                for (auto h: m_hWorkthreads)
                {
                    ::CloseHandle(h);
                }
            }
            WindowImplBase::OnFinalMessage(hWnd);
            ::PostThreadMessage(::GetCurrentThreadId(), WM_QUIT, 0, 0);
            logger().information("frame quit");
        }

        void CMainFrame::Notify(DuiLib::TNotifyUI & msg)
        {
            if (msg.sType == DUI_MSGTYPE_CLICK)
            {
                auto strName(msg.pSender->GetName());
                if (strName == _T("btnClose"))
                {
                    Close();
                }
                else if (strName == _T("btnNew"))
                {
                    PostMessage(WM_USER_CREATE_FRAME);
                }
              
                return;
            }

            if (msg.sType == DUI_MSGTYPE_SELECTCHANGED)
            {
                return;
            }

            if (msg.sType == DUI_MSGTYPE_TIMER)
            {
                return;
            }
        }

        LRESULT CMainFrame::HandleCustomMessage(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL & bHandled)
        {
            if (uMsg == WM_USER_ADD_WORKTHREAD_HANDLE)
            {
                if (wParam)
                {
                    m_hWorkthreads.push_back((HANDLE)wParam);
                }
                if (lParam)
                {
                    m_dwWorkThreadIds.push_back((DWORD)lParam);
                }
                return S_OK;
            }

            return WindowImplBase::HandleCustomMessage(uMsg, wParam, lParam, bHandled);
        }

        void CMainFrame::OnCreateFrame()
        {
        }

        UILIB_RESOURCETYPE CMainFrame::GetResourceType() const
        {
#if _DEBUG
            return WindowImplBase::GetResourceType();
#else
            return UILIB_ZIPRESOURCE;
#endif
        }

        LPCTSTR CMainFrame::GetResourceID() const
        {
#if _DEBUG
            return WindowImplBase::GetResourceID();
#else
            return MAKEINTRESOURCE(IDR_ZIP_RES);
#endif
        }

        CDuiString CMainFrame::GetSkinFolder()
        {
#if _DEBUG
            return _T("UIZIP");
#else
            return CDuiString();
#endif
        }

        CDuiString CMainFrame::GetSkinFile()
        {
            return _T("dlg_main_frame.xml");
        }

        LPCTSTR CMainFrame::GetWindowClassName(void) const
        {
            return _T("MyWeb::UI::CMainFrame");
        }

        CControlUI* CMainFrame::CreateControl(LPCTSTR pstrClass)
        {
			/*if (_tcscmp(pstrClass, CCameraVideoUI::DUI_CTR_CAMERA_VIDEO) == 0)
			{
				m_pCameraVideo = new DuiLib::CCameraVideoUI;
				return m_pCameraVideo;
			}
*/
            return WindowImplBase::CreateControl(pstrClass);
        }

        void CMainFrame::InitControls()
        {
            m_pCurveContainer = static_cast<CVerticalLayoutUI*>(m_PaintManager.FindControl(_T("curve.container#ver")));
            m_pRadarContainer = static_cast<CVerticalLayoutUI*>(m_PaintManager.FindControl(_T("test#ver")));
            m_pVoiceTxt = static_cast<CTextUI*>(m_PaintManager.FindControl(_T("voice#txt")));
            m_pVoiceLQuote = static_cast<CLabelUI*>(m_PaintManager.FindControl(_T("voice.l.quote#lbl")));
            m_pVoiceRQuote = static_cast<CLabelUI*>(m_PaintManager.FindControl(_T("voice.r.quote#lbl")));
            m_pEmotionAnalysisContainer = static_cast<CVerticalLayoutUI*>(m_PaintManager.FindControl(_T("emotion.analysis#ver")));

            m_pVoiceLQuote->SetText(_T("“"));
            m_pVoiceRQuote->SetText(_T("”"));

            CRadarChartUI* pRaderChart = new CRadarChartUI;
            m_pRadarContainer->Add(pRaderChart);
			std::vector<DWORD> angleColors;
			angleColors.emplace_back(0xf56c2e);
			angleColors.emplace_back(0xfab84a);
			angleColors.emplace_back(0xbbe28a);
			angleColors.emplace_back(0x41e9f8);
			angleColors.emplace_back(0xb45d95);
			angleColors.emplace_back(0xf43651);
			angleColors.emplace_back(0xf54a39);
			pRaderChart->SetAngleColors(angleColors);
            pRaderChart->SetTextColor(0xc6c7de);
            pRaderChart->SetPadding({ 30, 25, 30, 25 });

            std::vector<CDuiPoint> pts;
            pts.push_back({ -15, -23 });
            pts.push_back({ -30, -8 });
            pts.push_back({ -30, -5 });
            pts.push_back({ -20, 5 });
            pts.push_back({ 8, -3 });
            pts.push_back({ 10, -6 });
            pts.push_back({ 10, -8 });
            pRaderChart->SetRevisedTextPoses(pts);

            std::vector<std::wstring> texts;
            texts.emplace_back(L"气愤");
            texts.emplace_back(L"蔑视");
            texts.emplace_back(L"厌恶");
            texts.emplace_back(L"恐惧");
            texts.emplace_back(L"愉悦");
            texts.emplace_back(L"悲伤");
            texts.emplace_back(L"惊讶");
            pRaderChart->SetCornerTexts(texts);

            std::vector<DWORD> curveLineColors;
            curveLineColors.emplace_back(0x7E4EED);
            curveLineColors.emplace_back(0x6D4FEB);
            curveLineColors.emplace_back(0x5B53EF);
            curveLineColors.emplace_back(0x4C57EE);
            curveLineColors.emplace_back(0x3C58EA);
            curveLineColors.emplace_back(0x2D5FEF);
            curveLineColors.emplace_back(0x4C57EE);
            curveLineColors.emplace_back(0x2664F0);
            curveLineColors.emplace_back(0x1A66E9);
            curveLineColors.emplace_back(0x116FEC);
            curveLineColors.emplace_back(0x1077F1);
            curveLineColors.emplace_back(0x1085EC);
            curveLineColors.emplace_back(0x1796E2);
            curveLineColors.emplace_back(0x1CA7E3);
            curveLineColors.emplace_back(0x23BDDB);
            curveLineColors.emplace_back(0x2BC0D9);
            curveLineColors.emplace_back(0x2BC0D9);

#ifdef _DEBUG
            OnUpdateEmotionAnalysisData(L"12:56:22 客户在聊到`企业厂房`问题时表现出失望、难过的情绪");
            OnUpdateEmotionAnalysisData(L"12:59:22 客户在聊到`企业厂房`问题时表现出失望、难过的情绪");
            OnUpdateEmotionAnalysisData(L"13:02:22 客户在聊到`企业厂房`问题时表现出失望、难过的情绪");
            OnUpdateEmotionAnalysisData(L"13:06:22 客户在聊到`企业厂房`问题时表现出蔑视、惊讶、难过的情绪");
            OnUpdateEmotionAnalysisData(L"13:26:22 客户在聊到`创业成功时`问题时表现出愉悦、难过的情绪");
            OnUpdateEmotionAnalysisData(L"13:26:22 客户在聊到`创业成功时`问题时表现出愉悦、难过的情绪");
            OnUpdateEmotionAnalysisData(L"13:26:22 客户在聊到`创业成功时`问题时表现出愉悦、难过的情绪");
            OnUpdateEmotionAnalysisData(L"13:26:22 客户在聊到`创业成功时`问题时表现出愉悦、难过的情绪");
            OnUpdateEmotionAnalysisData(L"13:26:22 客户在聊到`创业成功时`问题时表现出愉悦、难过的情绪");
            OnUpdateEmotionAnalysisData(L"13:26:22 客户在聊到`创业成功时`问题时表现出愉悦、难过的情绪");
            OnUpdateEmotionAnalysisData(L"13:26:22 客户在聊到`创业成功时`问题时表现出愉悦、难过的情绪");
            OnUpdateEmotionAnalysisData(L"13:26:22 客户在聊到`创业成功时`问题时表现出愉悦、难过的情绪");
            OnUpdateEmotionAnalysisData(L"13:26:22 客户在聊到`创业成功时`问题时表现出愉悦、难过的情绪");
            OnUpdateEmotionAnalysisData(L"13:26:22 客户在聊到`创业成功时`问题时表现出愉悦、难过的情绪");
            OnUpdateEmotionAnalysisData(L"13:26:22 客户在聊到`创业成功时`问题时表现出愉悦、难过的情绪");
            OnUpdateEmotionAnalysisData(L"13:26:22 客户在聊到`创业成功时`问题时表现出愉悦、难过的情绪");
            OnUpdateEmotionAnalysisData(L"13:26:22 客户在聊到`创业成功时`问题时表现出愉悦、难过的情绪");
#endif
            for (int i = 0; i < m_pCurveContainer->GetCount(); ++i)
            {
                CHorizontalLayoutUI* pC = static_cast<CHorizontalLayoutUI*>(m_pCurveContainer->GetItemAt(i));
                CCurveChartUI* pCurveChart = new CCurveChartUI;
                static_cast<CContainerUI*>(pC->GetItemAt(1))->Add(pCurveChart);
                /*
                auto pFirst = pC->GetItemAt(0);

                pFirst->SetBorderStyle(PS_DOT);
                pFirst->SetBottomBorderSize(1);
                pFirst->SetBorderColor(0xdddddd);*/
				pCurveChart->SetBorderStyle(PS_DOT);
				pCurveChart->SetBottomBorderSize(1);
				pCurveChart->SetBorderColor(0xdddddd);
                pCurveChart->SetCurveLineColor(curveLineColors[i]);
                pCurveChart->SetPadding({ 1, 2, 0, 2 });
            }
        }

		void CMainFrame::OnUpdateEmotionAnalysisData(const std::wstring& str)
		{
			CTextUI* pLbl = new CTextUI;

			pLbl->SetText((std::wstring(L"<f 2>") + std::to_wstring(m_nEmotionAnalysis++) + L". " + str).c_str());
			pLbl->SetTextColor(0xc6c7de);
			pLbl->SetShowHtml(true);
			pLbl->SetTextPadding({ 5, 6, 5, 7 });

			m_pEmotionAnalysisContainer->AddAt(pLbl, 0);
		}
    } // UI
} // MyWeb
