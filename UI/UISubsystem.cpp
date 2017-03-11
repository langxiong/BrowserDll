#include "StdAfx.h"
#include "UISubsystem.h"
#include "Poco/Delegate.h"
#include "Poco/Util/Option.h"
#include "Poco/Util/OptionCallback.h"
#include "Poco/Util/PropertyFileConfiguration.h"
#include "FrameMgr.h"
#include "StringHelper.h"

using namespace Poco::Util;
using Poco::Logger;

namespace MyWeb {
    namespace UI {

        CUISubsystem::CUISubsystem() :
            m_pLogger(&Logger::get(name())),
            m_spConf(new LayeredConfiguration),
            m_spFrameMgr(new CFrameMgr(::GetCurrentThreadId()))
        {
        }

        CUISubsystem::~CUISubsystem()
        {

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
        }

        void CUISubsystem::uninitialize()
        {
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