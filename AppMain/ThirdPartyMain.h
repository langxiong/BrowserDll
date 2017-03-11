#pragma once

namespace Test { namespace Dialog{

	class ThirdPartyMain : public DuiLib::WindowImplBase
	{
	public:
		ThirdPartyMain();
		virtual ~ThirdPartyMain();

		virtual void InitWindow();
		virtual void OnFinalMessage( HWND hWnd );
		virtual void Notify(DuiLib::TNotifyUI& msg);

        virtual LRESULT HandleCustomMessage(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
		virtual DuiLib::UILIB_RESOURCETYPE GetResourceType() const;
		virtual LRESULT ThirdPartyMain::OnNcHitTest(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

	public:
		DuiLib::CEventSource OnWindowInit;
		DuiLib::CEventSource OnWindowClose;

		bool onWindowInit(void* p);
		bool onWindowClose(void* p);

	protected:
        virtual LPCTSTR GetResourceID() const;
        virtual DuiLib::CDuiString GetSkinFolder() /*= 0*/;
		virtual DuiLib::CDuiString GetSkinFile() /*= 0*/;
		virtual LPCTSTR GetWindowClassName(void) const /*= 0*/ ;
	private:

		void OnPrepare(const DuiLib::TNotifyUI& msg);

		bool OnUpdateWndPos(void* params);

		HWND m_hChildWnd;
	};
}/*Dialog*/} /*Test*/
