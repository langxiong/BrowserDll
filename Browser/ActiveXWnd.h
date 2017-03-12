#pragma once

#include "MyWindow.h"

namespace MyWeb
{
    class CActiveXCtrl;
    class CActiveXWnd : public MyWindow
    {
    public:
        HWND Init(CActiveXCtrl* pOwner, HWND hWndParent);

        LPCTSTR GetWindowClassName() const;
        void OnFinalMessage(HWND hWnd);

        LRESULT HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam);

    protected:
        void DoVerb(LONG iVerb);

        LRESULT OnMouseActivate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
        LRESULT OnSetFocus(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
        LRESULT OnKillFocus(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
        LRESULT OnEraseBkgnd(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
        LRESULT OnPaint(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

    protected:
        CActiveXCtrl* m_pOwner;
    };

}