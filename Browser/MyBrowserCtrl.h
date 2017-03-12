#pragma once

#include "MyString.h"
#include <memory>
#include <thread>
#include <ExDisp.h>
#include <Mshtml.h>

namespace MyWeb
{
    class MyBrowserCtrl
    {
        friend class CActiveXCtrl;

        struct TData
        {
            HANDLE _hThreadHandle;
            DWORD _dwThreadId;
            std::shared_ptr<MyBrowserCtrl> _spBrowserCtrl;
        };
    public:

        explicit MyBrowserCtrl(HWND hBindWnd);

        ~MyBrowserCtrl();

        void MessageLoopThread();

        bool HandleCustomThreadMsg(const MSG& msg);

        void WaitThreadMsgQueueCreate();

        HANDLE GetThreadHandle() const;
        DWORD GetThreadId() const;

        void SetBindWindow(HWND hBindWnd);
        HWND GetBindWindow() const;

        void SetHostWindow(HWND hHostWnd);
        HWND GetHostWindow() const;

        void NavigateUrl(const MyString& url);
        void ExecuteJscode(const MyString& jscode);

        bool CreateControl(const CLSID clsid);
        bool CreateControl(LPCTSTR pstrCLSID);

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

    private:

        bool GetHTMLWindow(CComPtr<IHTMLWindow2>& spHtmlWindow);

    public:
        static int CreateBrowserCtrl(HWND hBindWnd);
        static void DestroyBrowserCtrl(int nIndex);
        static void SetBrowserCtrlPos(int nIndex, RECT rc);
        static void BrowserCtrlNavigateUrl(int nIndex, const MyString& url);
        static void BrowserCtrlExecuteJscode(int nIndex, const MyString& jscode);

    private:
        virtual void ReleaseControl();
        virtual bool DoCreateControl();

    private:
        static int sm_nIndex;
        static std::map<int, TData> sm_spBrowserCtrls;

        CLSID m_clsid;
        RECT m_rcItem;
        IOleObject* m_pUnk;
        CActiveXCtrl* m_pControl;
        CComPtr<IWebBrowser2> m_spWebBrowser2;
        HWND m_hBindWnd;
        HWND m_hHostWnd;
        bool m_bCreated;

        HANDLE m_hInitEvent;
        DWORD m_dwWebWorkThreadId;
    };
}

