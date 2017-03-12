#include "stdafx.h"
#include "MyOleInitialize.h"

namespace MyWeb
{
    MyOleInitialize::MyOleInitialize():
        m_hr(::CoInitialize(NULL))
    {
    }

    MyOleInitialize::~MyOleInitialize()
    {
        ::CoUninitialize();
    }
}