#include "StdAfx.h"
#include "AddrHelper.h"
#include "AppCommon.h"

#pragma comment(lib, "ws2_32.lib")

bool FormatAddress(SOCKADDR_IN6* pSockAddr, std::wstring& addr)
{
    if (!pSockAddr)
    {
        return false;
    }

    DWORD cb = sizeof(struct sockaddr_in6);
    WCHAR buffer[64] = { 0 };
    DWORD dwBufferSize = 64;
    int ret = WSAAddressToStringW((LPSOCKADDR) pSockAddr, cb, NULL, buffer, &dwBufferSize);
    if (ret != 0)
    {
        poco_error_f1(logger(), "WSAAddressToStringW err[%d]", ret);
        return false;
    }

    addr = buffer;
    return true;
}
