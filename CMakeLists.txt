cmake_minimum_required(VERSION 3.5)

project(ytaudio LANGUAGES CXX)
ADD_DEFINITIONS(-DQT_NO_KEYWORDS)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# QtCreator supports the following variables for Android, which are identical to qmake Android variables.
# Check http://doc.qt.io/qt-5/deployment-android.html for more information.
# They need to be set before the find_package(Qt5 ...) call.

#if(ANDROID)
#    set(ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/android")
#    if (ANDROID_ABI STREQUAL "armeabi-v7a")
#        set(ANDROID_EXTRA_LIBS
#            ${CMAKE_CURRENT_SOURCE_DIR}/path/to/libcrypto.so
#            ${CMAKE_CURRENT_SOURCE_DIR}/path/to/libssl.so)
#    endif()
#endif()

find_package(Qt5 COMPONENTS Widgets REQUIRED)
find_package(Qt5 COMPONENTS Multimedia REQUIRED)
find_package (Python3 COMPONENTS Interpreter Development REQUIRED)
include_directories(${Python3_INCLUDE_DIRS})

# Copy python scripts to build
configure_file(src/geturls.py geturls.py COPYONLY)
# Add SDL2/Atffile COPYONLY)
# Add SDL2
#find_package(SDL2 REQUIRED)
#include_directories(${SDL2_INCLUDE_DIRS})

# Add SDL2_Mixer
#find_package(pkgconfig)
#pkg_search_module(SDL2MIXER REQUIRED SDL2_mixer)
#include_directories(${SDL2MIXER_INCLUDE_DIRS})

if(ANDROID)
  add_library(ytaudio SHARED
    src/main.cpp
    src/mainwindow.cpp
    src/mainwindow.h
    src/urlworker.cpp
    src/urlworker.h
    src/mainwindow.ui
  )
else()
  add_executable(ytaudio
    src/main.cpp
    src/mainwindow.cpp
    src/mainwindow.h
    src/urlworker.cpp
    src/urlworker.h
    src/mainwindow.ui
  )
endif()
target_link_libraries(ytaudio PUBLIC ${Python3_LIBRARIES})
target_link_libraries(ytaudio PRIVATE Qt5::Widgets Qt5::Multimedia Qt5::Core)# ${SDL2_LIBRARIES} ${SDL2MIXER_LDFLAGS})
