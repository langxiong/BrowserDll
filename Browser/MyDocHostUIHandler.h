#pragma once

#pragma warning(push, 3)
#include <unknwn.h>
#include <exdisp.h>
#include <mshtmhst.h>
#pragma warning(pop)

class JSExternal;
/** IE内核IDocHostUIHandler2接口转接
*/
class MyDocHostUIHandler:
    public IOleCommandTarget,
    public IDocHostUIHandler2
{
public:

    MyDocHostUIHandler() ;

    ~MyDocHostUIHandler() ;

public:

    /** 初始化
    */
    void Init(CComPtr<IWebBrowser2> spWebBrowser2);

protected:

    /** IUnknown接口增加引用计数
    */    
    virtual STDMETHODIMP_(ULONG) AddRef(void) ;

    /** IUnknown接口减少引用计数
    */
    virtual STDMETHODIMP_(ULONG) Release(void) ;

    /** IUnknown查询接口
    */
    virtual STDMETHODIMP QueryInterface(REFIID riid, void** ppv) ;

protected:

    /** IOleCommandTarget接口
    */
    virtual HRESULT STDMETHODCALLTYPE QueryStatus( /* [unique][in] */ const GUID *pguidCmdGroup,
        /* [in] */ ULONG cCmds,
        /* [out][in][size_is] */ OLECMD prgCmds[],
        /* [unique][out][in] */ OLECMDTEXT *pCmdText) ;

    /** IOleCommandTarget接口
    */
    virtual HRESULT STDMETHODCALLTYPE Exec( /* [unique][in] */ const GUID *pguidCmdGroup,
        /* [in] */ DWORD nCmdID,
        /* [in] */ DWORD nCmdexecopt,
        /* [unique][in] */ VARIANT *pvaIn,
        /* [unique][out][in] */ VARIANT *pvaOut) ;

    /** IDocHostUIHandler2接口
    */
    virtual HRESULT STDMETHODCALLTYPE ShowContextMenu( /* [in] */ DWORD dwID,
        /* [in] */ POINT *ppt,
        /* [in] */ IUnknown *pcmdtReserved,
        /* [in] */ IDispatch *pdispReserved) ;

    /** IDocHostUIHandler2接口
    */
    virtual HRESULT STDMETHODCALLTYPE GetHostInfo( /* [out][in] */ DOCHOSTUIINFO *pInfo) ;

    /** IDocHostUIHandler2接口
    */
    virtual HRESULT STDMETHODCALLTYPE ShowUI( /* [in] */ DWORD dwID,
        /* [in] */ IOleInPlaceActiveObject *pActiveObject,
        /* [in] */ IOleCommandTarget *pCommandTarget,
        /* [in] */ IOleInPlaceFrame *pFrame,
        /* [in] */ IOleInPlaceUIWindow *pDoc) ;

    /** IDocHostUIHandler2接口
    */
    virtual HRESULT STDMETHODCALLTYPE HideUI( void) ;

    /** IDocHostUIHandler2接口
    */
    virtual HRESULT STDMETHODCALLTYPE UpdateUI( void) ;

    /** IDocHostUIHandler2接口
    */
    virtual HRESULT STDMETHODCALLTYPE EnableModeless( /* [in] */ BOOL fEnable) ;

    /** IDocHostUIHandler2接口
    */
    virtual HRESULT STDMETHODCALLTYPE OnDocWindowActivate( /* [in] */ BOOL fActivate) ;

    /** IDocHostUIHandler2接口
    */
    virtual HRESULT STDMETHODCALLTYPE OnFrameWindowActivate( /* [in] */ BOOL fActivate) ;

    /** IDocHostUIHandler2接口
    */
    virtual HRESULT STDMETHODCALLTYPE ResizeBorder( /* [in] */ LPCRECT prcBorder,
        /* [in] */ IOleInPlaceUIWindow *pUIWindow,
        /* [in] */ BOOL fRameWindow) ;

    /** IDocHostUIHandler2接口
    */
    virtual  HRESULT STDMETHODCALLTYPE TranslateAccelerator( /* [in] */ LPMSG lpMsg,
        /* [in] */ const GUID *pguidCmdGroup,
        /* [in] */ DWORD nCmdID) ;

    /** IDocHostUIHandler2接口
    */
    virtual  HRESULT STDMETHODCALLTYPE GetOptionKeyPath( /* [out] */ LPOLESTR *pchKey,
        /* [in] */ DWORD dw) ;

    /** IDocHostUIHandler2接口
    */
    virtual  HRESULT STDMETHODCALLTYPE GetDropTarget( /* [in] */ IDropTarget *pDropTarget,
        /* [out] */ IDropTarget **ppDropTarget) ;

    /** IDocHostUIHandler2接口
    */
    virtual HRESULT STDMETHODCALLTYPE GetExternal( /* [out] */ IDispatch **ppDispatch) ;

    /** IDocHostUIHandler2接口
    */
    virtual HRESULT STDMETHODCALLTYPE TranslateUrl( /* [in] */ DWORD dwTranslate,
        /* [in] */ OLECHAR *pchURLIn,
        /* [out] */ OLECHAR **ppchURLOut) ;

    /** IDocHostUIHandler2接口
    */
    virtual HRESULT STDMETHODCALLTYPE FilterDataObject( /* [in] */ IDataObject *pDO,
        /* [out] */ IDataObject **ppDORet) ;

    /** IDocHostUIHandler2接口
    */
    virtual HRESULT STDMETHODCALLTYPE GetOverrideKeyPath( /* [annotation][out] */ LPOLESTR *pchKey,
        /* [in] */ DWORD dw) ;

private:

    /** 默认的IDocHostUIHandler
    */
    CComPtr<IDocHostUIHandler2> m_spDocHostUIHandler ;

    /** 默认的IOleCommandTarget
    */
    CComPtr<IOleCommandTarget> m_spOleCommandTarget ;

    /** 自定义external接口
    */ 
    CComPtr<JSExternal> m_spJsExternal ;

    /** 引用计数
    */
    volatile LONG m_lRefCount ;
} ;
