#include<qt4/QtGui/QApplication>
#include "casterGUI.hpp"

using namespace pylongps;

int main(int argc, char** argv)
{
QApplication app(argc, argv);

casterGUI *gui = new casterGUI();

gui->show();


return app.exec();
}


