import QtQml 2.2

import QtQuick 2.9
import QtQuick.Layouts 1.3
import QtQuick.Controls 2.2
import QtQuick.Window 2.3

import NetworkManager 1.0

ApplicationWindow {
    id: root

    visible: true
    visibility: Window.FullScreen

    Loader {
        active: NetworkManager.networkManager

        sourceComponent: Label {
            text: qsTr("Version: %1").arg(NetworkManager.networkManager.version)

            verticalAlignment: Text.AlignVCenter
            horizontalAlignment: Text.AlignHCenter
        }

        anchors.centerIn: parent
    }
}
