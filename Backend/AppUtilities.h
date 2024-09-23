#pragma once

#include <QObject>
#include <QQmlEngine>

class AppUtilities : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    QML_SINGLETON
public:
    explicit AppUtilities(QObject *parent = nullptr) : QObject(parent) {}

    //! Generating a Random Four-Digit Number
    Q_INVOKABLE QString generateRandomPassword();
    Q_INVOKABLE QString decodeLockPassword(QString pass);
};
