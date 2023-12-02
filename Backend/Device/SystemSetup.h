#pragma once

#include <QObject>
#include <QQmlEngine>

#include "UtilityHelper.h"
#include "QtQuickStream/Core/QSObjectCpp.h"

class SystemSetup : public QSObjectCpp
{
    Q_OBJECT

    Q_PROPERTY(int systemType            MEMBER  systemType          NOTIFY systemTypeChanged FINAL)
    Q_PROPERTY(int traditionalHeatStage  MEMBER traditionalHeatStage NOTIFY traditionalHeatStageChanged FINAL)
    Q_PROPERTY(int traditionalCoolStage  MEMBER traditionalCoolStage NOTIFY traditionalCoolStageChanged FINAL)

    Q_PROPERTY(int heatPumpStage     MEMBER heatPumpStage   NOTIFY heatPumpStageChanged FINAL)
    Q_PROPERTY(int heatPumpOBState   MEMBER heatPumpOBState NOTIFY heatPumpOBStateChanged FINAL)

    Q_PROPERTY(int coolStage     MEMBER coolStage  NOTIFY coolStageChanged FINAL)
    Q_PROPERTY(int heatStage     MEMBER heatStage  NOTIFY heatStageChanged FINAL)

    Q_PROPERTY(bool heatPumpEmergency MEMBER heatPumpEmergency NOTIFY heatPumpEmergencyChanged FINAL)

    QML_ELEMENT

public:
    explicit SystemSetup(QSObjectCpp *parent = nullptr);

public:
    STHERM::SystemType systemType;

    int traditionalHeatStage;
    int traditionalCoolStage;
    int heatPumpStage;
    int heatPumpOBState;
    int coolStage;
    int heatStage;
    int heatPumpEmergency;

signals:
    void systemTypeChanged();
    void traditionalHeatStageChanged();
    void traditionalCoolStageChanged();
    void heatPumpStageChanged();
    void heatPumpOBStateChanged();
    void coolStageChanged();
    void heatStageChanged();
    void heatPumpEmergencyChanged();

private:

};
