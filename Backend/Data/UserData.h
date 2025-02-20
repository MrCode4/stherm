#pragma once

#include <QQmlEngine>

#include "QtQuickStream/Core/QSObjectCpp.h"
#include "Property.h"

class UserData : public QSObjectCpp
{
    Q_OBJECT
    QML_ELEMENT

    PROPERTY_PUB(QString, name)
    PROPERTY_PUB(QString, email)

public:
    explicit UserData(QObject *parent = nullptr) : QSObjectCpp(parent) {}
};
