#include "Watchdog.h"

#include <sys/prctl.h>

#include <QCoreApplication>

int main(int argc, char *argv[])
{
    // Set the process name to "watchdog_stherm"
    if (prctl(PR_SET_NAME, "watchdog_stherm", 0, 0, 0) != 0) {
        perror("prctl");
        return EXIT_FAILURE;
    }

    QCoreApplication a(argc, argv);

    Watchdog watchdog;

    return a.exec();
}
