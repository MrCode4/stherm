#pragma once

#include <QObject>
#include <QQmlEngine>
#include <QColor>

#include "QrCode/qrcodegen.hpp"

//!
//! \brief The QRCodeGenerator class
//!
class QRCodeGenerator : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    QML_SINGLETON

public:
    explicit QRCodeGenerator(QObject* parent = nullptr) : QObject { parent } {}

    //!
    //! \brief getQRCodeSvg Returns a QR Code in svg form fot the given \a text
    //! \param text
    //! \param fillcolor
    //! \return
    //!
    Q_INVOKABLE QString     getQRCodeSvg(const QString& text, const QColor& fillcolor);

private:
    QString qrcodeToSvgString(const qrcodegen::QrCode& qrcode,
                              int border,
                              const QString& color);
};

inline QString QRCodeGenerator::getQRCodeSvg(const QString& text, const QColor& fillcolor)
{
    //! Generate a QR code
    auto qr = qrcodegen::QrCode::encodeText(text.toStdString().data(),
                                            qrcodegen::QrCode::Ecc::MEDIUM);
    return qrcodeToSvgString(qr, 4, fillcolor.name());
}

inline QString QRCodeGenerator::qrcodeToSvgString(const qrcodegen::QrCode& qrcode,
                                                  int border,
                                                  const QString& color)
{
    QString svg;
    QTextStream sb(&svg);
    sb << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
       << "<!DOCTYPE svg PUBLIC \"-//W3C//DTD SVG 1.1//EN\" \"http://www.w3.org/Graphics/SVG/1.1/DTD/svg11.dtd\">\n"
       << "<svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\" viewBox=\"0 0 "
       << (qrcode.getSize() + border * 2) << " " << (qrcode.getSize() + border * 2) << "\" stroke=\"none\">\n"
       << "\t<rect width=\"100%\" height=\"100%\" fill=\"#00000000\"/>\n"
       << "\t<path d=\"";

    for (int y = 0; y < qrcode.getSize(); y++) {
        for (int x = 0; x < qrcode.getSize(); x++) {
            if (qrcode.getModule(x, y)) {
                if (x != 0 || y != 0)
                    sb << " ";
                sb << "M" << (x + border) << "," << (y + border) << "h1v1h-1z";
            }
        }
    }

    sb << "\" fill=\"" << color << "\"/>\n";
    sb << "</svg>\n";

    return svg;
}
