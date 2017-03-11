#include "stdafx.h"

#include "MyDocHostUIHandler.h"

#pragma warning(push, 3)
#include <mshtmcid.h>
#include <strsafe.h>
#include <mshtml.h>
#include "JSExternal.h"
#pragma warning(pop)

MyDocHostUIHandler::MyDocHostUIHandler():
    m_lRefCount(0)
{
}

MyDocHostUIHandler::~MyDocHostUIHandler()
{
}

void MyDocHostUIHandler::Init(CComPtr<IWebBrowser2> spWebBrowser2)
{
    if (spWebBrowser2 != NULL)
    {
        CComPtr<IDocHostUIHandler2> spDocHostUIHandler ;
        CComPtr<IOleCommandTarget>  spOleCommandTarget ;
        CComPtr<IOleClientSite> spOleClientSite ;

        CComPtr<IDispatch> spDocument ;
        HRESULT hRet = spWebBrowser2->get_Document(&spDocument) ;
        if (SUCCEEDED(hRet) && spDocument != NULL)
        {
            CComQIPtr<IHTMLDocument2> spHtmlDoc = spDocument ;
            if (spHtmlDoc != NULL)
            {
                CComQIPtr<IOleObject, &IID_IOleObject> spOleObject(spDocument) ;
                if (spOleObject)
                {                
                    HRESULT hr = spOleObject->GetClientSite(&spOleClientSite) ;
                    if (SUCCEEDED(hr) && (spOleClientSite != NULL))
                    {
                        spOleClientSite->QueryInterface(IID_IOleCommandTarget, (void**)&spOleCommandTarget) ;
                        spOleClientSite->QueryInterface(IID_IDocHostUIHandler2, (void**)&spDocHostUIHandler) ;
                    }
                }            
            }
        }

        if ( (spOleCommandTarget != NULL) && (spDocHostUIHandler != NULL) )
        {
            if (m_spOleCommandTarget != spOleCommandTarget)
            {
                m_spOleCommandTarget = spOleCommandTarget ;
            }
            if (m_spDocHostUIHandler != spDocHostUIHandler)
            {
                m_spDocHostUIHandler = spDocHostUIHandler ;
            }
        }
    }
}
HRESULT MyDocHostUIHandler::ShowContextMenu(/* [in] */ DWORD dwID,
                                                 /* [in] */ POINT* ppt,
                                                 /* [in] */ IUnknown* pcmdtReserved,
                                                 /* [in] */ IDispatch* pdispReserved)
{
    if ( (ppt == NULL) || 
         (pcmdtReserved == NULL) || 
         (pdispReserved == NULL))
    {
        return E_NOTIMPL ;
    }
    if (dwID == CONTEXT_MENU_CONTROL )
    {
        HWND hwnd = NULL ;
        CComPtr<IOleCommandTarget> spCT ;
        CComPtr<IOleWindow> spWnd ;
        CComVariant var ;
        CComVariant var1 ;
        CComVariant var2 ;

        HRESULT hr = pcmdtReserved->QueryInterface(IID_IOleCommandTarget, (void**)&spCT) ;
        if ( (hr != S_OK) || (spCT == NULL) )
        {
            return E_NOTIMPL ;
        }

        hr = pcmdtReserved->QueryInterface(IID_IOleWindow, (void**)&spWnd) ;
        if ( (hr != S_OK) || (spWnd == NULL) )
        {
            return E_NOTIMPL ;
        }

        hr = spWnd->GetWindow(&hwnd) ;
        if ( (hr != S_OK) || (hwnd == NULL) )
        {
            return E_NOTIMPL ;
        }

		// TODO: 
		return E_NOTIMPL;
	}
    return S_OK ;
}


HRESULT MyDocHostUIHandler::GetHostInfo( /* [out][in] */ DOCHOSTUIINFO *pInfo)
{
    if (m_spDocHostUIHandler == NULL)
    {
        return E_NOTIMPL ;
    }

    HRESULT hr = m_spDocHostUIHandler->GetHostInfo(pInfo) ;
    const DWORD _DOCHOSTUIFLAG_THEME = 0x40000 ;
    const DWORD dwSetDocHostFlags = DOCHOSTUIFLAG_NO3DBORDER | _DOCHOSTUIFLAG_THEME ;
    pInfo->dwFlags |= dwSetDocHostFlags ;
    // 禁用IE的自动填表功能
    pInfo->dwFlags &= (~DOCHOSTUIFLAG_ENABLE_FORMS_AUTOCOMPLETE);
    pInfo->dwFlags |= DOCHOSTUIFLAG_SCROLL_NO;
    return hr ;
}

