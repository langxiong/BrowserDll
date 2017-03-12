#include "stdafx.h"
#include "MainFrame.h"
#include "AppCommon.h"
#include "Poco/FileStream.h"
#include "Poco/StreamCopier.h"
#include "Poco/JSON/JSON.h"
#include "Poco/JSON/Parser.h"
#include "Poco/Nullable.h"
#include "Poco/Dynamic/Struct.h"

#include <vector>
#include <unordered_map>
#include <random>
#include <ppltasks.h>

#include "DateTimeUtil.h"
#include "StringHelper.h"
#include "UITestBrowser.h"


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
            m_frameID(frameID),
            m_pTestBrowser(nullptr),
            m_pUrlEdit(nullptr),
            m_pJSCodeRichEdit(nullptr)
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

            if (msg.sType == DUI_MSGTYPE_RETURN)
            {
                OnMsgReturn(msg);
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

        void CMainFrame::OnMsgReturn(DuiLib::TNotifyUI & msg)
        {
            if (!m_pTestBrowser)
            {
                return;
            }

            if (msg.pSender == m_pUrlEdit)
            {
                m_pTestBrowser->NavigateUrl(m_pUrlEdit->GetText().GetData());
                return;
            }

            if (msg.pSender == m_pJSCodeRichEdit)
            {
                m_pTestBrowser->ExecuteJscode(m_pJSCodeRichEdit->GetText().GetData());
                return;
            }
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
			if (_tcscmp(pstrClass, _T("TestBrowser")) == 0)
			{
                m_pTestBrowser = new CTestBrowserUI;
                return m_pTestBrowser;
			}

            return WindowImplBase::CreateControl(pstrClass);
        }

        void CMainFrame::InitControls()
        {
            m_pUrlEdit = static_cast<CEditUI*>(m_PaintManager.FindControl(_T("url#edit")));
            m_pJSCodeRichEdit = static_cast<CRichEditUI*>(m_PaintManager.FindControl(_T("jscode#richedit")));
        }
	} // UI
} // MyWeb
