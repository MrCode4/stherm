#pragma once

#include <QCoreApplication>
#include <QDebug>

#define SW_VAL_PATH "/sys/class/gpio/gpio%0/value"

class GpioHandler : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString error READ error WRITE setError NOTIFY errorChanged FINAL)

public:
    explicit GpioHandler(int gpio_pin, QObject *parent = nullptr);

    ~GpioHandler();

    bool startConnection();

    bool openFile();

    void closeFile();

    QString error() const;
    void setError(const QString &newError);

    bool hasError() const {
        return !mError.isEmpty();
    }

    int fd() const;

    bool stopped() const;

    bool waitingReadProcessed() const;
    void readProcessed();

    void beforeReadyRead();

signals:
    void readyRead(QByteArray buffer);

    void errorChanged();

private:
    QString filePath;
    QString mError;
    bool _stopped = false;
    bool _waitingReadProcessed = false;

#ifdef __unix__
    int _fd;
    pthread_t poll_thread;
#endif
};

