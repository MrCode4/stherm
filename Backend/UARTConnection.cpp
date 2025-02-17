#include "UARTConnection.h"

#include <QDateTime>
#include "DataParser.h"
#include "LogHelper.h"

UARTConnection::UARTConnection(const QString &portName, bool debug, QObject *parent)
    : mSerial(new QSerialPort(this))
    , QObject(parent)
{
    m_debug = debug;
    #ifdef DEBUG_MODE
        // m_debug = true;
    #endif

    mSerial->setPortName(portName);
}

bool UARTConnection::startConnection(const qint32 &baudRate)
{
    bool isSuccess = mSerial->setBaudRate(baudRate) && // Set baud rate in all directions
                     mSerial->setDataBits(QSerialPort::Data8)
                     && mSerial->setParity(QSerialPort::NoParity)
                     && mSerial->setStopBits(QSerialPort::OneStop)
                     && mSerial->setFlowControl(QSerialPort::NoFlowControl); // Set non-blocking I/O

    if (!isSuccess) {
        TRACE << "Configuration failed, port name: " << mSerial->portName();
        return false;
    }

    // Check:  Use QSocketNotifier to monitor activity on a file descriptor

    bool isOpen = mSerial->isOpen();

    // Open Serial port & send beacon, ping packets to bring device into connected state
    if (!isOpen) {
        TRACE << "Serial port is not open, Port name:   " << mSerial->portName();
        TRACE << "Try to open, Port name:   " << mSerial->portName();
        isOpen = mSerial->open(QIODevice::ReadWrite);
        if (isOpen) {
            TRACE_CHECK(m_debug) << "Opened, Port name:   " << mSerial->portName();

            connect(mSerial, &QSerialPort::readyRead, this, &UARTConnection::onReadyRead);
            connect(mSerial, &QSerialPort::errorOccurred, this, &UARTConnection::onError);
        } else {
            // If open fails then return with an error
            TRACE << (QString("Can't open %1,%2 error code %3")
                          .arg(mSerial->portName())
                          .arg(mSerial->baudRate())
                          .arg(mSerial->error()));
        }
    } else {
        TRACE_CHECK(m_debug) << (QString("Already open, Port name: %1, baud rate: %2")
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
    TRACE_CHECK(m_debug) << this << data.toHex(' ').toUpper();
    auto resp = mSerial->write(data);
    TRACE_CHECK(resp == -1) << this << resp;
    return resp != -1;
}

bool UARTConnection::sendRequest(const char *data, qint64 len)
{
    return mSerial->write(data, len) != 1;
}

bool UARTConnection::sendRequest(const STHERM::SIOCommand &cmd, const STHERM::PacketType &packetType)
{
    // prepare request
    QByteArray packet = DataParser::preparePacket(cmd, packetType);

    // No data for sending
    if (packet.isEmpty()) {
        return false;
    }

    // write request
    if (!sendRequest(packet)) {
        return false;
    }

    // wait for response to be ready
// TODO this is blocking, does it sleep?  Worst case push this off to a low priority thread and come back when there is a response (note 10 bytes will take 10ms)
    if (mSerial->waitForBytesWritten(500)) {
        // read response
        // This is blocking,
        if (mSerial->waitForReadyRead()) {
            QByteArray responseData = mSerial->readAll();
            while (mSerial->waitForReadyRead())
                responseData += mSerial->readAll();

            const QString response = QString::fromUtf8(responseData);
            TRACE_CHECK(m_debug)<< response;

//            QVariantMap deserializeData = parser.deserializeData(responseData);
            return true;

        } else {
            TRACE << QString("Wait read response timeout %1")
                         .arg(QTime::currentTime().toString());
            return false;
        }
    } else {
        TRACE << QString("Wait write request timeout %1")
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
    TRACE_CHECK(m_debug) << this << dataBA.toHex(' ').toUpper();

    emit sendData(dataBA);
}

void UARTConnection::onError(QSerialPort::SerialPortError error)
{
    QString errorMessage = mSerial->errorString();
    TRACE << "Port name:   " << mSerial->portName()
          << "Error:   " << error << errorMessage;
    emit connectionError(errorMessage);
}
