/********************************************************************************
** Form generated from reading UI file 'transceiverFormFramework.ui'
**
** Created by: Qt User Interface Compiler version 4.8.6
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_TRANSCEIVERFORMFRAMEWORK_H
#define UI_TRANSCEIVERFORMFRAMEWORK_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QFrame>
#include <QtGui/QHBoxLayout>
#include <QtGui/QHeaderView>
#include <QtGui/QLabel>
#include <QtGui/QMainWindow>
#include <QtGui/QMenuBar>
#include <QtGui/QPushButton>
#include <QtGui/QSpacerItem>
#include <QtGui/QStackedWidget>
#include <QtGui/QVBoxLayout>
#include <QtGui/QWidget>

QT_BEGIN_NAMESPACE

class Ui_transceiverFormFramework
{
public:
    QWidget *mainWindowWidget;
    QHBoxLayout *horizontalLayout;
    QStackedWidget *mainWindowStackedWidget;
    QWidget *chooseDataReceiverPage;
    QHBoxLayout *horizontalLayout_2;
    QFrame *findCreateDataReceiverFrame;
    QVBoxLayout *verticalLayout;
    QLabel *label;
    QFrame *baseStationMap;
    QHBoxLayout *baseStationMapLayout;
    QFrame *selectedBasestationsFrame;
    QVBoxLayout *verticalLayout_2;
    QSpacerItem *verticalSpacer;
    QPushButton *toggleGUIPagePushButtonP1;
    QWidget *manageDataSenderPage;
    QHBoxLayout *horizontalLayout_3;
    QFrame *selectedDataReceiversFrame2;
    QVBoxLayout *verticalLayout_3;
    QLabel *label_2;
    QFrame *dataReceiverToDataSenderFrame;
    QFrame *dataSendersFrame;
    QVBoxLayout *verticalLayout_4;
    QSpacerItem *verticalSpacer_2;
    QPushButton *toggleGUIPagePushButtonP2;
    QMenuBar *menubar;

    void setupUi(QMainWindow *transceiverFormFramework)
    {
        if (transceiverFormFramework->objectName().isEmpty())
            transceiverFormFramework->setObjectName(QString::fromUtf8("transceiverFormFramework"));
        transceiverFormFramework->resize(800, 600);
        mainWindowWidget = new QWidget(transceiverFormFramework);
        mainWindowWidget->setObjectName(QString::fromUtf8("mainWindowWidget"));
        horizontalLayout = new QHBoxLayout(mainWindowWidget);
        horizontalLayout->setObjectName(QString::fromUtf8("horizontalLayout"));
        horizontalLayout->setContentsMargins(0, 0, 0, 0);
        mainWindowStackedWidget = new QStackedWidget(mainWindowWidget);
        mainWindowStackedWidget->setObjectName(QString::fromUtf8("mainWindowStackedWidget"));
        mainWindowStackedWidget->setLineWidth(0);
        chooseDataReceiverPage = new QWidget();
        chooseDataReceiverPage->setObjectName(QString::fromUtf8("chooseDataReceiverPage"));
        horizontalLayout_2 = new QHBoxLayout(chooseDataReceiverPage);
        horizontalLayout_2->setSpacing(0);
        horizontalLayout_2->setObjectName(QString::fromUtf8("horizontalLayout_2"));
        horizontalLayout_2->setContentsMargins(0, 0, 0, 0);
        findCreateDataReceiverFrame = new QFrame(chooseDataReceiverPage);
        findCreateDataReceiverFrame->setObjectName(QString::fromUtf8("findCreateDataReceiverFrame"));
        findCreateDataReceiverFrame->setFrameShape(QFrame::StyledPanel);
        findCreateDataReceiverFrame->setFrameShadow(QFrame::Raised);
        verticalLayout = new QVBoxLayout(findCreateDataReceiverFrame);
        verticalLayout->setSpacing(0);
        verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));
        verticalLayout->setContentsMargins(0, 0, 0, 0);
        label = new QLabel(findCreateDataReceiverFrame);
        label->setObjectName(QString::fromUtf8("label"));

        verticalLayout->addWidget(label);


        horizontalLayout_2->addWidget(findCreateDataReceiverFrame);

        baseStationMap = new QFrame(chooseDataReceiverPage);
        baseStationMap->setObjectName(QString::fromUtf8("baseStationMap"));
        QSizePolicy sizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(baseStationMap->sizePolicy().hasHeightForWidth());
        baseStationMap->setSizePolicy(sizePolicy);
        baseStationMap->setFrameShape(QFrame::StyledPanel);
        baseStationMap->setFrameShadow(QFrame::Raised);
        baseStationMapLayout = new QHBoxLayout(baseStationMap);
        baseStationMapLayout->setSpacing(0);
        baseStationMapLayout->setObjectName(QString::fromUtf8("baseStationMapLayout"));
        baseStationMapLayout->setContentsMargins(0, 0, 0, 0);

        horizontalLayout_2->addWidget(baseStationMap);

        selectedBasestationsFrame = new QFrame(chooseDataReceiverPage);
        selectedBasestationsFrame->setObjectName(QString::fromUtf8("selectedBasestationsFrame"));
        selectedBasestationsFrame->setFrameShape(QFrame::StyledPanel);
        selectedBasestationsFrame->setFrameShadow(QFrame::Raised);
        verticalLayout_2 = new QVBoxLayout(selectedBasestationsFrame);
        verticalLayout_2->setSpacing(0);
        verticalLayout_2->setObjectName(QString::fromUtf8("verticalLayout_2"));
        verticalLayout_2->setContentsMargins(0, 0, 0, 0);
        verticalSpacer = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);

        verticalLayout_2->addItem(verticalSpacer);

        toggleGUIPagePushButtonP1 = new QPushButton(selectedBasestationsFrame);
        toggleGUIPagePushButtonP1->setObjectName(QString::fromUtf8("toggleGUIPagePushButtonP1"));
        QSizePolicy sizePolicy1(QSizePolicy::Minimum, QSizePolicy::Minimum);
        sizePolicy1.setHorizontalStretch(0);
        sizePolicy1.setVerticalStretch(0);
        sizePolicy1.setHeightForWidth(toggleGUIPagePushButtonP1->sizePolicy().hasHeightForWidth());
        toggleGUIPagePushButtonP1->setSizePolicy(sizePolicy1);
        toggleGUIPagePushButtonP1->setMinimumSize(QSize(150, 60));

        verticalLayout_2->addWidget(toggleGUIPagePushButtonP1);


        horizontalLayout_2->addWidget(selectedBasestationsFrame);

        mainWindowStackedWidget->addWidget(chooseDataReceiverPage);
        manageDataSenderPage = new QWidget();
        manageDataSenderPage->setObjectName(QString::fromUtf8("manageDataSenderPage"));
        horizontalLayout_3 = new QHBoxLayout(manageDataSenderPage);
        horizontalLayout_3->setSpacing(0);
        horizontalLayout_3->setObjectName(QString::fromUtf8("horizontalLayout_3"));
        horizontalLayout_3->setContentsMargins(0, 0, 0, 0);
        selectedDataReceiversFrame2 = new QFrame(manageDataSenderPage);
        selectedDataReceiversFrame2->setObjectName(QString::fromUtf8("selectedDataReceiversFrame2"));
        selectedDataReceiversFrame2->setFrameShape(QFrame::StyledPanel);
        selectedDataReceiversFrame2->setFrameShadow(QFrame::Raised);
        verticalLayout_3 = new QVBoxLayout(selectedDataReceiversFrame2);
        verticalLayout_3->setSpacing(0);
        verticalLayout_3->setObjectName(QString::fromUtf8("verticalLayout_3"));
        verticalLayout_3->setContentsMargins(0, 0, 0, 0);
        label_2 = new QLabel(selectedDataReceiversFrame2);
        label_2->setObjectName(QString::fromUtf8("label_2"));

        verticalLayout_3->addWidget(label_2);


        horizontalLayout_3->addWidget(selectedDataReceiversFrame2);

        dataReceiverToDataSenderFrame = new QFrame(manageDataSenderPage);
        dataReceiverToDataSenderFrame->setObjectName(QString::fromUtf8("dataReceiverToDataSenderFrame"));
        sizePolicy.setHeightForWidth(dataReceiverToDataSenderFrame->sizePolicy().hasHeightForWidth());
        dataReceiverToDataSenderFrame->setSizePolicy(sizePolicy);
        dataReceiverToDataSenderFrame->setFrameShape(QFrame::StyledPanel);
        dataReceiverToDataSenderFrame->setFrameShadow(QFrame::Raised);

        horizontalLayout_3->addWidget(dataReceiverToDataSenderFrame);

        dataSendersFrame = new QFrame(manageDataSenderPage);
        dataSendersFrame->setObjectName(QString::fromUtf8("dataSendersFrame"));
        dataSendersFrame->setFrameShape(QFrame::StyledPanel);
        dataSendersFrame->setFrameShadow(QFrame::Raised);
        verticalLayout_4 = new QVBoxLayout(dataSendersFrame);
        verticalLayout_4->setSpacing(0);
        verticalLayout_4->setObjectName(QString::fromUtf8("verticalLayout_4"));
        verticalLayout_4->setContentsMargins(0, 0, 0, 0);
        verticalSpacer_2 = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);

        verticalLayout_4->addItem(verticalSpacer_2);

        toggleGUIPagePushButtonP2 = new QPushButton(dataSendersFrame);
        toggleGUIPagePushButtonP2->setObjectName(QString::fromUtf8("toggleGUIPagePushButtonP2"));
        toggleGUIPagePushButtonP2->setMinimumSize(QSize(150, 60));

        verticalLayout_4->addWidget(toggleGUIPagePushButtonP2);


        horizontalLayout_3->addWidget(dataSendersFrame);

        mainWindowStackedWidget->addWidget(manageDataSenderPage);

        horizontalLayout->addWidget(mainWindowStackedWidget);

        transceiverFormFramework->setCentralWidget(mainWindowWidget);
        menubar = new QMenuBar(transceiverFormFramework);
        menubar->setObjectName(QString::fromUtf8("menubar"));
        menubar->setGeometry(QRect(0, 0, 800, 25));
        transceiverFormFramework->setMenuBar(menubar);

        retranslateUi(transceiverFormFramework);

        mainWindowStackedWidget->setCurrentIndex(0);


        QMetaObject::connectSlotsByName(transceiverFormFramework);
    } // setupUi

    void retranslateUi(QMainWindow *transceiverFormFramework)
    {
        transceiverFormFramework->setWindowTitle(QApplication::translate("transceiverFormFramework", "Pylon GPS Transceiver - 2.0", 0, QApplication::UnicodeUTF8));
        label->setText(QApplication::translate("transceiverFormFramework", "DataReceiverSelection", 0, QApplication::UnicodeUTF8));
        toggleGUIPagePushButtonP1->setText(QApplication::translate("transceiverFormFramework", "Manage Senders", 0, QApplication::UnicodeUTF8));
        label_2->setText(QApplication::translate("transceiverFormFramework", "selectedDataReceivers", 0, QApplication::UnicodeUTF8));
        toggleGUIPagePushButtonP2->setText(QApplication::translate("transceiverFormFramework", "Manage Receivers", 0, QApplication::UnicodeUTF8));
    } // retranslateUi

};

namespace Ui {
    class transceiverFormFramework: public Ui_transceiverFormFramework {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_TRANSCEIVERFORMFRAMEWORK_H
