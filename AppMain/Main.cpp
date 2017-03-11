#include "StdAfx.h"

#include "Poco/AutoPtr.h"
#include "MyApp.h"
#include <windows.h>
#include <shellapi.h>
#include <memory>

#if 0

int _tmain(int argc, TCHAR** argv)
{
    Poco::AutoPtr<MyApp> pApp = new MyApp;    
    try                                    
    {                                    
        pApp->init(argc, argv);            
    }                                    
    catch (Poco::Exception& exc)        
    {                                    
        pApp->logger().log(exc);        
        return Poco::Util::Application::EXIT_CONFIG;
    }                                    
    return pApp->run();        
}

#else

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    int nArgs = 0;
    std::shared_ptr<LPWSTR> spArgList(::CommandLineToArgvW(GetCommandLineW(), &nArgs), LocalFree);
    if (!spArgList)
        return 0;

    Poco::AutoPtr<MyApp> pApp = new MyApp;
    try
    {
        pApp->init(nArgs, spArgList.get());
    }
    catch (Poco::Exception& exc)
    {
        pApp->logger().log(exc);
        return Poco::Util::Application::EXIT_CONFIG;
    }
    return pApp->run();
}

#endif