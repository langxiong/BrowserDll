#include "stdafx.h"
#include "DateTimeUtil.h"

#include "Poco/LocalDateTime.h"
#include "Poco/DateTimeFormatter.h"
#include "Poco/DateTimeFormat.h"
#include "Poco/DateTime.h"
#include "StringHelper.h"

namespace MyWeb
{
    std::wstring CDateTimeUtil::Now()
    {
        std::string message;
        Poco::LocalDateTime now;
        Poco::DateTimeFormatter::append(message, now, Poco::DateTimeFormat::SORTABLE_FORMAT);
        return A2W_UTF8(message);
    }

    std::wstring CDateTimeUtil::FormatLocalDateTime(const Poco::Timestamp& utcTimestamp, const std::string& strFormat)
    {
        Poco::LocalDateTime localDateTime(utcTimestamp);

        return A2W_UTF8(Poco::DateTimeFormatter::format(localDateTime, strFormat));
    }

}