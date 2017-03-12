#include "StdAfx.h"
#include "UISubsystem.h"
#include "Poco/Delegate.h"
#include "Poco/Util/Option.h"
#include "Poco/Util/OptionCallback.h"
#include "Poco/Util/PropertyFileConfiguration.h"
#include "FrameMgr.h"
#include "StringHelper.h"
#include "BrowserModuleApis.h"
#include "MyCast.h"

using namespace Poco::Util;
using Poco::Logger;

namespace MyWeb {
    namespace UI {

        CUISubsystem::CUISubsystem() :
            m_pLogger(&Logger::get(name())),
            m_spConf(new LayeredConfiguration),
            m_spFrameMgr(new CFrameMgr(::GetCurrentThreadId())),
            m_spBrowserModuleApis(new TBrowserModuleApis),
            m_hBrowserModule(nullptr)
        {
        }

        CUISubsystem::~CUISubsystem()
        {

        }

        int CUISubsystem::CreateBrowserCtrl(HWND hBindWnd)
        {
            if (m_spBrowserModuleApis->_createBrowserCtrl)
            {
                return m_spBrowserModuleApis->_createBrowserCtrl(hBindWnd);
            }
            return -1;
        }

        void CUISubsystem::DestroyBrowserCtrl(int nIndex)
        {
            if (m_spBrowserModuleApis->_destroyBrowserCtrl)
            {
                return m_spBrowserModuleApis->_destroyBrowserCtrl(nIndex);
            }
        }

        void CUISubsystem::SetBrowserCtrlPos(int nIndex, RECT rc)
        {
            if (m_spBrowserModuleApis->_setBrowserCtrlPos)
            {
                return m_spBrowserModuleApis->_setBrowserCtrlPos(nIndex, rc);
            }
        }

        void CUISubsystem::NavigateUrl(int nIndex, const MyString& url)
        {
            if (m_spBrowserModuleApis->_navigateUrl)
            {
                return m_spBrowserModuleApis->_navigateUrl(nIndex, url.c_str());
            }
        }

        void CUISubsystem::ExecuteJSCode(int nIndex, const MyString& jsCode)
        {
            if (m_spBrowserModuleApis->_executeJSCode)
            {
                return m_spBrowserModuleApis->_executeJSCode(nIndex, jsCode.c_str());
            }
        }

        bool CUISubsystem::RegisterBrowserCallback(int nIndex)
        {
            if (m_spBrowserModuleApis->_registerBrowserCallback)
            {
                return m_spBrowserModuleApis->_registerBrowserCallback(nIndex);
            }
            return false;
        }

        bool CUISubsystem::UnRegisterBrowserCallback(int nIndex)
        {
            if (m_spBrowserModuleApis->_unRegisterBrowserCallback)
            {
                return m_spBrowserModuleApis->_unRegisterBrowserCallback(nIndex);
            }
            return false;
        }

        const char* CUISubsystem::name() const
        {
            return "CUISubsystem";
        }

        void CUISubsystem::initialize(Poco::Util::Application& app)
        {
            std::string appDir = app.config().getString("application.dir");
            m_spConf->add(new Poco::Util::PropertyFileConfiguration(), 0, true, false);
            config().setUInt(Constants::key_nMainThreadId, ::GetCurrentThreadId());

            CreateFrame += Poco::delegate(m_spFrameMgr.get(), &CFrameMgr::OnCreateFrame);
            QuitFrame += Poco::delegate(m_spFrameMgr.get(), &CFrameMgr::OnQuitFrame);

            // load browser library;
            m_hBrowserModule = ::LoadLibraryA((appDir + "Browser.dll").c_str());
            _InitBrowserModuleApis();
        }

        void CUISubsystem::uninitialize()
        {
            if (m_hBrowserModule)
            {
                ::FreeLibrary(m_hBrowserModule);
            }
            if (!m_configPath.empty())
            {
            }
            CreateFrame -= Poco::delegate(m_spFrameMgr.get(), &CFrameMgr::OnCreateFrame);
        }

