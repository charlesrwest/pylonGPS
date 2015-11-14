#include<qt4/QtGui/QApplication>
#include "transceiverGUI.hpp"

using namespace pylongps;

int main(int argc, char** argv)
{
QApplication app(argc, argv);

transceiverGUI *gui = new transceiverGUI();

gui->show();


return app.exec();
}


