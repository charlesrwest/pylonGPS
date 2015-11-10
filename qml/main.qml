import QtQuick 2.3
import QtPositioning 5.4
import QtLocation 5.4


Map {
    id: map

    plugin: somePlugin

    center {
        latitude: -27
        longitude: 153
    }
    zoomLevel: map.minimumZoomLevel

    gesture.enabled: true
}


Plugin {
    id: somePlugin
    // code here to choose the plugin as necessary
    name: "osm"
    PluginParameter { name: "osm.useragent"; value: "My great Qt OSM application" }
    PluginParameter { name: "osm.mapping.host"; value: "http://osm.tile.server.address/" }
    PluginParameter { name: "osm.mapping.copyright"; value: "All mine" }
    PluginParameter { name: "osm.routing.host"; value: "http://osrm.server.address/viaroute" }
    PluginParameter { name: "osm.geocoding.host"; value: "http://geocoding.server.address" }
}



