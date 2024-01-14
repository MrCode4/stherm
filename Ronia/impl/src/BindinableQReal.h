#ifndef BINDINABLEQREAL_H
#define BINDINABLEQREAL_H

#include <QObject>
#include <QQmlEngine>
#include <QtQuick/QQuickItem>

/*!
 * \brief The BindableQReal class holds the data for range sliders handle
 */
class BindableQReal : public QObject
{
    Q_OBJECT
    Q_PROPERTY(qreal        value   READ getValue WRITE setValue NOTIFY valueChanged FINAL)
    QML_ELEMENT

public:
    explicit BindableQReal(QObject *parent = nullptr);

    qreal getValue() const;
    Q_INVOKABLE void setValue(qreal newValue);

signals:
    void valueChanged();

private:
    qreal       mValue = 0;
};

#endif // BINDINABLEQREAL_H
