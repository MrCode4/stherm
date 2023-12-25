#include "SystemAccessories.h"

SystemAccessories::SystemAccessories(QSObjectCpp *parent) :
    QSObjectCpp{parent}
{
    setSystemAccessories(AppSpecCPP::Humidifier, AppSpecCPP::T1PWRD);
}

void SystemAccessories::setSystemAccessories(AppSpecCPP::AccessoriesType accessoriesType, AppSpecCPP::AccessoriesWireType wireType) {
    mAccessoriesType  = accessoriesType;
    mIsWireTypeNone   = (wireType == AppSpecCPP::None);

    // Saved last selection
    if (!mIsWireTypeNone)
        mAccessoriesWireType = wireType;

    emit accessoriesChanged();

}
