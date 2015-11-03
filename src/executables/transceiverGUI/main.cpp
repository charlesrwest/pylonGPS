#include<QApplication>
#include<QQuickView>
#include<QUrl>
#include<memory>
#include "SOMException.hpp"

int main(int argc, char **argv)
{
QApplication app(argc, argv);
std::unique_ptr<QQuickView> widget;

SOM_TRY
widget.reset(new QQuickView);
SOM_CATCH("Error initializing widget\n")

QUrl source = QUrl::fromLocalFile("qml/main.qml");

widget->setSource(source);
//widget->setResizeMode(QQuickView::SizeViewToRootObject);
widget->setResizeMode(QQuickView::SizeRootObjectToView);

widget->show();

return app.exec();
} 
