#include "stdafx.h"
#include "MyWindow.h"

#include <WindowsX.h>

namespace MyWeb
{
    MyWindow::MyWindow() : 
        m_hWnd(NULL), 
        m_OldWndProc(::DefWindowProc), 
        m_bSubclassed(false)
    {
    }

    HWND MyWindow::GetHWND() const
    {
        return m_hWnd;
    }

    UINT MyWindow::GetClassStyle() const
    {
        return 0;
    }

    LPCTSTR MyWindow::GetSuperClassName() const
    {
        return NULL;
    }

    MyWindow::operator HWND() const
    {
        return m_hWnd;
    }

    HWND MyWindow::CreateDuiWindow(HWND hwndParent, LPCTSTR pstrWindowName, DWORD dwStyle /*=0*/, DWORD dwExStyle /*=0*/)
    {
        return Create(hwndParent, pstrWindowName, dwStyle, dwExStyle, 0, 0, 0, 0, NULL);
    }

    HWND MyWindow::Create(HWND hwndParent, LPCTSTR pstrName, DWORD dwStyle, DWORD dwExStyle, const RECT rc, HMENU hMenu)
    {
        return Create(hwndParent, pstrName, dwStyle, dwExStyle, rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top, hMenu);
    }

    HWND MyWindow::Create(HWND hwndParent, LPCTSTR pstrName, DWORD dwStyle, DWORD dwExStyle, int x, int y, int cx, int cy, HMENU hMenu)
    {
        if (GetSuperClassName() != NULL && !RegisterSuperclass()) return NULL;
        if (GetSuperClassName() == NULL && !RegisterWindowClass()) return NULL;
        m_hWnd = ::CreateWindowEx(dwExStyle, GetWindowClassName(), pstrName, dwStyle, x, y, cx, cy, hwndParent, hMenu, NULL, this);
        assert(m_hWnd != NULL);
        return m_hWnd;
    }

    HWND MyWindow::Subclass(HWND hWnd)
    {
        assert(::IsWindow(hWnd));
        assert(m_hWnd == NULL);
        m_OldWndProc = SubclassWindow(hWnd, __WndProc);
        if (m_OldWndProc == NULL) return NULL;
        m_bSubclassed = true;
        m_hWnd = hWnd;
        ::SetWindowLongPtr(hWnd, GWLP_USERDATA, reinterpret_cast<LPARAM>(this));
        return m_hWnd;
    }

    void MyWindow::Unsubclass()
    {
        assert(::IsWindow(m_hWnd));
        if (!::IsWindow(m_hWnd)) return;
        if (!m_bSubclassed) return;
        SubclassWindow(m_hWnd, m_OldWndProc);
        m_OldWndProc = ::DefWindowProc;
        m_bSubclassed = false;
    }

    void MyWindow::ShowWindow(bool bShow /*= true*/, bool bTakeFocus /*= false*/)
    {
        assert(::IsWindow(m_hWnd));
        if (!::IsWindow(m_hWnd)) return;
        ::ShowWindow(m_hWnd, bShow ? (bTakeFocus ? SW_SHOWNORMAL : SW_SHOWNOACTIVATE) : SW_HIDE);
    }

    void MyWindow::Close(UINT nRet)
    {
        assert(::IsWindow(m_hWnd));
        if (!::IsWindow(m_hWnd)) return;
        PostMessage(WM_CLOSE, (WPARAM)nRet, 0L);
    }

    bool MyWindow::RegisterWindowClass()
    {
        WNDCLASS wc = { 0 };
        wc.style = GetClassStyle();
        wc.cbClsExtra = 0;
        wc.cbWndExtra = 0;
        wc.hIcon = NULL;
        wc.lpfnWndProc = MyWindow::__WndProc;
        wc.hInstance = NULL;
        wc.hCursor = ::LoadCursor(NULL, IDC_ARROW);
        wc.hbrBackground = NULL;
        wc.lpszMenuName = NULL;
        wc.lpszClassName = GetWindowClassName();
        ATOM ret = ::RegisterClass(&wc);
        assert(ret != NULL || ::GetLastError() == ERROR_CLASS_ALREADY_EXISTS);
        return ret != NULL || ::GetLastError() == ERROR_CLASS_ALREADY_EXISTS;
    }

    bool MyWindow::RegisterSuperclass()
    {
        // Get the class information from an existing
        // window so we can subclass it later on...
        WNDCLASSEX wc = { 0 };
        wc.cbSize = sizeof(WNDCLASSEX);
        if (!::GetClassInfoEx(NULL, GetSuperClassName(), &wc)) {
            assert(!"Unable to locate window class");
            return NULL;
        }
        m_OldWndProc = wc.lpfnWndProc;
        wc.lpfnWndProc = MyWindow::__ControlProc;
        wc.hInstance = NULL;
        wc.lpszClassName = GetWindowClassName();
        ATOM ret = ::RegisterClassEx(&wc);
        assert(ret != NULL || ::GetLastError() == ERROR_CLASS_ALREADY_EXISTS);
        return ret != NULL || ::GetLastError() == ERROR_CLASS_ALREADY_EXISTS;
    }

