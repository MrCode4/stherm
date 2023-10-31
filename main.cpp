#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQuickStyle>
#include <QScreen>
#include <QQmlContext>
#include <QFontDatabase>
#include <QSysInfo>
#include <QProcess>


int main(int argc, char *argv[])
{
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
    const double refFontPt = 15;
    const double refDPI = 141;
    const double dpi = QGuiApplication::primaryScreen()->physicalDotsPerInch();
    const double scaleFactor = qMax(1., dpi / refDPI);

    //! Load default font -> Roboto-Regular for now
    int robotoId = QFontDatabase::addApplicationFont(":/Stherm/Fonts/Roboto-Regular.ttf");
    if (robotoId == -1) {
        qWarning() << "Could not load Roboto-Regular font.";
    } else {
        QStringList roboto = QFontDatabase::applicationFontFamilies(robotoId);
        QFont defaultFont(roboto[0], refFontPt * scaleFactor);
        qDebug() << "Default font pt: " << defaultFont.pointSize();

        qApp->setFont(defaultFont);
    }

    engine.rootContext()->setContextProperty("scaleFactor", scaleFactor);
    engine.rootContext()->setContextProperty("deviceInfo", deviceInfo);

    engine.load(url);

    return app.exec();
}
