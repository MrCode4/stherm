#pragma once

#include <QObject>
#include <QQmlEngine>

/*!
 * \brief The WifiInfo class holds information of a wifi network
 */
class WifiInfo : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool     connected       READ connected       NOTIFY connectedChanged)
    Q_PROPERTY(bool     isConnecting    READ isConnecting    NOTIFY isConnectingChanged)
    Q_PROPERTY(bool     isSaved         READ isSaved         NOTIFY isSavedChanged)
    Q_PROPERTY(int      strength        READ strength        NOTIFY strengthChanged)
    Q_PROPERTY(QString  ssid            READ ssid            NOTIFY ssidChanged)
    Q_PROPERTY(QString  bssid           READ bssid           NOTIFY bssidChanged)
    Q_PROPERTY(QString  security        READ security        NOTIFY securityChanged)
    QML_ELEMENT

public:
    explicit WifiInfo(QObject* parent=nullptr);
    WifiInfo(bool connected,
             bool isSaved,
             const QString& ssid,
             const QString& bssid,
             int strength,
             const QString& security,
             QObject* parent=nullptr);


    inline bool     connected()     const { return mConnected;      }
    inline bool     isConnecting()  const { return mIsConnecting ;  }
    inline bool     isSaved()       const { return mIsSaved;        }
    inline int      strength()      const { return mStrength;       }
    inline QString  incorrectSsid() const { return mIncorrectSsid;  }
    inline QString  ssid()          const { return mSsid;           }
    inline QString  bssid()         const { return mBssid;          }
    inline QString  security()      const { return mSecurity;       }

    void setConnected(bool connected);
    void setIsConnecting(bool isConnecting);
    void setIsSaved(bool isSaved);
    void setStrength(int strength);
    void setIncorrectSsid(const QString& incSs);
    void setSsid(const QString& ssid);
    void setBssid(const QString& bssid);
    void setSecurity(const QString& setcurity);

signals:
    void        connectedChanged();
    void        isSavedChanged();
    void        strengthChanged();
    void        ssidChanged();
    void        bssidChanged();
    void        securityChanged();
    void        isConnectingChanged();

private:
    bool        mConnected      = false;
    bool        mIsConnecting   = false;
    bool        mIsSaved        = false;    //! Whether this wifi is saved in NetworkManager (nmcli)
    int         mStrength       = 0;
    QString     mIncorrectSsid  = "";       //! The ssid that has some unsupported chars in it
    QString     mSsid           = "";
    QString     mBssid          = "";
    QString     mSecurity       = "";
};
