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

#define SCHEME_LOG_CHECK(check) TRACE_CATEGORY_CHECK(LogCategoires::schemeCategory(), check)
#define LOG_SCHEME SCHEME_LOG_CHECK(true)

#define LOG_CHECK_IO(check) TRACE_CATEGORY_CHECK(LogCategoires::ioControllerCategory(), check)
#define LOG_IO LOG_CHECK_IO(true)

#define LOG_CHECK_DC(check) TRACE_CATEGORY_CHECK(LogCategoires::deviceControllerCategory(), check)
#define LOG_DC LOG_CHECK_DC(true)

#define PERF_LOG_CHECK(check) TRACE_CATEGORY_CHECK(LogCategoires::perfTestCategory(), check)
#define PERF_LOG PERF_LOG_CHECK(true)



