#include <QCoreApplication>

#include "Watchdog.h"

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    Watchdog watchdog;

    return a.exec();
}
