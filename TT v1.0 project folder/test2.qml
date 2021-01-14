import QtQuick 2.0
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

//Rectangle {
//    width: 400
//    height: 400

//    MouseArea {
//        anchors.fill: parent
//        onClicked: {
//            if (parent.color == "#000000") parent.color = "red"
//            else parent.color = "black";
//        }
//    }
//}

Item {
    id:root
    objectName: "grid"
    signal clickIndexSignal(msg: string) // SIGNAL
//        onClickIndexSignal:{ // SIGNAL handler
//            console.log("button " + i + " clicked!")
//        }

//        function clickIndexFunction(i){
//            return(console.log("button " + i + " clicked!!!"))
//        }

    Grid {
        anchors.fill: parent
        columns: 10
        rows: 10
        spacing: 1
        Repeater {
            id: repeater_ae_set
            model: 100
            TestButton {
                onClicked:{
                                    clickIndexSignal(index)
                                    //clickIndexFunction(index)
                                }
            }
        }
    }
}


