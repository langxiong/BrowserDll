#pragma once
#include "MyString.h"
#include <functional>

struct TExternalItem
{
    MyString m_name;

    std::function<void(DISPPARAMS* params, VARIANT* result)>  m_pFnRun;
};
