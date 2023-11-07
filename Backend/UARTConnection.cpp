#include "UARTConnection.h"

#include <QJsonObject>
#include <QJsonDocument>
#include <QTimer>


UARTConnection::UARTConnection(QObject *parent) :
    QThread(parent),
    mSerial(new QSerialPort(this))
{

    mDataParser = new DataParser();
}

void UARTConnection::initConnection(const QString& portName, const qint32& baundRate)
{

    if (mSerial->isOpen()) {
        mSerial->close();

        return;
    }


    mSerial->setPortName(portName);
    bool issuccess = mSerial->setBaudRate(baundRate) && // Set bound rate in all directions
                     mSerial->setDataBits(QSerialPort::Data8) &&
                     mSerial->setParity(QSerialPort::NoParity) &&
                     mSerial->setStopBits(QSerialPort::OneStop) &&
                     mSerial->setFlowControl(QSerialPort::NoFlowControl); // Set non-blocking I/O

    if (!issuccess)
        qDebug() << Q_FUNC_INFO << __LINE__ << "Configuration failed, port name: " << portName;

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
    // Check:  Use QSocketNotifier to monitor activity on a file descriptor

    // Open Serial port & send beacon, ping packets to bring device into connected state
    if (!mSerial->isOpen()) {
        mSerial->open(QIODevice::ReadWrite);
    }

    // If open fails then return with an error
    if (!mSerial->isOpen()) {
        qDebug() << (QString("Can't open %1,%2 error code %3")
                         .arg(mSerial->portName())
                         .arg(mSerial->baudRate())
                         .arg(mSerial->error()));
        return false;
    }

    bool isOpen = mSerial->isOpen();
    if (isOpen)
        run();

    return isOpen;
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

bool UARTConnection::sendRequest(QByteArray data) {
    return mSerial->write(data);
}

bool UARTConnection::sendRequest(const char *data, qint64 len)
{
        return mSerial->write(data, len);
}

bool UARTConnection::sendRequest(const STHERM::SIOCommand &cmd, const STHERM::PacketType &packetType)
{
    QByteArray packet = mDataParser->preparePacket(cmd, packetType);

    return sendRequest(packet);
}

void UARTConnection::onReadyRead()
{
    // Handle data
    QByteArray dataBA = mSerial->readAll();

    qDebug() << Q_FUNC_INFO << __LINE__ << dataBA;

    QVariantMap deserializeData = mDataParser->deserializeMainData(dataBA);

    emit sendData(deserializeData);
}

void UARTConnection::run()
{
    m_mutex.lock();
//    m_cond.wait(&m_mutex);

    qDebug() << Q_FUNC_INFO << __LINE__;

    // As raw data
    // todo: change to serialize data
        while (true) {
            QVariantMap mainData = {{"temp", QVariant(18)}, {"hum", QVariant(30.24)}};
            QJsonObject obj;
            obj.insert("temp", 10);
            obj.insert("hum", 30.24);
            qDebug() << Q_FUNC_INFO << __LINE__ << QJsonDocument(obj).toJson();
            emit sendData(obj.toVariantMap());
        }

    m_mutex.unlock();
}
