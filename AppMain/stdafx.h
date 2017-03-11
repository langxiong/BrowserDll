// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#include "targetver.h"

#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
// Windows Header Files:
#define POCO_NO_UNWINDOWS 
#include <windows.h>
#include <memory>
#include <thread>
#include <vector>
#include <map>
#include <ppltasks.h>
#include "../UI/MyUI.h"

#include "Poco/Foundation.h"
#include "Poco/Util/Application.h"

// TODO: reference additional headers your program requires here
