cmake_minimum_required (VERSION 2.8.3)
PROJECT(pylonGPS)

#Tell compiler where to find required libraries
link_directories(/usr/lib/x86_64-linux-gnu/ lib/)

include_directories(./src/library/)

find_package(Threads)

#Get lists of the source files
FILE(GLOB SOURCEFILES ./src/library/*.cpp ./src/library/*.c)

FILE(GLOB CLIENT_EXAMPLE_EXECUTABLE_SOURCE ./src/executables/clientExample/*.cpp ./src/executables/clientExample/*.c)

#Set the binaries to be placed in the ./bin/ directory
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "bin/")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "lib/")


#Create a static library of the common source files so they are not built twice
add_library(pylongps SHARED  ${SOURCEFILES})


#Tell CMake what executables to make
ADD_EXECUTABLE(clientExample ${CLIENT_EXAMPLE_EXECUTABLE_SOURCE}) 

#link libraries to executable PocoNetd
target_link_libraries(clientExample pylongps PocoFoundation PocoNet ${CMAKE_THREAD_LIBS_INIT})
