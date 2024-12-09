#pragma once

#include "LogHelper.h"
#include <QLoggingCategory>

struct LogCategoires
{
    static const QLoggingCategory &schemeCategory();
    static const QLoggingCategory &ioControllerCategory();
    static const QLoggingCategory &deviceControllerCategory();
    static const QLoggingCategory &perfTestCategory();
};

#define LOG_CHECK_SCHEME(check) TRACE_CATEGORY_CHECK(LogCategoires::schemeCategory(), check)
#define SCHEME_LOG LOG_CHECK_SCHEME(true)

#define LOG_CHECK_IO(check) TRACE_CATEGORY_CHECK(LogCategoires::ioControllerCategory(), check)
#define LOG_IO LOG_CHECK_IO(true)

#define LOG_CHECK_DC(check) TRACE_CATEGORY_CHECK(LogCategoires::deviceControllerCategory(), check)
#define LOG_DC LOG_CHECK_DC(true)

#define LOG_CHECK_PERF(check) TRACE_CATEGORY_CHECK(LogCategoires::perfTestCategory(), check)
#define LOG_PERF LOG_CHECK_PERF(true)



