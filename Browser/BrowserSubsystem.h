/******************************************************************************
*  版权所有（C）2010-2015，xlfy.tech                      *
*  保留所有权利。                                                            *
******************************************************************************
*  作者 : 熊浪
*  版本 : 1.0
*****************************************************************************/
/*  修改记录:
日期       版本    修改人             修改内容
--------------------------------------------------------------------------
******************************************************************************/
#ifndef CameraSubsystem_h__
#define CameraSubsystem_h__ 1

#include "Poco/Logger.h"
#include "Poco/AutoPtr.h"
#include "Poco/Util/Subsystem.h"
#include "Poco/Util/LayeredConfiguration.h"
#include "Poco/BasicEvent.h"
#include <memory>
#include <windows.h>
#include <dshow.h>

namespace Poco
{
    namespace Util
    {
        class XMLConfiguration;
    }
}

namespace MyWeb {
     namespace Camera {

         class CCameraPlayCap;
        class MYCAMERA_API CCameraSubsystem :
            public    Poco::Util::Subsystem
        {
        public:
            CCameraSubsystem();

            virtual ~CCameraSubsystem();

        public:

            Poco::Logger& logger() const;

            Poco::Util::LayeredConfiguration& config() const;

            IBaseFilter* GetCaptureDeviceFilter(int nIndex);
            void ReloadSourceFilter();

            static void SetOwner(HWND hOwner);
			static HWND GetTrackbarHwnd();
			static HWND GetCameraHwnd();
            static int MyHook(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, int& was_processed);
            /** Subsystem interface
            */
        protected:
            virtual const char* name() const;
            /// Returns the name of the subsystem.
            /// Must be implemented by subclasses.

            virtual void initialize(Poco::Util::Application& app);
            /// Initializes the subsystem.

            virtual void uninitialize();
            /// Uninitializes the subsystem.

            virtual void reinitialize(Poco::Util::Application& app);
            /// Re-initializes the subsystem.
            ///
            /// The default implementation just calls
            /// uninitialize() followed by initialize().
            /// Actual implementations might want to use a
            /// less radical and possibly more performant 
            /// approach.

            virtual void defineOptions(Poco::Util::OptionSet& options);
            /// Called before the Application's command line processing begins.
            /// If a subsystem wants to support command line arguments,
            /// it must override this method.
            /// The default implementation does not define any options.
            ///
            /// To effectively handle options, a subsystem should either bind
            /// the option to a configuration property or specify a callback
            /// to handle the option.

        private:

            void handleTest(const std::string& name, const std::string& value);

        private:

			static HWND sm_hTrackBar;
			static HWND sm_hCamera;
            static HWND sm_hOwner;

            Poco::Logger* m_pLogger;

            Poco::AutoPtr<Poco::Util::LayeredConfiguration> m_spConf;

            std::string m_configPath;

            std::vector<IBaseFilter*> m_pCaptureDeviceFilters;
            std::map<std::wstring, IMoniker*> m_pMonikers;
            std::shared_ptr<CCameraPlayCap> m_spCameraPlayCap;
        };

        inline Poco::Logger& CCameraSubsystem::logger() const
        {
            poco_check_ptr(m_pLogger);
            return *m_pLogger;
        }
    } // Camera
} // MyWeb
#endif // CameraSubsystem_h__