#pragma once

#pragma warning(push, 3)
#include <unknwn.h>
#include <exdisp.h>
#include <mshtmhst.h>
#pragma warning(pop)

namespace MyWeb
{
    class MyWindow
    {
    public:
        MyWindow();

        HWND GetHWND() const;
        operator HWND() const;

        bool RegisterWindowClass();
        bool RegisterSuperclass();

        HWND Create(HWND hwndParent, LPCTSTR pstrName, DWORD dwStyle, DWORD dwExStyle, const RECT rc, HMENU hMenu = NULL);
        HWND Create(HWND hwndParent, LPCTSTR pstrName, DWORD dwStyle, DWORD dwExStyle, int x = CW_USEDEFAULT, int y = CW_USEDEFAULT, int cx = CW_USEDEFAULT, int cy = CW_USEDEFAULT, HMENU hMenu = NULL);
        HWND CreateDuiWindow(HWND hwndParent, LPCTSTR pstrWindowName, DWORD dwStyle = 0, DWORD dwExStyle = 0);
        HWND Subclass(HWND hWnd);
        void Unsubclass();
        void ShowWindow(bool bShow = true, bool bTakeFocus = true);
        void Close(UINT nRet = IDOK);
       
        LRESULT SendMessage(UINT uMsg, WPARAM wParam = 0, LPARAM lParam = 0L);
        LRESULT PostMessage(UINT uMsg, WPARAM wParam = 0, LPARAM lParam = 0L);
        void ResizeClient(int cx = -1, int cy = -1);

    protected:
        virtual LPCTSTR GetWindowClassName() const = 0;
        virtual LPCTSTR GetSuperClassName() const;
        virtual UINT GetClassStyle() const;

        virtual LRESULT HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam);
        virtual void OnFinalMessage(HWND hWnd);

        static LRESULT CALLBACK __WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
        static LRESULT CALLBACK __ControlProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

    protected:
        HWND m_hWnd;
        WNDPROC m_OldWndProc;
        bool m_bSubclassed;
    };
}

