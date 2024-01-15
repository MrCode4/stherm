#include "BindinableQReal.h"

BindableQReal::BindableQReal(QObject *parent)
    : QObject{parent}
{}

qreal BindableQReal::getValue() const
{
    return mValue;
}

void BindableQReal::setValue(qreal newValue)
{
    if (qFuzzyCompare(mValue, newValue)) {
        return;
    }

    mValue = newValue;
    emit valueChanged();
}
