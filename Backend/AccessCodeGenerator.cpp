#include "AccessCodeGenerator.h"

#include <random>
#include <sstream>

#include "LogHelper.h"

AccessCodeGenerator::AccessCodeGenerator(QObject *parent)
    : QObject{parent}
{

}

QString AccessCodeGenerator::generateRandomPassword() {
    const QString possibleCharacters("abcdefghijklmnopqrstuvwxyz0123456789");
    const int randomStringLength = 8;

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

    TRACE << randomString;
    return randomString;
}

QString AccessCodeGenerator::decodeLockPassword(QString pass)
{
    const int MOD = 10000;  // Modulo to ensure a 4-digit number
    int hashValue = 0;
    int prime = 31;  // A small prime number to generate a unique hash

    for (char c : pass.toStdString()) {
        // Calculate the contribution of each character (a=1, b=2, ..., z=26)
        int charValue = c - '0' + 1;
        hashValue = (hashValue * prime + charValue) % MOD;
    }

    std::ostringstream ss;
    ss << std::setw(4) << std::setfill('0') << hashValue;

    TRACE << QString::fromStdString(ss.str());
    return QString::fromStdString(ss.str());
}
