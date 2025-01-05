#include "WifiInfo.h"

WifiInfo::WifiInfo(QObject *parent)
    : QObject(parent)
{}

WifiInfo::WifiInfo(bool connected,
                   bool isSaved,
                   const QString& ssid,
                   const QString& bssid,
                   int strength,
                   const QString& security,
                   QObject* parent)
    : QObject(parent)
    , mConnected(connected)
    , mIsSaved(isSaved)
    , mStrength(strength)
    , mSsid(ssid)
    , mBssid(bssid)
    , mSecurity(security)
    , mIsConnecting(false)
{
}

void WifiInfo::setConnected(bool connected)
{
    if (mConnected == connected) {
        return;
    }

    mConnected = connected;
    emit connectedChanged();
}

void WifiInfo::setIsConnecting(bool isConnecting)
{
    if (mIsConnecting == isConnecting) {
        return;
    }

    mIsConnecting = isConnecting;
    emit isConnectingChanged();
}

void WifiInfo::setIsSaved(bool isSaved)
{
    if (mIsSaved == isSaved) {
        return;
    }

    mIsSaved = isSaved;
    emit isSavedChanged();
}

void WifiInfo::setStrength(int strength)
{
    if (mStrength == strength) {
        return;
    }

    mStrength = strength;
    emit strengthChanged();
}

void WifiInfo::setIncorrectSsid(const QString &incSs)
{
    mIncorrectSsid = incSs;
}

void WifiInfo::setSsid(const QString& ssid)
{
    if (mSsid == ssid) {
        return;
    }

    mSsid = ssid;
    emit ssidChanged();
}

void WifiInfo::setBssid(const QString& bssid)
{
    if (mBssid == bssid) {
        return;
    }

    mBssid = bssid;
    emit bssidChanged();
}

void WifiInfo::setSecurity(const QString& security)
{
    if (mSecurity == security) {
        return;
    }

    mSecurity = security;
    emit securityChanged();
}

QString WifiInfo::wifiInformation() const
{
    QString info = "ssid: " + ssid();
    info += ", incorrectSsid: " + incorrectSsid();
    info += ", bssid: " + bssid();
    info += ", security: " + security();
    info += ", connected: " + QString(connected() ? "true" : "false");
    info += ", isConnecting: " + QString(isConnecting() ? "true" : "false");
    info += ", isSaved: " + QString(isSaved() ? "true" : "false");
    info += ", strength: " + QString::number(strength());

    return info;
}
