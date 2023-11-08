#include "UARTConnection.h"
#include "qeventloop.h"

#include <QJsonObject>
#include <QJsonDocument>
#include <QTimer>


UARTConnection::UARTConnection(QObject *parent, bool isTi) :
    QThread(parent), mIsTi(isTi),
    mSerial(new QSerialPort(this))
{

    mDataParser = new DataParser();
}

void UARTConnection::initConnection(const QString& portName, const qint32& baundRate)
{

    if (mSerial->isOpen()) {
        mSerial->close();
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
        qDebug() << Q_FUNC_INFO << __LINE__ <<"Port name:   "<<mSerial->portName() <<
            "Error:   "<< error << errorMessage;
        emit connectionError(errorMessage);
    });
}

bool UARTConnection::connect()
{
    // Check:  Use QSocketNotifier to monitor activity on a file descriptor

    // Open Serial port & send beacon, ping packets to bring device into connected state
    if (!mSerial->isOpen()) {
        qDebug() << Q_FUNC_INFO << __LINE__ << "Serial port is not open, Port name:   " << mSerial->portName();
        qDebug() << Q_FUNC_INFO << __LINE__ << "Try to open, Port name:   " << mSerial->portName();
        mSerial->open(QIODevice::ReadWrite);
        if (!mSerial->isOpen()) {
            // If open fails then return with an error
            qDebug() << (QString("Can't open %1,%2 error code %3")
                             .arg(mSerial->portName())
                             .arg(mSerial->baudRate())
                             .arg(mSerial->error()));
            return false;
        } else {
            qDebug() << Q_FUNC_INFO << __LINE__ << "Opened, Port name:   " << mSerial->portName();
        }
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
    // write request
    QByteArray packet = mDataParser->preparePacket(cmd, packetType);
    sendRequest(packet);

//        QEventLoop loop;
//        QTimer timer;
//        timer.setSingleShot(true);
//        QObject::connect(&timer, &QTimer::timeout, &loop, &QEventLoop::quit);
//        QObject::connect(this, &UARTConnection::responseReceived, &loop, &QEventLoop::quit);
//        QObject::connect(this, &UARTConnection::connectionError, &loop, &QEventLoop::quit);

//        timer.start(30000);
//        loop.exec();

//        if (timer.isActive()) {
//            timer.stop();
//        }  else {
//            qDebug() << "Response timeout";
//        }

    if (mSerial->waitForBytesWritten()) {

        // read response
        if (mSerial->waitForReadyRead()) {
            QByteArray responseData = mSerial->readAll();
            while (mSerial->waitForReadyRead())
                responseData += mSerial->readAll();

            const QString response = QString::fromUtf8(responseData);
            qDebug() << Q_FUNC_INFO << __LINE__ << response;
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

void UARTConnection::onReadyRead()
{
    // Handle data
    QByteArray dataBA = mSerial->readAll();

    qDebug() << Q_FUNC_INFO << __LINE__ << dataBA;

    QVariantMap deserializeData = mDataParser->deserializeData(dataBA, mIsTi);

    emit sendData(deserializeData);
}

void UARTConnection::run()
{
    m_mutex.lock();
    //    m_cond.wait(&m_mutex);

    qDebug() << Q_FUNC_INFO << __LINE__;

    // As raw data
    // todo: change to serialize data
    while (!true) {
        QVariantMap mainData = {{"temp", QVariant(18)}, {"hum", QVariant(30.24)}};
        QJsonObject obj;
        obj.insert("temp", 10);
        obj.insert("hum", 30.24);
        qDebug() << Q_FUNC_INFO << __LINE__ << QJsonDocument(obj).toJson();
        emit sendData(obj.toVariantMap());
    }

    m_mutex.unlock();
}
