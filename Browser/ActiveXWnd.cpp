#include "stdafx.h"
#include "ActiveXCtrl.h"
#include "MyBrowserCtrl.h"
#include "ActiveXWnd.h"

namespace MyWeb
{
    HWND CActiveXWnd::Init(CActiveXCtrl* pOwner, HWND hWndParent)
    {
        m_pOwner = pOwner;
        UINT uStyle = WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN;
        Create(hWndParent, _T("MyUIActiveX"), uStyle, 0L, 0, 0, 0, 0, NULL);
        return m_hWnd;
    }

    LPCTSTR CActiveXWnd::GetWindowClassName() const
    {
        return _T("MyActiveXWnd");
    }

    void CActiveXWnd::OnFinalMessage(HWND hWnd)
    {
        //delete this; // 这里不需要清理，CActiveXUI会清理的
    }

    void CActiveXWnd::DoVerb(LONG iVerb)
    {
        if (m_pOwner == NULL) return;
        if (m_pOwner->m_pOwner == NULL) return;
        IOleObject* pUnk = NULL;
        m_pOwner->m_pOwner->GetControl(IID_IOleObject, (LPVOID*)&pUnk);
        if (pUnk == NULL) return;
        CComPtr<IOleObject> RefOleObject = pUnk;
        IOleClientSite* pOleClientSite = NULL;
        m_pOwner->QueryInterface(IID_IOleClientSite, (LPVOID*)&pOleClientSite);
        CComPtr<IOleClientSite> RefOleClientSite = pOleClientSite;
        pUnk->DoVerb(iVerb, NULL, pOleClientSite, 0, m_hWnd, &m_pOwner->m_pOwner->GetPos());
    }

    LRESULT CActiveXWnd::HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam)
    {
        LRESULT lRes = 0;
        BOOL bHandled = TRUE;
        switch (uMsg) {
        case WM_PAINT:         lRes = OnPaint(uMsg, wParam, lParam, bHandled); break;
        case WM_SETFOCUS:      lRes = OnSetFocus(uMsg, wParam, lParam, bHandled); break;
        case WM_KILLFOCUS:     lRes = OnKillFocus(uMsg, wParam, lParam, bHandled); break;
        case WM_ERASEBKGND:    lRes = OnEraseBkgnd(uMsg, wParam, lParam, bHandled); break;
        case WM_MOUSEACTIVATE: lRes = OnMouseActivate(uMsg, wParam, lParam, bHandled); break;
        case WM_MOUSEWHEEL: break;
        default:
            bHandled = FALSE;
        }
        if (!bHandled) return MyWindow::HandleMessage(uMsg, wParam, lParam);
        return lRes;
    }

    LRESULT CActiveXWnd::OnEraseBkgnd(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
    {
        if (m_pOwner->m_pViewObject == NULL) bHandled = FALSE;
        return 1;
    }

    LRESULT CActiveXWnd::OnMouseActivate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
    {
        IOleObject* pUnk = NULL;
        m_pOwner->m_pOwner->GetControl(IID_IOleObject, (LPVOID*)&pUnk);
        if (pUnk == NULL) return 0;
        CComPtr<IOleObject> RefOleObject = pUnk;
        DWORD dwMiscStatus = 0;
        pUnk->GetMiscStatus(DVASPECT_CONTENT, &dwMiscStatus);
        if ((dwMiscStatus & OLEMISC_NOUIACTIVATE) != 0) return 0;
        if (!m_pOwner->m_bInPlaceActive) DoVerb(OLEIVERB_INPLACEACTIVATE);
        bHandled = FALSE;
        return 0;
    }

    LRESULT CActiveXWnd::OnSetFocus(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
    {
        bHandled = FALSE;
        m_pOwner->m_bFocused = true;
        if (!m_pOwner->m_bUIActivated) DoVerb(OLEIVERB_UIACTIVATE);
        return 0;
    }

    LRESULT CActiveXWnd::OnKillFocus(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
    {
        bHandled = FALSE;
        m_pOwner->m_bFocused = false;
        return 0;
    }

    LRESULT CActiveXWnd::OnPaint(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
    {
        PAINTSTRUCT ps = { 0 };
        ::BeginPaint(m_hWnd, &ps);
        ::EndPaint(m_hWnd, &ps);
        return 1;
    }
}
