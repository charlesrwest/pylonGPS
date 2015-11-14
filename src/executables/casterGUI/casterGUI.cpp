#include "casterGUI.hpp"

using namespace pylongps;

/**
This function initializes the class, connecting widgets, setting up the form generated by Qt designer and adding the components that the designer doesn't handle well.

@throw: This function can throw exceptions
*/
casterGUI::casterGUI()
{
setupUi(this);
casterConfigurationStackedWidget->setCurrentIndex(0);

//Setup page control buttons
connect(configurationPushButton, SIGNAL(clicked(bool)), this, SLOT(switchToConfigureCasterPage()));
connect(createCredentialsPushButton, SIGNAL(clicked(bool)), this, SLOT(switchToCreateCredentialsPage()));
connect(addRemoveKeysPushButton, SIGNAL(clicked(bool)), this, SLOT(switchToAddRemoveKeysPage()));

//Add key generation button functionality
connect(generateKeysPushButton, SIGNAL(clicked(bool)), this, SLOT(selectDirectoryForKeyPairGeneration()));
connect(this, SIGNAL(keyPairGenerationFolderSelected(std::string)), this, SLOT(generateKeyPairFiles(std::string)));

//Add validators to line edits
casterIDLineEdit->setValidator(new QIntValidator(0, INT_MAX, this));

int portNumberMax = 65535;
dataSenderPortNumberLineEdit->setValidator( new QIntValidator(0, portNumberMax, this) );
clientRequestPortNumberLineEdit->setValidator( new QIntValidator(0, portNumberMax, this) );
clientStreamPublishingPortNumberLineEdit->setValidator( new QIntValidator(0, portNumberMax, this) );
proxyStreamPublishingPortNumberLineEdit->setValidator( new QIntValidator(0, portNumberMax, this) );
streamStatusNotificationPortNumberLineEdit->setValidator( new QIntValidator(0, portNumberMax, this) );
keyManagementPortNumberLineEdit->setValidator( new QIntValidator(0, portNumberMax, this) );
casterPortLineEdit->setValidator( new QIntValidator(0, portNumberMax, this) );

selectCredentialsBasestationLimit->setValidator( new QIntValidator(0, INT_MAX, this) );

//Connect add key buttons to add key dialogs
connect(selectCasterPublicSigningKeyPushButton, SIGNAL(clicked(bool)), this, SLOT(loadCasterConfigurationPublicSigningKey()));
connect(selectCasterPrivateSigningKeyPushButton, SIGNAL(clicked(bool)), this, SLOT(loadCasterConfigurationSecretSigningKey()));
connect(selectKeyManagementPublicSigningKeyPushButton, SIGNAL(clicked(bool)), this, SLOT(loadCasterConfigurationKeyManagementPublicSigningKey()));

connect(selectCredentialsPublicKeyPushButton, SIGNAL(clicked(bool)), this, SLOT(loadCredentialsPublicKey()));
connect(selectCredentialsPublicSigningKey, SIGNAL(clicked(bool)), this, SLOT(addCredentialsSigningPublicKey()));
connect(selectCredentialsPrivateSigningKey, SIGNAL(clicked(bool)), this, SLOT(addCredentialsSigningSecretKey()));

connect(selectKeyManagementPrivateKeyPushButton, SIGNAL(clicked(bool)), this, SLOT(loadKeyManagementSigningSecretKey()));
connect(selectOfficialPublicSigningKeyToAdd, SIGNAL(clicked(bool)), this, SLOT(loadOfficialPublicKeyForKeyManagementRequest()));
connect(selectRegisteredCommunitySigningKeyToAddPushButton, SIGNAL(clicked(bool)), this, SLOT(loadRegisteredCommunityPublicKeyForKeyManagementRequest()));
connect(selectBlackListPublicKey, SIGNAL(clicked(bool)), this, SLOT(loadPublicKeyToBlacklistForKeyManagementRequest()));
} 


/**
This function switches the main GUI's stacked widget to the configureCasterPage.
*/
void casterGUI::switchToConfigureCasterPage()
{
casterConfigurationStackedWidget->setCurrentIndex(0);
}

/**
This function switches the main GUI's stacked widget to the createCredentialsPage.
*/
void casterGUI::switchToCreateCredentialsPage()
{
casterConfigurationStackedWidget->setCurrentIndex(1);
}

/**
This function switches the main GUI's stacked widget to the addRemoveKeysPage.
*/
void casterGUI::switchToAddRemoveKeysPage()
{
casterConfigurationStackedWidget->setCurrentIndex(2);
}

/**
This function opens a file dialog menu to select the base path to generate key pair and emits keyPairGenerationFolderSelected with the selected base path.
*/
void casterGUI::selectDirectoryForKeyPairGeneration()
{
QString qfileName = QFileDialog::getSaveFileName(this, "Save File", QString(pathToFolderToPlaceGeneratedKeyPairIn.c_str()), "");
std::string fileName = qfileName.toStdString();

if(fileName.size() > 0)
{
pathToFolderToPlaceGeneratedKeyPairIn = fileName;
emit keyPairGenerationFolderSelected(fileName);
}
}

