#include "stdafx.h"
#include "MyBrowserCtrl.h"
#include "ActiveXCtrl.h"
#include "ActiveXWnd.h"
#include "BrowserDefine.h"
#include "MyOleInitialize.h"
#include "MyDocHostUIHandler.h"
#include "MyEventDispatch.h"
#include "JSExternal.h"

#include <WindowsX.h>
#include <comutil.h>
#pragma comment(lib, "comsuppw.lib")

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
    std::map<int, MyBrowserCtrl::TData> MyBrowserCtrl::sm_spBrowserCtrls;

    MyBrowserCtrl::MyBrowserCtrl(HWND hBindWnd) : 
        m_dwCookie(0),
        m_pUnk(NULL), 
        m_pControl(NULL), 
        m_hBindWnd(hBindWnd),
        m_hHostWnd(NULL), 
        m_bCreated(false),
        m_hInitEvent(::CreateEvent(NULL, FALSE, FALSE, NULL)),
        m_dwWebWorkThreadId(0)
    {
        m_spJsExternal = new JSExternal;
        m_spDocHostUIHandler = new MyDocHostUIHandler((IDispatch*)m_spJsExternal.p);
        m_spEventDispatch = new MyEventDispatch(this, m_spDocHostUIHandler);
        m_clsid = IID_NULL;
    }

    MyBrowserCtrl::~MyBrowserCtrl()
    {
        if (m_hInitEvent)
        {
            ::CloseHandle(m_hInitEvent);
        }
    }

    void MyBrowserCtrl::MessageLoopThread()
    {
        m_dwWebWorkThreadId = ::GetCurrentThreadId();
        MyOleInitialize oleInit;

        MSG msg = { 0 };
        ::PeekMessage(&msg, NULL, WM_USER, WM_USER, PM_NOREMOVE);

        ::PostThreadMessage(m_dwWebWorkThreadId, WM_WEB_THREAD_DO_INITIALIZE, NULL, NULL);

        while (::GetMessage(&msg, 0, 0, 0) > 0)
        {
            if (!msg.hwnd && HandleCustomThreadMsg(msg))
            {
                continue;
            }

			::TranslateMessage(&msg);
			::DispatchMessage(&msg);
        }

        ReleaseControl();
    }

    bool MyBrowserCtrl::HandleCustomThreadMsg(const MSG & msg)
    {
        if (msg.message == WM_WEB_THREAD_DO_INITIALIZE)
        {
            if (m_hInitEvent)
            {
                ::SetEvent(m_hInitEvent);
            }

            if (CreateControl(CLSID_WebBrowser))
            {
                NavigateUrl(_T("about:blank"));
            }

            return true;
        }
        if (msg.message == WM_WEB_THREAD_SET_BROWSER_POS)
        {
            if (msg.wParam)
            {
                LPRECT pRc = reinterpret_cast<LPRECT>(msg.wParam);
                SetPos(*pRc);
                delete pRc;
            }
            return true;
        }
        if (msg.message == WM_WEB_THREAD_NAVIGATE_URL)
        {
            if (msg.wParam)
            {
                MyString* pStr = reinterpret_cast<MyString*>(msg.wParam);
                NavigateUrl(*pStr);
                delete pStr;
            }
            return true;
        }

        if (msg.message == WM_WEB_THREAD_EXECUTE_JSCODE)
        {
            if (msg.wParam)
            {
                MyString* pStr = reinterpret_cast<MyString*>(msg.wParam);
                ExecuteJscode(*pStr);
                delete pStr;
            }
            return true;
        }

        if (msg.message == WM_WEB_THREAD_REGISTER_METHOD)
        {
            if (msg.wParam)
            {
                std::shared_ptr<TExternalItem> spExternalItem(reinterpret_cast<TExternalItem*>(msg.wParam));
                RegisterMethod(spExternalItem);
            }
            return true;
        }

        if (msg.message == WM_WEB_THREAD_UNREGISTER_METHOD)
        {
            if (msg.wParam)
            {
                MyString* pStr = reinterpret_cast<MyString*>(msg.wParam);
                UnregisterMethod(*pStr);
                delete pStr;
            }
            return true;
        }
        return false;
    }

    void MyBrowserCtrl::WaitThreadMsgQueueCreate()
    {
        DWORD ret = ::WaitForSingleObject(m_hInitEvent, INFINITE);
        if (ret == WAIT_FAILED)
        {
            std::cout << "WaitThreadMsgQueueCreate failed with err " << GetLastError() << std::endl;
        }
        ::CloseHandle(m_hInitEvent);
        m_hInitEvent = NULL;
    }

    HANDLE MyBrowserCtrl::GetThreadHandle() const
    {
        if (!m_dwWebWorkThreadId)
        {
            return NULL;
        }
        return ::OpenThread(SYNCHRONIZE, FALSE, m_dwWebWorkThreadId);
    }
    DWORD MyBrowserCtrl::GetThreadId() const
    {
        return m_dwWebWorkThreadId;
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

    void MyBrowserCtrl::NavigateUrl(const MyString& url)
    {
        if (m_spWebBrowser2 && (!url.empty()))
        {
            CComVariant vurl = url.c_str();
            CComVariant vempty;

            m_spWebBrowser2->Navigate2(&vurl, &vempty, &vempty, &vempty, &vempty);
        }
    }

    void MyBrowserCtrl::ExecuteJscode(const MyString& jscode)
    {
        CComPtr<IHTMLWindow2> spHtmlWindow;
        if (GetHTMLWindow(spHtmlWindow))
        {
            _bstr_t code = jscode.c_str();
            _bstr_t lang = _T("javascript");
            VARIANT ret;
            ret.vt = VT_EMPTY;
            spHtmlWindow->execScript(code, lang, &ret);
        }
    }

    void MyBrowserCtrl::RegisterMethod(const std::shared_ptr<TExternalItem>& spExternalItem)
    {
        if (!m_spJsExternal)
        {
            return;
        }

        m_spJsExternal->AddExternalItem(spExternalItem);
    }

    void MyBrowserCtrl::UnregisterMethod(const MyString & methodName)
    {
        if (!m_spJsExternal)
        {
            return;
        }

        m_spJsExternal->RemoveExternalItem(methodName);
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
        m_rcItem = rc;
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
                ::OffsetRect(&rcItem, -rcItem.left, -rcItem.top);
            }
            m_pControl->m_pInPlaceObject->SetObjectRects(&rcItem, &rcItem);
        }
        if (!m_pControl->m_bWindowless) {
            assert(m_pControl->m_pWindow);
            ::MoveWindow(*m_pControl->m_pWindow, 0, 0, m_rcItem.right - m_rcItem.left, m_rcItem.bottom - m_rcItem.top, TRUE);
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

    bool MyBrowserCtrl::GetHTMLWindow(CComPtr<IHTMLWindow2>& spHtmlWindow)
    {
        if (NULL != m_spWebBrowser2)
        {
            CComPtr<IDispatch> spDocument;
            HRESULT hRet = m_spWebBrowser2->get_Document(&spDocument);
            if (SUCCEEDED(hRet) && spDocument != NULL)
            {
                CComQIPtr<IHTMLDocument2> spHtmlDoc = spDocument;
                if (spHtmlDoc != NULL)
                {
                    CComPtr<IHTMLWindow2> spParentWindow;
                    hRet = spHtmlDoc->get_parentWindow(&spParentWindow);
                    if (spParentWindow != NULL)
                    {
                        spHtmlWindow = spParentWindow;
                        return true;
                    }
                }
            }
        }
        return false;
    }

    int MyBrowserCtrl::CreateBrowserCtrl(HWND hBindWnd)
    {
        if (!::IsWindow(hBindWnd))
        {
            return -1;
        }

        int nIndex = sm_nIndex;
        std::shared_ptr<MyBrowserCtrl> spBrowserCtrl(new MyBrowserCtrl(hBindWnd));
        
        std::thread t([&]() {
            spBrowserCtrl->MessageLoopThread();
        });

        t.detach();

        spBrowserCtrl->WaitThreadMsgQueueCreate();
        sm_spBrowserCtrls[sm_nIndex++] = { spBrowserCtrl->GetThreadHandle(), spBrowserCtrl->GetThreadId(), spBrowserCtrl};

        return nIndex;
    }

    void MyBrowserCtrl::DestroyBrowserCtrl(int nIndex)
    {
        auto it = sm_spBrowserCtrls.find(nIndex);
        if (it == sm_spBrowserCtrls.cend())
        {
            return;
        }

        DWORD ret = ::PostThreadMessage(it->second._dwThreadId, WM_QUIT, NULL, NULL);
        if (ret != 0)
        {
            ret = ::WaitForSingleObject(it->second._hThreadHandle, INFINITE);
            if (ret == WAIT_FAILED)
            {
                DWORD nLastErr = ::GetLastError();
                std::cout << "DestroyBrowserCtrl failed to waitthread exit with err " << GetLastError() << std::endl;
            }
            // 不在析构函数中消除， 因赋值时，临时变量的产生会导致析构函数的调用
            ::CloseHandle(it->second._hThreadHandle);
        }
        sm_spBrowserCtrls.erase(it);
    }

    void MyBrowserCtrl::SetBrowserCtrlPos(int nIndex, RECT rc)
    {
        auto it = sm_spBrowserCtrls.find(nIndex);
        if (it == sm_spBrowserCtrls.cend())
        {
            return;
        }

        LPRECT pRc = new RECT(rc);
        DWORD ret = ::PostThreadMessage(it->second._dwThreadId, WM_WEB_THREAD_SET_BROWSER_POS, (WPARAM)pRc, NULL);
        if (ret == 0)
        {
            delete pRc;
        }
    }

    void MyBrowserCtrl::BrowserCtrlNavigateUrl(int nIndex, const MyString & url)
    {
        auto it = sm_spBrowserCtrls.find(nIndex);
        if (it == sm_spBrowserCtrls.cend())
        {
            return;
        }

        MyString* pStr = new MyString(url);
        DWORD ret = ::PostThreadMessage(it->second._dwThreadId, WM_WEB_THREAD_NAVIGATE_URL, (WPARAM)pStr, NULL);
        if (ret == 0)
        {
            delete pStr;
        }
    }

    void MyBrowserCtrl::BrowserCtrlExecuteJscode(int nIndex, const MyString & jscode)
    {
        auto it = sm_spBrowserCtrls.find(nIndex);
        if (it == sm_spBrowserCtrls.cend())
        {
            return;
        }

        MyString* pStr = new MyString(jscode);
        DWORD ret = ::PostThreadMessage(it->second._dwThreadId, WM_WEB_THREAD_EXECUTE_JSCODE, (WPARAM)pStr, NULL);
        if (ret == 0)
        {
            delete pStr;
        }
    }

    bool MyBrowserCtrl::BrowserCtrlRegisterMethod(int nIndex, const TExternalItem & externalItem)
    {
        auto it = sm_spBrowserCtrls.find(nIndex);
        if (it == sm_spBrowserCtrls.cend())
        {
            return false;
        }

        TExternalItem* pExternalItem = new TExternalItem(externalItem);
        DWORD ret = ::PostThreadMessage(it->second._dwThreadId, WM_WEB_THREAD_REGISTER_METHOD, (WPARAM)pExternalItem, NULL);
        if (ret == 0)
        {
            delete pExternalItem;
            return false;
        }
        return true;
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

    HRESULT MyBrowserCtrl::RegisterEventHandler(BOOL inAdvise)
    {
        if (!m_spWebBrowser2 || !m_spEventDispatch)
        {
            return E_NOINTERFACE;
        }
        CComPtr<IConnectionPointContainer>  pCPC;
        HRESULT hr = m_spWebBrowser2->QueryInterface(IID_IConnectionPointContainer, (void **)&pCPC);
        if (FAILED(hr))
        {
            return hr;
        }

        CComPtr<IConnectionPoint> pCP;
        hr = pCPC->FindConnectionPoint(DIID_DWebBrowserEvents2, &pCP);
        if (FAILED(hr))
            return hr;

        if (inAdvise)
        {
            hr = pCP->Advise(m_spEventDispatch, &m_dwCookie);
        }
        else
        {
            hr = pCP->Unadvise(m_dwCookie);
        }
        return hr;
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
        RegisterEventHandler(FALSE);
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

        HRESULT hr = ::CoCreateInstance(m_clsid, NULL, CLSCTX_ALL, IID_IOleControl, (LPVOID*)&pOleControl);
        if (FAILED(hr))
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
            hr = pPersistStreamInit->InitNew();
            pPersistStreamInit->Release();
        }
        if (FAILED(hr)) return false;
        if ((dwMiscStatus & OLEMISC_SETCLIENTSITEFIRST) == 0) m_pUnk->SetClientSite(pOleClientSite);
        // Grab the view...
        hr = m_pUnk->QueryInterface(IID_IViewObjectEx, (LPVOID*)&m_pControl->m_pViewObject);
        if (FAILED(hr)) hr = m_pUnk->QueryInterface(IID_IViewObject2, (LPVOID*)&m_pControl->m_pViewObject);
        if (FAILED(hr)) hr = m_pUnk->QueryInterface(IID_IViewObject, (LPVOID*)&m_pControl->m_pViewObject);
        // Activate and done...
        m_pUnk->SetHostNames(OLESTR("UIActiveX"), NULL);
        if ((dwMiscStatus & OLEMISC_INVISIBLEATRUNTIME) == 0 && ::IsWindow(m_hHostWnd)) 
        {
            hr = m_pUnk->DoVerb(OLEIVERB_INPLACEACTIVATE, NULL, pOleClientSite, 0, m_hHostWnd, &m_rcItem);
        }

        {
            CComPtr<IObjectWithSite> spSite;
            hr = m_pUnk->QueryInterface(IID_IObjectWithSite, (LPVOID*)&spSite);
            if (spSite) {
                spSite->SetSite(static_cast<IOleClientSite*>(m_pControl));
            }
        }

        hr = m_pUnk->QueryInterface(IID_IWebBrowser2, (LPVOID*)&m_spWebBrowser2);
        if (FAILED(hr))
        {
            return false;
        }
        hr = RegisterEventHandler(TRUE);
        return SUCCEEDED(hr);
    }

    HRESULT MyBrowserCtrl::GetExternalCall(LPVOID* ppRet)
    {
        return E_NOTIMPL;
    }

    STDMETHODIMP MyBrowserCtrl::QueryInterface(REFIID iid, void ** ppvObject)
    {
        assert(ppvObject != NULL);
        //assert(*ppRet==NULL);
        if (ppvObject == NULL) return E_POINTER;
        *ppvObject = NULL;
        if (m_pUnk == NULL) return E_PENDING;
        return m_pUnk->QueryInterface(iid, (LPVOID*)ppvObject);
    }

    void MyBrowserCtrl::NavigateComplete2(IDispatch * pDisp, VARIANT * pvURL)
    {
        if (m_spDocHostUIHandler != NULL)
        {
            CComPtr<IDispatch> spDisp;
            HRESULT hr = S_FALSE;
            if (m_spWebBrowser2 != NULL)
            {
                hr = m_spWebBrowser2->get_Document(&spDisp);
            }
            if ((hr == S_OK) && (spDisp != NULL))
            {
                CComQIPtr<ICustomDoc, &IID_ICustomDoc> spCustomDoc(spDisp);
                if (spCustomDoc != NULL)
                {
                    CComQIPtr<IDocHostUIHandler> spDocHostUIHandler = m_spDocHostUIHandler;
                    assert(spDocHostUIHandler != NULL);
                    if (spDocHostUIHandler != NULL)
                    {
                        hr = spCustomDoc->SetUIHandler(spDocHostUIHandler);
                        assert(hr == S_OK);
                    }
                }
                else
                {
                    assert(false);
                }
            }
        }
    }


    CLSID MyBrowserCtrl::GetClisd() const
    {
        return m_clsid;
    }
}