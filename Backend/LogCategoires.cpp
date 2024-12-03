#include "LogCategoires.h"

Q_LOGGING_CATEGORY(SchemeLogCategory, "SchemeLog")
const QLoggingCategory &LogCategoires::schemeCategory() {return SchemeLogCategory();}

Q_LOGGING_CATEGORY(PerfTestLogCategory, "PerfTestServiceLog")
const QLoggingCategory &LogCategoires::perfTestCategory() {return PerfTestLogCategory();}
