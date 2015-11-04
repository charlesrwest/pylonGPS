import QtQuick 2.3
import QtQuick.Layouts 1.1

/**
Window size controlled from the C++ side of the application.
*/
Rectangle
{
id: root
width: 800; height: 800
color: "#000000"
//anchors.fill: parent

Rectangle
{
id: windowPanel
width: {return parent.width*2;}
height: parent.height;
color: "#EEEEEE"

property bool guiMode: true
anchors.left: parent.left
anchors.leftMargin:
{
if(!guiMode)
{
return -parent.width;
}
else
{
return 0;
}
}

Behavior on anchors.leftMargin 
{
NumberAnimation 
{
duration: 200 //Milliseconds?
}
}

Rectangle
{
id: sourceSelectionMenuBar
width: 
{
return Math.min(Math.max(root.width/5, 150), 200);
}
 
height: root.height;
radius: 3

color: "#FBFBFB"
border.color: "#C8C8C8"
anchors.left: parent.left;
}

Rectangle
{
id: selectedSourceMenuBar
width: 
{
return Math.min(Math.max(root.width/5, 150), 200);
}
height: root.height;

color: "#FBFBFB"
border.color: "#C8C8C8"
radius: 3
anchors.right:  parent.horizontalCenter;
anchors.rightMargin: 
{
if(windowPanel.guiMode)
{
return 0;
}
else
{
return -width;
}
}

Behavior on anchors.rightMargin 
{
NumberAnimation 
{
duration: 200 //Milliseconds?
}
}

ColumnLayout
{
spacing: 0
DataSourceEntry {mode: windowPanel.guiMode}
DataSourceEntry {mode: windowPanel.guiMode}
DataSourceEntry {mode: windowPanel.guiMode}
DataSourceEntry {mode: windowPanel.guiMode}
DataSourceEntry {mode: windowPanel.guiMode}
DataSourceEntry {mode: windowPanel.guiMode}
DataSourceEntry {mode: windowPanel.guiMode}
DataSourceEntry {mode: windowPanel.guiMode}
DataSourceEntry {mode: windowPanel.guiMode}
DataSourceEntry {mode: windowPanel.guiMode}



}

Rectangle
{
id: guiModeChangeButton
width: parent.width
height: parent.height/10;
color: "#EEEEEE"
border.color: "#C8C8C8"

anchors.bottom:  parent.bottom;
radius: 3

MouseArea 
{
anchors.fill: parent
onClicked: windowPanel.guiMode = !windowPanel.guiMode
}
}

}

Rectangle
{
id: senderDisplayAndRemovalMenuBar
width: 
{
return Math.min(Math.max(root.width/5, 150), 200);
}
 
height: root.height;

color: "#FBFBFB"
border.color: "#C8C8C8"
anchors.right: parent.right;
}

}

}



