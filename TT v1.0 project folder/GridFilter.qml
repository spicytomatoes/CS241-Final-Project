import QtQuick 2.0
import QtQuick.Window 2.14
import QtQuick.Controls 2.15
import QtLocation 5.15
import QtPositioning 5.6

Window {
    id:root
    property variant region: "shape()"
    property bool buttonsEnabled: false

    signal clickIndexSignal(msg: int)
    function loadMap() {
        map.fitViewportToGeoShape(region,0)
    }
    function checkAll() {
        for (var i = 0; i < 100; i++) {
            repeater.itemAt(i).checked = false;
        }
    }
    function unCheckAll() {
        for (var i = 0; i < 100; i++) {
            repeater.itemAt(i).checked = true;
        }
    }

    Plugin {
        id: mapPlugin
        name: "osm"
    }

    Map {
        id: map
        anchors.fill: parent
        plugin: mapPlugin
        copyrightsVisible: false
        gesture.enabled: false
        //to create greyed out effect
        center: QtPositioning.coordinate(0,0)
        zoomLevel: 20
    }

    Grid {
        anchors.fill: parent
        columns: 10
        rows: 10
        Repeater {
            id: repeater
            model: 100
            GridButton {
                id: control
                enabled: buttonsEnabled
                onToggled: {
                    clickIndexSignal(index)
                }

            }
        }
    }
}

