#pragma once

#include <WinDef.h>

namespace MyWeb
{
    class MyOleInitialize
    {

    public:

        /** 默认构造函数，初始化OLE
        */
        MyOleInitialize();

        /** 默认析构函数，解初始化OLE
        */
        ~MyOleInitialize();

    private:

        /** 初始化结果
        */
        HRESULT m_hr;
    };
}