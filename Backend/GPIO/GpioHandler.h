#pragma once

#include <QCoreApplication>
#include <QFile>
#include <QSocketNotifier>
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

    qint64 readFile(char *data, qint64 maxSize);

    qint64 writeFile(const char *data, qint64 maxSize);

    void seek(int position);

    QString error() const;
    void setError(const QString &newError);

    bool hasError() const {
        return !mError.isEmpty();
    }

public slots:
    // Slot to handle GPIO events
    void handleGpioEvent(QSocketDescriptor socket, QSocketNotifier::Type activationEvent);

signals:
    void readyRead(QByteArray buffer);

    void errorChanged();

private:
    QFile file;
    QSocketNotifier *notifier;
    QString filePath;
    QString mError;
};