HRESULT MyDocHostUIHandler::ShowUI( 
                                        /* [in] */ DWORD dwID,
                                        /* [in] */ IOleInPlaceActiveObject *pActiveObject,
                                        /* [in] */ IOleCommandTarget *pCommandTarget,
                                        /* [in] */ IOleInPlaceFrame *pFrame,
                                        /* [in] */ IOleInPlaceUIWindow *pDoc)
{
    if (m_spDocHostUIHandler == NULL)
    {
        return E_NOTIMPL ;
    }
    return m_spDocHostUIHandler->ShowUI(dwID, pActiveObject, pCommandTarget, pFrame, pDoc) ;
}

HRESULT MyDocHostUIHandler::HideUI( void)
{
    if (m_spDocHostUIHandler == NULL)
    {
        return E_NOTIMPL ;
    }
    return m_spDocHostUIHandler->HideUI() ;
}

HRESULT MyDocHostUIHandler::UpdateUI( void)
{
    if (m_spDocHostUIHandler == NULL)
    {
        return E_NOTIMPL ;
    }
    return m_spDocHostUIHandler->UpdateUI() ;
}

HRESULT MyDocHostUIHandler::EnableModeless(/* [in] */ BOOL fEnable)
{
    if (m_spDocHostUIHandler == NULL)
    {
        return E_NOTIMPL ;
    }
    return m_spDocHostUIHandler->EnableModeless(fEnable) ;
}

HRESULT MyDocHostUIHandler::OnDocWindowActivate(/* [in] */ BOOL fActivate)
{
    if (m_spDocHostUIHandler == NULL)
    {
        return E_NOTIMPL ;
    }
    return m_spDocHostUIHandler->OnDocWindowActivate(fActivate) ;
}

HRESULT MyDocHostUIHandler::OnFrameWindowActivate(/* [in] */ BOOL fActivate)
{
    if (m_spDocHostUIHandler == NULL)
    {
        return E_NOTIMPL ;
    }
    return m_spDocHostUIHandler->OnFrameWindowActivate(fActivate) ;
}

HRESULT MyDocHostUIHandler::ResizeBorder(/* [in] */ LPCRECT prcBorder,
                                              /* [in] */ IOleInPlaceUIWindow *pUIWindow,
                                              /* [in] */ BOOL fRameWindow)
{
    if (m_spDocHostUIHandler == NULL)
    {
        return E_NOTIMPL ;
    }
    return m_spDocHostUIHandler->ResizeBorder(prcBorder, pUIWindow, fRameWindow) ;
}

HRESULT MyDocHostUIHandler::TranslateAccelerator( 
    /* [in] */ LPMSG lpMsg,
    /* [in] */ const GUID *pguidCmdGroup,
    /* [in] */ DWORD nCmdID)
{
    if (m_spDocHostUIHandler == NULL)
    {
        return E_NOTIMPL ;
    }
    return m_spDocHostUIHandler->TranslateAccelerator(lpMsg, pguidCmdGroup, nCmdID) ;
}

HRESULT MyDocHostUIHandler::GetOptionKeyPath(/* [out] */ LPOLESTR *pchKey,
                                                  /* [in] */ DWORD dw)
{
    if (m_spDocHostUIHandler == NULL)
    {
        return E_NOTIMPL ;
    }
    return m_spDocHostUIHandler->GetOptionKeyPath(pchKey, dw) ;
}

HRESULT MyDocHostUIHandler::GetOverrideKeyPath(/* [out] */ LPOLESTR *pchKey,
                                                    /* [in] */ DWORD dw)
{
    if (m_spDocHostUIHandler == NULL)
    {
        return E_NOTIMPL ;
    }
    return m_spDocHostUIHandler->GetOverrideKeyPath(pchKey, dw) ;
}

HRESULT MyDocHostUIHandler::GetDropTarget(/* [in] */ IDropTarget* pDropTarget,
                                               /* [out] */ IDropTarget** ppDropTarget)
{
    return E_NOTIMPL ;
}

HRESULT MyDocHostUIHandler::GetExternal( /* [out] */ IDispatch **ppDispatch)
{
    if (m_spJsExternal == NULL)
    {
		m_spJsExternal = new JSExternal;
    }
    return m_spJsExternal->QueryInterface(IID_IDispatch, (void **)ppDispatch) ;
}

