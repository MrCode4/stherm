#include "SystemAccessories.h"

SystemAccessories::SystemAccessories(QSObjectCpp *parent) :
    QSObjectCpp{parent}
{
    setSystemAccessories(AppSpecCPP::Humidifier, AppSpecCPP::None);
}

void SystemAccessories::setSystemAccessories(AppSpecCPP::AccessoriesType accessoriesType, AppSpecCPP::AccessoriesWireType wireType) {
    mAccessoriesType  = accessoriesType;

    mAccessoriesWireType = wireType;

    emit accessoriesChanged();

}
