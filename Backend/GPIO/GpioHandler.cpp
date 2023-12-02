#include "GpioHandler.h"

#include <QTimer>

GpioHandler::GpioHandler(int gpio_pin, QObject *parent)
    : QObject{parent}
{
    // Construct the file path using QString
    filePath = QString(SW_VAL_PATH).arg(gpio_pin);
}

GpioHandler::~GpioHandler()
{
    closeFile();
}

bool GpioHandler::startConnection()
{
    // Open the file in non-blocking mode
    if (openFile()) {
        mError = QString();
        qDebug() << "File opened successfully";

        // Create a notifier to monitor file descriptor for readability
        notifier = new QSocketNotifier(QSocketNotifier::Read, this);

        // Connect the signal/slot for file descriptor events
        connect(notifier, &QSocketNotifier::activated, this, &GpioHandler::handleGpioEvent);

        // Start monitoring for events
        notifier->setSocket(file.handle());
        notifier->setEnabled(true);

    } else {
        mError = file.errorString();
        qDebug() << "Failed to open file:" << mError;
    }

    return !hasError();
}

bool GpioHandler::openFile()
{
    file.setFileName(filePath);
    return file.open(QIODevice::ReadOnly | QIODevice::Unbuffered);
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

void GpioHandler::handleGpioEvent(QSocketDescriptor socket, QSocketNotifier::Type activationEvent)
{
    char buffer[256];

    auto ba = file.bytesAvailable();
    if (ba == 0) {
        return;
    }

    //! disabling notifier for 5 millisecondsto lower cpu usage
    notifier->setEnabled(false);
    QTimer::singleShot(5, this, [=] () {
        notifier->setSocket(file.handle());
        notifier->setEnabled(true);
    });

    this->seek(SEEK_SET);
    qint64 bytesRead = readFile(buffer, sizeof(buffer));

    if (bytesRead > 0) {
        QByteArray bufferBA = buffer;
        if (bufferBA != dataLastRead) {
            dataLastRead = bufferBA;
            emit readyRead(bufferBA);
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
