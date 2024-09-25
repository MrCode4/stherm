#include "PerfTestService.h"

#include <QRandomGenerator64>
#include <QTimer>

#define TEN_AM QTime::fromString("11:45:00")

PerfTestService::PerfTestService(QObject *parent)
    : QObject{parent}
{
    if (QDateTime::currentDateTime().secsTo(QDateTime(QDate::currentDate(), QTime::fromString("11:45:00"))) >= 0) {

        auto time = QTime::currentTime() > TEN_AM ? QTime::currentTime() : TEN_AM;
        scheduleNextCheck(QDateTime(QDate::currentDate(), time));
    }
    else {
        scheduleNextCheck(QDateTime(QDate::currentDate().addDays(1), TEN_AM));
    }
}

void PerfTestService::scheduleNextCheck(const QDateTime& scheduleDate)
{
    auto secondsToNext10AM = QDateTime::currentDateTime().msecsTo(scheduleDate);

    QTimer::singleShot(10000, this, &PerfTestService::checkTestEligibility);
}


void PerfTestService::checkTestEligibility()
{

}
