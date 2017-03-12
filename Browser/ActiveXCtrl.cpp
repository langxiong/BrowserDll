#include "stdafx.h"
#include "MyBrowserCtrl.h"
#include "ActiveXWnd.h"
#include "ActiveXCtrl.h"

namespace MyWeb
{
    class CActiveXEnum : public IEnumUnknown
    {
    public:
        CActiveXEnum(IUnknown* pUnk) : m_pUnk(pUnk), m_dwRef(1), m_iPos(0)
        {
            m_pUnk->AddRef();
        }
        ~CActiveXEnum()
        {
            m_pUnk->Release();
        }

        LONG m_iPos;
        ULONG m_dwRef;
        IUnknown* m_pUnk;

        STDMETHOD_(ULONG, AddRef)()
        {
            return ++m_dwRef;
        }
        STDMETHOD_(ULONG, Release)()
        {
            LONG lRef = --m_dwRef;
            if (lRef == 0) delete this;
            return lRef;
        }
        STDMETHOD(QueryInterface)(REFIID riid, LPVOID *ppvObject)
        {
            *ppvObject = NULL;
            if (riid == IID_IUnknown) *ppvObject = static_cast<IEnumUnknown*>(this);
            else if (riid == IID_IEnumUnknown) *ppvObject = static_cast<IEnumUnknown*>(this);
            if (*ppvObject != NULL) AddRef();
            return *ppvObject == NULL ? E_NOINTERFACE : S_OK;
        }
        STDMETHOD(Next)(ULONG celt, IUnknown **rgelt, ULONG *pceltFetched)
        {
            if (pceltFetched != NULL) *pceltFetched = 0;
            if (++m_iPos > 1) return S_FALSE;
            *rgelt = m_pUnk;
            (*rgelt)->AddRef();
            if (pceltFetched != NULL) *pceltFetched = 1;
            return S_OK;
        }
        STDMETHOD(Skip)(ULONG celt)
        {
            m_iPos += celt;
            return S_OK;
        }
        STDMETHOD(Reset)(void)
        {
            m_iPos = 0;
            return S_OK;
        }
        STDMETHOD(Clone)(IEnumUnknown **ppenum)
        {
            return E_NOTIMPL;
        }
    };


    class CActiveXFrameWnd : public IOleInPlaceFrame
    {
    public:
        CActiveXFrameWnd(MyBrowserCtrl* pOwner) : m_dwRef(1), m_pOwner(pOwner), m_pActiveObject(NULL)
        {
        }
        ~CActiveXFrameWnd()
        {
            if (m_pActiveObject != NULL) m_pActiveObject->Release();
        }

        ULONG m_dwRef;
        MyBrowserCtrl* m_pOwner;
        IOleInPlaceActiveObject* m_pActiveObject;

