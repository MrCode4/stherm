#pragma once

#include <QObject>
#include <QQuickItem>

/*! ***********************************************************************************************
 * Base class for application specifications
 * All enums, consts and app specs should be defined here
 * A qml extension is also defined (AppSpec.qml)
 * ************************************************************************************************/

// TODO : Add control modes as Appspec properties

class AppSpecCPP : public QObject
{
    Q_OBJECT
    QML_ELEMENT
public:
    explicit AppSpecCPP(QObject *parent = nullptr);
    static AppSpecCPP *instance();

    // Useage in QML: ex. AppSpecCPP.Cooling
    enum SystemMode {
        Cooling = 0,
        Heating,
        Auto,
        Vacation,
        Off,
        Emergency
    };

    Q_ENUM(SystemMode)

signals:

private:
    //! Singleton instance
    static AppSpecCPP *mInstance;

};
