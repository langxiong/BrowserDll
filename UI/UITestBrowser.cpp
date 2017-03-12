#include "StdAfx.h"
#include "UITestBrowser.h"

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
        m_pBindWnd(NULL)
    {
    }

    CTestBrowserUI::~CTestBrowserUI()
    {
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
} // DuiLib