        // IUnknown
        STDMETHOD_(ULONG, AddRef)()
        {
            return ++m_dwRef;
        }
        STDMETHOD_(ULONG, Release)()
        {
            ULONG lRef = --m_dwRef;
            if (lRef == 0) delete this;
            return lRef;
        }
        STDMETHOD(QueryInterface)(REFIID riid, LPVOID *ppvObject)
        {
            *ppvObject = NULL;
            if (riid == IID_IUnknown) *ppvObject = static_cast<IOleInPlaceFrame*>(this);
            else if (riid == IID_IOleWindow) *ppvObject = static_cast<IOleWindow*>(this);
            else if (riid == IID_IOleInPlaceFrame) *ppvObject = static_cast<IOleInPlaceFrame*>(this);
            else if (riid == IID_IOleInPlaceUIWindow) *ppvObject = static_cast<IOleInPlaceUIWindow*>(this);
            if (*ppvObject != NULL) AddRef();
            return *ppvObject == NULL ? E_NOINTERFACE : S_OK;
        }
        // IOleInPlaceFrameWindow
        STDMETHOD(InsertMenus)(HMENU /*hmenuShared*/, LPOLEMENUGROUPWIDTHS /*lpMenuWidths*/)
        {
            return S_OK;
        }
        STDMETHOD(SetMenu)(HMENU /*hmenuShared*/, HOLEMENU /*holemenu*/, HWND /*hwndActiveObject*/)
        {
            return S_OK;
        }
        STDMETHOD(RemoveMenus)(HMENU /*hmenuShared*/)
        {
            return S_OK;
        }
        STDMETHOD(SetStatusText)(LPCOLESTR /*pszStatusText*/)
        {
            return S_OK;
        }
        STDMETHOD(EnableModeless)(BOOL /*fEnable*/)
        {
            return S_OK;
        }
        STDMETHOD(TranslateAccelerator)(LPMSG /*lpMsg*/, WORD /*wID*/)
        {
            return S_FALSE;
        }
        // IOleWindow
        STDMETHOD(GetWindow)(HWND* phwnd)
        {
            if (m_pOwner == NULL) return E_UNEXPECTED;
            *phwnd = m_pOwner->GetBindWindow();
            return S_OK;
        }
        STDMETHOD(ContextSensitiveHelp)(BOOL /*fEnterMode*/)
        {
            return S_OK;
        }
        // IOleInPlaceUIWindow
        STDMETHOD(GetBorder)(LPRECT /*lprectBorder*/)
        {
            return S_OK;
        }
        STDMETHOD(RequestBorderSpace)(LPCBORDERWIDTHS /*pborderwidths*/)
        {
            return INPLACE_E_NOTOOLSPACE;
        }
        STDMETHOD(SetBorderSpace)(LPCBORDERWIDTHS /*pborderwidths*/)
        {
            return S_OK;
        }
        STDMETHOD(SetActiveObject)(IOleInPlaceActiveObject* pActiveObject, LPCOLESTR /*pszObjName*/)
        {
            if (pActiveObject != NULL) pActiveObject->AddRef();
            if (m_pActiveObject != NULL) m_pActiveObject->Release();
            m_pActiveObject = pActiveObject;
            return S_OK;
        }
    };

    CActiveXCtrl::CActiveXCtrl() :
        m_lRefCount(1),
        m_pUnkSite(NULL),
        m_pViewObject(NULL),
        m_pInPlaceObject(NULL),
        m_bLocked(false),
        m_bFocused(false),
        m_bCaptured(false),
        m_bWindowless(true),
        m_bUIActivated(false),
        m_bInPlaceActive(false)
    {
        OleInitialize(NULL);
    }

    CActiveXCtrl::~CActiveXCtrl()
    {
        if (m_pUnkSite != NULL) m_pUnkSite->Release();
        if (m_pViewObject != NULL) m_pViewObject->Release();
        if (m_pInPlaceObject != NULL) m_pInPlaceObject->Release();
        OleUninitialize();
    }


    HRESULT CActiveXCtrl::CreateActiveXWnd()
    {
        if (m_pWindow != NULL)
        {
            return S_OK;
        }

        m_pWindow = new CActiveXWnd;
        if (m_pWindow == NULL)
        {
            return E_OUTOFMEMORY;
        }
        m_pOwner->SetHostWindow(m_pWindow->Init(this, m_pOwner->GetBindWindow()));
        return S_OK;
    }
    STDMETHODIMP CActiveXCtrl::QueryInterface(REFIID riid, LPVOID *ppvObject)
    {
        *ppvObject = NULL;
        if (riid == IID_IUnknown)                       *ppvObject = static_cast<IOleWindow*>(this);
        else if (riid == IID_IOleClientSite)            *ppvObject = static_cast<IOleClientSite*>(this);
        else if (riid == IID_IOleInPlaceSiteWindowless) *ppvObject = static_cast<IOleInPlaceSiteWindowless*>(this);
        else if (riid == IID_IOleInPlaceSiteEx)         *ppvObject = static_cast<IOleInPlaceSiteEx*>(this);
        else if (riid == IID_IOleInPlaceSite)           *ppvObject = static_cast<IOleInPlaceSite*>(this);
        else if (riid == IID_IOleWindow)                *ppvObject = static_cast<IOleWindow*>(this);
        else if (riid == IID_IOleControlSite)           *ppvObject = static_cast<IOleControlSite*>(this);
        else if (riid == IID_IOleContainer)             *ppvObject = static_cast<IOleContainer*>(this);
        else if (riid == IID_IObjectWithSite)           *ppvObject = static_cast<IObjectWithSite*>(this);
        else
        {
             
        }
             
        if (*ppvObject != NULL) AddRef();
        return *ppvObject == NULL ? E_NOINTERFACE : S_OK;
    }

