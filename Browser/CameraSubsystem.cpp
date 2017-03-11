#include "StdAfx.h"
#include "CameraSubsystem.h"
#include "Poco/Delegate.h"
#include "Poco/Util/Option.h"
#include "Poco/Util/OptionCallback.h"
#include "Poco/Util/PropertyFileConfiguration.h"
#include "StringHelper.h"

#include <opencv2/highgui/highgui_c.h>  // Video write
#include "../UI/UIDefine.h"

using namespace Poco::Util;
using Poco::Logger;

namespace MyWeb {
    namespace Camera {

        CCameraSubsystem::CCameraSubsystem() :
            m_pLogger(&Logger::get(name())),
            m_spConf(new LayeredConfiguration)
        {
        }

        CCameraSubsystem::~CCameraSubsystem()
        {

        }

        const char* CCameraSubsystem::name() const
        {
            return "CCameraSubsystem";
        }

        void CCameraSubsystem::initialize(Poco::Util::Application& app)
        {
            std::string appDir = app.config().getString("application.dir");
            m_spConf->add(new Poco::Util::PropertyFileConfiguration(), 0, true, false);
            cvSetPreprocessFuncWin32(&MyHook);
        }

        void CCameraSubsystem::uninitialize()
        {
            if (!m_configPath.empty())
            {
            }
            for (auto v: m_pCaptureDeviceFilters)
            {
                v->Release();
            }
            for (auto v : m_pMonikers)
            {
                v.second->Release();
            }
        }

        void CCameraSubsystem::reinitialize(Poco::Util::Application& app)
        {
        }

        void CCameraSubsystem::defineOptions(Poco::Util::OptionSet& options)
        {
            options.addOption(
                Option("include-dir", "I", "specify an include search path")
                .required(false)
                .repeatable(true)
                .argument("path")
                .callback(OptionCallback<CCameraSubsystem>(this, &CCameraSubsystem::handleTest)));
        }

        void CCameraSubsystem::handleTest(const std::string& name, const std::string& value)
        {
            poco_trace_f2(logger(), "name[%s] value[%s]", name, value);
        }
        IBaseFilter* CCameraSubsystem::GetCaptureDeviceFilter(int nIndex)
        {
            if (nIndex >= m_pCaptureDeviceFilters.size())
            {
                return nullptr;
            }
            return m_pCaptureDeviceFilters[nIndex];
        }

        void CCameraSubsystem::ReloadSourceFilter()
        {
        }


        Poco::Util::LayeredConfiguration& CCameraSubsystem::config() const
        {
            return *const_cast<Poco::Util::LayeredConfiguration*>(m_spConf.get());
        }

        int CCameraSubsystem::MyHook(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, int& was_processed)
        {
            if (uMsg == WM_LBUTTONDOWN || uMsg == WM_RBUTTONDOWN)
            {
                ::PostMessage(sm_hOwner, MyWeb::UI::WM_USER_CV_BUTTON_CLICK, 0, 0);
            }
            if (uMsg == WM_CREATE)
            {
                HWND hParent = (HWND)::GetWindowLong(hWnd, GWL_HWNDPARENT);
                if (hParent)
                {
                    ::SetWindowLong(hParent, GWL_STYLE, WS_VISIBLE | WS_CHILD | WS_CLIPSIBLINGS | WS_CLIPCHILDREN);
                    ::SetParent(hParent, sm_hOwner);

                    ::PostMessage(sm_hOwner, MyWeb::UI::WM_USER_CV_MAINFRAME_CREATED, (WPARAM)hParent, (LPARAM)hWnd);
                    sm_hTrackBar = hParent;
					sm_hCamera = hWnd;
                }
                return 0;
            }

            return 0;
        }

        void CCameraSubsystem::SetOwner(HWND hOwner)
        {
            sm_hOwner = hOwner;
        }

        HWND CCameraSubsystem::GetTrackbarHwnd()
        {
            return sm_hTrackBar;
        }

		HWND CCameraSubsystem::GetCameraHwnd()
		{
			return sm_hCamera;
		}

        HWND CCameraSubsystem::sm_hTrackBar;

		HWND CCameraSubsystem::sm_hCamera;

		HWND CCameraSubsystem::sm_hOwner;

		

    } // Camera
} // MyWeb