HRESULT MyDocHostUIHandler::TranslateUrl( 
    /* [in] */ DWORD dwTranslate,
    /* [in] */ OLECHAR *pchURLIn,
    /* [out] */ OLECHAR **ppchURLOut)
{
    if (m_spDocHostUIHandler == NULL)
    {
        return E_NOTIMPL ;
    }
    return m_spDocHostUIHandler->TranslateUrl(dwTranslate, pchURLIn, ppchURLOut) ;
}

HRESULT MyDocHostUIHandler::FilterDataObject( 
    /* [in] */  IDataObject *pDO,
    /* [out] */ IDataObject **ppDORet)

{
    if (m_spDocHostUIHandler == NULL)
    {
        return E_NOTIMPL ;
    }
    return m_spDocHostUIHandler->FilterDataObject(pDO, ppDORet) ;
}


HRESULT MyDocHostUIHandler::QueryStatus( 
    /* [unique][in] */ const GUID *pguidCmdGroup,
    /* [in] */ ULONG cCmds,
    /* [out][in][size_is] */ OLECMD prgCmds[  ],
    /* [unique][out][in] */ OLECMDTEXT *pCmdText)
{
    if (m_spOleCommandTarget == NULL)
    {
        return E_NOTIMPL ;
    }
    return m_spOleCommandTarget->QueryStatus(pguidCmdGroup, cCmds, prgCmds, pCmdText) ;
}

HRESULT MyDocHostUIHandler::Exec( 
                                      /* [unique][in] */ const GUID *pguidCmdGroup,
                                      /* [in] */ DWORD nCmdID,
                                      /* [in] */ DWORD nCmdexecopt,
                                      /* [unique][in] */ VARIANT *pvaIn,
                                      /* [unique][out][in] */ VARIANT *pvaOut)
{    
    if (nCmdID == OLECMDID_SHOWSCRIPTERROR)
    {
        if(pvaOut)
        {
            pvaOut->vt = VT_BOOL ;
            pvaOut->boolVal = VARIANT_TRUE ;
        }
        //屏蔽脚本错误对话框
        return S_OK ;
    }
    else if (nCmdID == OLECMDID_SHOWFIND)
    {
        //屏蔽页面本身的查找界面
        return S_OK ;
    }
    else if (nCmdID == OLECMDID_PAGEACTIONBLOCKED)
    {
        //自定义信息栏
    }
    else if (nCmdID == OLECMDID_PAGEACTIONUIQUERY)
    {
        //自定义信息栏
    }
    HRESULT hRet = E_NOTIMPL ;
    if (m_spOleCommandTarget == NULL)
    {
        return hRet ;
    }
    hRet = m_spOleCommandTarget->Exec(pguidCmdGroup, nCmdID, nCmdexecopt, pvaIn, pvaOut) ;
    return hRet ;
}

STDMETHODIMP_(ULONG) MyDocHostUIHandler::AddRef(void)
{
	assert(m_lRefCount);
	return (ULONG)::InterlockedIncrement(&m_lRefCount);
}

STDMETHODIMP_(ULONG) MyDocHostUIHandler::Release(void)
{
	assert(m_lRefCount);
    if (0 == ::InterlockedDecrement(&m_lRefCount))
    {
        delete this ;
        return 0L ;
    }
    return (ULONG)m_lRefCount ;
}

HRESULT MyDocHostUIHandler::QueryInterface(REFIID riid, void** ppv)
{
    if (ppv == NULL)
    {
        return E_POINTER ;
    }

    if (IID_IUnknown == riid)
    {
        *ppv = (IUnknown*)(IOleCommandTarget*)this ;
        ((LPUNKNOWN)*ppv)->AddRef() ;
        return S_OK ;
    }
    else if (IID_IOleCommandTarget == riid)
    {
        *ppv = (IOleCommandTarget*)this ;
        ((LPUNKNOWN)*ppv)->AddRef() ;
        return S_OK ;
    }
    else if (IID_IDocHostUIHandler == riid)
    {
        *ppv = (IDocHostUIHandler*)this ;
        ((LPUNKNOWN)*ppv)->AddRef() ;
        return S_OK ;
    }
    else if (IID_IDocHostUIHandler2 == riid)
    {
        *ppv = (IDocHostUIHandler2*)this ;
        ((LPUNKNOWN)*ppv)->AddRef() ;
        return S_OK ;
    }
    else
    {
		assert(false);
    }
    return E_NOINTERFACE ;
}