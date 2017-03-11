pragma once

namespace MyWeb {
    namespace UI {
        class CDlgCameraControl :public DuiLib::WindowImplBase
        {
        public:
            explicit CDlgCameraControl();
            virtual ~CDlgCameraControl();

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
        };
    } // UI
} // MyWeb
