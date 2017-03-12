#pragma once

#include <functional>
#include <WinDef.h>

template<typename Signature>
std::function<Signature> cast_to_function(FARPROC f)
{
    return std::function<Signature>(reinterpret_cast<Signature*>(f));
}



