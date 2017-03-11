#pragma once

#include "../DuiLib/UIlib.h"

namespace MyWeb {
    namespace UI {
        class CMyComboNumElement : public DuiLib::CListContainerElementUI
        {
        public:
            static const std::wstring DUI_CTR_MY_COMBO_NUM_ELEMENT;
            static const DWORD dwDHightLightBorderColor;
        public:
			CMyComboNumElement();
			~CMyComboNumElement();

            virtual LPCTSTR GetClass() const;
            virtual LPVOID GetInterface(LPCTSTR pstrName);

			virtual void DoInit();

		};
    } // UI
} // MyWeb
