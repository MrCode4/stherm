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
    QByteArray preparePacket(STHERM::SIOCommand cmd,
                     STHERM::PacketType packetType = STHERM::PacketType::UARTPacket);

    //! Deserialize TI data and send dataReay signal
    QVariantMap deserializeData(const QByteArray &serializeData, const bool &isTi = false);

	//! Check AQ_TH_PR_vals values to detect related alerts
    void checkAlert(const STHERM::AQ_TH_PR_vals &values);

signals:
    void alert(STHERM::AlertLevel alertLevel,
               STHERM::AlertTypes alertType,
               QString alertMessage = QString());

private:
    //! Deserialize TI data and send dataReay signal
    QVariantMap deserializeUARTData(const QByteArray &serializeData);

    //! Deserialize TI data and send dataReay signal
    QVariantMap deserializeTiData(const QByteArray &serializeData);

    STHERM::AQ_TH_PR_vals AQTHPRFromBytes(const QByteArray &bytes);

};

