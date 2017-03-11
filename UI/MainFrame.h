#pragma once

namespace MyWeb {
    namespace UI {
        class CMainFrame :public DuiLib::WindowImplBase
        {
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
            virtual DuiLib::CControlUI* CreateControl(LPCTSTR pstrClass);
        private:
            void InitControls();
        private:
            void OnCreateFrame();
            void OnUpdateEmotionAnalysisData(const std::wstring& str);
        private:
            uint32_t m_frameID;
            size_t m_nEmotionAnalysis;

            std::vector<HANDLE> m_hWorkthreads;
            std::vector<DWORD> m_dwWorkThreadIds;

			DuiLib::CVerticalLayoutUI* m_pCurveContainer;
			DuiLib::CVerticalLayoutUI* m_pRadarContainer;
			DuiLib::CVerticalLayoutUI* m_pEmotionAnalysisContainer;
			DuiLib::CTextUI* m_pVoiceTxt;
			DuiLib::CLabelUI* m_pVoiceLQuote;
			DuiLib::CLabelUI* m_pVoiceRQuote;
        };
    } // UI
} // MyWeb
