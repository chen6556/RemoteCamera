cmake_minimum_required(VERSION 3.0.0)
project(RemoteCamera VERSION 1.4.0)

cmake_policy(SET CMP0074 NEW)
enable_testing()

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Qt库
find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets)

# OpenCV库
set(OpenCV_DIR "D:/CPPLibs/opencv460/opencv/build/x64/vc16/lib") # 找到lib文件目录
# set(OpenCV_DIR "D:/CPPLibs/opencv/build/x64/vc16/lib")
find_package(OpenCV REQUIRED)	# 找到OpenCV库
include_directories(${OpenCV_INCLUDE_DIRS})

# Boost库
set(Boost_ROOT "D:/CPPLibs/boost_1_80_0")
find_package(Boost REQUIRED COMPONENTS)
link_directories(${Boost_LIBRARY_DIRS})
include_directories(${Boost_INCLUDE_DIRS})

SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/output/)

set(PROJECT_SOURCES
        main.cpp
        mainwindow.cpp
        mainwindow.h
        mainwindow.ui
        Client.cpp
        Sender.cpp
)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(ClientGUI
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
    )

else()
    if(ANDROID)
        add_library(ClientGUI SHARED
            ${PROJECT_SOURCES}
        )
    else()
        add_executable(Client
            ${PROJECT_SOURCES}
        )
    endif()
endif()

target_link_libraries(ClientGUI PRIVATE Qt${QT_VERSION_MAJOR}::Widgets ${OpenCV_LIBRARIES} ${Boost_LIBRARIES})

set_target_properties(ClientGUI PROPERTIES
    MACOSX_BUNDLE_GUI_IDENTIFIER my.example.com
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(ClientGUI)
endif()

add_executable(RemoteCamera RunRemoteCamera.cpp RemoteCamera.cpp)
target_link_libraries(RemoteCamera ${OpenCV_LIBRARIES} ${Boost_LIBRARIES})

add_executable(Client RunClient.cpp Client.cpp)
target_link_libraries(Client ${OpenCV_LIBRARIES} ${Boost_LIBRARIES})

add_executable(Sender RunSender.cpp Sender.cpp)
target_link_libraries(Sender ${OpenCV_LIBRARIES} ${Boost_LIBRARIES})

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)