#pragma once

#include <QObject>
#include <QQmlEngine>


class AccessCodeGenerator : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    QML_SINGLETON

public:
    explicit AccessCodeGenerator(QObject *parent = nullptr);

    //! Generating a Random Four-Digit Number
    Q_INVOKABLE QString generateRandomPassword();

    Q_INVOKABLE QString decodeLockPassword(QString pass);

signals:

};
