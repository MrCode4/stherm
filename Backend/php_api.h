#ifndef PHP_API_H
#define PHP_API_H

#include <QObject>
#include <QQmlEngine>


/* PHP, on V1, is used to extend the JS web browser based UI.
 * we intend to replace it with something more suitable
 *
*/

class php_api : public QObject
{
    Q_OBJECT
    /* TODO - FYR
     * The Q_PROPERTY macro declares a property that could be accessed from QML.
     * e.g.
     *  Q_PROPERTY(QString userName READ userName WRITE setUserName NOTIFY userNameChanged)
    QML_ELEMENT
    */
public:
    explicit php_api(QObject *parent = nullptr);



signals:

};

#endif // PHP_API_H
