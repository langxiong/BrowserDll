#pragma once

#include "MyString.h"

namespace MyWeb
{
    class MyBrowserCtrl
    {
        friend class CActiveXCtrl;
    public:
        explicit MyBrowserCtrl(HWND hBindWnd);

        ~MyBrowserCtrl();

        void SetBindWindow(HWND hBindWnd);
        HWND GetBindWindow() const;

        void SetHostWindow(HWND hHostWnd);
        HWND GetHostWindow() const;

        bool CreateControl(const CLSID clsid);
        bool CreateControl(LPCTSTR pstrCLSID);
        HRESULT GetControl(const IID iid, LPVOID* ppRet);

        virtual HRESULT GetExternalCall(LPVOID* ppRet);

        virtual STDMETHODIMP QueryInterface(REFIID iid, void ** ppvObject);
        /*virtual HRESULT Download(IMoniker *pmk,IBindCtx *pbc,DWORD dwBindVerb,LONG grfBINDF,BINDINFO *pBindInfo,
        LPCOLESTR pszHeaders,LPCOLESTR pszRedir,UINT uiCP);*/
        CLSID GetClisd() const;
      
        void SetVisible(bool bVisible = true);
        void SetPos(RECT rc);
        RECT GetPos() const;

        void SetFocus();

        LRESULT MessageHandler(UINT uMsg, WPARAM wParam, LPARAM lParam, bool& bHandled);

    protected:
        virtual void ReleaseControl();
        virtual bool DoCreateControl();

    protected:
        CLSID m_clsid;
        RECT m_rcItem;
        IOleObject* m_pUnk;
        CActiveXCtrl* m_pControl;
        HWND m_hBindWnd;
        HWND m_hHostWnd;
        bool m_bCreated;
    };
}

