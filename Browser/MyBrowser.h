// The following ifdef block is the standard way of creating macros which make exporting 
// from a DLL simpler. All files within this DLL are compiled with the MyUI_EXPORTS
// symbol defined on the command line. This symbol should not be defined on any project
// that uses this DLL. This way any other project whose source files include this file see 
// MyUI_API functions as being imported from a DLL, whereas this DLL sees symbols
// defined with this macro as being exported.
#ifdef BROWSER_EXPORTS
#define MYBROWSER_API __declspec(dllexport)
#else
#define MYBROWSER_API __declspec(dllimport)
#endif

#include <tchar.h>
#include <assert.h>
#include <iostream>
#include <string>
#include <atlbase.h>
#include <windows.h>

#include "BrowserDefine.h"
