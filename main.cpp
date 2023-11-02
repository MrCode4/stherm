#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQuickStyle>
#include <QScreen>
#include <QQmlContext>
#include <QFontDatabase>
#include <QSysInfo>
#include <QProcess>
#include <QFile>
#include <QRegularExpression>
#include <QDebug>

//! todo: Move to daemon
void exportGPIOPin(int pinNumber) {
    QFile exportFile("/sys/class/gpio/export");
    if (!exportFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
        qDebug() << Q_FUNC_INFO << __LINE__ << "Failed to open export file.";
        return;
    }

    // Convert pinNumber to string
    QString pinString = QString::number(pinNumber);

    // Write the pin number to the export file
    QTextStream out(&exportFile);
    out << pinString;
    exportFile.close();


    QString directionFilePath = QString("/sys/class/gpio/gpio%0/direction").arg(pinNumber);
    QFile directionFile(directionFilePath);

    if (!directionFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
        qDebug() << Q_FUNC_INFO << __LINE__ << "Failed to open direction file for pin " << pinNumber;
        return;
    }

    QTextStream outIn(&directionFile);
    outIn << "in";

    directionFile.close();

}

//! todo: Move to daemon
int getStartMode (int pinNumber) {
    exportGPIOPin(pinNumber);

    // Define the file path
    QString filePath = QString("/sys/class/gpio/gpio%0/value").arg(QString::number(pinNumber));

    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qDebug() << Q_FUNC_INFO << __LINE__ << "Failed to open the file.";
        return -1;
    }

    QTextStream in(&file);
    QString value;
    in >> value; // Read the content of the file

    qDebug() << Q_FUNC_INFO << __LINE__ << value;

    file.close();

    int result = (value.trimmed() == "0") ? 1 : 0;
    return result;

}

//! Get CPU info
//! todo: Move to daemon
void getCPUInfo() {
    QFile file("/proc/cpuinfo");
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qDebug() << Q_FUNC_INFO << __LINE__ <<"Failed to open the file.";
        return;
    }

    QString cpuInfo;
    QString serialNumber;
    QString line = file.readLine();

    while (!line.isEmpty()) {
        cpuInfo.append(line).append('\n');

        if (line.startsWith("Serial")) {
            QRegularExpression re("Serial:\\s*([A-Fa-f0-9]+)");
            QRegularExpressionMatch match = re.match(line);
            if (match.hasMatch()) {
                QString serialNumberHex = match.captured(1);
                serialNumberHex = serialNumberHex.simplified();  // Remove extra spaces
                QByteArray snByteArray = QByteArray(serialNumberHex.toLatin1());
                serialNumber = QByteArray::fromHex(snByteArray);
            }
        }

        line = file.readLine();
    }

    file.close();

    qDebug() << Q_FUNC_INFO << __LINE__ << "cpuInfo: " << cpuInfo;
    qDebug() << Q_FUNC_INFO << __LINE__ << "Serial Number: " << serialNumber;
}

//! setBrightness
//! todo: Move to daemon
void setBrightness(int value) {
    QFile brightnessFile("/sys/class/backlight/backlight_display/brightness");
    if (!brightnessFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
        qDebug() << Q_FUNC_INFO << __LINE__  << "Failed to open brightness file.";
        return;
    }

    QTextStream out(&brightnessFile);
    out << value; // Write the desired brightness value
    brightnessFile.close();

    qDebug() << "Brightness set successfully!";
}

//! Set time zone
//! todo: Move to daemon
void setTimeZone(int offset) {
    QString timezoneName = "GMT";
    QString offsetStr = (offset >= 0) ? "+" : "";
    offsetStr += QString::number(offset);

    QString timezonePath = "/usr/share/zoneinfo/Etc/";
    QString linkPath = "/etc/localtime";

    QString timezoneFile = timezonePath + timezoneName + offsetStr;

    if (!QFile::exists(timezoneFile)) {
        qDebug() << Q_FUNC_INFO << __LINE__  << "Timezone file not found.";
        return;
    }

    int exitCode = QProcess::execute("ln", {"-sf", timezoneFile, linkPath});

    if (exitCode >= 0) {
        qDebug() << Q_FUNC_INFO << __LINE__  << "Timezone set to" << timezoneFile;
    }
}

int main(int argc, char *argv[])
{
    qDebug() << Q_FUNC_INFO << __LINE__ << "getStartMode: " << getStartMode(90);

    // CPU info example
    getCPUInfo();

    // Brightness example
    setBrightness(50);

    // Time zoon example
    setTimeZone(8);

    //! Enable virtual keyboard
    qputenv("QT_IM_MODULE", QByteArray("qtvirtualkeyboard"));

    //! Setting application organization name and domain
    QGuiApplication::setOrganizationName("STherm");
    QGuiApplication::setOrganizationDomain("SThermOrg");

    QGuiApplication app(argc, argv);

    QQmlApplicationEngine engine;

    engine.addImportPath(":/");
    const QUrl url(u"qrc:/Stherm/Main.qml"_qs);
    /*    QObject::connect(&engine, &QQmlApplicationEngine::objectCreationFailed,
        &app, []() { QCoreApplication::exit(-1); },
        Qt::QueuedConnection)*/
    ;

    QScreen* screen = app.primaryScreen();

    QVariantMap deviceInfo;
    deviceInfo["Width"] = screen->size().width();
    deviceInfo["Height"] = screen->size().height();
    deviceInfo["DPR"] = screen->devicePixelRatio();
    deviceInfo["L-DPI"] = screen->logicalDotsPerInch();
    deviceInfo["P-DPI"] = screen->physicalDotsPerInch();
    //! Grab some system info also
    deviceInfo["Kernel"] = QSysInfo::kernelType();
    deviceInfo["Kernel Version"] = QSysInfo::kernelVersion();
    deviceInfo["OS"] = QSysInfo::prettyProductName();
    deviceInfo["Nmcli"] = QProcess().execute("command", { "-v", "nmcli" }) == 0 ? "True" : "False";

    //! Cacluate a font factor based on system specifications
    const double refFontPt = 13;
    const double refDPI = 141;
    const double dpi = QGuiApplication::primaryScreen()->physicalDotsPerInch();
    const double factor = qMax(1., dpi / refDPI);

    //! Load default font -> Roboto-Regular for now
    int robotoId = QFontDatabase::addApplicationFont(":/Stherm/Fonts/Roboto-Regular.ttf");
    if (robotoId == -1) {
        qWarning() << "Could not load Roboto-Regular font.";
    } else {
        QStringList roboto = QFontDatabase::applicationFontFamilies(robotoId);
        QFont defaultFont(roboto[0], refFontPt * factor);
        qDebug() << "Default font pt: " << defaultFont.pointSize();

        qApp->setFont(defaultFont);
    }

    engine.rootContext()->setContextProperty("deviceInfo", deviceInfo);

    engine.load(url);

    return app.exec();
}
