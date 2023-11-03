#ifndef CURRENTSTAGE_H
#define CURRENTSTAGE_H

#include <QObject>

#include "nuveTypes.h"

class CurrentStage : public QObject
{
    Q_OBJECT
public:
    uint32_t            mode;
    uint32_t            stage;
    timestamp_t         timestamp;
    uint32_t            blink_mode;
    timestamp_t         s2offtime;


    explicit CurrentStage(QObject *parent = nullptr);

signals:

};

#endif // CURRENTSTAGE_H
