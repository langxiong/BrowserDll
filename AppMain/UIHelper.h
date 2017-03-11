#pragma once

#include "Poco/logger.h"
#include "Poco/Util/LayeredConfiguration.h"
#include "UISubsystem.h"

namespace MyWeb {
    namespace UI {

        inline Poco::Logger& UILogger()
        {
            return Poco::Util::Application::instance().logger();
        }

        inline Poco::Util::LayeredConfiguration& UIConfig()
        {
            return Poco::Util::Application::instance().getSubsystem<CUISubsystem>().config();
        }

    } // UI
} // MyWeb
