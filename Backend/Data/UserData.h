#pragma once

#include <QObject>
#include <QQmlEngine>

#include "Property.h"

class UserData : public QObject
{
    Q_OBJECT
    QML_ELEMENT

    PROPERTY_PUBLIC(QString, name)
    PROPERTY_PUBLIC(QString, email)

public:
    explicit UserData(QObject *parent = nullptr) : QObject(parent) {}
};
