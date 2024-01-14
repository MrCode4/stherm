#pragma once

#include <QObject>
#include <QQmlEngine>
#include <QtQuick/QQuickItem>

/*!
 * \brief The RangeSliderHandleData class
 */
class RangeSliderHandleData : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QQuickItem* handle READ handle WRITE setHandle NOTIFY handleChanged FINAL)
    Q_PROPERTY(qreal value READ getValue WRITE setValue NOTIFY valueChanged FINAL)
    Q_PROPERTY(bool hovered READ getHovered WRITE setHovered NOTIFY hoveredChanged FINAL)
    Q_PROPERTY(bool pressed READ getPressed WRITE setPressed NOTIFY pressedChanged FINAL)
    Q_PROPERTY(qreal position READ getPosition NOTIFY positionChanged FINAL)
    Q_PROPERTY(qreal visualPosition READ getVisualPosition NOTIFY visualPositionChanged FINAL)
    Q_PROPERTY(qreal implicitHandleHeight READ getImplicitHandleHeight NOTIFY implicitHandleHeightChanged FINAL)
    Q_PROPERTY(qreal implicitHandleWidth READ getImplicitHandleWidth NOTIFY implicitHandleWidthChanged FINAL)

    QML_ELEMENT
public:
    explicit RangeSliderHandleData(QObject *parent = nullptr);

    qreal getValue() const;
    Q_INVOKABLE void setValue(qreal newValue);

    bool getHovered() const;
    Q_INVOKABLE void setHovered(bool newHovered);

    bool getPressed() const;
    Q_INVOKABLE void setPressed(bool newPressed);

    qreal getPosition() const;
    Q_INVOKABLE void setPosition(qreal newPosition);

    qreal getVisualPosition() const;
    Q_INVOKABLE void setVisualPosition(qreal newVisualPosition);

    qreal getImplicitHandleHeight() const;
    Q_INVOKABLE void setImplicitHandleHeight(qreal newImplicitHandleHeight);

    qreal getImplicitHandleWidth() const;
    Q_INVOKABLE void setImplicitHandleWidth(qreal newImplicitHandleWidth);

    QQuickItem* handle() const;
    Q_INVOKABLE void setHandle(QQuickItem* newHandle);

    qreal getMaxValue() const;
    Q_INVOKABLE void setMaxValue(qreal newMaxValue);

    qreal getMinValue() const;
    Q_INVOKABLE void setMinValue(qreal newMinValue);

signals:
    void valueChanged();
    void hoveredChanged();
    void pressedChanged();
    void positionChanged();
    void visualPositionChanged();
    void implicitHandleHeightChanged();
    void implicitHandleWidthChanged();
    void handleChanged();

private:
    QQuickItem* mHandle;
    qreal mValue = 0;
    /*! Max and min values are used to restrict values of handles
     *  Default values should be minus infinity and positive infinity, otherwise initialization of
     *  value won't work properly.
     */
    qreal mMaxValue = INT_MAX;
    qreal mMinValue = INT_MIN;
    bool mHovered = false;
    bool mPressed = false;
    qreal mPosition = 0;
    qreal mVisualPosition = 0.;
    qreal mImplicitHandleHeight = 0;
    qreal mImplicitHandleWidth = 0;
};
