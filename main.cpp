#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQuickStyle>
#include <QScreen>
#include <QQmlContext>
#include <QFontDatabase>


int main(int argc, char *argv[])
{
    qputenv("QSG_RHI_BACKEND", "opengl");
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
    deviceInfo["width"] = screen->size().width();
    deviceInfo["height"] = screen->size().height();
    deviceInfo["devicePixelRatio"] = screen->devicePixelRatio();
    deviceInfo["logicalDotsPerInch"] = screen->logicalDotsPerInch();
    deviceInfo["physicalDotsPerInch"] = screen->physicalDotsPerInch();

    //! Load default font -> Roboto-Regular for now
    int robotoId = QFontDatabase::addApplicationFont(":/Stherm/Fonts/Roboto-Regular.ttf");
    if (robotoId == -1) {
        qWarning() << "Could not load Roboto-Regular font.";
    } else {
        auto iransSansFontFamilies = QFontDatabase::applicationFontFamilies(robotoId);
        QFont defaultFont(iransSansFontFamilies[0], qApp->font().pointSize());
        qApp->setFont(defaultFont);
    }

    engine.rootContext()->setContextProperty("deviceInfo", deviceInfo);

    engine.load(url);

    return app.exec();
}
