/********************************************************************************
** Form generated from reading UI file 'casterConfigurationWindowTemplate.ui'
**
** Created by: Qt User Interface Compiler version 4.8.6
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_CASTERCONFIGURATIONWINDOWTEMPLATE_H
#define UI_CASTERCONFIGURATIONWINDOWTEMPLATE_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QDateTimeEdit>
#include <QtGui/QFrame>
#include <QtGui/QGridLayout>
#include <QtGui/QHBoxLayout>
#include <QtGui/QHeaderView>
#include <QtGui/QLabel>
#include <QtGui/QLineEdit>
#include <QtGui/QMainWindow>
#include <QtGui/QMenuBar>
#include <QtGui/QPushButton>
#include <QtGui/QSpacerItem>
#include <QtGui/QStackedWidget>
#include <QtGui/QVBoxLayout>
#include <QtGui/QWidget>

QT_BEGIN_NAMESPACE

class Ui_casterConfigurationWindowTemplate
{
public:
    QWidget *mainWindowCenterWidget;
    QHBoxLayout *horizontalLayout;
    QStackedWidget *casterConfigurationStackedWidget;
    QWidget *configureCasterPage;
    QGridLayout *gridLayout_2;
    QLineEdit *clientRequestPortNumberLineEdit;
    QLineEdit *dataSenderPortNumberLineEdit;
    QSpacerItem *horizontalSpacer_3;
    QSpacerItem *horizontalSpacer_2;
    QSpacerItem *horizontalSpacer_5;
    QLabel *label_6;
    QSpacerItem *verticalSpacer_2;
    QSpacerItem *horizontalSpacer_7;
    QLabel *label_5;
    QLabel *label_3;
    QSpacerItem *horizontalSpacer_6;
    QLineEdit *clientStreamPublishingPortNumberLineEdit;
    QLineEdit *streamStatusNotificationPortNumberLineEdit;
    QLabel *label_8;
    QSpacerItem *horizontalSpacer;
    QLineEdit *casterIDLineEdit;
    QSpacerItem *horizontalSpacer_4;
    QLabel *label;
    QPushButton *saveCasterConfigurationPushButton;
    QLabel *label_4;
    QLabel *label_2;
    QLineEdit *keyManagementPortNumberLineEdit;
    QLabel *label_7;
    QLineEdit *proxyStreamPublishingPortNumberLineEdit;
    QLabel *label_9;
    QLabel *label_10;
    QSpacerItem *horizontalSpacer_8;
    QSpacerItem *horizontalSpacer_9;
    QSpacerItem *horizontalSpacer_10;
    QPushButton *selectCasterPublicSigningKeyPushButton;
    QPushButton *selectCasterPrivateSigningKeyPushButton;
    QPushButton *selectKeyManagementPublicSigningKeyPushButton;
    QWidget *createCredentialsPage;
    QGridLayout *gridLayout;
    QPushButton *createCredentialsPushButton_2;
    QDateTimeEdit *selectCredentialsExpirationDateTimeEdit;
    QSpacerItem *verticalSpacer_3;
    QLabel *label_14;
    QSpacerItem *horizontalSpacer_11;
    QSpacerItem *horizontalSpacer_13;
    QPushButton *selectCredentialsPublicSigningKey;
    QSpacerItem *horizontalSpacer_14;
    QLabel *label_13;
    QLineEdit *selectCredentialsBasestationLimit;
    QPushButton *selectCredentialsPublicKeyPushButton;
    QLabel *label_12;
    QSpacerItem *horizontalSpacer_12;
    QLabel *label_11;
    QLabel *label_15;
    QPushButton *selectCredentialsPrivateSigningKey;
    QSpacerItem *horizontalSpacer_15;
    QWidget *addRemoveKeysPage;
    QVBoxLayout *verticalLayout_2;
    QFrame *frame;
    QGridLayout *gridLayout_3;
    QSpacerItem *horizontalSpacer_18;
    QLabel *label_16;
    QLineEdit *casterIPAddressLineEdit;
    QLineEdit *casterPortLineEdit;
    QLabel *label_21;
    QPushButton *selectKeyManagementPrivateKeyPushButton;
    QSpacerItem *horizontalSpacer_17;
    QSpacerItem *horizontalSpacer_16;
    QLabel *label_17;
    QFrame *frame_2;
    QGridLayout *gridLayout_4;
    QSpacerItem *horizontalSpacer_19;
    QPushButton *addOfficialSigningKeyPushButton;
    QLabel *label_19;
    QSpacerItem *horizontalSpacer_20;
    QPushButton *selectOfficialPublicSigningKeyToAdd;
    QLabel *label_20;
    QDateTimeEdit *officialPublicKeyToAddExpirationDateTimeEdit;
    QFrame *frame_3;
    QGridLayout *gridLayout_5;
    QSpacerItem *horizontalSpacer_22;
    QPushButton *addRegisteredCommunitySigningKeyPushButton;
    QSpacerItem *horizontalSpacer_21;
    QLabel *label_22;
    QDateTimeEdit *registeredCommunityPublicKeyToAddExpirationTimeDateTimeEdit;
    QLabel *label_18;
    QPushButton *selectRegisteredCommunitySigningKeyToAddPushButton;
    QFrame *frame_4;
    QGridLayout *gridLayout_6;
    QLabel *label_23;
    QSpacerItem *horizontalSpacer_23;
    QPushButton *selectBlackListPublicKey;
    QLabel *label_24;
    QSpacerItem *horizontalSpacer_24;
    QDateTimeEdit *blacklistPublicKeyExpirationTimeDateTimeEdit;
    QPushButton *blacklistKeyPushButton;
    QWidget *widget;
    QFrame *modeFrame;
    QVBoxLayout *verticalLayout;
    QSpacerItem *verticalSpacer;
    QPushButton *configurationPushButton;
    QPushButton *generateKeysPushButton;
    QPushButton *createCredentialsPushButton;
    QPushButton *addRemoveKeysPushButton;
    QMenuBar *menubar;

    void setupUi(QMainWindow *casterConfigurationWindowTemplate)
    {
        if (casterConfigurationWindowTemplate->objectName().isEmpty())
            casterConfigurationWindowTemplate->setObjectName(QString::fromUtf8("casterConfigurationWindowTemplate"));
        casterConfigurationWindowTemplate->resize(800, 600);
        mainWindowCenterWidget = new QWidget(casterConfigurationWindowTemplate);
        mainWindowCenterWidget->setObjectName(QString::fromUtf8("mainWindowCenterWidget"));
        horizontalLayout = new QHBoxLayout(mainWindowCenterWidget);
        horizontalLayout->setSpacing(0);
        horizontalLayout->setObjectName(QString::fromUtf8("horizontalLayout"));
        horizontalLayout->setContentsMargins(0, 0, 0, 0);
        casterConfigurationStackedWidget = new QStackedWidget(mainWindowCenterWidget);
        casterConfigurationStackedWidget->setObjectName(QString::fromUtf8("casterConfigurationStackedWidget"));
        configureCasterPage = new QWidget();
        configureCasterPage->setObjectName(QString::fromUtf8("configureCasterPage"));
        configureCasterPage->setLayoutDirection(Qt::LeftToRight);
        gridLayout_2 = new QGridLayout(configureCasterPage);
        gridLayout_2->setObjectName(QString::fromUtf8("gridLayout_2"));
        gridLayout_2->setContentsMargins(9, 9, 9, 9);
        clientRequestPortNumberLineEdit = new QLineEdit(configureCasterPage);
        clientRequestPortNumberLineEdit->setObjectName(QString::fromUtf8("clientRequestPortNumberLineEdit"));
        clientRequestPortNumberLineEdit->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        gridLayout_2->addWidget(clientRequestPortNumberLineEdit, 2, 3, 1, 1);

        dataSenderPortNumberLineEdit = new QLineEdit(configureCasterPage);
        dataSenderPortNumberLineEdit->setObjectName(QString::fromUtf8("dataSenderPortNumberLineEdit"));
        dataSenderPortNumberLineEdit->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        gridLayout_2->addWidget(dataSenderPortNumberLineEdit, 1, 3, 1, 1);

        horizontalSpacer_3 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        gridLayout_2->addItem(horizontalSpacer_3, 2, 2, 1, 1);

        horizontalSpacer_2 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        gridLayout_2->addItem(horizontalSpacer_2, 1, 2, 1, 1);

        horizontalSpacer_5 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        gridLayout_2->addItem(horizontalSpacer_5, 4, 2, 1, 1);

        label_6 = new QLabel(configureCasterPage);
        label_6->setObjectName(QString::fromUtf8("label_6"));

        gridLayout_2->addWidget(label_6, 5, 1, 1, 1);

        verticalSpacer_2 = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);

        gridLayout_2->addItem(verticalSpacer_2, 10, 1, 1, 1);

        horizontalSpacer_7 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        gridLayout_2->addItem(horizontalSpacer_7, 6, 2, 1, 1);

        label_5 = new QLabel(configureCasterPage);
        label_5->setObjectName(QString::fromUtf8("label_5"));

        gridLayout_2->addWidget(label_5, 4, 1, 1, 1);

        label_3 = new QLabel(configureCasterPage);
        label_3->setObjectName(QString::fromUtf8("label_3"));

        gridLayout_2->addWidget(label_3, 2, 1, 1, 1);

        horizontalSpacer_6 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        gridLayout_2->addItem(horizontalSpacer_6, 5, 2, 1, 1);

        clientStreamPublishingPortNumberLineEdit = new QLineEdit(configureCasterPage);
        clientStreamPublishingPortNumberLineEdit->setObjectName(QString::fromUtf8("clientStreamPublishingPortNumberLineEdit"));
        clientStreamPublishingPortNumberLineEdit->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        gridLayout_2->addWidget(clientStreamPublishingPortNumberLineEdit, 3, 3, 1, 1);

        streamStatusNotificationPortNumberLineEdit = new QLineEdit(configureCasterPage);
        streamStatusNotificationPortNumberLineEdit->setObjectName(QString::fromUtf8("streamStatusNotificationPortNumberLineEdit"));
        streamStatusNotificationPortNumberLineEdit->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        gridLayout_2->addWidget(streamStatusNotificationPortNumberLineEdit, 5, 3, 1, 1);

        label_8 = new QLabel(configureCasterPage);
        label_8->setObjectName(QString::fromUtf8("label_8"));

        gridLayout_2->addWidget(label_8, 7, 1, 1, 1);

        horizontalSpacer = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        gridLayout_2->addItem(horizontalSpacer, 0, 2, 1, 1);

        casterIDLineEdit = new QLineEdit(configureCasterPage);
        casterIDLineEdit->setObjectName(QString::fromUtf8("casterIDLineEdit"));
        QSizePolicy sizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(casterIDLineEdit->sizePolicy().hasHeightForWidth());
        casterIDLineEdit->setSizePolicy(sizePolicy);
        casterIDLineEdit->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        gridLayout_2->addWidget(casterIDLineEdit, 0, 3, 1, 1);

        horizontalSpacer_4 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        gridLayout_2->addItem(horizontalSpacer_4, 3, 2, 1, 1);

        label = new QLabel(configureCasterPage);
        label->setObjectName(QString::fromUtf8("label"));

        gridLayout_2->addWidget(label, 1, 1, 1, 1);

        saveCasterConfigurationPushButton = new QPushButton(configureCasterPage);
        saveCasterConfigurationPushButton->setObjectName(QString::fromUtf8("saveCasterConfigurationPushButton"));
        saveCasterConfigurationPushButton->setMinimumSize(QSize(150, 40));

        gridLayout_2->addWidget(saveCasterConfigurationPushButton, 11, 2, 1, 1);

        label_4 = new QLabel(configureCasterPage);
        label_4->setObjectName(QString::fromUtf8("label_4"));

        gridLayout_2->addWidget(label_4, 3, 1, 1, 1);

        label_2 = new QLabel(configureCasterPage);
        label_2->setObjectName(QString::fromUtf8("label_2"));

        gridLayout_2->addWidget(label_2, 0, 1, 1, 1);

        keyManagementPortNumberLineEdit = new QLineEdit(configureCasterPage);
        keyManagementPortNumberLineEdit->setObjectName(QString::fromUtf8("keyManagementPortNumberLineEdit"));
        keyManagementPortNumberLineEdit->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        gridLayout_2->addWidget(keyManagementPortNumberLineEdit, 6, 3, 1, 1);

        label_7 = new QLabel(configureCasterPage);
        label_7->setObjectName(QString::fromUtf8("label_7"));

        gridLayout_2->addWidget(label_7, 6, 1, 1, 1);

        proxyStreamPublishingPortNumberLineEdit = new QLineEdit(configureCasterPage);
        proxyStreamPublishingPortNumberLineEdit->setObjectName(QString::fromUtf8("proxyStreamPublishingPortNumberLineEdit"));
        proxyStreamPublishingPortNumberLineEdit->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        gridLayout_2->addWidget(proxyStreamPublishingPortNumberLineEdit, 4, 3, 1, 1);

        label_9 = new QLabel(configureCasterPage);
        label_9->setObjectName(QString::fromUtf8("label_9"));

        gridLayout_2->addWidget(label_9, 8, 1, 1, 1);

        label_10 = new QLabel(configureCasterPage);
        label_10->setObjectName(QString::fromUtf8("label_10"));

        gridLayout_2->addWidget(label_10, 9, 1, 1, 1);

        horizontalSpacer_8 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        gridLayout_2->addItem(horizontalSpacer_8, 7, 2, 1, 1);

        horizontalSpacer_9 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        gridLayout_2->addItem(horizontalSpacer_9, 8, 2, 1, 1);

        horizontalSpacer_10 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        gridLayout_2->addItem(horizontalSpacer_10, 9, 2, 1, 1);

        selectCasterPublicSigningKeyPushButton = new QPushButton(configureCasterPage);
        selectCasterPublicSigningKeyPushButton->setObjectName(QString::fromUtf8("selectCasterPublicSigningKeyPushButton"));
        sizePolicy.setHeightForWidth(selectCasterPublicSigningKeyPushButton->sizePolicy().hasHeightForWidth());
        selectCasterPublicSigningKeyPushButton->setSizePolicy(sizePolicy);

        gridLayout_2->addWidget(selectCasterPublicSigningKeyPushButton, 7, 3, 1, 1);

        selectCasterPrivateSigningKeyPushButton = new QPushButton(configureCasterPage);
        selectCasterPrivateSigningKeyPushButton->setObjectName(QString::fromUtf8("selectCasterPrivateSigningKeyPushButton"));
        sizePolicy.setHeightForWidth(selectCasterPrivateSigningKeyPushButton->sizePolicy().hasHeightForWidth());
        selectCasterPrivateSigningKeyPushButton->setSizePolicy(sizePolicy);

        gridLayout_2->addWidget(selectCasterPrivateSigningKeyPushButton, 8, 3, 1, 1);

        selectKeyManagementPublicSigningKeyPushButton = new QPushButton(configureCasterPage);
        selectKeyManagementPublicSigningKeyPushButton->setObjectName(QString::fromUtf8("selectKeyManagementPublicSigningKeyPushButton"));
        sizePolicy.setHeightForWidth(selectKeyManagementPublicSigningKeyPushButton->sizePolicy().hasHeightForWidth());
        selectKeyManagementPublicSigningKeyPushButton->setSizePolicy(sizePolicy);

        gridLayout_2->addWidget(selectKeyManagementPublicSigningKeyPushButton, 9, 3, 1, 1);

        casterConfigurationStackedWidget->addWidget(configureCasterPage);
        createCredentialsPage = new QWidget();
        createCredentialsPage->setObjectName(QString::fromUtf8("createCredentialsPage"));
        gridLayout = new QGridLayout(createCredentialsPage);
        gridLayout->setObjectName(QString::fromUtf8("gridLayout"));
        createCredentialsPushButton_2 = new QPushButton(createCredentialsPage);
        createCredentialsPushButton_2->setObjectName(QString::fromUtf8("createCredentialsPushButton_2"));
        createCredentialsPushButton_2->setMinimumSize(QSize(150, 40));

        gridLayout->addWidget(createCredentialsPushButton_2, 6, 2, 1, 1);

        selectCredentialsExpirationDateTimeEdit = new QDateTimeEdit(createCredentialsPage);
        selectCredentialsExpirationDateTimeEdit->setObjectName(QString::fromUtf8("selectCredentialsExpirationDateTimeEdit"));
        selectCredentialsExpirationDateTimeEdit->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);
        selectCredentialsExpirationDateTimeEdit->setCalendarPopup(true);

        gridLayout->addWidget(selectCredentialsExpirationDateTimeEdit, 2, 2, 1, 1);

        verticalSpacer_3 = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);

        gridLayout->addItem(verticalSpacer_3, 5, 0, 1, 1);

        label_14 = new QLabel(createCredentialsPage);
        label_14->setObjectName(QString::fromUtf8("label_14"));

        gridLayout->addWidget(label_14, 3, 0, 1, 1);

        horizontalSpacer_11 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        gridLayout->addItem(horizontalSpacer_11, 0, 1, 1, 1);

        horizontalSpacer_13 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        gridLayout->addItem(horizontalSpacer_13, 2, 1, 1, 1);

        selectCredentialsPublicSigningKey = new QPushButton(createCredentialsPage);
        selectCredentialsPublicSigningKey->setObjectName(QString::fromUtf8("selectCredentialsPublicSigningKey"));
        selectCredentialsPublicSigningKey->setMinimumSize(QSize(0, 0));

        gridLayout->addWidget(selectCredentialsPublicSigningKey, 3, 2, 1, 1);

        horizontalSpacer_14 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        gridLayout->addItem(horizontalSpacer_14, 3, 1, 1, 1);

        label_13 = new QLabel(createCredentialsPage);
        label_13->setObjectName(QString::fromUtf8("label_13"));

        gridLayout->addWidget(label_13, 1, 0, 1, 1);

        selectCredentialsBasestationLimit = new QLineEdit(createCredentialsPage);
        selectCredentialsBasestationLimit->setObjectName(QString::fromUtf8("selectCredentialsBasestationLimit"));
        selectCredentialsBasestationLimit->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        gridLayout->addWidget(selectCredentialsBasestationLimit, 1, 2, 1, 1);

        selectCredentialsPublicKeyPushButton = new QPushButton(createCredentialsPage);
        selectCredentialsPublicKeyPushButton->setObjectName(QString::fromUtf8("selectCredentialsPublicKeyPushButton"));
        selectCredentialsPublicKeyPushButton->setMinimumSize(QSize(0, 0));

        gridLayout->addWidget(selectCredentialsPublicKeyPushButton, 0, 2, 1, 1);

        label_12 = new QLabel(createCredentialsPage);
        label_12->setObjectName(QString::fromUtf8("label_12"));

        gridLayout->addWidget(label_12, 2, 0, 1, 1);

        horizontalSpacer_12 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        gridLayout->addItem(horizontalSpacer_12, 1, 1, 1, 1);

        label_11 = new QLabel(createCredentialsPage);
        label_11->setObjectName(QString::fromUtf8("label_11"));

        gridLayout->addWidget(label_11, 0, 0, 1, 1);

        label_15 = new QLabel(createCredentialsPage);
        label_15->setObjectName(QString::fromUtf8("label_15"));

        gridLayout->addWidget(label_15, 4, 0, 1, 1);

        selectCredentialsPrivateSigningKey = new QPushButton(createCredentialsPage);
        selectCredentialsPrivateSigningKey->setObjectName(QString::fromUtf8("selectCredentialsPrivateSigningKey"));
        selectCredentialsPrivateSigningKey->setMinimumSize(QSize(0, 0));

        gridLayout->addWidget(selectCredentialsPrivateSigningKey, 4, 2, 1, 1);

        horizontalSpacer_15 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        gridLayout->addItem(horizontalSpacer_15, 4, 1, 1, 1);

        casterConfigurationStackedWidget->addWidget(createCredentialsPage);
        addRemoveKeysPage = new QWidget();
        addRemoveKeysPage->setObjectName(QString::fromUtf8("addRemoveKeysPage"));
        verticalLayout_2 = new QVBoxLayout(addRemoveKeysPage);
        verticalLayout_2->setSpacing(6);
        verticalLayout_2->setObjectName(QString::fromUtf8("verticalLayout_2"));
        verticalLayout_2->setContentsMargins(0, 0, 0, 0);
        frame = new QFrame(addRemoveKeysPage);
        frame->setObjectName(QString::fromUtf8("frame"));
        frame->setFrameShape(QFrame::StyledPanel);
        frame->setFrameShadow(QFrame::Raised);
        gridLayout_3 = new QGridLayout(frame);
        gridLayout_3->setObjectName(QString::fromUtf8("gridLayout_3"));
        horizontalSpacer_18 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        gridLayout_3->addItem(horizontalSpacer_18, 3, 1, 1, 1);

        label_16 = new QLabel(frame);
        label_16->setObjectName(QString::fromUtf8("label_16"));

        gridLayout_3->addWidget(label_16, 0, 0, 1, 1);

        casterIPAddressLineEdit = new QLineEdit(frame);
        casterIPAddressLineEdit->setObjectName(QString::fromUtf8("casterIPAddressLineEdit"));
        casterIPAddressLineEdit->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        gridLayout_3->addWidget(casterIPAddressLineEdit, 0, 2, 1, 1);

        casterPortLineEdit = new QLineEdit(frame);
        casterPortLineEdit->setObjectName(QString::fromUtf8("casterPortLineEdit"));
        casterPortLineEdit->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        gridLayout_3->addWidget(casterPortLineEdit, 2, 2, 1, 1);

        label_21 = new QLabel(frame);
        label_21->setObjectName(QString::fromUtf8("label_21"));

        gridLayout_3->addWidget(label_21, 3, 0, 1, 1);

        selectKeyManagementPrivateKeyPushButton = new QPushButton(frame);
        selectKeyManagementPrivateKeyPushButton->setObjectName(QString::fromUtf8("selectKeyManagementPrivateKeyPushButton"));
        selectKeyManagementPrivateKeyPushButton->setMinimumSize(QSize(230, 0));

        gridLayout_3->addWidget(selectKeyManagementPrivateKeyPushButton, 3, 2, 1, 1);

        horizontalSpacer_17 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        gridLayout_3->addItem(horizontalSpacer_17, 2, 1, 1, 1);

        horizontalSpacer_16 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        gridLayout_3->addItem(horizontalSpacer_16, 0, 1, 1, 1);

        label_17 = new QLabel(frame);
        label_17->setObjectName(QString::fromUtf8("label_17"));

        gridLayout_3->addWidget(label_17, 2, 0, 1, 1);


        verticalLayout_2->addWidget(frame);

        frame_2 = new QFrame(addRemoveKeysPage);
        frame_2->setObjectName(QString::fromUtf8("frame_2"));
        frame_2->setFrameShape(QFrame::StyledPanel);
        frame_2->setFrameShadow(QFrame::Raised);
        gridLayout_4 = new QGridLayout(frame_2);
        gridLayout_4->setObjectName(QString::fromUtf8("gridLayout_4"));
        horizontalSpacer_19 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        gridLayout_4->addItem(horizontalSpacer_19, 0, 1, 1, 1);

        addOfficialSigningKeyPushButton = new QPushButton(frame_2);
        addOfficialSigningKeyPushButton->setObjectName(QString::fromUtf8("addOfficialSigningKeyPushButton"));
        addOfficialSigningKeyPushButton->setMinimumSize(QSize(230, 0));

        gridLayout_4->addWidget(addOfficialSigningKeyPushButton, 5, 2, 1, 1);

        label_19 = new QLabel(frame_2);
        label_19->setObjectName(QString::fromUtf8("label_19"));

        gridLayout_4->addWidget(label_19, 0, 0, 1, 1);

        horizontalSpacer_20 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        gridLayout_4->addItem(horizontalSpacer_20, 3, 1, 1, 1);

        selectOfficialPublicSigningKeyToAdd = new QPushButton(frame_2);
        selectOfficialPublicSigningKeyToAdd->setObjectName(QString::fromUtf8("selectOfficialPublicSigningKeyToAdd"));

        gridLayout_4->addWidget(selectOfficialPublicSigningKeyToAdd, 0, 2, 1, 1);

        label_20 = new QLabel(frame_2);
        label_20->setObjectName(QString::fromUtf8("label_20"));

        gridLayout_4->addWidget(label_20, 3, 0, 1, 1);

        officialPublicKeyToAddExpirationDateTimeEdit = new QDateTimeEdit(frame_2);
        officialPublicKeyToAddExpirationDateTimeEdit->setObjectName(QString::fromUtf8("officialPublicKeyToAddExpirationDateTimeEdit"));
        officialPublicKeyToAddExpirationDateTimeEdit->setCalendarPopup(true);

        gridLayout_4->addWidget(officialPublicKeyToAddExpirationDateTimeEdit, 3, 2, 1, 1);


        verticalLayout_2->addWidget(frame_2);

        frame_3 = new QFrame(addRemoveKeysPage);
        frame_3->setObjectName(QString::fromUtf8("frame_3"));
        frame_3->setFrameShape(QFrame::StyledPanel);
        frame_3->setFrameShadow(QFrame::Raised);
        gridLayout_5 = new QGridLayout(frame_3);
        gridLayout_5->setObjectName(QString::fromUtf8("gridLayout_5"));
        horizontalSpacer_22 = new QSpacerItem(225, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        gridLayout_5->addItem(horizontalSpacer_22, 1, 1, 1, 1);

        addRegisteredCommunitySigningKeyPushButton = new QPushButton(frame_3);
        addRegisteredCommunitySigningKeyPushButton->setObjectName(QString::fromUtf8("addRegisteredCommunitySigningKeyPushButton"));
        addRegisteredCommunitySigningKeyPushButton->setMinimumSize(QSize(230, 0));

        gridLayout_5->addWidget(addRegisteredCommunitySigningKeyPushButton, 2, 2, 1, 1);

        horizontalSpacer_21 = new QSpacerItem(225, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        gridLayout_5->addItem(horizontalSpacer_21, 0, 1, 1, 1);

        label_22 = new QLabel(frame_3);
        label_22->setObjectName(QString::fromUtf8("label_22"));

        gridLayout_5->addWidget(label_22, 1, 0, 1, 1);

        registeredCommunityPublicKeyToAddExpirationTimeDateTimeEdit = new QDateTimeEdit(frame_3);
        registeredCommunityPublicKeyToAddExpirationTimeDateTimeEdit->setObjectName(QString::fromUtf8("registeredCommunityPublicKeyToAddExpirationTimeDateTimeEdit"));
        registeredCommunityPublicKeyToAddExpirationTimeDateTimeEdit->setCalendarPopup(true);

        gridLayout_5->addWidget(registeredCommunityPublicKeyToAddExpirationTimeDateTimeEdit, 1, 2, 1, 1);

        label_18 = new QLabel(frame_3);
        label_18->setObjectName(QString::fromUtf8("label_18"));

        gridLayout_5->addWidget(label_18, 0, 0, 1, 1);

        selectRegisteredCommunitySigningKeyToAddPushButton = new QPushButton(frame_3);
        selectRegisteredCommunitySigningKeyToAddPushButton->setObjectName(QString::fromUtf8("selectRegisteredCommunitySigningKeyToAddPushButton"));

        gridLayout_5->addWidget(selectRegisteredCommunitySigningKeyToAddPushButton, 0, 2, 1, 1);


        verticalLayout_2->addWidget(frame_3);

        frame_4 = new QFrame(addRemoveKeysPage);
        frame_4->setObjectName(QString::fromUtf8("frame_4"));
        frame_4->setFrameShape(QFrame::StyledPanel);
        frame_4->setFrameShadow(QFrame::Raised);
        gridLayout_6 = new QGridLayout(frame_4);
        gridLayout_6->setObjectName(QString::fromUtf8("gridLayout_6"));
        label_23 = new QLabel(frame_4);
        label_23->setObjectName(QString::fromUtf8("label_23"));

        gridLayout_6->addWidget(label_23, 0, 0, 1, 1);

        horizontalSpacer_23 = new QSpacerItem(225, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        gridLayout_6->addItem(horizontalSpacer_23, 0, 1, 1, 1);

        selectBlackListPublicKey = new QPushButton(frame_4);
        selectBlackListPublicKey->setObjectName(QString::fromUtf8("selectBlackListPublicKey"));
        selectBlackListPublicKey->setMinimumSize(QSize(230, 0));

        gridLayout_6->addWidget(selectBlackListPublicKey, 0, 2, 1, 1);

        label_24 = new QLabel(frame_4);
        label_24->setObjectName(QString::fromUtf8("label_24"));

        gridLayout_6->addWidget(label_24, 1, 0, 1, 1);

        horizontalSpacer_24 = new QSpacerItem(225, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        gridLayout_6->addItem(horizontalSpacer_24, 1, 1, 1, 1);

        blacklistPublicKeyExpirationTimeDateTimeEdit = new QDateTimeEdit(frame_4);
        blacklistPublicKeyExpirationTimeDateTimeEdit->setObjectName(QString::fromUtf8("blacklistPublicKeyExpirationTimeDateTimeEdit"));
        blacklistPublicKeyExpirationTimeDateTimeEdit->setCalendarPopup(true);

        gridLayout_6->addWidget(blacklistPublicKeyExpirationTimeDateTimeEdit, 1, 2, 1, 1);

        blacklistKeyPushButton = new QPushButton(frame_4);
        blacklistKeyPushButton->setObjectName(QString::fromUtf8("blacklistKeyPushButton"));
        blacklistKeyPushButton->setMinimumSize(QSize(230, 0));

        gridLayout_6->addWidget(blacklistKeyPushButton, 2, 2, 1, 1);


        verticalLayout_2->addWidget(frame_4);

        casterConfigurationStackedWidget->addWidget(addRemoveKeysPage);
        widget = new QWidget();
        widget->setObjectName(QString::fromUtf8("widget"));
        casterConfigurationStackedWidget->addWidget(widget);

        horizontalLayout->addWidget(casterConfigurationStackedWidget);

        modeFrame = new QFrame(mainWindowCenterWidget);
        modeFrame->setObjectName(QString::fromUtf8("modeFrame"));
        modeFrame->setFrameShape(QFrame::NoFrame);
        modeFrame->setFrameShadow(QFrame::Raised);
        modeFrame->setLineWidth(0);
        verticalLayout = new QVBoxLayout(modeFrame);
        verticalLayout->setSpacing(2);
        verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));
        verticalLayout->setContentsMargins(9, 9, 9, 9);
        verticalSpacer = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);

        verticalLayout->addItem(verticalSpacer);

        configurationPushButton = new QPushButton(modeFrame);
        configurationPushButton->setObjectName(QString::fromUtf8("configurationPushButton"));
        configurationPushButton->setMinimumSize(QSize(150, 40));

        verticalLayout->addWidget(configurationPushButton);

        generateKeysPushButton = new QPushButton(modeFrame);
        generateKeysPushButton->setObjectName(QString::fromUtf8("generateKeysPushButton"));
        generateKeysPushButton->setMinimumSize(QSize(150, 40));

        verticalLayout->addWidget(generateKeysPushButton);

        createCredentialsPushButton = new QPushButton(modeFrame);
        createCredentialsPushButton->setObjectName(QString::fromUtf8("createCredentialsPushButton"));
        createCredentialsPushButton->setMinimumSize(QSize(150, 40));

        verticalLayout->addWidget(createCredentialsPushButton);

        addRemoveKeysPushButton = new QPushButton(modeFrame);
        addRemoveKeysPushButton->setObjectName(QString::fromUtf8("addRemoveKeysPushButton"));
        addRemoveKeysPushButton->setMinimumSize(QSize(150, 40));

        verticalLayout->addWidget(addRemoveKeysPushButton);


        horizontalLayout->addWidget(modeFrame);

        casterConfigurationWindowTemplate->setCentralWidget(mainWindowCenterWidget);
        menubar = new QMenuBar(casterConfigurationWindowTemplate);
        menubar->setObjectName(QString::fromUtf8("menubar"));
        menubar->setGeometry(QRect(0, 0, 800, 25));
        casterConfigurationWindowTemplate->setMenuBar(menubar);
        QWidget::setTabOrder(casterIDLineEdit, dataSenderPortNumberLineEdit);
        QWidget::setTabOrder(dataSenderPortNumberLineEdit, clientRequestPortNumberLineEdit);
        QWidget::setTabOrder(clientRequestPortNumberLineEdit, clientStreamPublishingPortNumberLineEdit);
        QWidget::setTabOrder(clientStreamPublishingPortNumberLineEdit, proxyStreamPublishingPortNumberLineEdit);
        QWidget::setTabOrder(proxyStreamPublishingPortNumberLineEdit, streamStatusNotificationPortNumberLineEdit);
        QWidget::setTabOrder(streamStatusNotificationPortNumberLineEdit, keyManagementPortNumberLineEdit);
        QWidget::setTabOrder(keyManagementPortNumberLineEdit, selectCasterPublicSigningKeyPushButton);
        QWidget::setTabOrder(selectCasterPublicSigningKeyPushButton, selectCasterPrivateSigningKeyPushButton);
        QWidget::setTabOrder(selectCasterPrivateSigningKeyPushButton, selectKeyManagementPublicSigningKeyPushButton);
        QWidget::setTabOrder(selectKeyManagementPublicSigningKeyPushButton, saveCasterConfigurationPushButton);
        QWidget::setTabOrder(saveCasterConfigurationPushButton, configurationPushButton);
        QWidget::setTabOrder(configurationPushButton, generateKeysPushButton);
        QWidget::setTabOrder(generateKeysPushButton, createCredentialsPushButton);
        QWidget::setTabOrder(createCredentialsPushButton, addRemoveKeysPushButton);
        QWidget::setTabOrder(addRemoveKeysPushButton, selectCredentialsPublicKeyPushButton);
        QWidget::setTabOrder(selectCredentialsPublicKeyPushButton, selectCredentialsPrivateSigningKey);
        QWidget::setTabOrder(selectCredentialsPrivateSigningKey, casterIPAddressLineEdit);
        QWidget::setTabOrder(casterIPAddressLineEdit, casterPortLineEdit);
        QWidget::setTabOrder(casterPortLineEdit, createCredentialsPushButton_2);
        QWidget::setTabOrder(createCredentialsPushButton_2, selectCredentialsExpirationDateTimeEdit);
        QWidget::setTabOrder(selectCredentialsExpirationDateTimeEdit, selectCredentialsPublicSigningKey);
        QWidget::setTabOrder(selectCredentialsPublicSigningKey, selectCredentialsBasestationLimit);
        QWidget::setTabOrder(selectCredentialsBasestationLimit, selectKeyManagementPrivateKeyPushButton);
        QWidget::setTabOrder(selectKeyManagementPrivateKeyPushButton, selectOfficialPublicSigningKeyToAdd);
        QWidget::setTabOrder(selectOfficialPublicSigningKeyToAdd, officialPublicKeyToAddExpirationDateTimeEdit);
        QWidget::setTabOrder(officialPublicKeyToAddExpirationDateTimeEdit, addOfficialSigningKeyPushButton);
        QWidget::setTabOrder(addOfficialSigningKeyPushButton, selectRegisteredCommunitySigningKeyToAddPushButton);
        QWidget::setTabOrder(selectRegisteredCommunitySigningKeyToAddPushButton, addRegisteredCommunitySigningKeyPushButton);
        QWidget::setTabOrder(addRegisteredCommunitySigningKeyPushButton, selectBlackListPublicKey);
        QWidget::setTabOrder(selectBlackListPublicKey, blacklistKeyPushButton);
        QWidget::setTabOrder(blacklistKeyPushButton, registeredCommunityPublicKeyToAddExpirationTimeDateTimeEdit);
        QWidget::setTabOrder(registeredCommunityPublicKeyToAddExpirationTimeDateTimeEdit, blacklistPublicKeyExpirationTimeDateTimeEdit);

        retranslateUi(casterConfigurationWindowTemplate);

        casterConfigurationStackedWidget->setCurrentIndex(0);


        QMetaObject::connectSlotsByName(casterConfigurationWindowTemplate);
    } // setupUi

    void retranslateUi(QMainWindow *casterConfigurationWindowTemplate)
    {
        casterConfigurationWindowTemplate->setWindowTitle(QApplication::translate("casterConfigurationWindowTemplate", "Pylon GPS 2.0 Caster Configuration", 0, QApplication::UnicodeUTF8));
        clientRequestPortNumberLineEdit->setText(QApplication::translate("casterConfigurationWindowTemplate", "10002", 0, QApplication::UnicodeUTF8));
        clientRequestPortNumberLineEdit->setPlaceholderText(QString());
        dataSenderPortNumberLineEdit->setText(QApplication::translate("casterConfigurationWindowTemplate", "10001", 0, QApplication::UnicodeUTF8));
        dataSenderPortNumberLineEdit->setPlaceholderText(QString());
        label_6->setText(QApplication::translate("casterConfigurationWindowTemplate", "Stream Status Notification Port Number", 0, QApplication::UnicodeUTF8));
        label_5->setText(QApplication::translate("casterConfigurationWindowTemplate", "Proxy Stream Publishing Port Number", 0, QApplication::UnicodeUTF8));
        label_3->setText(QApplication::translate("casterConfigurationWindowTemplate", "Client Request Port Number", 0, QApplication::UnicodeUTF8));
        clientStreamPublishingPortNumberLineEdit->setText(QApplication::translate("casterConfigurationWindowTemplate", "10003", 0, QApplication::UnicodeUTF8));
        clientStreamPublishingPortNumberLineEdit->setPlaceholderText(QString());
        streamStatusNotificationPortNumberLineEdit->setText(QApplication::translate("casterConfigurationWindowTemplate", "10005", 0, QApplication::UnicodeUTF8));
        label_8->setText(QApplication::translate("casterConfigurationWindowTemplate", "Caster Signing Key (Public)", 0, QApplication::UnicodeUTF8));
        casterIDLineEdit->setText(QApplication::translate("casterConfigurationWindowTemplate", "0", 0, QApplication::UnicodeUTF8));
        casterIDLineEdit->setPlaceholderText(QString());
        label->setText(QApplication::translate("casterConfigurationWindowTemplate", "Data Sender Registration Port Number", 0, QApplication::UnicodeUTF8));
        saveCasterConfigurationPushButton->setText(QApplication::translate("casterConfigurationWindowTemplate", "Save Caster Configuration", 0, QApplication::UnicodeUTF8));
        label_4->setText(QApplication::translate("casterConfigurationWindowTemplate", "Client Stream Publishing Port Number", 0, QApplication::UnicodeUTF8));
        label_2->setText(QApplication::translate("casterConfigurationWindowTemplate", "Caster ID Number", 0, QApplication::UnicodeUTF8));
        keyManagementPortNumberLineEdit->setText(QApplication::translate("casterConfigurationWindowTemplate", "10006", 0, QApplication::UnicodeUTF8));
        label_7->setText(QApplication::translate("casterConfigurationWindowTemplate", "Key Management Port Number", 0, QApplication::UnicodeUTF8));
        proxyStreamPublishingPortNumberLineEdit->setText(QApplication::translate("casterConfigurationWindowTemplate", "10004", 0, QApplication::UnicodeUTF8));
        proxyStreamPublishingPortNumberLineEdit->setPlaceholderText(QString());
        label_9->setText(QApplication::translate("casterConfigurationWindowTemplate", "Caster Signing Key (Private)", 0, QApplication::UnicodeUTF8));
        label_10->setText(QApplication::translate("casterConfigurationWindowTemplate", "Key Management Signing Key (Public)", 0, QApplication::UnicodeUTF8));
        selectCasterPublicSigningKeyPushButton->setText(QApplication::translate("casterConfigurationWindowTemplate", "Select Key File", 0, QApplication::UnicodeUTF8));
        selectCasterPrivateSigningKeyPushButton->setText(QApplication::translate("casterConfigurationWindowTemplate", "Select Key File", 0, QApplication::UnicodeUTF8));
        selectKeyManagementPublicSigningKeyPushButton->setText(QApplication::translate("casterConfigurationWindowTemplate", "Select Key File", 0, QApplication::UnicodeUTF8));
        createCredentialsPushButton_2->setText(QApplication::translate("casterConfigurationWindowTemplate", "Create Credentials", 0, QApplication::UnicodeUTF8));
        label_14->setText(QApplication::translate("casterConfigurationWindowTemplate", "Add Credentials Signing Key (Public)", 0, QApplication::UnicodeUTF8));
        selectCredentialsPublicSigningKey->setText(QApplication::translate("casterConfigurationWindowTemplate", "Select Key File", 0, QApplication::UnicodeUTF8));
        label_13->setText(QApplication::translate("casterConfigurationWindowTemplate", "Select Maximum Number of Simultaneous  Base Stations", 0, QApplication::UnicodeUTF8));
        selectCredentialsBasestationLimit->setText(QApplication::translate("casterConfigurationWindowTemplate", "1", 0, QApplication::UnicodeUTF8));
        selectCredentialsPublicKeyPushButton->setText(QApplication::translate("casterConfigurationWindowTemplate", "Select Key File", 0, QApplication::UnicodeUTF8));
        label_12->setText(QApplication::translate("casterConfigurationWindowTemplate", "Select Credentials Expiration Time", 0, QApplication::UnicodeUTF8));
        label_11->setText(QApplication::translate("casterConfigurationWindowTemplate", "Select Public Key", 0, QApplication::UnicodeUTF8));
        label_15->setText(QApplication::translate("casterConfigurationWindowTemplate", "Add Credentials Signing Key (Private)", 0, QApplication::UnicodeUTF8));
        selectCredentialsPrivateSigningKey->setText(QApplication::translate("casterConfigurationWindowTemplate", "Select Key File", 0, QApplication::UnicodeUTF8));
        label_16->setText(QApplication::translate("casterConfigurationWindowTemplate", "IP Address", 0, QApplication::UnicodeUTF8));
        casterIPAddressLineEdit->setText(QApplication::translate("casterConfigurationWindowTemplate", "192.168.1.1", 0, QApplication::UnicodeUTF8));
        casterPortLineEdit->setText(QApplication::translate("casterConfigurationWindowTemplate", "10006", 0, QApplication::UnicodeUTF8));
        label_21->setText(QApplication::translate("casterConfigurationWindowTemplate", "Select Key Management Private Key", 0, QApplication::UnicodeUTF8));
        selectKeyManagementPrivateKeyPushButton->setText(QApplication::translate("casterConfigurationWindowTemplate", "Select Key File", 0, QApplication::UnicodeUTF8));
        label_17->setText(QApplication::translate("casterConfigurationWindowTemplate", "Port", 0, QApplication::UnicodeUTF8));
        addOfficialSigningKeyPushButton->setText(QApplication::translate("casterConfigurationWindowTemplate", "Add Official Key", 0, QApplication::UnicodeUTF8));
        label_19->setText(QApplication::translate("casterConfigurationWindowTemplate", "Select Signing Key", 0, QApplication::UnicodeUTF8));
        selectOfficialPublicSigningKeyToAdd->setText(QApplication::translate("casterConfigurationWindowTemplate", "Select Key File", 0, QApplication::UnicodeUTF8));
        label_20->setText(QApplication::translate("casterConfigurationWindowTemplate", "Select Expiration Time", 0, QApplication::UnicodeUTF8));
        addRegisteredCommunitySigningKeyPushButton->setText(QApplication::translate("casterConfigurationWindowTemplate", "Add Registered Community Key", 0, QApplication::UnicodeUTF8));
        label_22->setText(QApplication::translate("casterConfigurationWindowTemplate", "Select Expiration Time", 0, QApplication::UnicodeUTF8));
        label_18->setText(QApplication::translate("casterConfigurationWindowTemplate", "Select Signing Key", 0, QApplication::UnicodeUTF8));
        selectRegisteredCommunitySigningKeyToAddPushButton->setText(QApplication::translate("casterConfigurationWindowTemplate", "Select Key File", 0, QApplication::UnicodeUTF8));
        label_23->setText(QApplication::translate("casterConfigurationWindowTemplate", "Select Signing Key", 0, QApplication::UnicodeUTF8));
        selectBlackListPublicKey->setText(QApplication::translate("casterConfigurationWindowTemplate", "Select Key File", 0, QApplication::UnicodeUTF8));
        label_24->setText(QApplication::translate("casterConfigurationWindowTemplate", "Select Expiration Time", 0, QApplication::UnicodeUTF8));
        blacklistKeyPushButton->setText(QApplication::translate("casterConfigurationWindowTemplate", "Blacklist Key", 0, QApplication::UnicodeUTF8));
        configurationPushButton->setText(QApplication::translate("casterConfigurationWindowTemplate", "Configure Caster", 0, QApplication::UnicodeUTF8));
        generateKeysPushButton->setText(QApplication::translate("casterConfigurationWindowTemplate", "Generate Keys", 0, QApplication::UnicodeUTF8));
        createCredentialsPushButton->setText(QApplication::translate("casterConfigurationWindowTemplate", "Create Credentials", 0, QApplication::UnicodeUTF8));
        addRemoveKeysPushButton->setText(QApplication::translate("casterConfigurationWindowTemplate", "Add/Remove Keys", 0, QApplication::UnicodeUTF8));
    } // retranslateUi

};

namespace Ui {
    class casterConfigurationWindowTemplate: public Ui_casterConfigurationWindowTemplate {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_CASTERCONFIGURATIONWINDOWTEMPLATE_H
