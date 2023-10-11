#ifndef APPSETTINGSCPP_H
#define APPSETTINGSCPP_H

#include <QObject>
#include <QQuickItem>
#include <QSettings>

/*! ***********************************************************************************************
 * This class is used for saving and loading app settings.
 * QSettings is used for this purpose.
 *
 * ************************************************************************************************/

class AppSettingsCPP : public QObject
{
    Q_OBJECT
    QML_ELEMENT
public:
    explicit AppSettingsCPP(QObject *parent = nullptr);
    static AppSettingsCPP *instance();

public slots:
    void setValue(const QString &propName, QVariant value);
    QVariant value(const QString &propName);

private:
    //! QSettings instance
    QSettings mSettings;

    //! Singleton instance
    static AppSettingsCPP *mInstance;
};

#endif // APPSETTINGSCPP_H
