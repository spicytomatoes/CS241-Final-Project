import QtQuick 2.0
import QtQuick.Window 2.14
import QtQuick.Controls 2.15
import QtLocation 5.15
import QtPositioning 5.6
import QtQml.Models 2.15

Window {
    id:root
    //corresponds to theGrid
    property variant region: "shape()"
    property double lngGap: 0
    property double latGap: 0

    signal selectStartSignal(msg: int)
    signal selectRouteSignal(msg: int)
    //initialize map grids
    function loadMap() {
        var topLeft = region.topLeft
        for (var i = 0; i < 10; i++) {
            for (var j = 0; j < 10; j++) {
                var tlLat = topLeft.latitude
                var tlLng = topLeft.longitude
                tlLat -= i*latGap
                tlLng += j*lngGap
                var brLat = tlLat
                var brLng = tlLng
                brLat -= latGap
                brLng += lngGap
                //console.log(tlLat,tlLng,brLat,brLng)
                mapGridModel.append({topLeftLat: tlLat, topLeftLng: tlLng,
                                        botRightLat: brLat, botRightLng: brLng})
            }
        }
    }
    //get heat map from c++ and draw it
    function loadHeatMap(heatMap, col) {
        var topLeft = region.topLeft
        var max = heatMap[100]

        heatMapModel.clear();
        for (var i = 0; i < 10; i++) {
            for (var j = 0; j < 10; j++) {
                var tlLat = topLeft.latitude
                var tlLng = topLeft.longitude
                tlLat -= i*latGap
                tlLng += j*lngGap
                var brLat = tlLat
                var brLng = tlLng
                brLat -= latGap
                brLng += lngGap
                //map heat to a value 0 and 0.6 as opacity
                var value = Math.sqrt(0.6*heatMap[10*i+j] / max)

                heatMapModel.append({topLeftLat: tlLat, topLeftLng: tlLng,
                                        botRightLat: brLat, botRightLng: brLng,
                                        heat: value, heatColor: col})
            }
        }
    }
    function unLoadHeatMap() {
        heatMapModel.clear();
    }
    function handleSelectGrid(coord) {
        var tlLat = region.topLeft.latitude
        var tlLng = region.topLeft.longitude

        //get gridID
        var x = (coord.longitude - tlLng) / lngGap
        var y = (tlLat - coord.latitude) / latGap
        x = Math.floor(x)
        y = Math.floor(y)
        if (x < 0 || x > 9 || y < 0 || y > 9) return;
        var gridID = 10*y + x

        //draw grid
        selectGridModel.clear();
        tlLat -= y*latGap
        tlLng += x*lngGap
        var brLat = tlLat
        var brLng = tlLng
        brLat -= latGap
        brLng += lngGap
        selectGridModel.append({topLeftLat: tlLat, topLeftLng: tlLng,
                                botRightLat: brLat, botRightLng: brLng,
                                col: "transparent", borderCol: "red", op:1})
        //send signal to c++
        selectStartSignal(gridID)
    }
    function handleSelectRoute(coord) {
        heatMapModel.clear();
        //clear selection and select new grid if there's already 2 selected
        if (selectGridModel.count == 2) {
            selectGridModel.clear()
            handleSelectGrid(coord)
            return
        }

        //change look of first grid
        selectGridModel.set(0,{"op":0.3,"col":"red"})

        var tlLat = region.topLeft.latitude
        var tlLng = region.topLeft.longitude

        //get gridID
        var x = (coord.longitude - tlLng) / lngGap
        var y = (tlLat - coord.latitude) / latGap
        x = Math.floor(x)
        y = Math.floor(y)
        if (x < 0 || x > 9 || y < 0 || y > 9) return;
        var gridID = 10*y + x

        //draw grid
        tlLat -= y*latGap
        tlLng += x*lngGap
        var brLat = tlLat
        var brLng = tlLng
        brLat -= latGap
        brLng += lngGap
        selectGridModel.append({topLeftLat: tlLat, topLeftLng: tlLng,
                                botRightLat: brLat, botRightLng: brLng,
                                col: "#00e600", borderCol: "#00e600", op:0.3})
        //send signal to c++
        selectRouteSignal(gridID)
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
        visibleRegion: region
        gesture.enabled: true
        //default grids
        MapItemView {
            model: mapGridModel
            delegate: MapRectangle {
                color: "transparent"
                border.width: 3
                border.color: "black"
                opacity: 0.3
                topLeft {
                    latitude: topLeftLat
                    longitude: topLeftLng
                }
                bottomRight {
                    latitude: botRightLat
                    longitude: botRightLng
                }
            }
        }
        //heat map grids
        MapItemView {
            model: heatMapModel
            delegate: MapRectangle {
                color: heatColor
                opacity: heat
                topLeft {
                    latitude: topLeftLat
                    longitude: topLeftLng
                }
                bottomRight {
                    latitude: botRightLat
                    longitude: botRightLng
                }
            }
        }
        //user selected grids
        MapItemView {
            model: selectGridModel
            delegate: MapRectangle {
                color: col
                opacity: op
                border.width: 3
                border.color: borderCol
                topLeft {
                    latitude: topLeftLat
                    longitude: topLeftLng
                }
                bottomRight {
                    latitude: botRightLat
                    longitude: botRightLng
                }
            }
        }
        MouseArea {
            anchors.fill: parent
            acceptedButtons: Qt.LeftButton | Qt.RightButton
            onClicked: {
                var coord = map.toCoordinate(Qt.point(mouse.x,mouse.y))

                if (mouse.button == Qt.LeftButton && selectGridModel.count == 0) {
                    handleSelectGrid(coord);
                }
                else if (mouse.button == Qt.LeftButton && selectGridModel.count != 0) {
                    handleSelectRoute(coord);
                }
                if (mouse.button == Qt.RightButton && selectGridModel.count != 0) {
                    selectGridModel.clear();
                    unLoadHeatMap();
                    selectStartSignal(-1);
                }
            }
        }
    }

    ListModel {
        id: mapGridModel
    }
    ListModel {
        id: heatMapModel
    }
    ListModel {
        id: selectGridModel
    }
}

