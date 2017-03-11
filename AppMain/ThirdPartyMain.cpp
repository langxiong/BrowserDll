#include "StdAfx.h"
#include "ThirdPartyMain.h"
#include "Poco/Process.h"
#include "Poco/Path.h"
#include "Poco/File.h"
#include "Poco/AutoPtr.h"
#include "Poco/Util/PropertyFileConfiguration.h"
#include "Poco/Buffer.h"

using namespace DuiLib;
using Poco::Util::Application;
using Poco::Util::PropertyFileConfiguration;
using Poco::Process;
using Poco::Path;
using Poco::File;
using Poco::AutoPtr;

namespace Test { namespace Dialog{
	ThirdPartyMain::ThirdPartyMain():
		m_hChildWnd(nullptr)
	{
		OnWindowInit += MakeDelegate(this, &ThirdPartyMain::onWindowInit);
	}

	void ThirdPartyMain::InitWindow()
	{
		Poco::Buffer<TCHAR> buffer(256);
		LoadString(m_PaintManager.GetResourceDll(), ISD_TEXT_TEST, buffer.begin(), buffer.size());
		
		CLabelUI* pLblTitle = static_cast<CLabelUI*>(m_PaintManager.FindControl(_T("lblTitle")));
		assert(pLblTitle);
		if (pLblTitle)
		{
			pLblTitle->SetText(buffer.begin());
		}
		OnWindowInit += MakeDelegate(this, &ThirdPartyMain::onWindowInit);
	}

	ThirdPartyMain::~ThirdPartyMain()
	{
		PostQuitMessage(0);
	}

	

	void ThirdPartyMain::OnFinalMessage( HWND hWnd )
	{
		OnWindowInit -= MakeDelegate(this, &ThirdPartyMain::onWindowInit);
		OnWindowInit -= MakeDelegate(this, &ThirdPartyMain::onWindowInit);
		WindowImplBase::OnFinalMessage(hWnd);
		delete this;
	}

	void ThirdPartyMain::Notify(TNotifyUI& msg)
	{
		if (msg.sType == DUI_MSGTYPE_WINDOWINIT)
		{
			OnPrepare(msg);
			return;
		}

		if (msg.sType == DUI_MSGTYPE_CLICK)
		{
			if (msg.pSender->GetName() == _T("btnClose"))
			{
				Close();
				return;
			}

			if (msg.pSender->GetName() == _T("btnMin"))
			{
				SendMessage(WM_SYSCOMMAND, SC_MINIMIZE, 0);
				return;
			}

			if (msg.pSender->GetName() == _T("btnStartXrush"))
			{
				// prepare configure file
				Path tmpPath(Path::temp());
				tmpPath.pushDirectory("PocoFileSample");
				File tmpDir(tmpPath);
				tmpDir.createDirectories();
				File tmpFile(Path(tmpPath, "xrush.properties"));
				if (tmpFile.createFile())
				{// make sure we have crated the configure file 

				}
				AutoPtr<PropertyFileConfiguration> pConf = new PropertyFileConfiguration(tmpFile.path());
				pConf->setString("hParent", std::to_string((long)GetHWND()));
				pConf->setInt("sponsorId", 123);

				CControlUI* pOptSpeedUp = m_PaintManager.FindControl(_T("optSpeedUp"));
				if (pOptSpeedUp)
				{
					auto pos = pOptSpeedUp->GetPos();
					POINT pt = {pos.left, pos.top};
					ClientToScreen(GetHWND(), &pt);
					pConf->setInt("optSpeedUpLeft", pt.x);
					pConf->setInt("optSpeedUpTop", pt.y);
				}

				pConf->save(tmpFile.path());
				//prepare cmdline
				auto& conf = Application::instance().config();
				std::vector<std::string> args;
				args.push_back(std::string(("/config-file=") + tmpFile.path()));

				Process::launch(conf.getString("application.path"), args, conf.getString("application.dir"));
				return;
			}
		}

	}

