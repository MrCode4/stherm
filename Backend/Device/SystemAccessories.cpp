#include "SystemAccessories.h"
#include "LogHelper.h"

SystemAccessories::SystemAccessories(QSObjectCpp *parent) :
    QSObjectCpp{parent}
{
    mLastAccessoriesType = AppSpecCPP::Humidifier;
    mLastAccessoriesWireType = AppSpecCPP::None;

    setSystemAccessories(AppSpecCPP::Humidifier, AppSpecCPP::None);
}

void SystemAccessories::setSystemAccessories(AppSpecCPP::AccessoriesType accessoriesType, AppSpecCPP::AccessoriesWireType wireType) {

    mLastAccessoriesType = mAccessoriesType;
    mLastAccessoriesWireType = mAccessoriesWireType;

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

void SystemAccessories::backToLastState() {
    setSystemAccessories(mLastAccessoriesType, mLastAccessoriesWireType);
}
