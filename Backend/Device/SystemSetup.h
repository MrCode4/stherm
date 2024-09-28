#pragma once

#include <QObject>
#include <QQmlEngine>

#include "AppSpecCPP.h"
#include "SystemAccessories.h"
#include "UtilityHelper.h"
#include "QtQuickStream/Core/QSObjectCpp.h"

class SystemSetup : public QSObjectCpp
{
    Q_OBJECT

    Q_PROPERTY(SystemAccessories* systemAccessories MEMBER systemAccessories NOTIFY systemAccessoriesChanged FINAL)

    Q_PROPERTY(AppSpecCPP::SystemType systemType            MEMBER  systemType          NOTIFY systemTypeChanged FINAL)

    Q_PROPERTY(int heatPumpOBState   MEMBER heatPumpOBState NOTIFY heatPumpOBStateChanged FINAL)

    Q_PROPERTY(int coolStage     MEMBER coolStage  NOTIFY coolStageChanged FINAL)
    Q_PROPERTY(int heatStage     MEMBER heatStage  NOTIFY heatStageChanged FINAL)

    Q_PROPERTY(int systemRunDelay     MEMBER systemRunDelay  NOTIFY systemRunDelayChanged FINAL)

    Q_PROPERTY(AppSpecCPP::SystemMode systemMode     MEMBER systemMode  NOTIFY systemModeChanged FINAL)

    Q_PROPERTY(bool heatPumpEmergency MEMBER heatPumpEmergency NOTIFY heatPumpEmergencyChanged FINAL)

    Q_PROPERTY(bool isVacation        MEMBER isVacation        NOTIFY isVacationChanged FINAL)
    Q_PROPERTY(bool _isSystemShutoff  MEMBER _mIsSystemShutoff NOTIFY isSystemShutoffChanged FINAL)

    Q_PROPERTY(double dualFuelThreshod  MEMBER dualFuelThreshod NOTIFY dualFuelThreshodChanged FINAL)

    QML_ELEMENT

public:
    explicit SystemSetup(QSObjectCpp *parent = nullptr);

public:
    AppSpecCPP::SystemType systemType;

    void updateMode(AppSpecCPP::SystemMode mode);

    // 0: cooling, 1: heating
    int heatPumpOBState;

    int coolStage;
    int heatStage;

    // System run delay
    int systemRunDelay;

    AppSpecCPP::SystemMode systemMode;

    bool _mIsSystemShutoff;

    bool heatPumpEmergency;

    SystemAccessories* systemAccessories;

    bool isVacation;

    //! This is usually the outdoor temperature
    //! at which the heat pump becomes less efficient than the furnace.
    //! Celsius
    double dualFuelThreshod;

signals:
    void systemTypeChanged();
    void heatPumpOBStateChanged();
    void coolStageChanged();
    void heatStageChanged();
    void systemRunDelayChanged();
    void systemModeChanged();
    void heatPumpEmergencyChanged();
    void systemAccessoriesChanged();
    void isVacationChanged();
    void isSystemShutoffChanged();
    void dualFuelThreshodChanged();

};
