/*
 * Copyright 2012 Canonical Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation; version 3.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

import QtQuick 1.1

/*!
    \qmlclass ColoredButton
    \inqmlmodule UbuntuUIToolkit
    \brief The ColoredButton class adds a colored background to the Button.

    \b{This component is under heavy development.}

    It adds a colored background, border, and changes of background color
    depending on the state, to the Button.

    Examples:
    \qml
        Column {
            width: 155
            spacing: 5

            ColoredButton {
                text: "text only (centered)\nwith border"
                onClicked: print("clicked text-only ColoredButton")
            }
            ColoredButton {
                iconSource: "call_icon.png"
                onClicked: print("clicked icon-only ColoredButton")
                iconPosition: "top"
            }
            ColoredButton {
                iconSource: "call_icon.png"
                text: "Icon on right"
                iconPosition: "right"
                onClicked: print("clicked on ColoredButton with text and icon")
            }
        }
    \endqml
*/
Button {
    id: button

    /*!
       \preliminary
       DOCME
    */
    property color color: "#8888cc"

    /*!
       \preliminary
       DOCME
    */
    property color borderColor: "black"

    /*!
      \preliminary
      DOCME
    */
    property int borderWidth: 2.0

    /*!
       \preliminary
       DOCME
    */
    property real radius: 3.0

    /*!
       \preliminary
       DOCME
    */
    property color pressColor: Qt.darker(button.color, 1.2)

    /*!
       \preliminary
       DOCME
    */
    property color hoverColor: Qt.lighter(button.color, 1.25)

    /*!
      \preliminary
      DOCME
    */
    property color disabledColor: "#888888"

    property alias background: rectbg

    Rectangle {
        z: -1
        id: rectbg
        radius: parent.radius
        width: parent.width
        height: parent.height
        color: button.color
        border.color: parent.borderColor
        border.width: parent.borderWidth
    } // background

    states: [
        State {
            name: "idle"
            PropertyChanges { target: background; color: button.color }
        },
        State {
            name: "pressed"
            PropertyChanges { target: background; color: button.pressColor }
        },
        State {
            name: "hovered"
            PropertyChanges { target: background; color: button.hoverColor }
        },
        State {
            name: "disabled"
            PropertyChanges { target: background; color: button.disabledColor }
        }
    ]
}
