#pragma once

#include "../DuiLib/UIlib.h"

namespace MyWeb {
    namespace UI {
        class CListHeaderNumItem : public DuiLib::CListHeaderItemUI
        {
        public:
            static const std::wstring DUI_CTR_THUBNAIL_HEADER_ITEM;

			static int sm_nIndex;
        public:
            CListHeaderNumItem();
            ~CListHeaderNumItem();
            
            static void ResetIndex();

            virtual LPCTSTR GetClass() const;
            virtual LPVOID GetInterface(LPCTSTR pstrName);

            virtual void DoInit();

            virtual SIZE EstimateSize(SIZE szAvailable) override;

        };

        class CListHeaderDateItem : public DuiLib::CListHeaderItemUI
        {
        public:
            static const std::wstring DUI_CTR_LEADER_HEADER_ITEM;
        public:
            CListHeaderDateItem();
            ~CListHeaderDateItem();

            virtual LPCTSTR GetClass() const;
            virtual LPVOID GetInterface(LPCTSTR pstrName);

            virtual SIZE EstimateSize(SIZE szAvailable) override;

        };
    } // UI
} // MyWeb