    STDMETHODIMP_(ULONG) CActiveXCtrl::AddRef()
    {
        return (ULONG)::InterlockedIncrement(&m_lRefCount);
    }

    STDMETHODIMP_(ULONG) CActiveXCtrl::Release()
    {
        assert(m_lRefCount);
        if (0 == ::InterlockedDecrement(&m_lRefCount))
        {
            delete this;
            return 0L;
        }
        return (ULONG)m_lRefCount;
    }

    STDMETHODIMP CActiveXCtrl::SetSite(IUnknown *pUnkSite)
    {
        // DUITRACE(_T("AX: CActiveXCtrl::SetSite"));
        if (m_pUnkSite != NULL) {
            m_pUnkSite->Release();
            m_pUnkSite = NULL;
        }
        if (pUnkSite != NULL) {
            m_pUnkSite = pUnkSite;
            m_pUnkSite->AddRef();
        }
        return S_OK;
    }

    STDMETHODIMP CActiveXCtrl::GetSite(REFIID riid, LPVOID* ppvSite)
    {
        // DUITRACE(_T("AX: CActiveXCtrl::GetSite"));
        if (ppvSite == NULL) return E_POINTER;
        *ppvSite = NULL;
        if (m_pUnkSite == NULL) return E_FAIL;
        return m_pUnkSite->QueryInterface(riid, ppvSite);
    }

    STDMETHODIMP CActiveXCtrl::SaveObject(void)
    {
        // DUITRACE(_T("AX: CActiveXCtrl::SaveObject"));
        return E_NOTIMPL;
    }

    STDMETHODIMP CActiveXCtrl::GetMoniker(DWORD dwAssign, DWORD dwWhichMoniker, IMoniker** ppmk)
    {
        // DUITRACE(_T("AX: CActiveXCtrl::GetMoniker"));
        if (ppmk != NULL) *ppmk = NULL;
        return E_NOTIMPL;
    }

    STDMETHODIMP CActiveXCtrl::GetContainer(IOleContainer** ppContainer)
    {
        // DUITRACE(_T("AX: CActiveXCtrl::GetContainer"));
        if (ppContainer == NULL) return E_POINTER;
        *ppContainer = NULL;
        HRESULT Hr = E_NOTIMPL;
        if (m_pUnkSite != NULL) Hr = m_pUnkSite->QueryInterface(IID_IOleContainer, (LPVOID*)ppContainer);
        if (FAILED(Hr)) Hr = QueryInterface(IID_IOleContainer, (LPVOID*)ppContainer);
        return Hr;
    }

    STDMETHODIMP CActiveXCtrl::ShowObject(void)
    {
        // DUITRACE(_T("AX: CActiveXCtrl::ShowObject"));
        if (m_pOwner == NULL) return E_UNEXPECTED;
        HDC hDC = ::GetDC(m_pOwner->m_hHostWnd);
        if (hDC == NULL) return E_FAIL;
        if (m_pViewObject != NULL) m_pViewObject->Draw(DVASPECT_CONTENT, -1, NULL, NULL, NULL, hDC, (RECTL*)&m_pOwner->m_rcItem, (RECTL*)&m_pOwner->m_rcItem, NULL, NULL);
        ::ReleaseDC(m_pOwner->m_hHostWnd, hDC);
        return S_OK;
    }

    STDMETHODIMP CActiveXCtrl::OnShowWindow(BOOL fShow)
    {
        // DUITRACE(_T("AX: CActiveXCtrl::OnShowWindow"));
        return E_NOTIMPL;
    }

    STDMETHODIMP CActiveXCtrl::RequestNewObjectLayout(void)
    {
        // DUITRACE(_T("AX: CActiveXCtrl::RequestNewObjectLayout"));
        return E_NOTIMPL;
    }

    STDMETHODIMP CActiveXCtrl::CanWindowlessActivate(void)
    {
        // DUITRACE(_T("AX: CActiveXCtrl::CanWindowlessActivate"));
        return S_OK;  // Yes, we can!!
    }

