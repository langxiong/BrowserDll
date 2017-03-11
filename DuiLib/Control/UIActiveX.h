#ifndef __UIACTIVEX_H__
#define __UIACTIVEX_H__
#include <atlcomcli.h>
#include <MsHTML.h>
#include <mshtmhst.h>
#pragma once

struct IOleObject;


namespace DuiLib {
/////////////////////////////////////////////////////////////////////////////////////
//

class CActiveXCtrl;

template< class T >
class CSafeRelease
{
public:
    CSafeRelease(T* p) : m_p(p) { };
    ~CSafeRelease() { if( m_p != NULL ) m_p->Release(); };
    T* Detach() { T* t = m_p; m_p = NULL; return t; };
    T* m_p;
};

/////////////////////////////////////////////////////////////////////////////////////
//

class UILIB_API CActiveXUI : public CControlUI, public IMessageFilterUI
{
    friend class CActiveXCtrl;
public:
    CActiveXUI();
    virtual ~CActiveXUI();

    LPCTSTR GetClass() const;
    LPVOID GetInterface(LPCTSTR pstrName);

    void SetHostWindow(HWND hHostWnd);
    HWND GetHostWindow() const;

    bool IsDelayCreate() const;
    void SetDelayCreate(bool bDelayCreate = true);

    bool CreateControl(const CLSID clsid);
    bool CreateControl(LPCTSTR pstrCLSID);
    HRESULT GetControl(const IID iid, LPVOID* ppRet);

    virtual HRESULT GetExternalCall(LPVOID* ppRet);

    virtual STDMETHODIMP QueryInterface(REFIID iid, void ** ppvObject);
    /*virtual HRESULT Download(IMoniker *pmk,IBindCtx *pbc,DWORD dwBindVerb,LONG grfBINDF,BINDINFO *pBindInfo,
        LPCOLESTR pszHeaders,LPCOLESTR pszRedir,UINT uiCP);*/
    CLSID GetClisd() const;
    CDuiString GetModuleName() const;
    void SetModuleName(LPCTSTR pstrText);

    void SetVisible(bool bVisible = true);
    void SetInternVisible(bool bVisible = true);
    void SetPos(RECT rc);
    void DoPaint(HDC hDC, const RECT& rcPaint);

    void SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue);

    LRESULT MessageHandler(UINT uMsg, WPARAM wParam, LPARAM lParam, bool& bHandled);

protected:
    virtual void ReleaseControl();
    virtual bool DoCreateControl();

protected:
    CLSID m_clsid;
    CDuiString m_sModuleName;
    bool m_bCreated;
    bool m_bDelayCreate;
    IOleObject* m_pUnk;
    CActiveXCtrl* m_pControl;
    HWND m_hwndHost;
};


/////////////////////////////////////////////////////////////////////////////////////
//

