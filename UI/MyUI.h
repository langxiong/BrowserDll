// The following ifdef block is the standard way of creating macros which make exporting 
// from a DLL simpler. All files within this DLL are compiled with the MyUI_EXPORTS
// symbol defined on the command line. This symbol should not be defined on any project
// that uses this DLL. This way any other project whose source files include this file see 
// MyUI_API functions as being imported from a DLL, whereas this DLL sees symbols
// defined with this macro as being exported.
#ifdef MYUI_EXPORTS
#define MYUI_API __declspec(dllexport)
#else
#define MYUI_API __declspec(dllimport)
#endif

#include <tchar.h>
#include <assert.h>
#include <iostream>
#include <string>
#include "UISubsystem.h"
#include "../DuiLib/UIlib.h"

#ifdef _DEBUG
#pragma comment(lib, "DuiLib_ud.lib")
#else
#pragma comment(lib, "DuiLib_u.lib")
#endif