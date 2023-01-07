cmake_minimum_required(VERSION 3.0.0)
project(RemoteCamera VERSION 1.6.0)

cmake_policy(SET CMP0074 NEW)
enable_testing()


aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/ui _UI_SOURCES)
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/base _BASE_SOURCES)

set(_UI_SOURCES ${_UI_SOURCES} PARENT_SCOPE)
set(_BASE_SOURCES ${_BASE_SOURCES} PARENT_SCOPE)