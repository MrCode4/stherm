#pragma once

#include <QQmlEngine>

#include "Property.h"

class UserData : public QObject
{
    Q_OBJECT
    QML_ELEMENT

    PROPERTY_PUB(QString, name)
    PROPERTY_PUB(QString, email)

public:
    explicit UserData(QObject *parent = nullptr) : QObject(parent) {}
};
