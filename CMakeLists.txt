cmake_minimum_required(VERSION 3.22.0)
project(RemoteCamera VERSION 1.6.2)

cmake_policy(SET CMP0074 NEW)
enable_testing()

if (MSVC)
    # 设置 msvc 输入代码编码格式为 utf-8
    add_compile_options("$<$<C_COMPILER_ID:MSVC>:/utf-8>")
    add_compile_options("$<$<CXX_COMPILER_ID:MSVC>:/utf-8>")
endif()

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
find_package(OpenCV REQUIRED)	# 找到OpenCV库
include_directories(${OpenCV_INCLUDE_DIRS})

# Boost库
set(Boost_ROOT "D:/CPPLibs/boost_1_80_0")
find_package(Boost REQUIRED COMPONENTS)
link_directories(${Boost_LIBRARY_DIRS})
include_directories(${Boost_INCLUDE_DIRS})

SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/output/)

add_subdirectory(src)
include_directories(include)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(ClientGUI
        MANUAL_FINALIZATION
        src/main.cpp
        include/ui/mainwindow.h
        include/ui/pathdialog.h
        ${_UI_SOURCES}
        ${_BASE_SOURCES}
        ${PROJECT_SOURCE_DIR}/output/resource/favicon.rc
    )
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

add_executable(RemoteCamera src/RunRemoteCamera.cpp src/base/RemoteCamera.cpp ${PROJECT_SOURCE_DIR}/output/resource/favicon.rc)
target_link_libraries(RemoteCamera ${OpenCV_LIBRARIES} ${Boost_LIBRARIES})

add_executable(Client src/RunClient.cpp src/base/Client.cpp ${PROJECT_SOURCE_DIR}/output/resource/favicon.rc)
target_link_libraries(Client ${OpenCV_LIBRARIES} ${Boost_LIBRARIES})

add_executable(Sender src/RunSender.cpp src/base/Sender.cpp ${PROJECT_SOURCE_DIR}/output/resource/favicon.rc)
target_link_libraries(Sender ${OpenCV_LIBRARIES} ${Boost_LIBRARIES})

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)