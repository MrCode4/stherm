#ifndef WIRINGS_H
#define WIRINGS_H

#include <QObject>

class wirings : public QObject
{
    Q_OBJECT
public:
    explicit wirings(QObject *parent = nullptr);


    uint32_t        id = 0;
    std::string     name;
    bool            type;
    uint32_t        order_id;
    std::string     alias;

signals:

};

#endif // WIRINGS_H
