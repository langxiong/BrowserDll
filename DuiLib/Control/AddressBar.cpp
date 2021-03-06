#include "stdafx.h"
#include "../MainFrame.hpp"
#include "../Core/Core_Instance.h"
#include "../Thread/NewRop.hpp"
#include <Poco/UnicodeConverter.h>
#include "AddressBar.hpp"

namespace Xgamer{
	namespace UI
	{
		///////////////////////////////
		CAddrListItem::CAddrListItem()
			:m_pCloseLayout(NULL)
			,m_pIcon(NULL)
			,m_pTitle(NULL)
			,m_pUrl(NULL)
		{

		}

		CAddrListItem::~CAddrListItem()
		{

		}

		LPCTSTR CAddrListItem::GetClass() const
		{
			return _T("AddressListItem");
		}

		LPVOID CAddrListItem::GetInterface(LPCTSTR pstrName)
		{
			if (_tcscmp(pstrName, _T("AddressListItem")) == 0)
			{
				return static_cast<CAddrListItem*>(this);
			}
			return CListContainerElementUI::GetInterface(pstrName);
		}

		//void CAddrListItem::SetInit(const CDuiString& sIcon, const CDuiString& sTitle, const CDuiString& sUrl)
		//{
		//	m_sIcon = sIcon;
		//	m_sTitle = sTitle;
		//	m_sUrl = sUrl;
		//}

		void CAddrListItem::ReSetData(const CDuiString& sIcon, const CDuiString& sTitle, const CDuiString& sUrl)
		{
			m_pIcon->SetIcoImage(sIcon);
			m_pTitle->SetText(sTitle);
			m_pUrl->SetText(sUrl);
			SetVisible(true);
		}

		void CAddrListItem::DoEvent(TEventUI& event)
		{
			switch(event.Type)
			{
			case UIEVENT_MOUSEENTER:
				if (IsVisible())
				{
					m_pCloseLayout->SetVisible(true);
				}
				break;
			case UIEVENT_MOUSELEAVE:
				if (IsVisible())
				{
					m_pCloseLayout->SetVisible(false);
				}
				break;
			default:
				break;
			}
			CListContainerElementUI::DoEvent(event);
		}

		CDuiString CAddrListItem::GetUrl() const
		{
			return m_pUrl->GetText();
		}

		CDuiString CAddrListItem::GetIcon() const
		{
			return m_pIcon->GetText();
		}

		CDuiString CAddrListItem::GetTitle() const
		{
			return m_pTitle->GetText();
		}


		void CAddrListItem::DoInit()
		{
			m_pCloseLayout = static_cast<CContainerUI*>(m_pManager->FindSubControlByName(this, _T("layout")));
			m_pIcon = static_cast<CIconUI*>(m_pManager->FindSubControlByName(this, _T("icon")));
			m_pTitle = static_cast<CLabelUI*>(m_pManager->FindSubControlByName(this, _T("title")));
			m_pUrl = static_cast<CLabelUI*>(m_pManager->FindSubControlByName(this, _T("url")));
		}


		///////////////////////////////
		//SearchItem
		CDuiString CSearchItem::sm_sHead(_T("http://www.baidu.com/s?wd="));
		CDuiString CSearchItem::sm_sTail(_T("  百度搜索"));
		CSearchItem::CSearchItem()
			:m_pTitle(NULL)
			,m_sSearchText(_T("这样"))
		{

		}

		CSearchItem::~CSearchItem()
		{

		}

		LPCTSTR CSearchItem::GetClass() const
		{
			return _T("SearchItem");
		}

		LPVOID CSearchItem::GetInterface(LPCTSTR pstrName)
		{
			if (_tcscmp(pstrName, _T("SearchItem")) == 0)
			{
				return static_cast<CSearchItem*>(this);
			}
			return CListContainerElementUI::GetInterface(pstrName);
		}

		void CSearchItem::SetSearchExtraInfo(const CDuiString& sHead, const CDuiString& sTail)
		{
			sm_sHead = sHead;
			sm_sTail = sTail;
		}