        void CUISubsystem::reinitialize(Poco::Util::Application& app)
        {

        }

        void CUISubsystem::defineOptions(Poco::Util::OptionSet& options)
        {
            options.addOption(
                Option("include-dir", "I", "specify an include search path")
                .required(false)
                .repeatable(true)
                .argument("path")
                .callback(OptionCallback<CUISubsystem>(this, &CUISubsystem::handleTest)));
        }

        void CUISubsystem::_InitBrowserModuleApis()
        {
            if (!m_hBrowserModule)
            {
                return;
            }

            FARPROC pFunc = ::GetProcAddress(m_hBrowserModule, "CreateBrowserCtrl");
            if (pFunc)
            {
                m_spBrowserModuleApis->_createBrowserCtrl = cast_to_function<int(HWND hBindWnd)>(pFunc);
            }

            pFunc = ::GetProcAddress(m_hBrowserModule, "DestroyBrowserCtrl");
            if (pFunc)
            {
                m_spBrowserModuleApis->_destroyBrowserCtrl = cast_to_function<void (int)>(pFunc);
            }

            pFunc = ::GetProcAddress(m_hBrowserModule, "SetBrowserCtrlPos");
            if (pFunc)
            {
                m_spBrowserModuleApis->_setBrowserCtrlPos = cast_to_function<void(int, RECT)>(pFunc);
            }

            pFunc = ::GetProcAddress(m_hBrowserModule, "NavigateUrl");
            if (pFunc)
            {
                m_spBrowserModuleApis->_navigateUrl = cast_to_function<void(int, const TCHAR*)>(pFunc);
            }

            pFunc = ::GetProcAddress(m_hBrowserModule, "ExecuteJSCode");
            if (pFunc)
            {
                m_spBrowserModuleApis->_executeJSCode = cast_to_function<void (int, const TCHAR*)>(pFunc);
            }

            pFunc = ::GetProcAddress(m_hBrowserModule, "RegisterBrowserCallback");
            if (pFunc)
            {
                m_spBrowserModuleApis->_registerBrowserCallback = cast_to_function<bool (int)>(pFunc);
            }

            pFunc = ::GetProcAddress(m_hBrowserModule, "UnRegisterBrowserCallback");
            if (pFunc)
            {
                m_spBrowserModuleApis->_unRegisterBrowserCallback = cast_to_function<bool (int)>(pFunc);
            }
        }

        void CUISubsystem::handleTest(const std::string& name, const std::string& value)
        {
            poco_trace_f2(logger(), "name[%s] value[%s]", name, value);
        }

        Poco::Util::LayeredConfiguration& CUISubsystem::config() const
        {
            return *const_cast<Poco::Util::LayeredConfiguration*>(m_spConf.get());
        }

        int CUISubsystem::RunUISubsystem()
        {
            logger().information("Ui subsystem begin");
            MSG msg = { 0 };
            ::PeekMessage(&msg, NULL, NULL, NULL, PM_NOREMOVE);
            ::PostThreadMessage(::GetCurrentThreadId(), WM_THREAD_MAIN_CREATE_FRAME, 0, 0);
            while (::GetMessage(&msg, nullptr, 0, 0) != 0)
            {
                if (msg.message == WM_THREAD_MAIN_CREATE_FRAME)
                {
                    CreateFrame.notify(this);
                }
                else if (msg.message == WM_THREAD_FRAME_QUIT)
                {
                    QuitFrame.notify(this, msg.wParam);
                }
                else if (msg.message == WM_THREAD_MAIN_CREATE_RENDER_PARENT_WND)
                {
                    CreateRenderParentWnd.notify(this, msg.wParam);
                }
                else
                {
                    ::TranslateMessage(&msg);
                    ::DispatchMessage(&msg);
                }
               
            }
            logger().information("Ui subsystem end");
            return 0;
        }
    } // UI
} // MyWeb