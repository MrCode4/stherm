#pragma once

#include <QObject>

#include "AppSpecCPP.h"

#include "QtQuickStream/Core/QSObjectCpp.h"

class ScheduleCPP : public QSObjectCpp
{
    Q_OBJECT

    Q_PROPERTY(QString name       MEMBER name       NOTIFY nameChanged FINAL)
    Q_PROPERTY(QString startTime  MEMBER startTime  NOTIFY startTimeChanged FINAL)
    Q_PROPERTY(QString endTime    MEMBER endTime    NOTIFY endTimeChanged FINAL)
    Q_PROPERTY(QString dataSource MEMBER dataSource NOTIFY dataSourceChanged FINAL)

    Q_PROPERTY(QString repeats MEMBER repeats NOTIFY repeatsChanged FINAL)

    Q_PROPERTY(int type        MEMBER type       NOTIFY typeChanged FINAL)
    Q_PROPERTY(AppSpecCPP::SystemMode systemMode  MEMBER systemMode NOTIFY systemModeChanged FINAL)

    Q_PROPERTY(double maximumTemperature MEMBER maximumTemperature NOTIFY maximumTemperatureChanged FINAL)
    Q_PROPERTY(double minimumTemperature MEMBER minimumTemperature NOTIFY minimumTemperatureChanged FINAL)

    Q_PROPERTY(double humidity   MEMBER humidity   NOTIFY humidityChanged FINAL)

    Q_PROPERTY(bool enable  MEMBER enable  NOTIFY enableChanged FINAL)
    Q_PROPERTY(bool active  MEMBER active  NOTIFY activeChanged FINAL)

    QML_ELEMENT

public:
    explicit ScheduleCPP(QSObjectCpp *parent = nullptr);


    QString name;

    //! ScheduleCPP start time
    QString startTime;

    //! ScheduleCPP end time
    QString endTime;

    QString dataSource;

    //! Ex: "Mo,Fr"
    QString repeats;

    int type;

    //! Determine when a schedule was created or if the current schedule is enabled.
    AppSpecCPP::SystemMode systemMode;

    double temprature;

    //! Temperature: This is always in Celsius
    double minimumTemperature;
    double maximumTemperature;

    double humidity;

    //! Whether this ScheduleCPP is active or not
    bool enable;

    bool active;

signals:
    void nameChanged();
    void typeChanged();
    void systemModeChanged();
    void startTimeChanged();
    void endTimeChanged();
    void dataSourceChanged();
    void repeatsChanged();
    void tempratureChanged();
    void maximumTemperatureChanged();
    void minimumTemperatureChanged();
    void humidityChanged();
    void enableChanged();
    void activeChanged();
};
