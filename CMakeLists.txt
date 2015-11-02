cmake_minimum_required (VERSION 2.8.3)
PROJECT(pylonGPS)

#Enable doxygen documentation generation
find_package(Doxygen)
if(DOXYGEN_FOUND)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile.in ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile @ONLY)
add_custom_target(doc
${DOXYGEN_EXECUTABLE} ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile
WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
COMMENT "Generating API documentation with Doxygen" VERBATIM
)
endif(DOXYGEN_FOUND)

#QT stuff
#Access QT5 packages
find_package(Qt5Widgets)
find_package(Qt5Quick)

#Make moc called as nessisary
set(CMAKE_AUTOMOC ON)

#Get c++11
ADD_DEFINITIONS(-std=c++11)

find_package(Protobuf REQUIRED)

#Generate the C++ for the messages from the libprotobuf markup
add_subdirectory(./messages)


#Tell compiler where to find required libraries
link_directories(/usr/lib/x86_64-linux-gnu/ /usr/local/lib/ lib/ ./messages)

include_directories(./src/library/ ./messages ./cppzmq-master ./src/executables/unitTests ./src/executables/transceiverGUI)

find_package(Threads)

#Get lists of the source files
FILE(GLOB SOURCEFILES ./src/library/*.cpp ./src/library/*.c)

FILE(GLOB UNIT_TESTS_EXECUTABLE_SOURCE ./src/executables/unitTests/*.cpp ./src/executables/unitTests/*.c)

FILE(GLOB TRANSCEIVER_GUI_HEADER_FILES ./src/executables/transceiverGUI/*.hpp ./src/executables/transceiverGUI/*.h ${QT_USE_FILE} ${QT_HEADERS})

FILE(GLOB TRANSCEIVER_GUI_EXECUTABLE_SOURCE ./src/executables/transceiverGUI/*.cpp ./src/executables/transceiverGUI/*.c)

SET(QT_USE_QTCORE TRUE)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

#Add QT definitions
ADD_DEFINITIONS(${QT_DEFINITIONS})

#Set the binaries to be placed in the ./bin/ directory
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "bin/")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "lib/")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "lib/")


#Create a libraries
add_library(pylongps SHARED  ${SOURCEFILES})


#Tell CMake what executables to make
ADD_EXECUTABLE(unitTests ${UNIT_TESTS_EXECUTABLE_SOURCE} ) 

ADD_EXECUTABLE(transceiverGUI ${TRANSCEIVER_GUI_EXECUTABLE_SOURCE} ${TRANSCEIVER_GUI_HEADER_FILES_MOC} ${CMAKE_CURRENT_BINARY_DIR}) 

target_link_libraries(pylongps dl PocoFoundation PocoNet PocoUtil sqlite3 pylonGPSMessages zmq ${PROTOBUF_LIBRARY} ${CMAKE_THREAD_LIBS_INIT} sodium)

#link libraries to executable
target_link_libraries(unitTests pylongps )

target_link_libraries(transceiverGUI pylongps Qt5::Widgets Qt5::Quick)


