#include <QDebug>
#include <QFile>
#include <QFontDatabase>
#include <QGuiApplication>
#include <QProcess>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QQuickStyle>
#include <QRegularExpression>
#include <QScreen>
#include <QSysInfo>

#include <csignal>

#include "UtilityHelper.h"

void signalHandler(int signal) {
    if (signal == SIGTERM
#ifdef __unix__
        || signal == SIGHUP
#endif
    ) {
        qDebug() << "Received Signal, quitting the application gracefully (updating)." << signal;
        QGuiApplication::instance()->exit();
    }
}

int main(int argc, char *argv[])
{
    // CPU info example
    QString cpuid = UtilityHelper::getCPUInfo();
    qDebug() << "CPU ID: " << cpuid;

    // Brightness example
    UtilityHelper::setBrightness(200);

    // Time zone example
    //    UtilityHelper::setTimeZone(8);

    //! Enable virtual keyboard
    qputenv("QT_IM_MODULE", QByteArray("qtvirtualkeyboard"));

    //! Setting application organization name and domain
    QGuiApplication::setOrganizationName("STherm");
    QGuiApplication::setOrganizationDomain("SThermOrg");

    QCoreApplication::setApplicationVersion(PROJECT_VERSION_STRING);

    QGuiApplication app(argc, argv);

    signal(SIGTERM, signalHandler);
#ifdef __unix__
    signal(SIGHUP, signalHandler);
#endif

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

    //! Calculate a font factor based on system specifications
    //! disabled for now! not needed as fonts are large enough
    const double refFontPt = 15;
    const double refDPI = 141;
    const double dpi = QGuiApplication::primaryScreen()->physicalDotsPerInch();
    const double scaleFactor = qMax(1., dpi / refDPI);

    //! Load default font -> Montserrat-Regular for now
    int fontId = QFontDatabase::addApplicationFont(":/Stherm/Fonts/Montserrat-Regular.ttf");
    if (fontId == -1) {
        qWarning() << "Could not load Montserrat-Regular font.";
    } else {
        QStringList fonts = QFontDatabase::applicationFontFamilies(fontId);
        QFont defaultFont(fonts[0], refFontPt * scaleFactor);
        defaultFont.setCapitalization(QFont::MixedCase);
        qDebug() << "Default font pt: " << defaultFont.pointSize();

        qApp->setFont(defaultFont);
    }

    engine.rootContext()->setContextProperties({
        {"scaleFactor", scaleFactor},
        {"deviceInfo", deviceInfo},
    });
    engine.rootContext()->setContextProperty("deviceInfo", deviceInfo);

    engine.load(url);

    return app.exec();
}
