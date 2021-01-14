import QtQuick 2.0
import QtQuick.Controls 2.15

Button {
    id: control
    checkable: true
    background: Rectangle {
        implicitWidth: 40
        implicitHeight: 40
        opacity: if (!control.enabled) {
                    0.6
                 } else if (control.hovered) {
                     0.6
                 } else {
                     0.3
                 }
        color: if (!control.enabled) {
                    "#b5b5b5"
               } else if (control.checked) {
                   control.hovered ? "#46d246" : "transparent"
               } else {
                   "#46d246"
               }

        border.color: if (!control.enabled) {
                          "grey"
                      } else {
                          "#1e7b1e"
                      }
        border.width: 1
        radius: 1
    }
}
