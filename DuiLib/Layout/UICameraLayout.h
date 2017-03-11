#pragma once

namespace DuiLib
{
    class UILIB_API CCameraLayoutUI : public CTileLayoutUI
    {
    public:
        CCameraLayoutUI();
        ~CCameraLayoutUI();

        virtual LPCTSTR GetClass() const;
        virtual LPVOID GetInterface(LPCTSTR pstrName);

        bool SelectItem(int nIndex);
        int GetCurSel() const;

        void SplicScreen();

        virtual void SetPos(RECT rc);

    private:
        int m_nCurSel;
    };
} // namespace DuiLib
