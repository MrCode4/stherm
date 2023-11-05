#include "UARTConnection.h"

UARTConnection::UARTConnection()
{

}

void UARTConnection::initConnection()
{
    if (this->mSerial->isOpen()) {
        this->mSerial->close();

        return;
    }

    QObject::connect(mSerial, &QSerialPort::readyRead, this, &UARTConnection::onReadyRead);

    // Handle errors
    QObject::connect(mSerial, &QSerialPort::errorOccurred, [this](QSerialPort::SerialPortError error) {

        QString errorMessage = mSerial->errorString();
        qDebug() << Q_FUNC_INFO << __LINE__ << error << errorMessage;
        emit connectionError(errorMessage);
    });
}

bool UARTConnection::connect()
{
    // Open Serial port & send beacon, ping packets to bring device into connected state
    if (!this->mSerial->isOpen()) {
        this->mSerial->open(QIODevice::ReadWrite);
    }

    // If open fails then return with an error
    if (!this->mSerial->isOpen()) {
        qDebug() << (QString("Can't open %1,%2 error code %3")
                         .arg(this->mSerial->portName())
                         .arg(this->mSerial->baudRate())
                         .arg(this->mSerial->error()));
        return false;
    }

    return this->mSerial->isOpen();
}


bool UARTConnection::disconnect()
{
    // Close Serial port if open
    if (this->mSerial->isOpen())
    {
        this->mSerial->close();
    }

    return !this->mSerial->isOpen();
}

bool UARTConnection::isConnected()
{
    return this->mSerial->isOpen();
}

bool UARTConnection::writeData(QByteArray data) {
    mSerial->write(data);
}

void UARTConnection::onReadyRead()
{

}