    STDMETHODIMP CActiveXCtrl::GetCapture(void)
    {
        // DUITRACE(_T("AX: CActiveXCtrl::GetCapture"));
        if (m_pOwner == NULL) return E_UNEXPECTED;
        return m_bCaptured ? S_OK : S_FALSE;
    }

    STDMETHODIMP CActiveXCtrl::SetCapture(BOOL fCapture)
    {
        // DUITRACE(_T("AX: CActiveXCtrl::SetCapture"));
        if (m_pOwner == NULL) return E_UNEXPECTED;
        m_bCaptured = (fCapture == TRUE);
        if (fCapture) ::SetCapture(m_pOwner->m_hHostWnd); else ::ReleaseCapture();
        return S_OK;
    }

    STDMETHODIMP CActiveXCtrl::GetFocus(void)
    {
        // DUITRACE(_T("AX: CActiveXCtrl::GetFocus"));
        if (m_pOwner == NULL) return E_UNEXPECTED;
        return m_bFocused ? S_OK : S_FALSE;
    }

    STDMETHODIMP CActiveXCtrl::SetFocus(BOOL fFocus)
    {
        // DUITRACE(_T("AX: CActiveXCtrl::SetFocus"));
        if (m_pOwner == NULL) return E_UNEXPECTED;
        if (fFocus) m_pOwner->SetFocus();
        m_bFocused = (fFocus == TRUE);
        return S_OK;
    }

    STDMETHODIMP CActiveXCtrl::GetDC(LPCRECT pRect, DWORD grfFlags, HDC* phDC)
    {
        // DUITRACE(_T("AX: CActiveXCtrl::GetDC"));
        if (phDC == NULL) return E_POINTER;
        if (m_pOwner == NULL) return E_UNEXPECTED;
        *phDC = ::GetDC(m_pOwner->m_hHostWnd);
        if ((grfFlags & OLEDC_PAINTBKGND) != 0) {
            RECT rcItem = m_pOwner->GetPos();
            if (!m_bWindowless)
            {
                ::OffsetRect(&rcItem, rcItem.left, rcItem.top);
            }
            ::FillRect(*phDC, &rcItem, (HBRUSH)(COLOR_WINDOW + 1));
        }
        return S_OK;
    }

    STDMETHODIMP CActiveXCtrl::ReleaseDC(HDC hDC)
    {
        // DUITRACE(_T("AX: CActiveXCtrl::ReleaseDC"));
        if (m_pOwner == NULL) return E_UNEXPECTED;
        ::ReleaseDC(m_pOwner->m_hHostWnd, hDC);
        return S_OK;
    }

    STDMETHODIMP CActiveXCtrl::InvalidateRect(LPCRECT pRect, BOOL fErase)
    {
        // DUITRACE(_T("AX: CActiveXCtrl::InvalidateRect"));
        if (m_pOwner == NULL) return E_UNEXPECTED;
        if (m_pOwner->m_hHostWnd == NULL) return E_FAIL;
        return ::InvalidateRect(m_pOwner->m_hHostWnd, pRect, fErase) ? S_OK : E_FAIL;
    }

    STDMETHODIMP CActiveXCtrl::InvalidateRgn(HRGN hRGN, BOOL fErase)
    {
        // DUITRACE(_T("AX: CActiveXCtrl::InvalidateRgn"));
        if (m_pOwner == NULL) return E_UNEXPECTED;
        return ::InvalidateRgn(m_pOwner->m_hHostWnd, hRGN, fErase) ? S_OK : E_FAIL;
    }

    STDMETHODIMP CActiveXCtrl::ScrollRect(INT dx, INT dy, LPCRECT pRectScroll, LPCRECT pRectClip)
    {
        // DUITRACE(_T("AX: CActiveXCtrl::ScrollRect"));
        return S_OK;
    }

    STDMETHODIMP CActiveXCtrl::AdjustRect(LPRECT prc)
    {
        // DUITRACE(_T("AX: CActiveXCtrl::AdjustRect"));
        return S_OK;
    }

