#pragma once
#include <memory>
#include <atlcomcli.h>
#include <map>
#include <thread>

namespace DuiLib
{
    class CWebWnd;
    class UILIB_API CNewWebBrowserUI : public CContainerUI
    {
        struct CWebCtrl
        {
            std::shared_ptr<CActiveXUI> m_spActiveCtl;
            CComPtr<IWebBrowser2> m_spWebBrowser2;
        };
    public:
        CNewWebBrowserUI();
        ~CNewWebBrowserUI();

        virtual LPCTSTR GetClass() const;
        virtual LPVOID GetInterface(LPCTSTR pstrName);

        virtual bool IsVisible() const;
        virtual void SetVisible(bool bVisible = true);

    private:
        virtual void DoInit();

        bool onSize(void* pParam);
        void EnterWebThreadMessageLoop();
        bool HandleWebMessage(CPaintManagerUI* pPaintManager, MSG &msg);

        void OnCreateBrowser(CPaintManagerUI* pPaintManager, CWebCtrl* pWebCtrl, CWebWnd* pHostWnd);
        void OnQuitBrowser();
        void OnSetBrowserPos(MSG msg, CWebCtrl* pWebCtrl, CWebWnd* pHostWnd);
        void OnShowBrowser(bool isVisible, CWebWnd* pHostWnd);

    private:
        CWindowWnd* m_pBindWnd;

    private:
        std::thread* m_pThread;
        HANDLE m_hInitEvent;
        DWORD m_dwWebWorkThreadId;
    };
} // namespace DuiLib
