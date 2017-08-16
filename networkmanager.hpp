#pragma once

#include "networkmanagerinterface.hpp"

#include <QtCore>
#include <QtDBus>
#include <QtQml>

#include <openssl/evp.h>

Q_DECLARE_LOGGING_CATEGORY(networkManagerCategory)

struct auto_ptr_cast
{

    union // UB
    {
        const void * const c;
        void * const v;
    };

    template< typename P >
    auto_ptr_cast(P const * const p) : c(p) { ; }

    template< typename P >
    auto_ptr_cast(P * const p) : v(p) { ; }

    template< typename type >
    operator type const * () const
    {
        return static_cast< type const * >(c);
    }

    template< typename type >
    operator type * () const
    {
        return static_cast< type * >(v);
    }

};

inline
QByteArray
WPA_PSK(QByteArray secret, QByteArray salt) // why WPA? https://wigle.net/stats#
{
    QByteArray result{32, Qt::Uninitialized};
    PKCS5_PBKDF2_HMAC_SHA1(secret.constData(), secret.length(), auto_ptr_cast(salt.constData()), salt.length(), 4096, result.length(), auto_ptr_cast(result.data()));
    return result;
}

inline
NMVariantMapMap
MakeWirelessConnectionParameters(QByteArray password, QByteArray ssid, bool hashed = false)
{
    // qdbus --literal --system org.freedesktop.NetworkManager /org/freedesktop/NetworkManager/Settings/? org.freedesktop.NetworkManager.Settings.Connection.GetSecrets 802-11-wireless-security
    // dbus-send --system --print-reply --dest=org.freedesktop.NetworkManager /org/freedesktop/NetworkManager/Settings/? org.freedesktop.NetworkManager.Settings.Connection.GetSecrets string:802-11-wireless-security
    NMVariantMapMap connectionParameters;
    connectionParameters["802-11-wireless"]["security"] = QStringLiteral("802-11-wireless-security");
    auto & security = connectionParameters["802-11-wireless-security"];
    security["key-mgmt"] = QStringLiteral("wpa-psk");
    security["psk"] = QString::fromUtf8(hashed ? WPA_PSK(password, ssid).toHex() : password);
    return connectionParameters;
}

class NetworkManager // translate NetworkManager naming of signals/slots/properties to QML style
        : public QObject
{

    Q_OBJECT

    Q_PROPERTY(QString version READ version NOTIFY versionChanged)

public :

    NetworkManager(QDBusConnection const & connection,
                   QObject * const parent)
        : QObject{parent}
        , networkManagerInterface{connection}
    {
        Q_CHECK_PTR(parent);
        if (!networkManagerInterface.isValid()) {
            deleteLater(); // is it correct to call deleteLater() in constructor?
            return;
        }
        connect(&networkManagerInterface, &NetworkManagerInterface::VersionChanged,
                this, &NetworkManager::versionChanged); // lowercase capitalized first letter of signal name
    }

    QString version() const
    {
        return networkManagerInterface.property("Version").toString();
    }

    Q_INVOKABLE
    QString addConnection(QString device, QString accessPoint, QString ssid, QString psk, bool hashed = false)
    {
        return networkManagerInterface.AddAndActivateConnection(MakeWirelessConnectionParameters(psk.toUtf8(), ssid.toUtf8(), hashed),
                                                                QDBusObjectPath{device},
                                                                QDBusObjectPath{accessPoint},
                                                                activatingConnection).path();
    }

Q_SIGNALS :

    void versionChanged();

private :

    Q_DISABLE_COPY(NetworkManager)

    NetworkManagerInterface networkManagerInterface;

    QDBusObjectPath activatingConnection;

};

class NetworkManagerSingleton
        : public QObject
{

    Q_OBJECT

    Q_PROPERTY(NetworkManager* networkManager MEMBER networkManager NOTIFY networkManagerChanged)

    QDBusConnection connection = QDBusConnection::systemBus();
    QDBusConnectionInterface * const dbus = connection.interface();
    QDBusServiceWatcher serviceRegistarationWatcher;
    QDBusServiceWatcher serviceUnregistrationWatcher;

public :

    explicit NetworkManagerSingleton(QObject * const parent = Q_NULLPTR)
        : QObject{parent}
    {
        serviceRegistarationWatcher.setWatchMode(QDBusServiceWatcher::WatchForRegistration);
        serviceRegistarationWatcher.addWatchedService(NM_DBUS_SERVICE);
        const auto onRegistration = [&] (QString const & serviceName)
        {
            Q_ASSERT(serviceName == NM_DBUS_SERVICE);
            qCInfo(networkManagerCategory).noquote()
                    << tr("Service %1 is registered")
                       .arg(serviceName);
            createNetworkManager();
        };
        //connect(dbus, &QDBusConnectionInterface::serviceRegistered, this, onRegistration); // not works as expected
        connect(&serviceRegistarationWatcher, &QDBusServiceWatcher::serviceRegistered, this, onRegistration);

        serviceUnregistrationWatcher.setWatchMode(QDBusServiceWatcher::WatchForUnregistration);
        serviceUnregistrationWatcher.addWatchedService(NM_DBUS_SERVICE);
        const auto onUnregistration = [&] (QString const & serviceName)
        {
            Q_ASSERT(serviceName == NM_DBUS_SERVICE);
            qCInfo(networkManagerCategory).noquote()
                    << tr("Service %1 is unregistered")
                       .arg(serviceName);
            Q_ASSERT(networkManager);
            networkManager->deleteLater();
        };
        //connect(dbus, &QDBusConnectionInterface::serviceUnregistered, this, onUnregistration); // not works as expected
        connect(&serviceUnregistrationWatcher, &QDBusServiceWatcher::serviceUnregistered, this, onUnregistration);

        if (dbus->isServiceRegistered(QStringLiteral(NM_DBUS_SERVICE))) {
            createNetworkManager();
        }

        serviceRegistarationWatcher.setConnection(connection);
        serviceUnregistrationWatcher.setConnection(connection);
    }

Q_SIGNALS :

    void networkManagerChanged(NetworkManager * networkManager);

private :

    Q_DISABLE_COPY(NetworkManagerSingleton)

    QPointer< NetworkManager > networkManager;

    void createNetworkManager()
    {
        Q_ASSERT(!networkManager);
        // will QPointer be cleared just before QObject::destroyed signal is emitted and called onDestroyed?
        if (!setProperty("networkManager", QVariant::fromValue(::new NetworkManager{connection, this}))) {
            Q_ASSERT(false);
        }
        connect(networkManager.data(), &QObject::destroyed, this, [&] { Q_ASSERT(!networkManager); Q_EMIT networkManagerChanged(Q_NULLPTR); });
    }

};
