#pragma once
#include <memory>
#include <map>
#include <thread>

#include "../DuiLib/UIlib.h"

namespace DuiLib
{
    class CTestBrowserUI : public CContainerUI
    {
    public:
        CTestBrowserUI();
        ~CTestBrowserUI();

        virtual LPCTSTR GetClass() const;
        virtual LPVOID GetInterface(LPCTSTR pstrName);

        virtual bool IsVisible() const;
        virtual void SetVisible(bool bVisible = true);

        void NavigateUrl(const MyString& url);
        void ExecuteJscode(const MyString& jscode);

    private:
        virtual void DoInit();

        virtual void SetPos(RECT rc);
    private:
        CWindowWnd* m_pBindWnd;
        int m_nIndex;
    };
} // namespace DuiLib
