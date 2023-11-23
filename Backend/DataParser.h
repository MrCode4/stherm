#pragma once

#include <QThread>

#include "UtilityHelper.h"

/*! ***********************************************************************************************
 * This class oversees the management of the TI and UART threads and
 *  handles the processing of associated signals
 * (Convert serialized data into its original data types and
 *  structure it for utilization within the user interface.).
 * ************************************************************************************************/

class DataParser : public QObject
{
    Q_OBJECT

public:
    DataParser(QObject *parent = nullptr);

    //! Prepare packet
    static QByteArray preparePacket(STHERM::SIOCommand cmd,
                             STHERM::PacketType packetType = STHERM::PacketType::UARTPacket,
                             QVariantList data = QVariantList());

    //! Deserialize TI and NRF data and send dataReay signal
    //! In OLD Code: ti: Line 494-515 / NRF: Line 1068-1089
    static STHERM::SIOPacket deserializeData(const QByteArray &serializeData);

signals:
    void alert(STHERM::AlertLevel alertLevel,
               STHERM::AlertTypes alertType,
               QString alertMessage = QString());

private:
    STHERM::AQ_TH_PR_vals AQTHPRFromBytes(const QByteArray &bytes);
};

