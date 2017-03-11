#pragma once

#include <string>

/** Unicode版本的String
*/
typedef std::wstring MyStringW;

/** Ansi版本的String
*/
typedef std::string  MyStringA;

/** String 类型宏定义
*/
#ifdef UNICODE

typedef MyStringW  MyString;
#else
#error "Please consider unicode string for better compaitible"
typedef MyStringA  MyString;

#endif // UNICODE


