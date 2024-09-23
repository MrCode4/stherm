#include "PerfTestService.h"

#include <QDateTime>
#include <QTimer>

PerfTestService::PerfTestService(QObject *parent)
    : RestApiExecutor{parent}
{
    if (QDateTime::currentDateTime().secsTo(QDateTime(QDate::currentDate(), QTime::fromString("10:00:00"))) >= 0) {
        scheduleNextCheck(QDate::currentDate());
    }
    else {
        scheduleNextCheck(QDate::currentDate().addDays(1));
    }
}

void PerfTestService::scheduleNextCheck(const QDate& scheduleDate)
{
    auto secondsToNext10AM = QDateTime::currentDateTime().msecsTo(QDateTime(scheduleDate, QTime::fromString("10:00:00")));
}
