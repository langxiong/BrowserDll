#pragma once

#include "../DuiLib/UIlib.h"

namespace MyWeb {
    namespace UI {
        class CMyListContainer : public DuiLib::CListContainerElementUI
        {
        public:
            static const std::wstring DUI_CTR_MY_LIST_CONTAINER;
        public:
            CMyListContainer();
            ~CMyListContainer();

            virtual LPCTSTR GetClass() const;
            virtual LPVOID GetInterface(LPCTSTR pstrName);

            virtual void DoInit() override;

            void SetLeaderText(const std::wstring& strLeaderText);
            const std::wstring& GetLeaderText() const;

            void UpdateFollowNums(std::vector<TItemData>& itemDatas);

            virtual void DoPaint(HDC hDC, const RECT& rcPaint) override;

            virtual void DrawItemBk(HDC hDC, const RECT& rcItem) override;

        private:

            std::wstring m_strLeaderText;

            DuiLib::CLabelUI* m_pCtrLeader;
        };
    } // UI
} // MyWeb
