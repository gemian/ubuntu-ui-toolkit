import QtQuick 1.1
import Ubuntu.Components 0.1

Page {
    title: "My custom page"
    Rectangle {
        anchors.fill: parent
        color: "#dddddd"
        TextCustom {
            anchors.centerIn: parent
            text: "This is an external page."
            color: "#757373"
        }
    }
}