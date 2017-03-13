#include "StdAfx.h"
#include "UITestBrowser.h"

namespace
{
    MyWeb::UI::CUISubsystem& GetUISubsystem()
    {
        return Poco::Util::Application::instance().getSubsystem<MyWeb::UI::CUISubsystem>();
    }

}
namespace DuiLib
{
    class CBindWnd : public CWindowWnd
    {
    public:
        virtual LPCTSTR GetWindowClassName() const
        {
            return _T("TestBrowserWndByXL");
        }
    };

    CTestBrowserUI::CTestBrowserUI():
        m_pBindWnd(NULL),
        m_nIndex(-1)
    {
    }

    CTestBrowserUI::~CTestBrowserUI()
    {
        if (m_nIndex != -1)
        {
            GetUISubsystem().DestroyBrowserCtrl(m_nIndex);
        }
        if (m_pBindWnd)
        {
            delete m_pBindWnd;
        }

    }

    void CTestBrowserUI::DoInit()
    {
        if (m_pBindWnd)
        {
            return;
        }
        m_pBindWnd = new CBindWnd;
        if (m_pManager)
        {
            DWORD dwStyle = WS_CHILD | WS_CLIPSIBLINGS | WS_CLIPCHILDREN;
            if (IsVisible())
            {
                dwStyle |= WS_VISIBLE;
            }
            HWND hWnd = m_pBindWnd->CreateDuiWindow(m_pManager->GetPaintWindow(), NULL, dwStyle);
            if (!hWnd)
            {
                return;
            }

            m_nIndex = GetUISubsystem().CreateBrowserCtrl(hWnd);
        }
    }

    LPCTSTR CTestBrowserUI::GetClass() const
    {
        return _T("CameraVideoUI");
    }

    LPVOID CTestBrowserUI::GetInterface(LPCTSTR pstrName)
    {
        if (_tcscmp(pstrName, _T("TestBrowser")) == 0) return static_cast<CTestBrowserUI*>(this);
        return CContainerUI::GetInterface(pstrName);
    }

    void CTestBrowserUI::SetPos(RECT rc)
    {
        CControlUI::SetPos(rc);

		if (m_pBindWnd && IsVisible())
		{
			::SetWindowPos(m_pBindWnd->GetHWND(), NULL, rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top,
				SWP_NOACTIVATE | SWP_NOREDRAW | SWP_NOZORDER | SWP_SHOWWINDOW);

            GetUISubsystem().SetBrowserCtrlPos(m_nIndex, rc);
		}
    }

    bool CTestBrowserUI::IsVisible() const
    {
        return CContainerUI::IsVisible();
        bool isVisible = CContainerUI::IsVisible();
        if (isVisible && m_pBindWnd)
        {
            return ::IsWindowVisible(m_pBindWnd->GetHWND()) ? true : false;
        }
        return isVisible;
    }

    void CTestBrowserUI::SetVisible(bool bVisible /*= true*/)
    {
        CContainerUI::SetVisible(bVisible);
        
        if (m_pBindWnd)
        {
            m_pBindWnd->ShowWindow(bVisible);
        }
    }
    void CTestBrowserUI::NavigateUrl(const MyString & url)
    {
        GetUISubsystem().NavigateUrl(m_nIndex, url);
    }
    void CTestBrowserUI::ExecuteJscode(const MyString & jscode)
    {
        GetUISubsystem().ExecuteJscode(m_nIndex, jscode);
    }
    void CTestBrowserUI::RegisterMethod(const MyString & methodName, void pFun(DISPPARAMS *params, VARIANT *result))
    {
        GetUISubsystem().RegisterBrowserCallback(m_nIndex, methodName.c_str(), pFun);
    }
} // DuiLib
