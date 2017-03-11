#include "StdAfx.h"
#include "UINewWebBrowser.h"
#include "../Core/UIWebDefine.h"

namespace DuiLib
{
    class CHostWnd : public CWindowWnd
    {
    public:
        virtual LPCTSTR GetWindowClassName() const
        {
            return _T("HostWndForUIWebBrowserByXL");
        }

        virtual void OnFinalMessage(HWND hWnd)
        {
            CWindowWnd::OnFinalMessage(hWnd);
        }
    };
    
    class CWebWnd : public CWindowWnd
    {
    public:
        CWebWnd(CPaintManagerUI* pPaintManager):
            m_pPaintManager(pPaintManager)
        {

        }

        ~CWebWnd()
        {
            ::PostQuitMessage(0);
        }

        void WaitWndClosed()
        {
        }

        virtual LPCTSTR GetWindowClassName() const
        {
            return _T("WebWndForUIWebBrowserByXL");
        }

        virtual void OnFinalMessage(HWND hWnd)
        {
            CWindowWnd::OnFinalMessage(hWnd);
        }

        virtual LRESULT HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam)
        {
            LRESULT lRes = S_OK;
            if (m_pPaintManager->MessageHandler(uMsg, wParam, lParam, lRes))
                return lRes;

      /*      if (uMsg == WM_CLOSE)
            {
                PostMessage(WM_DESTROY);
                return S_OK;
            }

            if (uMsg == WM_DESTROY)
            {
                if (CWindowWnd::HandleMessage(uMsg, wParam, lParam))
                {
                }
                PostQuitMessage(0);
                return 0;
            }
*/
            return CWindowWnd::HandleMessage(uMsg, wParam, lParam);
        }

