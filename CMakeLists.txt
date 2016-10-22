cmake_minimum_required (VERSION 2.8.3)
PROJECT(pylonGPS)

#Build GUIs?
option(BUILD_GUIS "Build Qt5 based GUIs" OFF)
option(BUILD_DOCS "Build Documentation" OFF)

#Get c++11
ADD_DEFINITIONS(-std=c++11)

find_package(Protobuf REQUIRED)

find_package(Threads)

#Generate the C++ for the messages from the libprotobuf markup
add_subdirectory(./messages)



#Enable doxygen documentation generation
if(BUILD_DOCS)

find_package(Doxygen)
if(DOXYGEN_FOUND)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile.in ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile @ONLY)
add_custom_target(doc
${DOXYGEN_EXECUTABLE} ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile
WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
COMMENT "Generating API documentation with Doxygen" VERBATIM
)
endif(DOXYGEN_FOUND)

endif(BUILD_DOCS)


if(BUILD_GUIS)
find_package(Qt5Widgets)
find_package(Qt5Svg)

#Make moc called as nessisary
set(CMAKE_AUTOMOC ON)
endif(BUILD_GUIS)



#Tell compiler where to find required libraries
link_directories(/usr/lib/x86_64-linux-gnu/ lib/ ./messages)

include_directories(./src/library/ ./messages ./cppzmq ./src/executables/unitTests ./src/executables/transceiverGUI ./src/executables/casterGUI ./src/executables/caster ./ ./jsoncpp/json ./src/executables/transceiver)

if(BUILD_GUIS)
#Convert Qt .ui forms to sources
FILE(GLOB TRANSCEIVER_FORMS src/executables/transceiverGUI/*.ui)

QT5_WRAP_UI(TRANSCEIVER_FORMS_HEADERS ${TRANSCEIVER_FORMS})

FILE(GLOB CASTER_FORMS src/executables/casterGUI/*.ui)

QT5_WRAP_UI(CASTER_FORMS_HEADERS ${CASTER_FORMS})

FILE(GLOB TRANSCEIVER_GUI_HEADER_FILES ./src/executables/transceiverGUI/*.hpp ./src/executables/transceiverGUI/*.h)

FILE(GLOB CASTER_GUI_HEADER_FILES ./src/executables/casterGUI/*.hpp ./src/executables/casterGUI/*.h)

FILE(GLOB TRANSCEIVER_GUI_EXECUTABLE_SOURCE ./src/executables/transceiverGUI/*.cpp ./src/executables/transceiverGUI/*.c)

FILE(GLOB CASTER_GUI_EXECUTABLE_SOURCE ./src/executables/casterGUI/*.cpp ./src/executables/casterGUI/*.c)

#Add the QT modules we are going to use
#QT_USE_QTNETWORK QT_USE_QTOPENGL QT_USE_QTSQL QT_USE_QTXML QT_USE_QTSVG QT_USE_QTTEST QT_USE_QTDBUS QT_USE_QTSCRIPT QT_USE_QTWEBKIT QT_USE_QTXMLPATTERNS QT_USE_PHONON
SET(QT_USE_QTCORE TRUE)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
#set(CMAKE_INCLUDE_CURRENT_BINARY_DIR ON)

#Add QT definitions
ADD_DEFINITIONS(${QT_DEFINITIONS})

INCLUDE_DIRECTORIES(${CMAKE_CURRENT_BINARY_DIR})
endif(BUILD_GUIS)

#Get lists of the source files
FILE(GLOB SOURCEFILES ./src/library/*.cpp ./src/library/*.c ./jsoncpp/*.cpp)

FILE(GLOB UNIT_TESTS_EXECUTABLE_SOURCE ./src/executables/unitTests/*.cpp ./src/executables/unitTests/*.c)

FILE(GLOB CASTER_SOURCE_FILES ./src/executables/caster/*.cpp ./src/executables/caster/*.c)

FILE(GLOB TRANSCEIVER_SOURCE_FILES ./src/executables/transceiver/*.cpp ./src/executables/transceiver/*.c)

FILE(GLOB STRESS_TESTER ./src/executables/stressTester/*.cpp ./src/executables/stressTester/*.c)

FILE(GLOB TEST_DATA_SENDERS_SOURCE_FILES ./src/executables/testDataSenders/*.cpp ./src/executables/testDataSenders/*.c)

#Set the binaries to be placed in the ./bin/ directory
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "bin/")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "lib/")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "lib/")

#Create a libraries
add_library(pylongps SHARED  ${SOURCEFILES})

#Tell CMake what executables to make
ADD_EXECUTABLE(unitTests ${UNIT_TESTS_EXECUTABLE_SOURCE} ) 

if(BUILD_GUIS)
ADD_EXECUTABLE(transceiverGUI ${TRANSCEIVER_GUI_EXECUTABLE_SOURCE} ${TRANSCEIVER_GUI_HEADER_FILES} ${TRANSCEIVER_FORMS_HEADERS} ${CMAKE_CURRENT_BINARY_DIR}) 

ADD_EXECUTABLE(casterGUI ${CASTER_GUI_EXECUTABLE_SOURCE} ${CASTER_GUI_HEADER_FILES} ${CASTER_FORMS_HEADERS} ${CMAKE_CURRENT_BINARY_DIR}) 
endif(BUILD_GUIS)

ADD_EXECUTABLE(caster ${CASTER_SOURCE_FILES} ${CMAKE_CURRENT_BINARY_DIR})

ADD_EXECUTABLE(transceiver ${TRANSCEIVER_SOURCE_FILES} ${CMAKE_CURRENT_BINARY_DIR})

ADD_EXECUTABLE(testDataSender ${TEST_DATA_SENDERS_SOURCE_FILES} ${CMAKE_CURRENT_BINARY_DIR})

ADD_EXECUTABLE(stressTester ${STRESS_TESTER} ${CMAKE_CURRENT_BINARY_DIR})


target_link_libraries(pylongps dl PocoFoundation PocoNet PocoUtil sqlite3 pylonGPSMessages zmq ${PROTOBUF_LIBRARY} ${CMAKE_THREAD_LIBS_INIT} sodium)

#link libraries to executable
target_link_libraries(unitTests pylongps )

if(BUILD_GUIS)
target_link_libraries(transceiverGUI pylongps Qt5::Widgets marblewidget-qt5 Qt5::Svg)

target_link_libraries(casterGUI pylongps Qt5::Widgets)
endif(BUILD_GUIS)

target_link_libraries(caster pylongps)

target_link_libraries(transceiver pylongps)

target_link_libraries(testDataSender pylongps)

target_link_libraries(stressTester pylongps)