		void CSearchItem::SetSearchText(const CDuiString& sText)
		{
			m_sSearchText = sText;
			m_pTitle->SetText(Convert(sText));
		}

		void CSearchItem::DoEvent(TEventUI& event)
		{
			CListContainerElementUI::DoEvent(event);
		}

		CDuiString CSearchItem::GetUrl() const
		{
			return sm_sHead + m_sSearchText;
		}

		void CSearchItem::DoInit()
		{
			m_pTitle = static_cast<CLabelUI*>(m_pManager->FindSubControlByName(this, _T("title")));
		}

		CDuiString CSearchItem::Convert(const CDuiString& sText)
		{
			return sText + sm_sTail;
		}

		const std::string CGameItem::sm_sIconPath(Core_Instance::instance()->GetPath("icon").toString());

		//CGameItem
		///////////////////////////////

		CGameItem::CGameItem()
			:m_pTitle(NULL)
			,m_pDesc(NULL)
			,m_pImg(NULL)
		{
			
		}

		CGameItem::~CGameItem()
		{
			m_pair_cts.second.cancel();
		}

		LPCTSTR CGameItem::GetClass() const
		{
			return _T("GameItem");
		}

		LPVOID CGameItem::GetInterface(LPCTSTR pstrName)
		{
			if (_tcscmp(pstrName, _T("GameItem")) == 0)
			{
				return static_cast<CGameItem*>(this);
			}
			return CListContainerElementUI::GetInterface(pstrName);
		}

		std::wstring CGameItem::GetGameId() const
		{
			return m_data._sId;
		}


		std::wstring CGameItem::GetIconPath() const
		{
			return m_data._sIconPath;
		}

		void CGameItem::InitData(const std::wstring& sId, const std::wstring& sIconSrcPath
				, const std::wstring& sGameName, const std::wstring& sGameType
				, const std::wstring& sGameDesc)
		{
			// caculate local iconPath;


			m_data._sId = sId;

			std::string fileName(BuildSha1Up(W2A_UTF8(sGameName)));
			m_data._sGameName = sGameName;
			m_data._sGameType = sGameType;
			m_data._sGameDesc = sGameDesc;
			m_data._sIconPath = A2W_UTF8(sm_sIconPath + fileName + ".png");

			CDuiString sText;
			sText.SmallFormat(_T("<f 1><c #333333>%s</c><c #666666>(%s)</c></f>"), sGameName.c_str(), sGameType.c_str());
			m_pTitle->SetText(sText);
			m_pDesc->SetText(sGameDesc.c_str());
			m_pDesc->SetToolTip(sGameDesc.c_str());

			m_pair_cts.second.cancel();
			auto token = m_pair_cts.first.get_token();
			m_pair_cts.second = m_pair_cts.first;
			m_pair_cts.first = concurrency::cancellation_token_source();
			auto t = concurrency::create_task([sIconSrcPath, this]()
			{
				using Xgamer::Thread::NewRop;
				if (NewRop::HttpGetFile(W2A_UTF8(sIconSrcPath), W2A_UTF8(m_data._sIconPath)))
				{
					return true;
				}
				return false;
			}, token);

			t.then([this](bool result)
			{
				if (result)
				{
					m_pImg->SetBkImage(m_data._sIconPath.c_str());
				}
			}, token);

		}

		std::wstring CGameItem::GetUrl() const
		{
			return std::wstring(XGAMER_GAME_SEARCH_HEAD_W) + m_data._sGameName;
		}

		void CGameItem::DoEvent(TEventUI& event)
		{
			CListContainerElementUI::DoEvent(event);
		}

		void CGameItem::DoInit()
		{
			m_pImg = static_cast<CLabelUI*>(m_pManager->FindSubControlByName(this, _T("img")));
			m_pTitle = static_cast<CLabelUI*>(m_pManager->FindSubControlByName(this, _T("title")));
			m_pDesc = static_cast<CLabelUI*>(m_pManager->FindSubControlByName(this, _T("desc")));
			assert(m_pImg && m_pTitle && m_pDesc);
		}

	}//UI
}//Xgamer
