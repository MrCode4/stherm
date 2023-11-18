#include "GpioHandler.h"

GpioHandler::GpioHandler(int gpio_pin, QObject *parent)
    : QObject{parent}
{
    // Construct the file path using QString
    filePath = QString(SW_VAL_PATH).arg(gpio_pin);

    // Open the file in non-blocking mode
    if (openFile()) {
        mError = QString();
        qDebug() << "File opened successfully";

        // Create a notifier to monitor file descriptor for readability
        notifier = new QSocketNotifier(file.handle(), QSocketNotifier::Read, this);

        // Connect the signal/slot for file descriptor events
        connect(notifier, &QSocketNotifier::activated, this, &GpioHandler::handleGpioEvent);

        // Start monitoring for events
        notifier->setEnabled(true);

    } else {
        mError = file.errorString();
        qDebug() << "Failed to open file:" << mError;
    }
}

GpioHandler::~GpioHandler()
{
    closeFile();
}

bool GpioHandler::openFile()
{
    file.setFileName(filePath);
    return file.open(QIODevice::ReadWrite | QIODevice::Unbuffered);
}

void GpioHandler::closeFile()
{
    if (file.isOpen()) {
        file.close();
    }
}

qint64 GpioHandler::readFile(char *data, qint64 maxSize)
{
    return file.read(data, maxSize);
}

qint64 GpioHandler::writeFile(const char *data, qint64 maxSize)
{
    return file.write(data, maxSize);
}

void GpioHandler::seek(int position)
{
    file.seek(position);
}

void GpioHandler::handleGpioEvent()
{
    static QByteArray Data = "";

    if (file.bytesAvailable() == 0) {
        return;
    }

    auto data = file.readAll();

    this->seek(SEEK_SET);

    char buffer[256];
    qint64 bytesRead = readFile(buffer, sizeof(buffer));

    if (bytesRead > 0) {
        QByteArray bufferBA = buffer;
        if (bufferBA != Data) {
            qDebug() << "GPIO Value: " << buffer << data;
            Data = bufferBA;
            emit readyRead(QByteArray(buffer));
        }
    }
}

QString GpioHandler::error() const
{
    return mError;
}

void GpioHandler::setError(const QString &newError)
{
    if (mError == newError)
        return;
    mError = newError;
    emit errorChanged();
}
