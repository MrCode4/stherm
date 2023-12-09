#include "GpioHandler.h"

#include <fcntl.h>
#include <stdio.h>
#ifdef __unix__
#include <poll.h>
#include <unistd.h>
#endif
#include <sys/types.h>
#include <signal.h>

#include <QElapsedTimer>
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

void *nrf_uart_thrd(void *a) {
    auto parent = static_cast<GpioHandler *>(a);

#ifdef __unix__
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

#endif
    return nullptr;
}

bool GpioHandler::startConnection()
{
    // Open the file in non-blocking mode
    if (openFile()) {
        mError = QString();
        qInfo() << "File opened successfully";

#ifdef __unix__
        // Create a notifier to monitor file descriptor for readability
        pthread_create(&poll_thread, nullptr, &nrf_uart_thrd, this);
#endif

    } else {
        mError = "Failed to open file";
        qWarning() << "Failed to open file:" << mError;
    }

    return !hasError();
}

bool GpioHandler::openFile()
{
#ifdef __unix__
    _fd = open(filePath.toStdString().c_str(), O_RDONLY | O_NONBLOCK);;

    return _fd != -1;
#else
    return false;
#endif
}

void GpioHandler::closeFile()
{
#ifdef __unix__
    if (_fd != -1) {
        close(_fd);
        _fd = -1;
    }
    pthread_join(poll_thread, nullptr);
#endif
}


int GpioHandler::fd() const
{
#ifdef __unix__
    return _fd;
#else
    return -1;
#endif
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
