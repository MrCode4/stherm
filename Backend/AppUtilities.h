#pragma once

#include <QObject>
#include <QQmlEngine>

class AppUtilities : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    QML_SINGLETON
public:
    explicit AppUtilities(QObject *parent = nullptr) : QObject(parent) {}

    //! Generating a Random Four-Digit Number
    Q_INVOKABLE QString generateRandomPassword();
    Q_INVOKABLE QString decodeLockPassword(QString pass);

    //! Truncate value to a specified number of decimal places
    Q_INVOKABLE static double  getTruncatedvalue(double value, int decimalCount = 0);

    //! Remove the directory and its contents
    Q_INVOKABLE static bool removeDirectory(const QString &path);

    //! Remove contents of directories and keep the directory tree.
    Q_INVOKABLE static bool removeContentDirectory(const QString &path);

    Q_INVOKABLE static int getStorageFreeBytes(const QString path);
    Q_INVOKABLE static int getStorageTotalBytes(const QString path);
    Q_INVOKABLE static int getStorageAvailableBytes(const QString path);
    Q_INVOKABLE static int getFolderUsedBytes(const QString path);
    Q_INVOKABLE static int getFileSizeBytes(const QString file);

    //! Convert bytes to the nearest big unit.
    Q_INVOKABLE  static QString bytesToNearestBigUnit(int bytes);

    Q_INVOKABLE static QString userVersion(const QString &fullVersion);
};
