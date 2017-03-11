#pragma once

#include <set>

namespace MyWeb {
    namespace UI {
        struct ItemData;
        class CMainFrame :
            public DuiLib::WindowImplBase,
            public DuiLib::IListCallbackUI
        {
            enum
            {
                WM_USER_CREATE_FRAME = WM_USER + 0x300,
                WM_USER_ADD_CAIPIAO_DATA,
                WM_USER_UPDATE_CAIPIAO_DATA,
                WM_USER_LOAD_CAIPIAO_DATA_FINISH,
                WM_USER_UPDATE_CAIPIAO_DATA_FINISH,
                WM_USER_UPDATE_STATUS_TEXT,
            };

            enum
            {
                TIMER_UPDATE_CAIPIAO_DATA_ID = 1000,
            };

            struct TCaipiaoData
            {
                std::wstring _date;
                std::vector<std::pair<std::wstring, std::vector<int>>> _data;
            };

            // (L"2016-01-12 005", 5)
            typedef std::map<std::wstring, int> TGapItemDataByPos;
        public:
            explicit CMainFrame(uint32_t frameID);
            virtual ~CMainFrame();

            virtual void InitWindow();


            virtual void OnFinalMessage(HWND hWnd);
            virtual void Notify(DuiLib::TNotifyUI& msg);
            virtual LRESULT HandleCustomMessage(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
        public:
            // Inherited via WindowImplBase
            virtual DuiLib::UILIB_RESOURCETYPE GetResourceType() const;
            virtual LPCTSTR GetResourceID() const;
            virtual DuiLib::CDuiString GetSkinFolder() override;
            virtual DuiLib::CDuiString GetSkinFile() override;
            virtual LPCTSTR GetWindowClassName(void) const override;

        private:
            void InitListTable();
            void InitList();
            void RunDataInit();
            void RunDataUpdate();
            void UpdateTitleText(const std::wstring& strKey, const std::wstring& strVal);
            void UpdateStatusText(const std::wstring& strStatus);
            void UpdateHorColumns(int nIndex, const std::map<int, int>& data);
        private:
            void OnHightlightDataByMouse(int nIndex, int nPos, bool isHighLight);
            void OnHightlightDataByCombo(int nPos, int nNum, bool isHighLight);
            void OnCreateFrame();
            void OnAddCaipiaoData(const TCaipiaoData& newData);
            void OnUpdateCaipiaoData(const TCaipiaoData& newData);
            void OnSelectComboDate(int nSelected);

            void OnEditReturn(const std::wstring& strEdtName, const std::wstring& strText);

            void OnSelectCombo0(int nSelected);
            void OnSelectCombo1(int nSelected);
            void OnSelectCombo2(int nSelected);
            void OnSelectCombo3(int nSelected);
            void OnSelectCombo4(int nSelected);
            void OnUpdateListTable(int nPos, const std::set<int>& nums);

            virtual LPCTSTR GetItemText(DuiLib::CControlUI* pList, int iItem, int iSubItem) override;
        private:
            uint32_t m_frameID;
            std::vector<std::pair<std::wstring, std::vector<TItemData>>> m_data;
            std::map < std::wstring, std::vector<std::pair<std::wstring, std::vector<int>>>> m_allTheDatas;
            std::map < std::wstring, std::vector<std::pair<std::wstring, std::vector<int>>>> m_allTheDatasUpdate;
            std::vector<std::vector<std::pair<std::wstring, std::wstring>>> m_tableTexts;
            std::vector<std::vector<TGapItem>> m_columns;

            std::vector<TGapItemDataByPos> m_gapItemDatas;
            std::vector<TGapItemDataByPos> m_gapItemDatasUpdate;
            std::wstring m_selectedDate;

            DuiLib::CTabLayoutUI* m_pTabMain;

            DuiLib::CListUI* m_pListTable;
            DuiLib::CListUI* m_pListMain;
            DuiLib::CNewWebBrowserUI* m_pBrowserEChart;

            DuiLib::CLabelUI* m_pLblTitle;
            DuiLib::CLabelUI* m_pLblStatus;
            DuiLib::CVolumeUI* m_pVolume;
            DuiLib::CComboUI* m_pComboDate;

            DuiLib::CComboUI* m_pCombo0;
            DuiLib::CComboUI* m_pCombo1;
            DuiLib::CComboUI* m_pCombo2;
            DuiLib::CComboUI* m_pCombo3;
            DuiLib::CComboUI* m_pCombo4;

            DuiLib::CLabelUI* m_pLbl0;
            DuiLib::CLabelUI* m_pLbl1;
            DuiLib::CLabelUI* m_pLbl2;
            DuiLib::CLabelUI* m_pLbl3;
            DuiLib::CLabelUI* m_pLbl4;

            DuiLib::CHorizontalLayoutUI* m_pHor0;
            DuiLib::CHorizontalLayoutUI* m_pHor1;
            DuiLib::CHorizontalLayoutUI* m_pHor2;
            DuiLib::CHorizontalLayoutUI* m_pHor3;
            DuiLib::CHorizontalLayoutUI* m_pHor4;

            concurrency::cancellation_token_source m_cts;
        };
    } // UI
} // MyWeb
