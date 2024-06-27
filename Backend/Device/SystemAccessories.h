#pragma once

#include <QObject>
#include <QQmlEngine>

#include "AppSpecCPP.h"
#include "QtQuickStream/Core/QSObjectCpp.h"

/*
 *The SystemAccessories stores/manages accessories such as humidifiers and dehumidifiers.
*/
class SystemAccessories : public QSObjectCpp
{
    Q_OBJECT

    Q_PROPERTY(AppSpecCPP::AccessoriesType     accessoriesType     MEMBER mAccessoriesType     NOTIFY accessoriesChanged FINAL)
    Q_PROPERTY(AppSpecCPP::AccessoriesWireType accessoriesWireType MEMBER mAccessoriesWireType NOTIFY accessoriesChanged FINAL)

    QML_ELEMENT
public:
    SystemAccessories(QSObjectCpp *parent = nullptr);


    Q_INVOKABLE void setSystemAccessories(AppSpecCPP::AccessoriesType accessoriesType,
                                          AppSpecCPP::AccessoriesWireType wireType);

    AppSpecCPP::AccessoriesType getAccessoriesType() const;

    AppSpecCPP::AccessoriesWireType getAccessoriesWireType() const;

    Q_INVOKABLE void backToLastState();

signals:
    void accessoriesChanged();

private:

    // The humidifier and dehumidifier cannot be activated simultaneously.
    AppSpecCPP::AccessoriesType     mAccessoriesType;
    AppSpecCPP::AccessoriesWireType mAccessoriesWireType;

    // Last accessories
    AppSpecCPP::AccessoriesType     mLastAccessoriesType;
    AppSpecCPP::AccessoriesWireType mLastAccessoriesWireType;
};
