cmake_minimum_required(VERSION 3.0.0)
project(RemoteCamera VERSION 1.4.0)

# if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
#    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D_GLIBCXX_USE_CXX11_ABI=0")
# endif()

cmake_policy(SET CMP0074 NEW)
enable_testing()

# OpenCV库
set(OpenCV_DIR "D:/CPPLibs/opencv460/opencv/build/x64/vc16/lib") # 找到lib文件目录
# set(OpenCV_DIR "D:/CPPLibs/opencv/build/x64/vc16/lib")
find_package(OpenCV REQUIRED)	# 找到OpenCV库
include_directories(${OpenCV_INCLUDE_DIRS})

# Eigen库
set(Eigen3_INCLUDE_DIRS "D:/CPPLibs/eigen-3.4.0") # 找到eigen文件夹
include_directories(${Eigen3_INCLUDE_DIRS})

# Boost库
set(Boost_ROOT "D:/CPPLibs/boost_1_80_0")
find_package(Boost REQUIRED COMPONENTS)
link_directories(${Boost_LIBRARY_DIRS})
include_directories(${Boost_INCLUDE_DIRS})

SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/output/)

add_executable(RemoteCamera RunRemoteCamera.cpp RemoteCamera.cpp)
target_link_libraries(RemoteCamera ${OpenCV_LIBRARIES} ${Boost_LIBRARIES})

add_executable(Client RunClient.cpp Client.cpp)
target_link_libraries(Client ${OpenCV_LIBRARIES} ${Boost_LIBRARIES})

add_executable(Sender RunSender.cpp Sender.cpp)
target_link_libraries(Sender ${OpenCV_LIBRARIES} ${Boost_LIBRARIES})

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)