class CActiveXCtrl :
    public IOleClientSite,
    public IOleInPlaceSiteWindowless,
    public IOleControlSite,
    public IObjectWithSite,
    public IOleContainer,
    //by xl
    public IDocHostUIHandler,
    public IOleCommandTarget,
    public IDocHostShowUI
    //    public IDownloadManager
{
    friend class CActiveXUI;
    friend class CActiveXWnd;
public:
    CActiveXCtrl();
    ~CActiveXCtrl();

    // IUnknown
    STDMETHOD_(ULONG,AddRef)();
    STDMETHOD_(ULONG,Release)();
    STDMETHOD(QueryInterface)(REFIID riid, LPVOID *ppvObject);

    // IObjectWithSite
    STDMETHOD(SetSite)(IUnknown *pUnkSite);
    STDMETHOD(GetSite)(REFIID riid, LPVOID* ppvSite);

    // IOleClientSite
    STDMETHOD(SaveObject)(void);       
    STDMETHOD(GetMoniker)(DWORD dwAssign, DWORD dwWhichMoniker, IMoniker** ppmk);
    STDMETHOD(GetContainer)(IOleContainer** ppContainer);        
    STDMETHOD(ShowObject)(void);        
    STDMETHOD(OnShowWindow)(BOOL fShow);        
    STDMETHOD(RequestNewObjectLayout)(void);

    // IOleInPlaceSiteWindowless
    STDMETHOD(CanWindowlessActivate)(void);
    STDMETHOD(GetCapture)(void);
    STDMETHOD(SetCapture)(BOOL fCapture);
    STDMETHOD(GetFocus)(void);
    STDMETHOD(SetFocus)(BOOL fFocus);
    STDMETHOD(GetDC)(LPCRECT pRect, DWORD grfFlags, HDC* phDC);
    STDMETHOD(ReleaseDC)(HDC hDC);
    STDMETHOD(InvalidateRect)(LPCRECT pRect, BOOL fErase);
    STDMETHOD(InvalidateRgn)(HRGN hRGN, BOOL fErase);
    STDMETHOD(ScrollRect)(INT dx, INT dy, LPCRECT pRectScroll, LPCRECT pRectClip);
    STDMETHOD(AdjustRect)(LPRECT prc);
    STDMETHOD(OnDefWindowMessage)(UINT msg, WPARAM wParam, LPARAM lParam, LRESULT* plResult);

    // IOleInPlaceSiteEx
    STDMETHOD(OnInPlaceActivateEx)(BOOL *pfNoRedraw, DWORD dwFlags);        
    STDMETHOD(OnInPlaceDeactivateEx)(BOOL fNoRedraw);       
    STDMETHOD(RequestUIActivate)(void);

    // IOleInPlaceSite
    STDMETHOD(CanInPlaceActivate)(void);       
    STDMETHOD(OnInPlaceActivate)(void);        
    STDMETHOD(OnUIActivate)(void);
    STDMETHOD(GetWindowContext)(IOleInPlaceFrame** ppFrame, IOleInPlaceUIWindow** ppDoc, LPRECT lprcPosRect, LPRECT lprcClipRect, LPOLEINPLACEFRAMEINFO lpFrameInfo);
    STDMETHOD(Scroll)(SIZE scrollExtant);
    STDMETHOD(OnUIDeactivate)(BOOL fUndoable);
    STDMETHOD(OnInPlaceDeactivate)(void);
    STDMETHOD(DiscardUndoState)( void);
    STDMETHOD(DeactivateAndUndo)( void);
    STDMETHOD(OnPosRectChange)(LPCRECT lprcPosRect);

    // IOleWindow
    STDMETHOD(GetWindow)(HWND* phwnd);
    STDMETHOD(ContextSensitiveHelp)(BOOL fEnterMode);

    // IOleControlSite
    STDMETHOD(OnControlInfoChanged)(void);      
    STDMETHOD(LockInPlaceActive)(BOOL fLock);       
    STDMETHOD(GetExtendedControl)(IDispatch** ppDisp);        
    STDMETHOD(TransformCoords)(POINTL* pPtlHimetric, POINTF* pPtfContainer, DWORD dwFlags);       
    STDMETHOD(TranslateAccelerator)(MSG* pMsg, DWORD grfModifiers);
    STDMETHOD(OnFocus)(BOOL fGotFocus);
    STDMETHOD(ShowPropertyFrame)(void);

    // IOleContainer
    STDMETHOD(EnumObjects)(DWORD grfFlags, IEnumUnknown** ppenum);
    STDMETHOD(LockContainer)(BOOL fLock);

    // IParseDisplayName
    STDMETHOD(ParseDisplayName)(IBindCtx* pbc, LPOLESTR pszDisplayName, ULONG* pchEaten, IMoniker** ppmkOut);

    // IDocHostUIHandler2
    STDMETHOD(ShowContextMenu)(DWORD dwID, POINT* pptPosition, IUnknown* pCommandTarget, IDispatch* pDispatchObjectHit);
    STDMETHOD(GetHostInfo)(DOCHOSTUIINFO* pInfo);
    STDMETHOD(ShowUI)(DWORD dwID, IOleInPlaceActiveObject* pActiveObject, IOleCommandTarget* pCommandTarget, IOleInPlaceFrame* pFrame, IOleInPlaceUIWindow* pDoc);
    STDMETHOD(HideUI)();
    STDMETHOD(UpdateUI)();
    STDMETHOD(EnableModeless)(BOOL fEnable);
    STDMETHOD(OnDocWindowActivate)(BOOL fActivate);
    STDMETHOD(OnFrameWindowActivate)(BOOL fActivate);
    STDMETHOD(ResizeBorder)(LPCRECT prcBorder, IOleInPlaceUIWindow* pUIWindow, BOOL fFrameWindow);
    STDMETHOD(TranslateAccelerator)(LPMSG lpMsg, const GUID* pguidCmdGroup, DWORD nCmdID);    //ä¯ÀÀÆ÷ÏûÏ¢¹ýÂË
    STDMETHOD(GetOptionKeyPath)(LPOLESTR* pchKey, DWORD dwReserved);
    STDMETHOD(GetDropTarget)(IDropTarget* pDropTarget, IDropTarget** ppDropTarget);
    STDMETHOD(GetExternal)(IDispatch** ppDispatch);
    STDMETHOD(TranslateUrl)(DWORD dwTranslate, OLECHAR* pchURLIn, OLECHAR** ppchURLOut);
    STDMETHOD(FilterDataObject)(IDataObject* pDO, IDataObject** ppDORet);
    // IOleCommandTarget
    HRESULT STDMETHODCALLTYPE QueryStatus( __RPC__in_opt const GUID *pguidCmdGroup, ULONG cCmds, __RPC__inout_ecount_full(cCmds ) OLECMD prgCmds[ ], __RPC__inout_opt OLECMDTEXT *pCmdText);
    HRESULT STDMETHODCALLTYPE Exec( __RPC__in_opt const GUID *pguidCmdGroup, DWORD nCmdID, DWORD nCmdexecopt, __RPC__in_opt VARIANT *pvaIn, __RPC__inout_opt VARIANT *pvaOut );
    // IDocHostShowUI
    STDMETHOD(ShowMessage)(/* [in] */ HWND hwnd, /* [annotation][in] */ _In_  LPOLESTR lpstrText, /* [annotation][in] */ _In_  LPOLESTR lpstrCaption, /* [in] */ DWORD dwType,/* [annotation][in] */  _In_  LPOLESTR lpstrHelpFile, /* [in] */ DWORD dwHelpContext,/* [out] */ LRESULT *plResult) ;
    STDMETHOD(ShowHelp)(/* [in] */ HWND hwnd,/* [annotation][in] */_In_  LPOLESTR pszHelpFile,/* [in] */ UINT uCommand,/* [in] */ DWORD dwData,/* [in] */ POINT ptMouse,/* [out] */ IDispatch *pDispatchObjectHit);
        
    //// IDownloadManager
    //STDMETHOD(Download)(IMoniker *pmk,IBindCtx *pbc,DWORD dwBindVerb,LONG grfBINDF,BINDINFO *pBindInfo,
    //    LPCOLESTR pszHeaders,LPCOLESTR pszRedir,UINT uiCP);
protected:
    HRESULT CreateActiveXWnd();

protected:
    LONG m_dwRef;
    CActiveXUI* m_pOwner;
    CActiveXWnd* m_pWindow;
    IUnknown* m_pUnkSite;
    IViewObject* m_pViewObject;
    IOleInPlaceObjectWindowless* m_pInPlaceObject;
    bool m_bLocked;
    bool m_bFocused;
    bool m_bCaptured;
    bool m_bUIActivated;
    bool m_bInPlaceActive;
    bool m_bWindowless;
};


} // namespace DuiLib

#endif // __UIACTIVEX_H__
