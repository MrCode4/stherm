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
    Q_PROPERTY(AppSpecCPP::DualFuelManualHeating dualFuelHeatingModeDefault  MEMBER dualFuelHeatingModeDefault NOTIFY dualFuelHeatingModeDefaultChanged FINAL)

    Q_PROPERTY(double dualFuelThreshod  MEMBER dualFuelThreshod NOTIFY dualFuelThreshodChanged FINAL)

    Q_PROPERTY(int    emergencyMinimumTime           MEMBER emergencyMinimumTime  NOTIFY emergencyMinimumTimeChanged FINAL)
    Q_PROPERTY(bool   useAuxiliaryParallelHeatPump    MEMBER useAuxiliaryParallelHeatPump     NOTIFY useAuxiliaryParallelHeatPumpChanged FINAL)
    Q_PROPERTY(bool   driveAux1AndETogether           MEMBER driveAux1AndETogether            NOTIFY driveAux1AndETogetherChanged FINAL)
    Q_PROPERTY(bool   enableEmergencyModeForAuxStages MEMBER enableEmergencyModeForAuxStages  NOTIFY enableEmergencyModeForAuxStagesChanged FINAL)
    Q_PROPERTY(bool   auxiliaryHeating                MEMBER auxiliaryHeating                 NOTIFY auxiliaryHeatingChanged FINAL)

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
    AppSpecCPP::DualFuelManualHeating   dualFuelHeatingModeDefault;

    //! Emergency properties
    //! In minutes
    int emergencyMinimumTime;

    //! Auxiliary properties
    //! Would you like to turn on auxiliary heating in parallel with your heat pump when it's cold outside and the heat pump alone can't keep up?
    bool useAuxiliaryParallelHeatPump;

    //! Do you want to drive W1(Aux 1) and W3(E) terminals together?
    bool driveAux1AndETogether;

    //! Do you want to drive all stages of auxiliary as Emergency in Auxiliary mode?
    bool enableEmergencyModeForAuxStages;

    bool auxiliaryHeating;

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
    void emergencyMinimumTimeChanged();
    void isAUXAutoChanged();
    void dualFuelManualHeatingChanged();
    void dualFuelHeatingModeDefaultChanged();

    void useAuxiliaryParallelHeatPumpChanged();
    void driveAux1AndETogetherChanged();
    void enableEmergencyModeForAuxStagesChanged();
    void auxiliaryHeatingChanged();

};