    private:
        CPaintManagerUI* m_pPaintManager;
    };

    CNewWebBrowserUI::CNewWebBrowserUI():
        m_pBindWnd(NULL),
        m_pThread(NULL),
        m_hInitEvent(NULL),
        m_dwWebWorkThreadId(0)
    {
    }

    CNewWebBrowserUI::~CNewWebBrowserUI()
    {
        // ::PostThreadMessage(m_dwWebWorkThreadId, WM_QUIT, 0, 0);
        ::PostThreadMessage(m_dwWebWorkThreadId, WM_THREAD_WEB_QUIT_BROWSER, 0, 0);
        Sleep(0);
        if (m_pBindWnd)
        {
            if (::IsWindow(m_pBindWnd->GetHWND()))
            {
                m_pBindWnd->Close();
            }
            m_pManager->AddDelayedDestroyedWnd(m_pBindWnd);
        }
        m_pManager->AddDelayedDestroyedThread(m_pThread);
    }

    void CNewWebBrowserUI::DoInit()
    {
        if (m_pBindWnd)
        {
            return;
        }
        m_pBindWnd = new CHostWnd;
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
            OnSize += MakeDelegate(this, &CNewWebBrowserUI::onSize);
            m_hInitEvent = ::CreateEvent(NULL, FALSE, FALSE, NULL);
            m_pThread = new std::thread([&]()
            {
                EnterWebThreadMessageLoop();
            });
            ::WaitForSingleObject(m_hInitEvent, INFINITE);
        }
    }

    LPCTSTR CNewWebBrowserUI::GetClass() const
    {
        return _T("NewWebBrowserUI");
    }

    LPVOID CNewWebBrowserUI::GetInterface(LPCTSTR pstrName)
    {
        if (_tcscmp(pstrName, DUI_CTR_NEWWEBBROWSER) == 0) return static_cast<CNewWebBrowserUI*>(this);
        return CContainerUI::GetInterface(pstrName);
    }

    bool CNewWebBrowserUI::onSize(void* pParam)
    {
        if (m_pBindWnd)
        {
            RECT rc = GetPos();
            ::SetWindowPos(m_pBindWnd->GetHWND(), NULL, rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top,
                SWP_NOACTIVATE | SWP_NOREDRAW | SWP_NOZORDER);
            LPRECT pRC = new RECT;
            pRC->left = pRC->top = 0;
            pRC->right = rc.right - rc.left;
            pRC->bottom = rc.bottom - rc.top;

            if (::PostThreadMessage(m_dwWebWorkThreadId, WM_USER_WEBBROWSER_SIZE_CHANGED, (WPARAM)pRC, NULL))
            {
                
            }
            else
            {
                delete pRC;
            }
        }
        return true;
    }

    bool CNewWebBrowserUI::IsVisible() const
    {
        return CContainerUI::IsVisible();
        bool isVisible = CContainerUI::IsVisible();
        if (isVisible && m_pBindWnd)
        {
            return ::IsWindowVisible(m_pBindWnd->GetHWND()) ? true : false;
        }
        return isVisible;
    }

    void CNewWebBrowserUI::SetVisible(bool bVisible /*= true*/)
    {
        CContainerUI::SetVisible(bVisible);
        
        if (m_pBindWnd)
        {
            m_pBindWnd->ShowWindow(bVisible);
        }
        ::PostThreadMessage(m_dwWebWorkThreadId, WM_USER_SHOW_WEBBROWSER, bVisible ? TRUE : FALSE, NULL);
    }

    void CNewWebBrowserUI::EnterWebThreadMessageLoop()
    {
        CPaintManagerUI* pManager = new CPaintManagerUI;
        CWebCtrl* pWebCtrl = new CWebCtrl;
        CWebWnd* pHostWnd = new CWebWnd(pManager);
        MSG msg = { 0 };
        HRESULT hr = ::CoInitialize(nullptr);
        ::PeekMessage(&msg, NULL, NULL, NULL, PM_NOREMOVE);
        ::PostThreadMessage(::GetCurrentThreadId(), WM_THREAD_WEB_CREATE_BROWSER, 0, 0);
        m_dwWebWorkThreadId = ::GetCurrentThreadId();
        ::SetEvent(m_hInitEvent);

        while (::GetMessage(&msg, nullptr, 0, 0) != 0)
        {
            if (msg.message == WM_THREAD_WEB_CREATE_BROWSER)
            {
                OnCreateBrowser(pManager, pWebCtrl, pHostWnd);
            }
            else if (msg.message == WM_THREAD_WEB_QUIT_BROWSER)
            {
                OnQuitBrowser();
                if (pHostWnd)
                {
                    SetParent(pHostWnd->GetHWND(), nullptr);
                }
                if (pWebCtrl)
                {
                    delete pWebCtrl;
                    pWebCtrl = nullptr;
                }
                if (::IsWindow(pHostWnd->GetHWND()))
                {
                    pHostWnd->Close();
                }
            }
            else if (msg.message == WM_USER_WEBBROWSER_SIZE_CHANGED)
            {
                OnSetBrowserPos(msg, pWebCtrl, pHostWnd);
            }
            else if (msg.message == WM_USER_SHOW_WEBBROWSER)
            {
                OnShowBrowser(msg.wParam ? true : false, pHostWnd);
            }

            if (!HandleWebMessage(pManager, msg))
            {
                ::TranslateMessage(&msg);
                ::DispatchMessage(&msg);
            }
        }
        if (pWebCtrl)
        {
            delete pWebCtrl;
        }

        if (pManager)
        {
            delete pManager;
        }
        if (pHostWnd)
        {
            delete pHostWnd;
        }
        ::CoUninitialize();
    }

    void CNewWebBrowserUI::OnCreateBrowser(CPaintManagerUI* pPaintManager, CWebCtrl* pWebCtrl, CWebWnd* pHostWnd)
    {
        DWORD dwStyle = IsVisible() ? (WS_VISIBLE | WS_POPUP) : WS_POPUP;
        HWND hWnd = pHostWnd->CreateDuiWindow(NULL, NULL, dwStyle);
        pPaintManager->Init(hWnd, false);

        CActiveXUI* pActiveUI = new CActiveXUI;
        pPaintManager->InitControls(pActiveUI);
        pWebCtrl->m_spActiveCtl.reset(pActiveUI);
        pActiveUI->SetDelayCreate(false);
        if (pActiveUI->CreateControl(CLSID_WebBrowser))
        {
            LONG styleValue = ::GetWindowLong(pHostWnd->GetHWND(), GWL_STYLE);
            styleValue &= ~WS_POPUP;
            ::SetWindowLong(pHostWnd->GetHWND(), GWL_STYLE, styleValue | WS_CHILD);
            ::SetParent(pHostWnd->GetHWND(), m_pBindWnd->GetHWND());
        }
        HRESULT hr = pActiveUI->GetControl(IID_IWebBrowser2, (LPVOID*) &pWebCtrl->m_spWebBrowser2);
        if (SUCCEEDED(hr))
        {
            CVariant url;
            url.vt = VT_BSTR;
            url.bstrVal = T2BSTR(_T("about:blank"));
            pWebCtrl->m_spWebBrowser2->Navigate2(&url, NULL, NULL, NULL, NULL);
        }
    }

    void CNewWebBrowserUI::OnQuitBrowser()
    {
    }

    void CNewWebBrowserUI::OnSetBrowserPos(MSG msg, CWebCtrl* pWebCtrl, CWebWnd* pHostWnd)
    {
        LPRECT pRc = reinterpret_cast<LPRECT>(msg.wParam);
        if (!pRc)
        {
            return;
        }
        ::SetWindowPos(pHostWnd->GetHWND(), NULL, 0, 0, pRc->right - pRc->left, pRc->bottom - pRc->top,
            SWP_NOACTIVATE | SWP_NOREDRAW | SWP_NOZORDER);
        pWebCtrl->m_spActiveCtl->SetPos(*pRc);
        delete pRc;

    }

    void CNewWebBrowserUI::OnShowBrowser(bool isVisible, CWebWnd* pHostWnd)
    {
        if (pHostWnd)
        {
            pHostWnd->ShowWindow(isVisible);
        }
    }

    bool CNewWebBrowserUI::HandleWebMessage(CPaintManagerUI* pPaintManager, MSG &msg)
    {
        bool isWebPaintManagerHandle = false;
        if (msg.hwnd == pPaintManager->GetPaintWindow())
        {
            isWebPaintManagerHandle = true;
        }
        else
        {
            HWND hWndParent = NULL;
            UINT uStyle = GetWindowStyle(hWndParent);
            if (uStyle & WS_CHILD)
            {
                hWndParent = ::GetParent(msg.hwnd);
            }
            while (hWndParent)
            {
                if (hWndParent == pPaintManager->GetPaintWindow())
                {
                    isWebPaintManagerHandle = true;
                    break;
                }

                uStyle = GetWindowStyle(hWndParent);
                if (uStyle & WS_CHILD)
                {
                    hWndParent = ::GetParent(hWndParent);
                }
                else
                {
                    break;
                }
            }
        }

        if (isWebPaintManagerHandle)
        {
            if (pPaintManager->TranslateAccelerator(&msg))
            {
                return true;
            }
            LRESULT lRes = 0;
            if (pPaintManager->PreMessageHandler(msg.message, msg.wParam, msg.lParam, lRes))
            {
                return true;
            }
        }
        return false;
    }

} // DuiLib