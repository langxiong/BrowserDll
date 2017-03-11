/******************************************************************************
 *  版权所有（C）2012-2016                                                    *
 *  保留所有权利。                                                            *
 *****************************************************************************
 *  作者 : 熊浪
 *  版本 : 1.0
 *****************************************************************************/
/*  修改记录: 
      日期       版本    修改人             修改内容
    --------------------------------------------------------------------------
******************************************************************************/
#ifndef DateTimeUtil_h_
#define DateTimeUtil_h_

#include <vector>
#include <set>

namespace MyWeb
{
    class CDateTimeUtil
    {
    public:
        static std::wstring Now();
        static std::wstring FormatLocalDateTime(const Poco::Timestamp& utcTimestamp, const std::string& strFormat);
    };
}

#endif // DateTimeUtil_h_