#pragma once

#include "LogHelper.h"
#include <QLoggingCategory>

struct LogCategoires
{
    static const QLoggingCategory &schemeCategory();
    static const QLoggingCategory &perfTestCategory();
};

#define SCHEME_LOG_CHECK(check) TRACE_CATEGORY_CHECK(LogCategoires::schemeCategory(), check)
#define SCHEME_LOG SCHEME_LOG_CHECK(true)

#define PERF_LOG_CHECK(check) TRACE_CATEGORY_CHECK(LogCategoires::perfTestCategory(), check)
#define PERF_LOG PERF_LOG_CHECK(true)



