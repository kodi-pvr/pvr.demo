cmake_minimum_required(VERSION 3.5)
project(pvr.demo)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${PROJECT_SOURCE_DIR})

find_package(Kodi REQUIRED)
find_package(kodiplatform REQUIRED)
find_package(p8-platform REQUIRED)

include_directories(${kodiplatform_INCLUDE_DIRS}
                    ${p8-platform_INCLUDE_DIRS}
                    ${KODI_INCLUDE_DIR}/..) # Hack way with "/..", need bigger Kodi cmake rework to match right include ways

set(DEPLIBS ${kodiplatform_LIBRARIES}
            ${p8-platform_LIBRARIES})

set(PVRDEMO_SOURCES src/client.cpp
                    src/PVRDemoData.cpp)

set(PVRDEMO_HEADERS src/client.h
                    src/PVRDemoData.h)

build_addon(pvr.demo PVRDEMO DEPLIBS)

include(CPack)
