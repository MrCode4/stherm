#pragma once

#include <QObject>

#include "QtQuickStream/Core/QSObjectCpp.h"

class ScheduleCPP : public QSObjectCpp
{
    Q_OBJECT

    Q_PROPERTY(QString name       MEMBER name       NOTIFY nameChanged FINAL)
    Q_PROPERTY(QString type       MEMBER type       NOTIFY typeChanged FINAL)
    Q_PROPERTY(QString startTime  MEMBER startTime  NOTIFY startTimeChanged FINAL)
    Q_PROPERTY(QString endTime    MEMBER endTime    NOTIFY endTimeChanged FINAL)
    Q_PROPERTY(QString dataSource MEMBER dataSource NOTIFY dataSourceChanged FINAL)

    Q_PROPERTY(QString repeats MEMBER repeats NOTIFY repeatsChanged FINAL)

    Q_PROPERTY(double temprature MEMBER temprature NOTIFY tempratureChanged FINAL)
    Q_PROPERTY(double humidity   MEMBER humidity   NOTIFY humidityChanged FINAL)

    Q_PROPERTY(bool enable  MEMBER enable  NOTIFY enableChanged FINAL)
    Q_PROPERTY(bool _active MEMBER _active NOTIFY activeChanged FINAL)

    QML_ELEMENT

public:
    explicit ScheduleCPP(QSObjectCpp *parent = nullptr);


    QString name;
    QString type;

    //! ScheduleCPP start time
    QString startTime;

    //! ScheduleCPP end time
    QString endTime;

    QString dataSource;

    //! Ex: "Mo,Fr"
    QString repeats;

    //! ScheduleCPP temprature: This is always in Celsius
    double temprature;

    double humidity;

    //! Whether this ScheduleCPP is active or not
    bool enable;

    //! _active is established through a schedule controller when a schedule is in progress.
    bool _active;

signals:
    void nameChanged();
    void typeChanged();
    void startTimeChanged();
    void endTimeChanged();
    void dataSourceChanged();
    void repeatsChanged();
    void tempratureChanged();
    void humidityChanged();
    void enableChanged();
    void activeChanged();


};
