cmake_minimum_required(VERSION 3.16)

project(kmre-apk-installer VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 REQUIRED COMPONENTS Core Gui Widgets Network Concurrent DBus Svg)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core Gui Widgets Network Concurrent DBus Svg)

if(QT_VERSION_MAJOR EQUAL 5)
    find_package(Qt5 REQUIRED COMPONENTS X11Extras)
endif()

find_package(PkgConfig REQUIRED)

find_program(LRELEASE_EXECUTABLE lrelease)
if(NOT LRELEASE_EXECUTABLE)
    message(FATAL_ERROR "lrelease not found!")
endif()

file(GLOB TS_FILES translations/*.ts)
foreach(ts_file ${TS_FILES})
    string(REPLACE ".ts" ".qm" qm_file "${ts_file}")
    add_custom_command(
        OUTPUT "${qm_file}"
        COMMAND ${LRELEASE_EXECUTABLE} "${ts_file}" -qm "${qm_file}"
        MAIN_DEPENDENCY "${ts_file}"
        COMMENT "Compiling translation ${ts_file}"
    )
    list(APPEND QM_FILES "${qm_file}")
endforeach()

add_custom_target(translations ALL DEPENDS ${QM_FILES})
pkg_check_modules(LIBCRYPTO REQUIRED libcrypto)

add_definitions(-DSINGLE_INSTANCE)
add_definitions(-DQT_DEPRECATED_WARNINGS)

execute_process(COMMAND lsb_release -d OUTPUT_VARIABLE DISTRIB_DESCRIPTION)
execute_process(COMMAND grep -ci -e "Kylin\ V10\ Professional" -e "Kylin\ V10\ SP1" -e "Kylin\ V10\ SP2" /etc/lsb-release OUTPUT_VARIABLE V10_PRO OUTPUT_STRIP_TRAILING_WHITESPACE)
if(V10_PRO STREQUAL "0")
    add_definitions(-DKYLIN_V10)
endif()

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -fpermissive -fPIC")

set(PROJECT_SOURCES
    main.cpp
    mainwindow.cpp
    titlebar.cpp
    importwidget.cpp
    installwidget.cpp
    infopage.cpp
    utils.cpp
    copythread.cpp
    backendworker.cpp
    xatom-helper.cpp
    textlabel.cpp
    qtsingleapplication/qtsingleapplication.cpp
    qtsingleapplication/qtlocalpeer.cpp

    mainwindow.h
    titlebar.h
    importwidget.h
    installwidget.h
    infopage.h
    common.h
    utils.h
    copythread.h
    backendworker.h
    xatom-helper.h
    textlabel.h
    qtsingleapplication/qtsingleapplication.h
    qtsingleapplication/qtlocalpeer.h
    qtsingleapplication/qtlockedfile.h

    res.qrc
)

add_executable(kmre-apk-installer
    ${PROJECT_SOURCES}
)

add_dependencies(kmre-apk-installer translations)

set(QT_LIBRARIES
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Gui
    Qt${QT_VERSION_MAJOR}::Widgets
    Qt${QT_VERSION_MAJOR}::Network
    Qt${QT_VERSION_MAJOR}::Concurrent
    Qt${QT_VERSION_MAJOR}::DBus
    Qt${QT_VERSION_MAJOR}::Svg
)

if(QT_VERSION_MAJOR EQUAL 5)
    list(APPEND QT_LIBRARIES Qt5::X11Extras)
endif()

target_link_libraries(kmre-apk-installer
    PRIVATE
    ${QT_LIBRARIES}
    ${LIBCRYPTO_LIBRARIES}
    dl
    X11
)

target_include_directories(kmre-apk-installer
    PRIVATE
    ${PROJECT_SOURCE_DIR}/qtsingleapplication
    ${LIBCRYPTO_INCLUDE_DIRS}
)

install(TARGETS kmre-apk-installer DESTINATION bin)
install(FILES kmre-apk-installer.desktop DESTINATION share/applications)
install(FILES kmre-apk-installer.applications DESTINATION share/application-registry)
install(FILES kmre-apk-installer.xml DESTINATION share/mime/packages)
install(FILES res/kmre-apk-installer.svg DESTINATION share/pixmaps)
