#include "stdafx.h"
#include "MyBrowserCtrl.h"
#include "ActiveXCtrl.h"
#include "ActiveXWnd.h"

#include <WindowsX.h>

#define lengthof(x) (sizeof(x)/sizeof(*x))

namespace MyWeb
{
    namespace
    {
        static void PixelToHiMetric(const SIZEL* lpSizeInPix, LPSIZEL lpSizeInHiMetric)
        {
#define HIMETRIC_PER_INCH   2540
#define MAP_PIX_TO_LOGHIM(x,ppli)   MulDiv(HIMETRIC_PER_INCH, (x), (ppli))
#define MAP_LOGHIM_TO_PIX(x,ppli)   MulDiv((ppli), (x), HIMETRIC_PER_INCH)
            int nPixelsPerInchX;    // Pixels per logical inch along width
            int nPixelsPerInchY;    // Pixels per logical inch along height
            HDC hDCScreen = ::GetDC(NULL);
            nPixelsPerInchX = ::GetDeviceCaps(hDCScreen, LOGPIXELSX);
            nPixelsPerInchY = ::GetDeviceCaps(hDCScreen, LOGPIXELSY);
            ::ReleaseDC(NULL, hDCScreen);
            lpSizeInHiMetric->cx = MAP_PIX_TO_LOGHIM(lpSizeInPix->cx, nPixelsPerInchX);
            lpSizeInHiMetric->cy = MAP_PIX_TO_LOGHIM(lpSizeInPix->cy, nPixelsPerInchY);
        }
    }

    int MyBrowserCtrl::sm_nIndex = 0;
    std::map<int, std::shared_ptr<MyBrowserCtrl>> MyBrowserCtrl::sm_spBrowserCtrls;

    MyBrowserCtrl::MyBrowserCtrl(HWND hBindWnd) : 
        m_pUnk(NULL), 
        m_pControl(NULL), 
        m_hBindWnd(hBindWnd),
        m_hHostWnd(NULL), 
        m_bCreated(false),
        m_pThread(new std::thread)
    {
        m_clsid = IID_NULL;
    }

    MyBrowserCtrl::~MyBrowserCtrl()
    {
        ReleaseControl();
    }

    void MyBrowserCtrl::SetBindWindow(HWND hBindWnd)
    {
        m_hBindWnd = hBindWnd;
    }

    HWND MyBrowserCtrl::GetBindWindow() const
    {
        return m_hBindWnd;
    }

    void MyBrowserCtrl::SetHostWindow(HWND hHostWnd)
    {
        m_hHostWnd = hHostWnd;
    }

    HWND MyBrowserCtrl::GetHostWindow() const
    {
        return m_hHostWnd;
    }

    void MyBrowserCtrl::SetVisible(bool bVisible)
    {
        if (m_hHostWnd)
        {
            ::ShowWindow(m_hHostWnd, bVisible ? SW_SHOW : SW_HIDE);
        }
    }

    void MyBrowserCtrl::SetPos(RECT rc)
    {
        if (m_pUnk == NULL) return;
        if (m_pControl == NULL) return;

        SIZEL hmSize = { 0 };
        SIZEL pxSize = { 0 };
        pxSize.cx = m_rcItem.right - m_rcItem.left;
        pxSize.cy = m_rcItem.bottom - m_rcItem.top;
        PixelToHiMetric(&pxSize, &hmSize);

        if (m_pUnk != NULL) {
            m_pUnk->SetExtent(DVASPECT_CONTENT, &hmSize);
        }
        if (m_pControl->m_pInPlaceObject != NULL) {
            RECT rcItem = m_rcItem;
            if (!m_pControl->m_bWindowless)
            {
                // rcItem.ResetOffset();
            }
            m_pControl->m_pInPlaceObject->SetObjectRects(&rcItem, &rcItem);
        }
        if (!m_pControl->m_bWindowless) {
            assert(m_pControl->m_pWindow);
            ::MoveWindow(*m_pControl->m_pWindow, m_rcItem.left, m_rcItem.top, m_rcItem.right - m_rcItem.left, m_rcItem.bottom - m_rcItem.top, TRUE);
        }
    }

    RECT MyBrowserCtrl::GetPos() const
    {
        return m_rcItem;
    }

    void MyBrowserCtrl::SetFocus()
    {
        if (!::IsWindow(m_hHostWnd))
        {
            return;
        }
        HWND hFocusWnd = ::GetFocus();
        if (hFocusWnd != m_hHostWnd)
        {
            ::SetFocus(m_hHostWnd);
        }
    }

