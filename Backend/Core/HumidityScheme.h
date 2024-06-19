#pragma once

#include "BaseScheme.h"

/*! ***********************************************************************************************
 * This class controls the humidity.
 * ************************************************************************************************/

class HumidityScheme : public BaseScheme
{
    Q_OBJECT

public:
    explicit HumidityScheme(DeviceAPI *deviceAPI, QSharedPointer<SchemeDataProvider> schemeDataProvider,
                            QObject *parent = nullptr);

    ~HumidityScheme();

    void setSystemSetup() override;

    //! Update vacation data
    void setVacation() override;

    void setRequestedHumidity(const double& setPointHumidity);

    //! Stop the Humidity control
    void stop();

    //! Restart the worker thread
    void restartWork() override;

protected:
    void run() override;

private:

    //! Vacation loop
    void VacationLoop();

    //! Normal loop
    void normalLoop();

    //! Update relays but not sent to device.
    //! None sets the humidity wirings to off.
    void updateAccessoriesRelays(AppSpecCPP::AccessoriesWireType accessoriesWireType = AppSpecCPP::None);

    //! Check the humidity range from vacation
    bool checkVacationRange();

    //! Return the effective humidity
    double effectiveHumidity();

    //! To monitor data change: current Humidity, set Humidity, mode
    //! use low values for timeout in exit cases as it might had abrupt changes previously
    int waitLoop(int timeout = 10000, AppSpecCPP::ChangeTypes overrideModes = AppSpecCPP::ChangeType::ctAll) override;

    void OffLoop();


    void sendRelays(bool forceSend = false);


private:

    AppSpecCPP::AccessoriesType     mAccessoriesType;
    AppSpecCPP::AccessoriesWireType mAccessoriesWireType;

    //! Vacation properites (Percentage)
    double mVacationMinimumHumidity;
    double mVacationMaximumHumidity;

};

