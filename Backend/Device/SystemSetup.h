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
    QML_ELEMENT

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
    Q_PROPERTY(bool isAUXAuto  MEMBER isAUXAuto NOTIFY isAUXAutoChanged FINAL)
    Q_PROPERTY(AppSpecCPP::DualFuelManualHeating dualFuelManualHeating  MEMBER dualFuelManualHeating NOTIFY dualFuelManualHeatingChanged FINAL)

    Q_PROPERTY(double dualFuelThreshod  MEMBER dualFuelThreshod NOTIFY dualFuelThreshodChanged FINAL)

    Q_PROPERTY(double emergencyTemperatureDiffrence  MEMBER emergencyTemperatureDiffrence NOTIFY emergencyTemperatureDiffrenceChanged FINAL)
    Q_PROPERTY(int    emergencyMinimumTime           MEMBER emergencyMinimumTime  NOTIFY emergencyMinimumTimeChanged FINAL)
    Q_PROPERTY(AppSpecCPP::emergencyControlType    emergencyControlType           MEMBER emergencyControlType  NOTIFY emergencyControlTypeChanged FINAL)

public:
    explicit SystemSetup(QSObjectCpp *parent = nullptr);

public:
    AppSpecCPP::SystemType systemType;

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

    bool   isAUXAuto;
    AppSpecCPP::DualFuelManualHeating   dualFuelManualHeating;

    //! Emergency properties
    //! In minutes
    int emergencyMinimumTime;

    AppSpecCPP::emergencyControlType emergencyControlType;

    //! In celcius
    double emergencyTemperatureDiffrence;

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
    void emergencyTemperatureDiffrenceChanged();
    void emergencyMinimumTimeChanged();
    void emergencyControlTypeChanged();
    void isAUXAutoChanged();
    void dualFuelManualHeatingChanged();

};
