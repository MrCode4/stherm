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

    // TODO: remove
    Q_PROPERTY(double temprature MEMBER temprature NOTIFY tempratureChanged FINAL)

    Q_PROPERTY(double maximumTemprature MEMBER maximumTemprature NOTIFY maximumTempratureChanged FINAL)
    Q_PROPERTY(double minimumTemprature MEMBER minimumTemprature NOTIFY minimumTempratureChanged FINAL)

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

    //! ScheduleCPP temprature: This is always in Celsius
    double temprature;

    double minimumTemprature;
    double maximumTemprature;

    double humidity;

    //! Whether this ScheduleCPP is active or not
    bool enable;

    bool active;

    //! Return the effective temperature based on the system mode
    double effectiveTemperature(AppSpecCPP::SystemMode systemMode);

signals:
    void nameChanged();
    void typeChanged();
    void startTimeChanged();
    void endTimeChanged();
    void dataSourceChanged();
    void repeatsChanged();
    void tempratureChanged();
    void maximumTempratureChanged();
    void minimumTempratureChanged();
    void humidityChanged();
    void enableChanged();
    void activeChanged();
};
