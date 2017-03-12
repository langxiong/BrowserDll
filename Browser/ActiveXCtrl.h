#pragma once

#pragma warning(push, 3)
#include <unknwn.h>
#include <exdisp.h>
#include <mshtmhst.h>
#pragma warning(pop)

namespace MyWeb
{
    class CActiveXWnd;
    class MyBrowserCtrl;
    class CActiveXCtrl :
        public IOleClientSite,
        public IOleInPlaceSiteWindowless,
        public IOleControlSite,
        public IObjectWithSite,
        public IOleContainer,
        public IDocHostShowUI
    {
        friend class MyBrowserCtrl;
        friend class CActiveXWnd;
    public:
        CActiveXCtrl();
        ~CActiveXCtrl();

        HRESULT CreateActiveXWnd();

        // IUnknown
        STDMETHOD_(ULONG, AddRef)();
        STDMETHOD_(ULONG, Release)();
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
        STDMETHOD(DiscardUndoState)(void);
        STDMETHOD(DeactivateAndUndo)(void);
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

        // IDocHostShowUI
        STDMETHOD(ShowMessage)(/* [in] */ HWND hwnd, /* [annotation][in] */ _In_  LPOLESTR lpstrText, /* [annotation][in] */ _In_  LPOLESTR lpstrCaption, /* [in] */ DWORD dwType,/* [annotation][in] */  _In_  LPOLESTR lpstrHelpFile, /* [in] */ DWORD dwHelpContext,/* [out] */ LRESULT *plResult);
        STDMETHOD(ShowHelp)(/* [in] */ HWND hwnd,/* [annotation][in] */_In_  LPOLESTR pszHelpFile,/* [in] */ UINT uCommand,/* [in] */ DWORD dwData,/* [in] */ POINT ptMouse,/* [out] */ IDispatch *pDispatchObjectHit);

    protected:
        volatile LONG m_lRefCount;
        MyBrowserCtrl* m_pOwner;
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


}

