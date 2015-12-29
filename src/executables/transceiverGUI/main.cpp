#include<QApplication>
#include "transceiverGUI.hpp"
#include "IPBasedLocationRetriever.hpp"

using namespace pylongps;

int main(int argc, char** argv)
{
//IPBasedLocationRetriever bob;

//bob.operate();

QApplication app(argc, argv);

transceiverGUI *gui = new transceiverGUI();

gui->show();


return app.exec();
}


