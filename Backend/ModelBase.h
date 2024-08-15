#pragma once

#include <QObject>

class ModelBase : public QObject
{
    Q_OBJECT
public:
    explicit ModelBase(QObject *parent = nullptr);

signals:
};
