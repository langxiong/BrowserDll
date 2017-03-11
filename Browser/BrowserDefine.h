#pragma once

#include "MyString.h"
#include <functional>

/** JAVASCRIPT 通信项
*/
struct TExternalItem
{
	/** 名称
	*/
	MyString m_name;

	/** 函数
	*/
	std::function<void(DISPPARAMS* params, VARIANT* result)>  m_pFnRun;
};