    LRESULT CALLBACK MyWindow::__WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
    {
        MyWindow* pThis = NULL;
        if (uMsg == WM_NCCREATE) {
            LPCREATESTRUCT lpcs = reinterpret_cast<LPCREATESTRUCT>(lParam);
            pThis = static_cast<MyWindow*>(lpcs->lpCreateParams);
            pThis->m_hWnd = hWnd;
            ::SetWindowLongPtr(hWnd, GWLP_USERDATA, reinterpret_cast<LPARAM>(pThis));
        }
        else {
            pThis = reinterpret_cast<MyWindow*>(::GetWindowLongPtr(hWnd, GWLP_USERDATA));
            if (uMsg == WM_NCDESTROY && pThis != NULL) {
                LRESULT lRes = ::CallWindowProc(pThis->m_OldWndProc, hWnd, uMsg, wParam, lParam);
                ::SetWindowLongPtr(pThis->m_hWnd, GWLP_USERDATA, 0L);
                if (pThis->m_bSubclassed) pThis->Unsubclass();
                pThis->m_hWnd = NULL;
                pThis->OnFinalMessage(hWnd);
                return lRes;
            }
        }
        if (pThis != NULL)
        {
            return pThis->HandleMessage(uMsg, wParam, lParam);
        }
        else
        {
            return ::DefWindowProc(hWnd, uMsg, wParam, lParam);
        }
    }

    LRESULT CALLBACK MyWindow::__ControlProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
    {
        MyWindow* pThis = NULL;
        if (uMsg == WM_NCCREATE) {
            LPCREATESTRUCT lpcs = reinterpret_cast<LPCREATESTRUCT>(lParam);
            pThis = static_cast<MyWindow*>(lpcs->lpCreateParams);
            ::SetProp(hWnd, _T("WndX"), (HANDLE)pThis);
            pThis->m_hWnd = hWnd;
        }
        else {
            pThis = reinterpret_cast<MyWindow*>(::GetProp(hWnd, _T("WndX")));
            if (uMsg == WM_NCDESTROY && pThis != NULL) {
                LRESULT lRes = ::CallWindowProc(pThis->m_OldWndProc, hWnd, uMsg, wParam, lParam);
                if (pThis->m_bSubclassed) pThis->Unsubclass();
                ::SetProp(hWnd, _T("WndX"), NULL);
                pThis->m_hWnd = NULL;
                pThis->OnFinalMessage(hWnd);
                return lRes;
            }
        }
        if (pThis != NULL) {
            return pThis->HandleMessage(uMsg, wParam, lParam);
        }
        else {
            return ::DefWindowProc(hWnd, uMsg, wParam, lParam);
        }
    }

    LRESULT MyWindow::SendMessage(UINT uMsg, WPARAM wParam /*= 0*/, LPARAM lParam /*= 0*/)
    {
        assert(::IsWindow(m_hWnd));
        return ::SendMessage(m_hWnd, uMsg, wParam, lParam);
    }

    LRESULT MyWindow::PostMessage(UINT uMsg, WPARAM wParam /*= 0*/, LPARAM lParam /*= 0*/)
    {
        assert(::IsWindow(m_hWnd));
        return ::PostMessage(m_hWnd, uMsg, wParam, lParam);
    }

    void MyWindow::ResizeClient(int cx /*= -1*/, int cy /*= -1*/)
    {
        assert(::IsWindow(m_hWnd));
        RECT rc = { 0 };
        if (!::GetClientRect(m_hWnd, &rc)) return;
        if (cx != -1) rc.right = cx;
        if (cy != -1) rc.bottom = cy;
        if (!::AdjustWindowRectEx(&rc, GetWindowStyle(m_hWnd), (!(GetWindowStyle(m_hWnd) & WS_CHILD) && (::GetMenu(m_hWnd) != NULL)), GetWindowExStyle(m_hWnd))) return;
        ::SetWindowPos(m_hWnd, NULL, 0, 0, rc.right - rc.left, rc.bottom - rc.top, SWP_NOZORDER | SWP_NOMOVE | SWP_NOACTIVATE);
    }

    LRESULT MyWindow::HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam)
    {
        return ::CallWindowProc(m_OldWndProc, m_hWnd, uMsg, wParam, lParam);
    }

    void MyWindow::OnFinalMessage(HWND /*hWnd*/)
    {
    }
}