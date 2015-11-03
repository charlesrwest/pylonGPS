import QtQuick 2.3

/**
Window size controlled from the C++ side of the application.
*/
Rectangle
{
width: 200; height: 200
color: "white"
//border.color: "blue"
//anchors.fill: parent

Rectangle
{
id: one
width: 
{
if(thingy == true) 
{
return parent.width/4;
}
else 
{
return parent.width/2;
}
}
 
height: 
{
if(thingy == true) 
{
return parent.height/4;
}
else 
{
return parent.height/2;
}
}
color: "red"
anchors.horizontalCenter: parent.horizontalCenter;
anchors.verticalCenter: parent.verticalCenter;
property bool thingy: false

/*
onThingyChanged: 
{
if(thingy == true) 
{
width = parent.width/4;
height = parent.height/4;
}
else 
{
width = parent.width/2;
height = parent.height/2;
}
}
*/


MouseArea 
{
anchors.fill: parent
onClicked: parent.thingy = !parent.thingy
}

}

}



