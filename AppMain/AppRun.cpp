#include "stdafx.h"
#include "AppRun.h"
#include "StringHelper.h"
#include "AppCommon.h"
#include "MyPipe.h"
#include "Poco/Process.h"
#include "Poco/PipeStream.h"
#include "../UI/MyUI.h"

using namespace Poco;
using Poco::Util::Application;
using namespace DuiLib;
using namespace MyWeb;

AppRun::AppRun(void)
{
}

AppRun::~AppRun(void)
{
}

int AppRun::RunUIProcess()
{
    // Initialize UI Subsystem.
    return Application::instance().getSubsystem<MyWeb::UI::CUISubsystem>().RunUISubsystem();
   /* std::string cmd(config().getString("application.path"));
    std::vector<std::string> args;
    args.push_back("/render=1");
    Pipe inPipe;
    Pipe outPipe;
    ProcessHandle ph = Process::launch(cmd, args, &inPipe, &outPipe, 0);
    Process::PID id = ph.id();
    PipeOutputStream outputStream(inPipe);
    PipeInputStream inputStream(outPipe);
    std::string ret;
    for (size_t i = 0; i < 50; i++)
    {
        outputStream << "UI send: " <<  " " << std::endl;
        std::getline(inputStream, ret);
        logger().trace(ret);
    }
    outputStream << "Exit" << std::endl;
    return ph.wait();*/
}

int AppRun::RunRenderProcess()
{
    std::string str;
    int i = 0;
    while (std::getline(std::cin, str) && i < 300)
    {
        std::cout << "Render reply: " << i++ << " " << std::endl;
        if (str == "Exit")
        {
            break;
        }
    }
    return 0;
}
