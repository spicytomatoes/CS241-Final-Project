import QtQuick 2.0
import  QtQuick.Controls 2.15

Button {
    id: control

    background: Rectangle {
        implicitWidth: 30
        implicitHeight: 30
        opacity: 0.3
        color: control.down ? "red" : "black"
//        border.color: "white"
//        border.width: 1
//        radius: 2
    }
}