    LRESULT MyBrowserCtrl::MessageHandler(UINT uMsg, WPARAM wParam, LPARAM lParam, bool& bHandled)
    {
        if (m_pControl == NULL) return 0;
        assert(m_pControl->m_bWindowless);
        if (!m_pControl->m_bInPlaceActive) return 0;
        if (m_pControl->m_pInPlaceObject == NULL) return 0;
        bool bWasHandled = true;
        if ((uMsg >= WM_MOUSEFIRST && uMsg <= WM_MOUSELAST) || uMsg == WM_SETCURSOR) {
            // Mouse message only go when captured or inside rect
            DWORD dwHitResult = m_pControl->m_bCaptured ? HITRESULT_HIT : HITRESULT_OUTSIDE;
            if (dwHitResult == HITRESULT_OUTSIDE && m_pControl->m_pViewObject != NULL) {
                IViewObjectEx* pViewEx = NULL;
                m_pControl->m_pViewObject->QueryInterface(IID_IViewObjectEx, (LPVOID*)&pViewEx);
                if (pViewEx != NULL) {
                    POINT ptMouse = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
                    pViewEx->QueryHitPoint(DVASPECT_CONTENT, &m_rcItem, ptMouse, 0, &dwHitResult);
                    pViewEx->Release();
                }
            }
            if (dwHitResult != HITRESULT_HIT) return 0;
            if (uMsg == WM_SETCURSOR) bWasHandled = false;
        }
        else if (uMsg >= WM_KEYFIRST && uMsg <= WM_KEYLAST) {
            // Keyboard messages just go when we have focus
            // if (!IsFocused()) return 0;
        }
        else {
            switch (uMsg) {
            case WM_HELP:
            case WM_CONTEXTMENU:
                bWasHandled = false;
                break;
            default:
                return 0;
            }
        }
        LRESULT lResult = 0;
        HRESULT Hr = m_pControl->m_pInPlaceObject->OnWindowMessage(uMsg, wParam, lParam, &lResult);
        if (Hr == S_OK) bHandled = bWasHandled;
        return lResult;
    }

    int MyBrowserCtrl::CreateBrowserCtrl(HWND hBindWnd)
    {
        if (!::IsWindow(hBindWnd))
        {
            return -1;
        }

        int nIndex = sm_nIndex;
        sm_spBrowserCtrls[sm_nIndex++] = std::make_shared<MyBrowserCtrl>(MyBrowserCtrl(hBindWnd));

        return nIndex;
    }

    void MyBrowserCtrl::DestroyBrowserCtrl(int nIndex)
    {
        sm_spBrowserCtrls.erase(nIndex);
    }

    bool MyBrowserCtrl::CreateControl(LPCTSTR pstrCLSID)
    {
        CLSID clsid = { 0 };
        OLECHAR szCLSID[100] = { 0 };
#ifndef _UNICODE
        ::MultiByteToWideChar(::GetACP(), 0, pstrCLSID, -1, szCLSID, lengthof(szCLSID) - 1);
#else
        _tcsncpy_s(szCLSID, pstrCLSID, lengthof(szCLSID) - 1);
#endif
        if (pstrCLSID[0] == '{') ::CLSIDFromString(szCLSID, &clsid);
        else ::CLSIDFromProgID(szCLSID, &clsid);
        return CreateControl(clsid);
    }

    bool MyBrowserCtrl::CreateControl(const CLSID clsid)
    {
        assert(clsid != IID_NULL);
        if (clsid == IID_NULL) return false;
        m_bCreated = false;
        m_clsid = clsid;
        DoCreateControl();
        return true;
    }

    void MyBrowserCtrl::ReleaseControl()
    {
        m_hHostWnd = NULL;
        if (m_pUnk != NULL) {
            IObjectWithSite* pSite = NULL;
            m_pUnk->QueryInterface(IID_IObjectWithSite, (LPVOID*)&pSite);
            if (pSite != NULL) {
                pSite->SetSite(NULL);
                pSite->Release();
            }
            m_pUnk->Close(OLECLOSE_NOSAVE);
            m_pUnk->SetClientSite(NULL);
            m_pUnk->Release();
            m_pUnk = NULL;
        }
        if (m_pControl != NULL) {
            m_pControl->m_pOwner = NULL;
            m_pControl->Release();
            m_pControl = NULL;
        }
    }

