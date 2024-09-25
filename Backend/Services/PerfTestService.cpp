#include "PerfTestService.h"
#include "Config.h"
#include "DeviceInfo.h"
#include "LogHelper.h"

#include <QRandomGenerator64>
#include <QTimer>

#define TEN_AM QTime::fromString("10:00:00")

PerfTestService::PerfTestService(QObject *parent)
    : DevApiExecutor{parent}
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
    auto secondsToNextCheck = QDateTime::currentDateTime().msecsTo(scheduleDate);
    QTimer::singleShot(secondsToNextCheck, this, &PerfTestService::checkTestEligibility);
}


void PerfTestService::checkTestEligibility()
{
    if (Device->serialNumber().isEmpty()) {
        qWarning() << "Sn is not ready! can not get user-data!";
        return;
    }

    auto callback = [this](QNetworkReply *, const QByteArray &, QJsonObject &data) {
        if (data.isEmpty()) {
            TRACE << "Received perf-test-schedule-data corrupted";
        }
        else {
            //emit perfTestScheduleFetched(data.value("status").toBool(), data.value("cooling").toBool());
        }

        fetchingPerfTestSchedule(false);
    };

    fetchingPerfTestSchedule(true);
    callGetApi(API_SERVER_BASE_URL + QString("api/sync/perftest/schedule?sn=%0").arg(Device->serialNumber()), callback);
}