    STDMETHODIMP CActiveXCtrl::OnDefWindowMessage(UINT msg, WPARAM wParam, LPARAM lParam, LRESULT* plResult)
    {
        // DUITRACE(_T("AX: CActiveXCtrl::OnDefWindowMessage"));
        if (m_pOwner == NULL) return E_UNEXPECTED;
        *plResult = ::DefWindowProc(m_pOwner->m_hHostWnd, msg, wParam, lParam);
        return S_OK;
    }

    STDMETHODIMP CActiveXCtrl::OnInPlaceActivateEx(BOOL* pfNoRedraw, DWORD dwFlags)
    {
        // DUITRACE(_T("AX: CActiveXCtrl::OnInPlaceActivateEx"));
        assert(m_pInPlaceObject == NULL);
        if (m_pOwner == NULL) return E_UNEXPECTED;
        if (m_pOwner->m_pUnk == NULL) return E_UNEXPECTED;
        ::OleLockRunning(m_pOwner->m_pUnk, TRUE, FALSE);
        HWND hHostWnd = m_pOwner->GetHostWindow();
        HRESULT Hr = E_FAIL;
        if ((dwFlags & ACTIVATE_WINDOWLESS) != 0) {
            m_bWindowless = true;
            Hr = m_pOwner->m_pUnk->QueryInterface(IID_IOleInPlaceObjectWindowless, (LPVOID*)&m_pInPlaceObject);
            m_pOwner->m_hHostWnd = hHostWnd;
        }
        if (FAILED(Hr)) {
            m_bWindowless = false;
            Hr = CreateActiveXWnd();
            if (FAILED(Hr)) return Hr;
            Hr = m_pOwner->m_pUnk->QueryInterface(IID_IOleInPlaceObject, (LPVOID*)&m_pInPlaceObject);
        }
        if (m_pInPlaceObject != NULL) {
            RECT rcItem = m_pOwner->m_rcItem;
            if (!m_bWindowless)
            {
                ::OffsetRect(&rcItem, rcItem.left, rcItem.top);
            }
            m_pInPlaceObject->SetObjectRects(&rcItem, &rcItem);
        }
        m_bInPlaceActive = SUCCEEDED(Hr);
        return Hr;
    }

    STDMETHODIMP CActiveXCtrl::OnInPlaceDeactivateEx(BOOL fNoRedraw)
    {
        // DUITRACE(_T("AX: CActiveXCtrl::OnInPlaceDeactivateEx"));
        m_bInPlaceActive = false;
        if (m_pInPlaceObject != NULL) {
            m_pInPlaceObject->Release();
            m_pInPlaceObject = NULL;
        }
        if (m_pWindow != NULL) {
            ::DestroyWindow(*m_pWindow);
            delete m_pWindow;
            m_pWindow = NULL;
        }
        return S_OK;
    }

    STDMETHODIMP CActiveXCtrl::RequestUIActivate(void)
    {
        // DUITRACE(_T("AX: CActiveXCtrl::RequestUIActivate"));
        return S_OK;
    }

    STDMETHODIMP CActiveXCtrl::CanInPlaceActivate(void)
    {
        // DUITRACE(_T("AX: CActiveXCtrl::CanInPlaceActivate"));
        return S_OK;
    }

    STDMETHODIMP CActiveXCtrl::OnInPlaceActivate(void)
    {
        // DUITRACE(_T("AX: CActiveXCtrl::OnInPlaceActivate"));
        BOOL bDummy = FALSE;
        return OnInPlaceActivateEx(&bDummy, 0);
    }

    STDMETHODIMP CActiveXCtrl::OnUIActivate(void)
    {
        // DUITRACE(_T("AX: CActiveXCtrl::OnUIActivate"));
        m_bUIActivated = true;
        return S_OK;
    }

