#pragma once

#include <QtCore>
#include <QtDBus>

#include <NetworkManager.h>
#include <dbus/dbus.h>
#include <glib.h>

Q_DECLARE_LOGGING_CATEGORY(networkManagerInterfaceCategory)

using NMObjectPathsList = QList< QDBusObjectPath >;
Q_DECLARE_METATYPE(NMObjectPathsList)

using NMVariantMapMap = QMap< QString, QVariantMap >; // a{sa{sv}}
Q_DECLARE_METATYPE(NMVariantMapMap)

using NMStringMap = QMap< QString, QString >;
using NMStringMapIterator = QMapIterator< QString, QString >;
Q_DECLARE_METATYPE(NMStringMap)

inline
QDBusArgument &
operator << (QDBusArgument & argument, NMStringMap const & stringMap)
{
    argument.beginMap(QVariant::String, QVariant::String);
    NMStringMapIterator i{stringMap};
    while (i.hasNext()) {
        i.next();
        argument.beginMapEntry();
        argument << i.key() << i.value();
        argument.endMapEntry();
    }
    argument.endMap();
    return argument;
}

inline
QDBusArgument const &
operator >> (QDBusArgument const & argument, NMStringMap & stringMap)
{
    argument.beginMap();
    stringMap.clear();
    while (!argument.atEnd()) {
        QString key;
        QString value;
        argument.beginMapEntry();
        argument >> key >> value;
        argument.endMapEntry();
        stringMap.insert(key, value);
    }
    argument.endMap();
    return argument;
}

