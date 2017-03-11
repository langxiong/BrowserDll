#include "stdafx.h"
#include "DlgCameraControl.h"
#include "AppCommon.h"
#include "Poco/FileStream.h"
#include "Poco/StreamCopier.h"
#include "Poco/JSON/JSON.h"
#include "Poco/JSON/Parser.h"
#include "Poco/Nullable.h"
#include "Poco/Dynamic/Struct.h"

#include "StringHelper.h"

using namespace DuiLib;
using namespace Poco;
using namespace Poco::JSON;
using namespace Poco::Dynamic;

namespace MyWeb {
    namespace UI {

        inline Poco::Logger& logger()
        {
            return Poco::Util::Application::instance().getSubsystem<CUISubsystem>().logger();
        }

        CDlgCameraControl::CDlgCameraControl()
        {
        }
        CDlgCameraControl::~CDlgCameraControl()
        {
        }

        void CDlgCameraControl::InitWindow()
        {
            InitControls();
            CenterWindow();
        }

        void CDlgCameraControl::OnFinalMessage(HWND hWnd)
        {
            WindowImplBase::OnFinalMessage(hWnd);
        }

        void CDlgCameraControl::Notify(DuiLib::TNotifyUI & msg)
        {
            if (msg.sType == DUI_MSGTYPE_CLICK)
            {
                auto strName(msg.pSender->GetName());
                if (strName == _T("btnClose"))
                {
                    Close();
                }
                return;
            }
        }

        UILIB_RESOURCETYPE CDlgCameraControl::GetResourceType() const
        {
#if _DEBUG
            return WindowImplBase::GetResourceType();
#else
            return UILIB_ZIPRESOURCE;
#endif
        }

        LPCTSTR CDlgCameraControl::GetResourceID() const
        {
#if _DEBUG
            return WindowImplBase::GetResourceID();
#else
            return MAKEINTRESOURCE(IDR_ZIP_RES);
#endif
        }

        CDuiString CDlgCameraControl::GetSkinFolder()
        {
#if _DEBUG
            return _T("UIZIP");
#else
            return CDuiString();
#endif
        }

        CDuiString CDlgCameraControl::GetSkinFile()
        {
            return _T("dlg_camera_control.xml");
        }

        LPCTSTR CDlgCameraControl::GetWindowClassName(void) const
        {
            return _T("MyWeb::UI::CDlgCameraControl");
        }

        CControlUI* CDlgCameraControl::CreateControl(LPCTSTR pstrClass)
        {
            return WindowImplBase::CreateControl(pstrClass);
        }

        void CDlgCameraControl::InitControls()
        {
        }

        LRESULT CDlgCameraControl::HandleCustomMessage(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
        {
            return WindowImplBase::HandleCustomMessage(uMsg, wParam, lParam, bHandled);
        }

    } // UI
} // MyWeb
