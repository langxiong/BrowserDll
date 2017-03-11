#include "StdAfx.h"
#include "MyComboNumElement.h"

using namespace DuiLib;

namespace MyWeb {
    namespace UI {
        CMyComboNumElement::CMyComboNumElement()
        {
        }

        CMyComboNumElement::~CMyComboNumElement(void)
        {
        }

        LPCTSTR CMyComboNumElement::GetClass() const
        {
            return _T("MyComboNumElement");
        }

        LPVOID CMyComboNumElement::GetInterface(LPCTSTR pstrName)
        {
			if (DUI_CTR_MY_COMBO_NUM_ELEMENT == pstrName)
            {
                return this;
            }
            return CListContainerElementUI::GetInterface(pstrName);
        }

		void CMyComboNumElement::DoInit()
		{
			COptionUI* pOpt = new COptionUI;
			pOpt->SetText(GetUserData());
			pOpt->SetFont(0);
		}

		const std::wstring CMyComboNumElement::DUI_CTR_MY_COMBO_NUM_ELEMENT(_T("MyComboNumElement"));
    }
}