    typedef HRESULT(__stdcall *DllGetClassObjectFunc)(REFCLSID rclsid, REFIID riid, LPVOID* ppv);

    bool MyBrowserCtrl::DoCreateControl()
    {
        ReleaseControl();
        // At this point we'll create the ActiveX control
        m_bCreated = true;
        
        IOleControl* pOleControl = NULL;

        HRESULT Hr = ::CoCreateInstance(m_clsid, NULL, CLSCTX_ALL, IID_IOleControl, (LPVOID*)&pOleControl);
        if (FAILED(Hr))
        {
            return false;
        }
        pOleControl->QueryInterface(IID_IOleObject, (LPVOID*)&m_pUnk);
        pOleControl->Release();
        if (m_pUnk == NULL) return false;
        // Create the host too
        m_pControl = new CActiveXCtrl();
        m_pControl->m_pOwner = this;
        // More control creation stuff
        DWORD dwMiscStatus = 0;
        m_pUnk->GetMiscStatus(DVASPECT_CONTENT, &dwMiscStatus);
        IOleClientSite* pOleClientSite = NULL;
        m_pControl->QueryInterface(IID_IOleClientSite, (LPVOID*)&pOleClientSite);
        CComPtr<IOleClientSite> RefOleClientSite = pOleClientSite;
        // Initialize control
        if ((dwMiscStatus & OLEMISC_SETCLIENTSITEFIRST) != 0) m_pUnk->SetClientSite(pOleClientSite);
        IPersistStreamInit* pPersistStreamInit = NULL;
        m_pUnk->QueryInterface(IID_IPersistStreamInit, (LPVOID*)&pPersistStreamInit);
        if (pPersistStreamInit != NULL) {
            Hr = pPersistStreamInit->InitNew();
            pPersistStreamInit->Release();
        }
        if (FAILED(Hr)) return false;
        if ((dwMiscStatus & OLEMISC_SETCLIENTSITEFIRST) == 0) m_pUnk->SetClientSite(pOleClientSite);
        // Grab the view...
        Hr = m_pUnk->QueryInterface(IID_IViewObjectEx, (LPVOID*)&m_pControl->m_pViewObject);
        if (FAILED(Hr)) Hr = m_pUnk->QueryInterface(IID_IViewObject2, (LPVOID*)&m_pControl->m_pViewObject);
        if (FAILED(Hr)) Hr = m_pUnk->QueryInterface(IID_IViewObject, (LPVOID*)&m_pControl->m_pViewObject);
        // Activate and done...
        m_pUnk->SetHostNames(OLESTR("UIActiveX"), NULL);
        if ((dwMiscStatus & OLEMISC_INVISIBLEATRUNTIME) == 0 && ::IsWindow(m_hHostWnd)) 
        {
            Hr = m_pUnk->DoVerb(OLEIVERB_INPLACEACTIVATE, NULL, pOleClientSite, 0, m_hHostWnd, &m_rcItem);
        }
        IObjectWithSite* pSite = NULL;
        m_pUnk->QueryInterface(IID_IObjectWithSite, (LPVOID*)&pSite);
        if (pSite != NULL) {
            pSite->SetSite(static_cast<IOleClientSite*>(m_pControl));
            pSite->Release();
        }
        return SUCCEEDED(Hr);
    }

    HRESULT MyBrowserCtrl::GetControl(const IID iid, LPVOID* ppRet)
    {
        assert(ppRet != NULL);
        //assert(*ppRet==NULL);
        if (ppRet == NULL) return E_POINTER;
        *ppRet = NULL;
        if (m_pUnk == NULL) return E_PENDING;
        return m_pUnk->QueryInterface(iid, (LPVOID*)ppRet);
    }

    HRESULT MyBrowserCtrl::GetExternalCall(LPVOID* ppRet)
    {
        return E_NOTIMPL;
    }

    STDMETHODIMP MyBrowserCtrl::QueryInterface(REFIID iid, void ** ppvObject)
    {
        return E_NOTIMPL;
    }
    //HRESULT MyBrowserCtrl::Download(IMoniker *pmk,IBindCtx *pbc,DWORD dwBindVerb,LONG grfBINDF,BINDINFO *pBindInfo,
    //                         LPCOLESTR pszHeaders,LPCOLESTR pszRedir,UINT uiCP)
    //{
    //    return S_FALSE;
    //}

    CLSID MyBrowserCtrl::GetClisd() const
    {
        return m_clsid;
    }
}