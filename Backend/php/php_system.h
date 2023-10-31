#ifndef PHP_SYSTEM_H
#define PHP_SYSTEM_H

#include <QObject>
#include <QQmlEngine>

class php_system : public QObject
{
    Q_OBJECT
    //    QML_ELEMENT
public:
    explicit php_system(QObject *parent = nullptr);

    void getSystemType(void);
    void setSystemType(void);

    void getTraditionalStages(void);
    void setTraditionalStages(void);

    void getHeatPumpStages(void);
    void setHeatPumpStages(void);

    void getAccessories(void);
    void setAccessories(void);

    void getSystemDelay(void);
    void setSystemDelay(void);

    void getCoolStages(void);
    void setCoolStages(void);

    void getHeatStages(void);
    void setHeatStages(void);

    void getMode(void);
    void setMode(void);

    void getVentilator(void);


    void getHumidity(void);
    void setHumidity(void);

    void getFan(void);
    void setFan(void);

    void getMainStatic(void);
    void getMainData(void);
    void setMeasure(void);
    void setAlertAsRead(void);
    void getVacationData(void);
    void enableVacation(void);
    void setVacation(void);
    void setOff(void);
    void getAlerts(void);
    void getAlertData(void);
    void setTemperature(void);
    void setHold(void);

    void getQR(void);

    void getQRAnswer(void);

    void getUidSV(void);
    void about(void);
    void userGuide(void);
    void getVersionsIpSN(void);
    void shutDown(void);
    void rebootDevice(void);

    void requestJob(void);



signals:

};

#endif // PHP_SYSTEM_H
