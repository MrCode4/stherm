#include "SystemAccessories.h"
#include "LogHelper.h"

SystemAccessories::SystemAccessories(QSObjectCpp *parent) :
    QSObjectCpp{parent}
{
    setSystemAccessories(AppSpecCPP::Humidifier, AppSpecCPP::None);
}

void SystemAccessories::setSystemAccessories(AppSpecCPP::AccessoriesType accessoriesType, AppSpecCPP::AccessoriesWireType wireType) {

    if (mAccessoriesType     != accessoriesType ||
        mAccessoriesWireType != wireType) {

        mAccessoriesType  = accessoriesType;
        mAccessoriesWireType = wireType;

        emit accessoriesChanged();
    }
}

AppSpecCPP::AccessoriesType SystemAccessories::getAccessoriesType() const
{
    return mAccessoriesType;
}

AppSpecCPP::AccessoriesWireType SystemAccessories::getAccessoriesWireType() const
{
    return mAccessoriesWireType;
}