	LRESULT ThirdPartyMain::HandleCustomMessage(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		/*if (uMsg == WM_USER_XRUSH_2_THIRD_PARTY)
		{
			bHandled = TRUE;
			switch (wParam)
			{
			case REPORT_CLIENT_HWND:
				m_hChildWnd = reinterpret_cast<HWND>(lParam);
				break;
			case REPORT_CLINET_CLOSED:
				break;
			default:
				break;
			}
			return S_OK;
		}*/
		return WindowImplBase::HandleCustomMessage(uMsg, wParam, lParam, bHandled);
	}

	UILIB_RESOURCETYPE ThirdPartyMain::GetResourceType() const
	{
		return UILIB_ZIPRESOURCE;
	}

	LRESULT ThirdPartyMain::OnNcHitTest(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		/*POINT pt; pt.x = GET_X_LPARAM(lParam); pt.y = GET_Y_LPARAM(lParam);
		::ScreenToClient(*this, &pt);

		RECT rcClient;
		::GetClientRect(*this, &rcClient);

		if ( !::IsZoomed(*this) ) {
			RECT rcSizeBox = m_PaintManager.GetSizeBox();
			if ( pt.y < rcClient.top + rcSizeBox.top ) {
				if ( pt.x < rcClient.left + rcSizeBox.left ) return HTTOPLEFT;
				if ( pt.x > rcClient.right - rcSizeBox.right ) return HTTOPRIGHT;
				return HTTOP;
			}
			else if ( pt.y > rcClient.bottom - rcSizeBox.bottom ) {
				if ( pt.x < rcClient.left + rcSizeBox.left ) return HTBOTTOMLEFT;
				if ( pt.x > rcClient.right - rcSizeBox.right ) return HTBOTTOMRIGHT;
				return HTBOTTOM;
			}
			if ( pt.x < rcClient.left + rcSizeBox.left ) return HTLEFT;
			if ( pt.x > rcClient.right - rcSizeBox.right ) return HTRIGHT;
		}

		RECT rcCaption = m_PaintManager.GetCaptionRect();
		if ( pt.x >= rcClient.left + rcCaption.left && pt.x < rcClient.right - rcCaption.right \
			&& pt.y >= rcCaption.top && pt.y < rcCaption.bottom ) {
				CControlUI* pControl = static_cast<CControlUI*>(m_PaintManager.FindControl(pt));
				if (pControl)
				{
					if (pControl->IsCaption())
						return HTCAPTION;
				}
		}

		return HTCLIENT;*/
		return WindowImplBase::OnNcHitTest(uMsg, wParam, lParam, bHandled);
	}


    LPCTSTR ThirdPartyMain::GetResourceID() const
    {
        return MAKEINTRESOURCE(IDR_ZIP_RES);
    }

	CDuiString ThirdPartyMain::GetSkinFolder()
	{
		return _T("");
	}

	CDuiString ThirdPartyMain::GetSkinFile()
	{
		return _T("third_party_main.xml");
	}

	LPCTSTR ThirdPartyMain::GetWindowClassName(void) const 
	{
		return _T("ThirdPartyMain");
	}

	void ThirdPartyMain::OnPrepare(const DuiLib::TNotifyUI& msg)
	{
		//CControlUI* pContainerArea = m_PaintManager.FindControl(_T("optSpeedUp"));
		//assert(pContainerArea);
		//pContainerArea->OnSize += 
		//MakeDelegate<ThirdPartyMain, ThirdPartyMain>(this, &ThirdPartyMain::OnUpdateWndPos);
	}

	bool ThirdPartyMain::OnUpdateWndPos(void* params)
	{
		if (m_hChildWnd != nullptr)
		{
			CControlUI* pC = reinterpret_cast<CControlUI*>(params);
			auto pos = pC->GetPos();
			POINT pt = {pos.left, pos.top};
			ClientToScreen(GetHWND(), &pt);

			//::SendMessage(m_hChildWnd, WM_USER_THIRD_PARTY_2_XRUSH, NOTICE_CLIENT_ADJUST_POS, MAKELPARAM(pt.x, pt.y));
		}
		return true;
	}

	bool ThirdPartyMain::onWindowInit(void* p)
	{
		return true;
	}

	bool ThirdPartyMain::onWindowClose(void* p)
	{
		return true;
	}

}/*Dialog*/} /*Test*/