    STDMETHODIMP CActiveXCtrl::GetWindowContext(IOleInPlaceFrame** ppFrame, IOleInPlaceUIWindow** ppDoc, LPRECT lprcPosRect, LPRECT lprcClipRect, LPOLEINPLACEFRAMEINFO lpFrameInfo)
    {
        // DUITRACE(_T("AX: CActiveXCtrl::GetWindowContext"));
        if (ppDoc == NULL) return E_POINTER;
        if (ppFrame == NULL) return E_POINTER;
        if (lprcPosRect == NULL) return E_POINTER;
        if (lprcClipRect == NULL) return E_POINTER;
        if (m_pWindow)
        {
            ::GetClientRect(m_pWindow->GetHWND(), lprcPosRect);
            ::GetClientRect(m_pWindow->GetHWND(), lprcClipRect);
        }
        else
        {
            RECT rcItem = m_pOwner->GetPos();
            memcpy(lprcPosRect, &rcItem, sizeof(rcItem));
            memcpy(lprcClipRect, &rcItem, sizeof(rcItem));
        }
        *ppFrame = new CActiveXFrameWnd(m_pOwner);
        *ppDoc = NULL;
        ACCEL ac = { 0 };
        HACCEL hac = ::CreateAcceleratorTable(&ac, 1);
        lpFrameInfo->cb = sizeof(OLEINPLACEFRAMEINFO);
        lpFrameInfo->fMDIApp = FALSE;
        lpFrameInfo->hwndFrame = m_pOwner->GetHostWindow();
        lpFrameInfo->haccel = hac;
        lpFrameInfo->cAccelEntries = 1;
        return S_OK;
    }

    STDMETHODIMP CActiveXCtrl::Scroll(SIZE scrollExtant)
    {
        // DUITRACE(_T("AX: CActiveXCtrl::Scroll"));
        return E_NOTIMPL;
    }

    STDMETHODIMP CActiveXCtrl::OnUIDeactivate(BOOL fUndoable)
    {
        // DUITRACE(_T("AX: CActiveXCtrl::OnUIDeactivate"));
        m_bUIActivated = false;
        return S_OK;
    }

    STDMETHODIMP CActiveXCtrl::OnInPlaceDeactivate(void)
    {
        // DUITRACE(_T("AX: CActiveXCtrl::OnInPlaceDeactivate"));
        return OnInPlaceDeactivateEx(TRUE);
    }

    STDMETHODIMP CActiveXCtrl::DiscardUndoState(void)
    {
        // DUITRACE(_T("AX: CActiveXCtrl::DiscardUndoState"));
        return E_NOTIMPL;
    }

    STDMETHODIMP CActiveXCtrl::DeactivateAndUndo(void)
    {
        // DUITRACE(_T("AX: CActiveXCtrl::DeactivateAndUndo"));
        return E_NOTIMPL;
    }

    STDMETHODIMP CActiveXCtrl::OnPosRectChange(LPCRECT lprcPosRect)
    {
        // DUITRACE(_T("AX: CActiveXCtrl::OnPosRectChange"));
        return E_NOTIMPL;
    }

    STDMETHODIMP CActiveXCtrl::GetWindow(HWND* phwnd)
    {
        // DUITRACE(_T("AX: CActiveXCtrl::GetWindow"));
        if (m_pOwner == NULL) return E_UNEXPECTED;
        if (m_pOwner->m_hHostWnd == NULL) CreateActiveXWnd();
        if (m_pOwner->m_hHostWnd == NULL) return E_FAIL;
        *phwnd = m_pOwner->m_hHostWnd;
        return S_OK;
    }

    STDMETHODIMP CActiveXCtrl::ContextSensitiveHelp(BOOL fEnterMode)
    {
        // DUITRACE(_T("AX: CActiveXCtrl::ContextSensitiveHelp"));
        return S_OK;
    }

    STDMETHODIMP CActiveXCtrl::OnControlInfoChanged(void)
    {
        // DUITRACE(_T("AX: CActiveXCtrl::OnControlInfoChanged"));
        return S_OK;
    }

    STDMETHODIMP CActiveXCtrl::LockInPlaceActive(BOOL fLock)
    {
        // DUITRACE(_T("AX: CActiveXCtrl::LockInPlaceActive"));
        return S_OK;
    }

    STDMETHODIMP CActiveXCtrl::GetExtendedControl(IDispatch** ppDisp)
    {
        // DUITRACE(_T("AX: CActiveXCtrl::GetExtendedControl"));
        if (ppDisp == NULL) return E_POINTER;
        if (m_pOwner == NULL) return E_UNEXPECTED;
        if (m_pOwner->m_pUnk == NULL) return E_UNEXPECTED;
        return m_pOwner->m_pUnk->QueryInterface(IID_IDispatch, (LPVOID*)ppDisp);
    }

