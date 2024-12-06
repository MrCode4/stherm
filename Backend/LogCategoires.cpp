#include "LogCategoires.h"

Q_LOGGING_CATEGORY(SchemeLogCategory, "SchemeLog")
const QLoggingCategory &LogCategoires::schemeCategory() {return SchemeLogCategory();}

Q_LOGGING_CATEGORY(IOControllerLogCategory, "IOControllerLog")
const QLoggingCategory &LogCategoires::ioControllerCategory() {return IOControllerLogCategory();}

Q_LOGGING_CATEGORY(DeviceControllerLogCategory, "DeviceControllerLog")
const QLoggingCategory &LogCategoires::deviceControllerCategory() {return DeviceControllerLogCategory();}

Q_LOGGING_CATEGORY(PerfTestLogCategory, "PerfTestServiceLog")
const QLoggingCategory &LogCategoires::perfTestCategory() {return PerfTestLogCategory();}
