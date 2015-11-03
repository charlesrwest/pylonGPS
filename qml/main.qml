import QtQuick 2.3
import QtQuick.Layouts 1.1

/**
Window size controlled from the C++ side of the application.
*/
Rectangle
{
id: root
width: 800; height: 800
color: "white"
//anchors.fill: parent

Rectangle
{
id: windowPanel
width: {return parent.width*2;}
height: parent.height;
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

color: "red"
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

color: "blue"
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


MouseArea 
{
anchors.fill: parent
onClicked: windowPanel.guiMode = !windowPanel.guiMode
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

color: "green"
anchors.right: parent.right;
}

}

}