    STDMETHODIMP CActiveXCtrl::TransformCoords(POINTL* pPtlHimetric, POINTF* pPtfContainer, DWORD dwFlags)
    {
        // DUITRACE(_T("AX: CActiveXCtrl::TransformCoords"));
        return S_OK;
    }

    STDMETHODIMP CActiveXCtrl::TranslateAccelerator(MSG *pMsg, DWORD grfModifiers)
    {
        // DUITRACE(_T("AX: CActiveXCtrl::TranslateAccelerator"));
        return S_FALSE;
    }

    STDMETHODIMP CActiveXCtrl::OnFocus(BOOL fGotFocus)
    {
        // DUITRACE(_T("AX: CActiveXCtrl::OnFocus"));
        m_bFocused = (fGotFocus == TRUE);
        return S_OK;
    }

    STDMETHODIMP CActiveXCtrl::ShowPropertyFrame(void)
    {
        // DUITRACE(_T("AX: CActiveXCtrl::ShowPropertyFrame"));
        return E_NOTIMPL;
    }

    STDMETHODIMP CActiveXCtrl::EnumObjects(DWORD grfFlags, IEnumUnknown** ppenum)
    {
        // DUITRACE(_T("AX: CActiveXCtrl::EnumObjects"));
        if (ppenum == NULL) return E_POINTER;
        if (m_pOwner == NULL) return E_UNEXPECTED;
        *ppenum = new CActiveXEnum(m_pOwner->m_pUnk);
        return S_OK;
    }

    STDMETHODIMP CActiveXCtrl::LockContainer(BOOL fLock)
    {
        // DUITRACE(_T("AX: CActiveXCtrl::LockContainer"));
        m_bLocked = fLock != FALSE;
        return S_OK;
    }

    STDMETHODIMP CActiveXCtrl::ParseDisplayName(IBindCtx *pbc, LPOLESTR pszDisplayName, ULONG* pchEaten, IMoniker** ppmkOut)
    {
        // DUITRACE(_T("AX: CActiveXCtrl::ParseDisplayName"));
        return E_NOTIMPL;
    }

    STDMETHODIMP CActiveXCtrl::ShowMessage(
        /* [in] */ HWND hwnd,
        /* [annotation][in] */
        _In_  LPOLESTR lpstrText,
        /* [annotation][in] */
        _In_  LPOLESTR lpstrCaption,
        /* [in] */ DWORD dwType,
        /* [annotation][in] */
        _In_  LPOLESTR lpstrHelpFile,
        /* [in] */ DWORD dwHelpContext,
        /* [out] */ LRESULT *plResult)
    {
        USES_CONVERSION;
        TCHAR pBuffer[50];

        // resource identifier for window caption "Microsoft Internet Explorer"
#define IDS_MESSAGE_BOX_TITLE 2213

        // Load Shdoclc.dll and the IE message box title string
        HINSTANCE hinstSHDOCLC = LoadLibrary(TEXT("SHDOCLC.DLL"));

        if (hinstSHDOCLC == NULL)
        {
            // Error loading module -- fail as securely as possible
            return S_FALSE;
        }
        LoadString(hinstSHDOCLC, IDS_MESSAGE_BOX_TITLE, pBuffer, 50);

        // Compare the IE message box title string with lpstrCaption
        // If they're the same, substitute your own Caption
        if (_tcscmp(OLE2T(lpstrCaption), pBuffer) == 0)
            lpstrCaption = L"by XL";

        // Create your own message box and display it
        *plResult = ::MessageBox(hwnd, OLE2T(lpstrText), OLE2T(lpstrCaption), dwType);
        // Unload Shdoclc.dll and return
        FreeLibrary(hinstSHDOCLC);
        return S_OK;
    }

    STDMETHODIMP CActiveXCtrl::ShowHelp(
        /* [in] */ HWND hwnd,
        /* [annotation][in] */
        _In_  LPOLESTR pszHelpFile,
        /* [in] */ UINT uCommand,
        /* [in] */ DWORD dwData,
        /* [in] */ POINT ptMouse,
        /* [out] */ IDispatch *pDispatchObjectHit)
    {
        return E_NOTIMPL;
    }
}