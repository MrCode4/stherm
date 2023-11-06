#include "UARTConnection.h"

#include <QJsonObject>
#include <QJsonDocument>


UARTConnection::UARTConnection(QObject *parent) :
    QThread(parent),
    mSerial(new QSerialPort(this))
{

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

bool UARTConnection::writeData(QByteArray data) {
    return mSerial->write(data);
}

void UARTConnection::onReadyRead()
{
    // Handle data
    QByteArray dataBA = mSerial->readAll();

    sendData(dataBA);
}

void UARTConnection::run()
{
    m_mutex.lock();
//    m_cond.wait(&m_mutex);

    // As raw data
    // todo: change to serialize data
    QVariantMap mainData = {{"temp", QVariant(18)}, {"hum", QVariant(30.24)}};
    QJsonObject obj;
    obj.insert("temp", 10);
    obj.insert("hum", 30.24);
    emit sendData(QJsonDocument(obj).toJson());

    m_mutex.unlock();
}
