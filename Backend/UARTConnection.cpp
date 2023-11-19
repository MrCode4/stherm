#include "UARTConnection.h"

#include <QDateTime>
#include "DataParser.h"

UARTConnection::UARTConnection(const QString &portName, const qint32 &baundRate, QObject *parent)
    : QObject(parent)
    , mSerial(new QSerialPort(this))
{
    mSerial->setPortName(portName);
    bool isSuccess = mSerial->setBaudRate(baundRate) && // Set baud rate in all directions
                     mSerial->setDataBits(QSerialPort::Data8)
                     && mSerial->setParity(QSerialPort::NoParity)
                     && mSerial->setStopBits(QSerialPort::OneStop)
                     && mSerial->setFlowControl(QSerialPort::NoFlowControl); // Set non-blocking I/O

    if (!isSuccess) {
        qDebug() << Q_FUNC_INFO << __LINE__ << "Configuration failed, port name: " << portName;
    }
}

bool UARTConnection::startConnection()
{
    // Check:  Use QSocketNotifier to monitor activity on a file descriptor

    bool isOpen = mSerial->isOpen();

    // Open Serial port & send beacon, ping packets to bring device into connected state
    if (!isOpen) {
        qDebug() << Q_FUNC_INFO << __LINE__ << "Serial port is not open, Port name:   " << mSerial->portName();
        qDebug() << Q_FUNC_INFO << __LINE__ << "Try to open, Port name:   " << mSerial->portName();
        isOpen = mSerial->open(QIODevice::ReadWrite);
        if (isOpen) {
            qDebug() << Q_FUNC_INFO << __LINE__ << "Opened, Port name:   " << mSerial->portName();

            connect(mSerial, &QSerialPort::readyRead, this, &UARTConnection::onReadyRead);
            connect(mSerial, &QSerialPort::errorOccurred, this, &UARTConnection::onError);
        } else {
            // If open fails then return with an error
            qDebug() << (QString("Can't open %1,%2 error code %3")
                             .arg(mSerial->portName())
                             .arg(mSerial->baudRate())
                             .arg(mSerial->error()));
        }
    } else {
        qDebug() << (QString("Already open, Port name: %1, baud rate: %2")
                         .arg(mSerial->portName())
                         .arg(mSerial->baudRate()));
    }

    return isOpen;
}

bool UARTConnection::disconnectDevice()
{
    // Close Serial port if open
    if (mSerial->isOpen()) {
        mSerial->close();
    }
    disconnect(mSerial, &QSerialPort::readyRead, this, &UARTConnection::onReadyRead);
    disconnect(mSerial, &QSerialPort::errorOccurred, this, &UARTConnection::onError);

    return !mSerial->isOpen();
}

bool UARTConnection::isConnected()
{
    return mSerial->isOpen();
}

bool UARTConnection::sendRequest(QByteArray data) {
    return mSerial->write(data) != -1;
}

bool UARTConnection::sendRequest(const char *data, qint64 len)
{
    return mSerial->write(data, len) != 1;
}

bool UARTConnection::sendRequest(const STHERM::SIOCommand &cmd, const STHERM::PacketType &packetType)
{
    DataParser parser;

    // prepare request
    QByteArray packet = parser.preparePacket(cmd, packetType);

    // write request
    sendRequest(packet);

    // wait for response to be ready
// TODO this is blocking, does it sleep?  Worst case push this off to a low priority thread and come back when there is a response (note 10 bytes will take 10ms)
    if (mSerial->waitForBytesWritten()) {
        // read response
        if (mSerial->waitForReadyRead()) {
            QByteArray responseData = mSerial->readAll();
            while (mSerial->waitForReadyRead())
                responseData += mSerial->readAll();

            const QString response = QString::fromUtf8(responseData);
            qDebug() << Q_FUNC_INFO << __LINE__ << response;

//            QVariantMap deserializeData = parser.deserializeData(responseData);
            return true;

        } else {
            qDebug() << Q_FUNC_INFO << __LINE__ << QString("Wait read response timeout %1")
                                                       .arg(QTime::currentTime().toString());
            return false;
        }
    } else {
        qDebug() << Q_FUNC_INFO << __LINE__ << QString("Wait write request timeout %1")
                                                   .arg(QTime::currentTime().toString());
        return false;
    }

    return true;
}

bool UARTConnection::seek(qint64 pos)
{
    return mSerial->seek(0);
}

void UARTConnection::onReadyRead()
{
    // Handle data
    QByteArray dataBA = mSerial->readAll();
    qDebug() << Q_FUNC_INFO << __LINE__ << dataBA.toHex(' ').toUpper();;

    emit sendData(dataBA);
}

void UARTConnection::onError(QSerialPort::SerialPortError error)
{
    QString errorMessage = mSerial->errorString();
    qDebug() << Q_FUNC_INFO << __LINE__ << "Port name:   " << mSerial->portName()
             << "Error:   " << error << errorMessage;
    emit connectionError(errorMessage);
}