class NetworkManagerInterface
        : public QDBusAbstractInterface
{

    Q_OBJECT

    Q_PROPERTY(QDBusObjectPath ActivatingConnection MEMBER ActivatingConnection NOTIFY ActivatingConnectionChanged)
    Q_PROPERTY(NMObjectPathsList ActiveConnections MEMBER ActiveConnections NOTIFY ActiveConnectionsChanged)
    Q_PROPERTY(NMObjectPathsList AllDevices MEMBER AllDevices NOTIFY AllDevicesChanged)
    Q_PROPERTY(uint Connectivity MEMBER Connectivity NOTIFY ConnectivityChanged)
    Q_PROPERTY(NMObjectPathsList Devices MEMBER Devices NOTIFY DevicesChanged)
    Q_PROPERTY(QVariantMap GlobalDnsConfiguration MEMBER GlobalDnsConfiguration NOTIFY GlobalDnsConfigurationChanged)
    Q_PROPERTY(uint Metered MEMBER Metered NOTIFY MeteredChanged)
    Q_PROPERTY(bool NetworkingEnabled MEMBER NetworkingEnabled NOTIFY NetworkingEnabledChanged)
    Q_PROPERTY(QDBusObjectPath PrimaryConnection MEMBER PrimaryConnection NOTIFY PrimaryConnectionChanged)
    Q_PROPERTY(QString PrimaryConnectionType MEMBER PrimaryConnectionType NOTIFY PrimaryConnectionTypeChanged)
    Q_PROPERTY(bool Startup MEMBER Startup NOTIFY StartupChanged)
    Q_PROPERTY(uint State MEMBER State NOTIFY StateChanged)
    Q_PROPERTY(QString Version MEMBER Version NOTIFY VersionChanged)
    Q_PROPERTY(bool WimaxEnabled MEMBER WimaxEnabled NOTIFY WimaxEnabledChanged)
    Q_PROPERTY(bool WimaxHardwareEnabled MEMBER WimaxHardwareEnabled NOTIFY WimaxHardwareEnabledChanged)
    Q_PROPERTY(bool WirelessEnabled MEMBER WirelessEnabled NOTIFY WirelessEnabledChanged)
    Q_PROPERTY(bool WirelessHardwareEnabled MEMBER WirelessHardwareEnabled NOTIFY WirelessHardwareEnabledChanged)
    Q_PROPERTY(bool WwanEnabled MEMBER WwanEnabled NOTIFY WwanEnabledChanged)
    Q_PROPERTY(bool WwanHardwareEnabled MEMBER WwanHardwareEnabled NOTIFY WwanHardwareEnabledChanged)

public :

    NetworkManagerInterface(QDBusConnection const & connection,
                            QObject * const parent = Q_NULLPTR)
        : QDBusAbstractInterface{{NM_DBUS_SERVICE}, {NM_DBUS_PATH}, NM_DBUS_INTERFACE,
                                 connection,
                                 parent}
    {
        qDBusRegisterMetaType< NMObjectPathsList >();
        qDBusRegisterMetaType< NMVariantMapMap >();
        qDBusRegisterMetaType< NMStringMap >();
        qDBusRegisterMetaType< QVariantMap >();

        if (!this->connection().connect({NM_DBUS_SERVICE}, path(), {DBUS_INTERFACE_PROPERTIES}, {"PropertiesChanged"},
                                  //QString::fromLatin1(QMetaObject::normalizedSignature("PropertiesChanged(QString,QVariantMap,QStringList)")), // not works as expected
                                  this, SLOT(propertiesChanged(QString, QVariantMap, QStringList)))) {
            Q_ASSERT(false);
        }
    }

public Q_SLOTS :

    Q_SCRIPTABLE
    QDBusObjectPath
    ActivateConnection(QDBusObjectPath connection,
                       QDBusObjectPath device,
                       QDBusObjectPath specificObject)
    {
        const auto message = call(QDBus::BlockWithGui, {"ActivateConnection"},
                                  QVariant::fromValue(connection),
                                  QVariant::fromValue(device),
                                  QVariant::fromValue(specificObject));
        QDBusPendingReply< QDBusObjectPath > pendingReply = message;
        Q_ASSERT(pendingReply.isFinished());
        if (pendingReply.isError()) {
            qCWarning(networkManagerInterfaceCategory).noquote()
                    << tr("Asynchronous call finished with error: %1")
                       .arg(pendingReply.error().message());
            return {};
        }
        return pendingReply.value();
    }

    Q_SCRIPTABLE
    QDBusObjectPath
    AddAndActivateConnection(NMVariantMapMap connection,
                             QDBusObjectPath device,
                             QDBusObjectPath specificObject,
                             QDBusObjectPath & activeConnection)
    {
        const auto message = call(QDBus::BlockWithGui, {"AddAndActivateConnection"},
                                  QVariant::fromValue(connection),
                                  QVariant::fromValue(device),
                                  QVariant::fromValue(specificObject));
        QDBusPendingReply< QDBusObjectPath, QDBusObjectPath > pendingReply = message;
        Q_ASSERT(pendingReply.isFinished());
        if (pendingReply.isError()) {
            qCWarning(networkManagerInterfaceCategory).noquote()
                    << tr("Asynchronous call finished with error: %1")
                       .arg(pendingReply.error().message());
            return {};
        }
        activeConnection = pendingReply.argumentAt< 1 >();
        return pendingReply.argumentAt< 0 >();
    }

    Q_SCRIPTABLE
    uint CheckConnectivity()
    {
        const auto message = call(QDBus::BlockWithGui, {"CheckConnectivity"});
        QDBusPendingReply< uint > pendingReply = message;
        Q_ASSERT(pendingReply.isFinished());
        if (pendingReply.isError()) {
            qCWarning(networkManagerInterfaceCategory).noquote()
                    << tr("Asynchronous call finished with error: %1")
                       .arg(pendingReply.error().message());
            return {};
        }
        return pendingReply.value();
    }

    Q_SCRIPTABLE
    void DeactivateConnection(QDBusObjectPath activeConnection)
    {
        const auto message = call(QDBus::BlockWithGui, {"DeactivateConnection"},
                                  QVariant::fromValue(activeConnection));
        QDBusPendingReply<> pendingReply = message;
        Q_ASSERT(pendingReply.isFinished());
        if (pendingReply.isError()) {
            qCWarning(networkManagerInterfaceCategory).noquote()
                    << tr("Asynchronous call finished with error: %1")
                       .arg(pendingReply.error().message());
            return;
        }
    }

    Q_SCRIPTABLE
    void Enable(bool enable)
    {
        const auto message = call(QDBus::BlockWithGui, {"Enable"},
                                  QVariant::fromValue(enable));
        QDBusPendingReply<> pendingReply = message;
        Q_ASSERT(pendingReply.isFinished());
        if (pendingReply.isError()) {
            qCWarning(networkManagerInterfaceCategory).noquote()
                    << tr("Asynchronous call finished with error: %1")
                       .arg(pendingReply.error().message());
            return;
        }
    }

    Q_SCRIPTABLE
    NMObjectPathsList
    GetAllDevices()
    {
        const auto message = call(QDBus::BlockWithGui, {"GetAllDevices"});
        QDBusPendingReply< NMObjectPathsList > pendingReply = message;
        Q_ASSERT(pendingReply.isFinished());
        if (pendingReply.isError()) {
            qCWarning(networkManagerInterfaceCategory).noquote()
                    << tr("Asynchronous call finished with error: %1")
                       .arg(pendingReply.error().message());
            return {};
        }
        return pendingReply.value();
    }

    Q_SCRIPTABLE
    QDBusObjectPath
    GetDeviceByIpIface(QString iface)
    {
        const auto message = call(QDBus::BlockWithGui, {"GetDeviceByIpIface"},
                                  QVariant::fromValue(iface));
        QDBusPendingReply< QDBusObjectPath > pendingReply = message;
        Q_ASSERT(pendingReply.isFinished());
        if (pendingReply.isError()) {
            qCWarning(networkManagerInterfaceCategory).noquote()
                    << tr("Asynchronous call finished with error: %1")
                       .arg(pendingReply.error().message());
            return {};
        }
        return pendingReply.value();
    }

    Q_SCRIPTABLE
    NMObjectPathsList
    GetDevices()
    {
        const auto message = call(QDBus::BlockWithGui, {"GetDevices"});
        QDBusPendingReply< NMObjectPathsList > pendingReply = message;
        Q_ASSERT(pendingReply.isFinished());
        if (pendingReply.isError()) {
            qCWarning(networkManagerInterfaceCategory).noquote()
                    << tr("Asynchronous call finished with error: %1")
                       .arg(pendingReply.error().message());
            return {};
        }
        return pendingReply.value();
    }

    Q_SCRIPTABLE
    QString
    GetLogging(QString & domains)
    {
        const auto message = call(QDBus::BlockWithGui, {"GetLogging"});
        QDBusPendingReply< QString, QString > pendingReply = message;
        Q_ASSERT(pendingReply.isFinished());
        if (pendingReply.isError()) {
            qCWarning(networkManagerInterfaceCategory).noquote()
                    << tr("Asynchronous call finished with error: %1")
                       .arg(pendingReply.error().message());
            return {};
        }
        domains = pendingReply.argumentAt< 1 >();
        return pendingReply.argumentAt< 0 >();
    }

    Q_SCRIPTABLE
    NMStringMap
    GetPermissions()
    {
        const auto message = call(QDBus::BlockWithGui, {"GetPermissions"});
        QDBusPendingReply< NMStringMap > pendingReply = message;
        Q_ASSERT(pendingReply.isFinished());
        if (pendingReply.isError()) {
            qCWarning(networkManagerInterfaceCategory).noquote()
                    << tr("Asynchronous call finished with error: %1")
                       .arg(pendingReply.error().message());
            return {};
        }
        return pendingReply.value();
    }

    Q_SCRIPTABLE
    void Reload(uint flags)
    {
        const auto message = call(QDBus::BlockWithGui, {"Reload"},
                                  QVariant::fromValue(flags));
        QDBusPendingReply<> pendingReply = message;
        Q_ASSERT(pendingReply.isFinished());
        if (pendingReply.isError()) {
            qCWarning(networkManagerInterfaceCategory).noquote()
                    << tr("Asynchronous call finished with error: %1")
                       .arg(pendingReply.error().message());
            return;
        }
    }

    Q_SCRIPTABLE
    void SetLogging(QString level,
                    QString domains)
    {
        const auto message = call(QDBus::BlockWithGui, {"SetLogging"},
                                  QVariant::fromValue(level),
                                  QVariant::fromValue(domains));
        QDBusPendingReply<> pendingReply = message;
        Q_ASSERT(pendingReply.isFinished());
        if (pendingReply.isError()) {
            qCWarning(networkManagerInterfaceCategory).noquote()
                    << tr("Asynchronous call finished with error: %1")
                       .arg(pendingReply.error().message());
            return;
        }
    }

    Q_SCRIPTABLE
    void Sleep(bool sleep)
    {
        const auto message = call(QDBus::BlockWithGui, {"Sleep"},
                                  QVariant::fromValue(sleep));
        QDBusPendingReply<> pendingReply = message;
        Q_ASSERT(pendingReply.isFinished());
        if (pendingReply.isError()) {
            qCWarning(networkManagerInterfaceCategory).noquote()
                    << tr("Asynchronous call finished with error: %1")
                       .arg(pendingReply.error().message());
            return;
        }
    }

    Q_SCRIPTABLE
    uint state() // u state()
    {
        const auto message = call(QDBus::BlockWithGui, {"state"});
        QDBusPendingReply< uint > pendingReply = message;
        Q_ASSERT(pendingReply.isFinished());
        if (pendingReply.isError()) {
            qCWarning(networkManagerInterfaceCategory).noquote()
                    << tr("Asynchronous call finished with error: %1")
                       .arg(pendingReply.error().message());
            return {};
        }
        return pendingReply.value();
    }

Q_SIGNALS :

    Q_SCRIPTABLE void CheckPermissions();
    Q_SCRIPTABLE void DeviceAdded(QDBusObjectPath);
    Q_SCRIPTABLE void DeviceRemoved(QDBusObjectPath);
    Q_SCRIPTABLE void PropertiesChanged(QVariantMap);
    Q_SCRIPTABLE void StateChanged(uint);

private Q_SLOTS :

    void propertyChanged(QString const & propertyName)
    {
        const auto signature = QStringLiteral("%1Changed()").arg(propertyName);
        const int signalIndex = staticMetaObject.indexOfSignal(QMetaObject::normalizedSignature(qUtf8Printable(signature)).constData());
        if (signalIndex < 0) {
            qCCritical(networkManagerInterfaceCategory).noquote()
                    << tr("There is no signal with %1 signature")
                       .arg(signature);
            return;
        }
        const auto signal = staticMetaObject.method(signalIndex);
        if (!signal.invoke(this, Qt::DirectConnection)) {
            qCCritical(networkManagerInterfaceCategory).noquote()
                    << tr("Unable to emit %1 signal for %2 property")
                       .arg(signature,
                            propertyName);
        }
    }

    void propertiesChanged(QString interfaceName, QVariantMap changedProperties, QStringList invalidatedProperties)
    {
        if (interfaceName != interface()) {
            return;
        }
        QMapIterator< QString, QVariant > i{changedProperties};
        while (i.hasNext()) {
            i.next();
            propertyChanged(i.key());
        }
        for (QString const & invalidatedProperty : invalidatedProperties) {
            propertyChanged(invalidatedProperty);
        }
    }

Q_SIGNALS :

    void ActivatingConnectionChanged();
    void ActiveConnectionsChanged();
    void AllDevicesChanged();
    void ConnectivityChanged();
    void DevicesChanged();
    void GlobalDnsConfigurationChanged();
    void MeteredChanged();
    void NetworkingEnabledChanged();
    void PrimaryConnectionChanged();
    void PrimaryConnectionTypeChanged();
    void StartupChanged();
    void StateChanged();
    void VersionChanged();
    void WimaxEnabledChanged();
    void WimaxHardwareEnabledChanged();
    void WirelessEnabledChanged();
    void WirelessHardwareEnabledChanged();
    void WwanEnabledChanged();
    void WwanHardwareEnabledChanged();

private :

    Q_DISABLE_COPY(NetworkManagerInterface)

    QDBusObjectPath ActivatingConnection;
    NMObjectPathsList ActiveConnections;
    NMObjectPathsList AllDevices;
    uint Connectivity;
    NMObjectPathsList Devices;
    QVariantMap GlobalDnsConfiguration;
    uint Metered;
    bool NetworkingEnabled;
    QDBusObjectPath PrimaryConnection;
    QString PrimaryConnectionType;
    bool Startup;
    uint State;
    QString Version;
    bool WimaxEnabled;
    bool WimaxHardwareEnabled;
    bool WirelessEnabled;
    bool WirelessHardwareEnabled;
    bool WwanEnabled;
    bool WwanHardwareEnabled;

};