/**
This function generates a new signing key pair and outputs it to files with the .pylonPublicKey and .pylonPrivateKey extensions in Z85 text format (ZMQ standard).
@param inputBasePath: The base file name to add the extensions to/write

@throw: This function can throw exceptions
*/
void casterGUI::generateKeyPairFiles(const std::string &inputBasePath)
{
//Generate signing key pair
std::string binaryPublicKey;
std::string binarySecretKey;

std::tie(binaryPublicKey, binarySecretKey) = generateSigningKeys();

//Convert to Z85 format
std::string z85PublicKey;
std::string z85SecretKey;

SOM_TRY
z85PublicKey = convertStringToZ85Format(binaryPublicKey);
SOM_CATCH("Error unable to convert to z85 format\n") 

SOM_TRY
z85SecretKey = convertStringToZ85Format(binarySecretKey);
SOM_CATCH("Error unable to convert to z85 format\n") 

if(!saveStringToFile(z85PublicKey.substr(0, z85PublicKey.size()-1), inputBasePath+".pylonPublicKey"))
{
emit couldNotWriteKeyPairFiles();
return;
}

if(!saveStringToFile(z85SecretKey.substr(0, z85SecretKey.size()-1), inputBasePath+".pylonSecretKey"))
{
emit couldNotWriteKeyPairFiles();
return;
}
}

/**
This function loads the public signing key to assign to the caster.
*/
void casterGUI::loadCasterConfigurationPublicSigningKey()
{
runKeyLoadDialogForString(casterConfigurationPublicSigningKey, true, lastPublicKeyPath);
}

/**
This function loads the secret signing key to assign to the caster.
*/
void casterGUI::loadCasterConfigurationSecretSigningKey()
{
runKeyLoadDialogForString(casterConfigurationSecretSigningKey, false, lastSecretKeyPath);
}

/**
This function loads the public signing key to assign to the caster.
*/
void casterGUI::loadCasterConfigurationKeyManagementPublicSigningKey()
{
runKeyLoadDialogForString(casterConfigurationKeyManagementPublicSigningKey, true, lastPublicKeyPath);
}

/**
This function loads the public key to generate a credentials message for.
*/
void casterGUI::loadCredentialsPublicKey()
{
runKeyLoadDialogForString(credentialsPublicKey, true, lastPublicKeyPath);
}

/**
This function loads a public key to sign the credentials with.
*/
void casterGUI::addCredentialsSigningPublicKey()
{
std::string keyBuffer;
if(runKeyLoadDialogForString(keyBuffer, true, lastPublicKeyPath) == true)
{
credentialsSigningPublicKeys.push_back(keyBuffer);
}
}

/**
This function loads a secret key to sign the credentials with.
*/
void casterGUI::addCredentialsSigningSecretKey()
{
std::string keyBuffer;
if(runKeyLoadDialogForString(keyBuffer, false, lastSecretKeyPath) == true)
{
credentialsSigningPrivateKeys.push_back(keyBuffer);
}
}

/**
This function loads the key management key to use with key management requests.
*/
void casterGUI::loadKeyManagementSigningSecretKey()
{
runKeyLoadDialogForString(keyManagementPrivateKeyToUse, false, lastSecretKeyPath);
}

/**
This function loads a public key to add to the Official list using a key managment request.
*/
void casterGUI::loadOfficialPublicKeyForKeyManagementRequest()
{
runKeyLoadDialogForString(officialPublicKeyToAdd, true, lastPublicKeyPath);
}

/**
This function loads a public key to add to the Registered Community list using a key managment request.
*/
void casterGUI::loadRegisteredCommunityPublicKeyForKeyManagementRequest()
{
runKeyLoadDialogForString(registeredCommunityPublicKeyToAdd, true, lastPublicKeyPath);
}

/**
This function loads a public key to add to the black list using a key managment request.
*/
void casterGUI::loadPublicKeyToBlacklistForKeyManagementRequest()
{
runKeyLoadDialogForString(publicKeyToBlacklist, true, lastPublicKeyPath);
}

/**
This function opens a dialog box asking for a key file, with the default path being given by inputPathStartFormAt.  If a proper key file is selected and is valid, it loads the key into the string given by inputKeyStringToLoadTo.
@param inputKeyStringToLoadTo: The string buffer to load the decoded string into
@param inputIsPublicKey: True if the key to load is a public key and false if it is a private key
@param inputPathToStartFormAt: A string containing where the file dialog should start looking (typically last variable loaded).  The selected path will be saved to this variable
@return: True if this function loaded a key
*/
bool casterGUI::runKeyLoadDialogForString(std::string &inputKeyStringToLoadTo, bool inputIsPublicKey, std::string &inputPathStartFormAt)
{
QString qPath;
if(inputIsPublicKey)
{
qPath = QFileDialog::getOpenFileName(this, "Open Public Key File", QString(inputPathStartFormAt.c_str()), "Public Keys (*.pylonPublicKey)");
}
else
{
qPath = QFileDialog::getOpenFileName(this, "Open Secret Key File", QString(inputPathStartFormAt.c_str()), "Secret Keys (*.pylonSecretKey)");
}
std::string path = qPath.toStdString();

//Try loading the key
std::string loadedKey;
if(inputIsPublicKey)
{
loadedKey = loadPublicKeyFromFile(path);
if(loadedKey.size() == 0)
{
emit couldNotReadPublicKeyFile();
}
}
else
{
loadedKey = loadSecretKeyFromFile(path);
if(loadedKey.size() == 0)
{
emit couldNotReadSecretKeyFile();
}
}

//Key loaded OK
inputKeyStringToLoadTo = loadedKey;
inputPathStartFormAt = path;
return true;
}

