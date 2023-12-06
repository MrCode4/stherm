#include "GpioHandler.h"

#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <poll.h>
#include <signal.h>
#include <unistd.h>

#include <QElapsedTimer>
#include <QTimer>

#include "LogHelper.h"

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

void *nrf_uart_thrd(void *a) {
    auto parent = static_cast<GpioHandler *>(a);

    struct pollfd *fds = new pollfd();

    fds->fd = parent->fd();
    fds->events = POLLPRI | POLLERR;

    char buffer[256];
    while (parent->fd() != -1) {
        int ready = poll(fds, 1, -1);  // wait indefinitely for events

        if (ready > 0) {
            if (fds->revents & POLLPRI) {
                lseek(fds->fd, 0, SEEK_SET);
                qint64 bytesRead = read(fds->fd, buffer, sizeof(buffer));
                fds->revents  = 0;

                if (bytesRead > 0) {
                    QByteArray bufferBA(buffer, bytesRead);

                    emit parent->readyRead(bufferBA);
                }
            }
        }
    }

    return nullptr;
}

bool GpioHandler::startConnection()
{
    // Open the file in non-blocking mode
    if (openFile()) {
        mError = QString();
        qInfo() << "File opened successfully";

        // Create a notifier to monitor file descriptor for readability
        pthread_create(&poll_thread, nullptr, &nrf_uart_thrd, this);

    } else {
        mError = "Failed to open file";
        qWarning() << "Failed to open file:" << mError;
    }

    return !hasError();
}

bool GpioHandler::openFile()
{
    _fd = open(filePath.toStdString().c_str(), O_RDONLY | O_NONBLOCK);;

    return _fd != -1;
}

void GpioHandler::closeFile()
{
    if (_fd != -1) {
        close(_fd);
        _fd = -1;
    }
    pthread_join(poll_thread, nullptr);
}


int GpioHandler::fd() const
{
    return _fd;
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
