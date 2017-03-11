#pragma once

#include "../DuiLib/UIlib.h"

namespace MyWeb {
    namespace UI {
        class CMyListSubContainer : public DuiLib::CHorizontalLayoutUI
        {
        public:
            static const std::wstring DUI_CTR_MY_LIST_SUB_CONTAINER;
        public:
            CMyListSubContainer();
            ~CMyListSubContainer();

            virtual LPCTSTR GetClass() const;
            virtual LPVOID GetInterface(LPCTSTR pstrName);

            virtual void DoInit() override;

            void HighLightItem(int nNum);

            void UpdateListInfo(DuiLib::TListInfoUI* pListInfo);

            CMyListElementItem* UpdateSelectIndex(int nIndex);

        private:
            int m_nSelectIndex;

            DuiLib::TListInfoUI* m_pListInfo;
        };
    } // UI
} // MyWeb
