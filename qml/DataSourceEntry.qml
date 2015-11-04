import QtQuick 2.3
import QtQuick.Layouts 1.1
import QtQuick.Controls 1.3

//Aparently the object names have to start with capital letters

Rectangle
{
property bool mode: true
color: mode ? "#A0A0A9" : "#A0A0A9";
border.color: "#C8C8C8"
radius: 5

Layout.alignment: Qt.AlignCenter
Layout.preferredWidth: selectedSourceMenuBar.width;
Layout.preferredHeight: mouseArea.containsMouse ? selectedSourceMenuBar.height*.2 : selectedSourceMenuBar.height*.05


Behavior on Layout.preferredHeight
{
NumberAnimation 
{
duration: 500 //Milliseconds?
}
}

MouseArea 
{
id: mouseArea
hoverEnabled: true
anchors.fill: parent
}

Label
{
color: "#FFFFFF"
anchors.left: parent.left;
anchors.leftMargin: 5
anchors.top: parent.top;
anchors.topMargin: 5
text: "Hello world";
}

}
