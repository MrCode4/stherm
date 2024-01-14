#include "RangeSliderHandleData.h"

RangeSliderHandleData::RangeSliderHandleData(QObject *parent)
    : QObject{parent}
{}

qreal RangeSliderHandleData::getValue() const
{
    return mValue;
}

void RangeSliderHandleData::setValue(qreal newValue)
{
    if (qFuzzyCompare(mValue, newValue) || (newValue > mMaxValue || newValue < mMinValue)) {
        return;
    }

    mValue = newValue;
    emit valueChanged();
}

bool RangeSliderHandleData::getHovered() const
{
    return mHovered;
}

void RangeSliderHandleData::setHovered(bool newHovered)
{
    if (mHovered == newHovered) {
        return;
    }

    mHovered = newHovered;
    emit hoveredChanged();
}

bool RangeSliderHandleData::getPressed() const
{
    return mPressed;
}

void RangeSliderHandleData::setPressed(bool newPressed)
{
    if (mPressed == newPressed) {
        return;
    }

    mPressed = newPressed;
    emit pressedChanged();
}

qreal RangeSliderHandleData::getPosition() const
{
    return mPosition;
}

void RangeSliderHandleData::setPosition(qreal newPosition)
{
    if (qFuzzyCompare(mPosition, newPosition)) {
        return;
    }

    mPosition = newPosition;
    emit positionChanged();
}

qreal RangeSliderHandleData::getVisualPosition() const
{
    return mVisualPosition;
}

void RangeSliderHandleData::setVisualPosition(qreal newVisualPosition)
{
    if (qFuzzyCompare(mVisualPosition, newVisualPosition)) {
        return;
    }

    mVisualPosition = newVisualPosition;
    emit visualPositionChanged();
}

qreal RangeSliderHandleData::getImplicitHandleHeight() const
{
    return mImplicitHandleHeight;
}

void RangeSliderHandleData::setImplicitHandleHeight(qreal newImplicitHandleHeight)
{
    if (qFuzzyCompare(mImplicitHandleHeight, newImplicitHandleHeight)) {
        return;
    }

    mImplicitHandleHeight = newImplicitHandleHeight;
    emit implicitHandleHeightChanged();
}

qreal RangeSliderHandleData::getImplicitHandleWidth() const
{
    return mImplicitHandleWidth;
}

void RangeSliderHandleData::setImplicitHandleWidth(qreal newImplicitHandleWidth)
{
    if (qFuzzyCompare(mImplicitHandleWidth, newImplicitHandleWidth)) {
        return;
    }

    mImplicitHandleWidth = newImplicitHandleWidth;
    emit implicitHandleWidthChanged();
}

QQuickItem* RangeSliderHandleData::handle() const
{
    return mHandle;
}

void RangeSliderHandleData::setHandle(QQuickItem* newHandle)
{
    if (mHandle == newHandle) {
        return;
    }

    mHandle = newHandle;
    emit handleChanged();
}

qreal RangeSliderHandleData::getMaxValue() const
{
    return mMaxValue;
}

void RangeSliderHandleData::setMaxValue(qreal newMaxValue)
{
    mMaxValue = newMaxValue;
}

qreal RangeSliderHandleData::getMinValue() const
{
    return mMinValue;
}

void RangeSliderHandleData::setMinValue(qreal newMinValue)
{
    mMinValue = newMinValue;
}
