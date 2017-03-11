#pragma once

#include "../DuiLib/UIlib.h"

namespace MyWeb {
    namespace UI {
        class CMyListElementItem : public DuiLib::CLabelUI
        {
        public:
            static const std::wstring DUI_CTR_MY_LIST_ELEMENT_ITEM;
            static const DWORD dwDHightLightBorderColor;
        public:
            CMyListElementItem();
            ~CMyListElementItem();

            virtual LPCTSTR GetClass() const;
            virtual LPVOID GetInterface(LPCTSTR pstrName);

            virtual void DoEvent(DuiLib::TEventUI& event) override;

            void HighlightItem(bool isHighLight);
        private:

            DWORD m_dwBackupBorderColor;
            bool m_isHighLight;

        };
    } // UI
} // MyWeb
