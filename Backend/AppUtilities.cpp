#include "AppUtilities.h"

#include <random>
#include <sstream>
#include <QDir>
#include <QStorageInfo>

#include "LogHelper.h"

QString AppUtilities::generateRandomPassword()
{
    const QString possibleCharacters("0123456789");
    const int randomStringLength = 6;

    QString randomString;
    for(int i = 0; i < randomStringLength; ++i)
    {
        std::random_device rd;  // Non-deterministic random number generator
        std::mt19937 generator(rd());  // Seed the generator
        std::uniform_int_distribution<> distribution(0, possibleCharacters.size() - 1);

        auto rd2 = distribution(generator);
        int index = rd2 % possibleCharacters.length();
        QChar nextChar = possibleCharacters.at(rd2);
        randomString.append(nextChar);
    }

    TRACE << "Generated Random String: " << randomString;
    return randomString;
}

QString AppUtilities::decodeLockPassword(QString pass)
{
    const int MOD = 10000;  // Modulo to ensure a 4-digit number
    int hashValue = 0;
    int prime = 26;  // A small prime number to generate a unique hash

    for (char c : pass.toStdString()) {
        // Calculate the contribution of each character (a=1, b=2, ..., z=26)
        int charValue = c - '0' + 1;
        hashValue = (hashValue * prime + charValue) % MOD;
    }

    std::ostringstream ss;
    ss << std::setw(4) << std::setfill('0') << hashValue;

    TRACE << "Decoded Password: " << QString::fromStdString(ss.str());
    return QString::fromStdString(ss.str());
}

double AppUtilities::getTruncatedvalue(double value, int decimalCount) 
{
    if (decimalCount < 0) {
        return value; // Return the original value if decimal is less than 0;
    }
    const double factor = qPow(10, decimalCount);
    return qFloor(value * factor) / factor;
}

bool AppUtilities::removeDirectory(const QString &path)
{
    QDir dir(path);

    if (dir.exists()) {
        if (dir.removeRecursively()) {
            TRACE << "Successfully removed:" << path;
        } else {
            TRACE << "Failed to remove:" << path;
            return false;
        }
    } else {
        TRACE << "Directory does not exist:" << path;
    }

    return true;
}

bool AppUtilities::removeContentDirectory(const QString &path)
{
    QDir dir(path);

    if (dir.exists()) {
        QFileInfoList fileList = dir.entryInfoList(QDir::Files);

        for (const QFileInfo& fileInfo : fileList) {
            QFile::remove(fileInfo.absoluteFilePath());
        }

        // Recursively remove contents of subfolders
        QFileInfoList subDirs = dir.entryInfoList(QDir::Dirs | QDir::NoDotAndDotDot);
        for (const QFileInfo& subDirInfo : subDirs) {
            AppUtilities::removeContentDirectory(subDirInfo.absoluteFilePath());
        }
    }

    return true;
}

int AppUtilities::getStorageFreeBytes(const QString path) {
    QStorageInfo storageInfo (path);

    if (!storageInfo.isValid()) {
        return 0;
    }

    return storageInfo.bytesFree();
}

int AppUtilities::getStorageTotalBytes(const QString path) {
    QStorageInfo storageInfo (path);

    if (!storageInfo.isValid()) {
        return 0;
    }

    return storageInfo.bytesTotal();
}

int AppUtilities::getStorageAvailableBytes(const QString path) {
    QStorageInfo storageInfo (path);

    if (!storageInfo.isValid()) {
        return 0;
    }

    return storageInfo.bytesAvailable();
}

int AppUtilities::getFolderUsedBytes(const QString path) {
    int totalSize = 0;
    QDir dir(path);

    if (dir.exists()) {
        QFileInfoList fileList = dir.entryInfoList(QDir::Files | QDir::NoDotAndDotDot, QDir::Size | QDir::Name);

        for (const QFileInfo& fileInfo : fileList) {
            totalSize += fileInfo.size();
        }

        // Recursively calculate the size of subfolders
        QFileInfoList subDirs = dir.entryInfoList(QDir::Dirs | QDir::NoDotAndDotDot);
        for (const QFileInfo& subDirInfo : subDirs) {
            totalSize += AppUtilities::getFolderUsedBytes(subDirInfo.absoluteFilePath());
        }
    }

    return totalSize;
}

int AppUtilities::getFileSizeBytes(const QString file) {
    QFileInfo fileInfo(file);

    if(!fileInfo.exists()) {
        return 0;
    }

    return fileInfo.size();
}

QString AppUtilities::bytesToNearestBigUnit(int bytes) {

    if (bytes < 0) {
        return "0 bytes";

    } else if (bytes < 1024) {
        return QString::number(bytes) + " bytes";

    } else if (bytes < qPow(1024, 2)) {
        return QString::number(static_cast<double>(bytes) / 1024, 'f', 2) + " KB";

    } else if (bytes < qPow(1024, 3)) {
        return QString::number(static_cast<double>(bytes) / qPow(1024, 2), 'f', 2) + " MB";

    } else if (bytes < qPow(1024, 4)) {
        return QString::number(static_cast<double>(bytes) / qPow(1024, 3), 'f', 2) + " GB";

    } else if (bytes < qPow(1024, 5)) {
        return QString::number(static_cast<double>(bytes) / qPow(1024, 4), 'f', 2) + " TB";

    } else {
        return QString::number(static_cast<double>(bytes) / qPow(1024, 5), 'f', 2) + " PB";
    }
}
