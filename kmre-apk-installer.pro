# Copyright (c) KylinSoft Co., Ltd. 2016-2024.All rights reserved.
#
# Authors:
#  Kobe Lee    lixiang@kylinos.cn
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; version 3.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.

QT      += gui core network concurrent x11extras dbus gui-private svg

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

DEFINES += SINGLE_INSTANCE

isEmpty(PREFIX) {
    PREFIX = /usr
}

TARGET = kmre-apk-installer
TEMPLATE = app

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

!system($$PWD/translations/generate_translations_pm.sh): error("Failed to generate pm")

desktop.files += kmre-apk-installer.desktop
desktop.path = /usr/share/applications
registry.files += kmre-apk-installer.applications
registry.path = /usr/share/application-registry/
mimexml.files += kmre-apk-installer.xml
mimexml.path = /usr/share/mime/packages/
logo.files += res/kmre-apk-installer.svg
logo.path = /usr/share/pixmaps/
target.source  += $$TARGET
target.path = $${PREFIX}/bin/
INSTALLS += desktop \
    registry \
    mimexml \
    logo \
    target

# is V10 Professional or not
isEmpty(lsb_release):lsb_release=lsb_release
DISTRIB_DESCRIPTION = $$system($$lsb_release -d)
V10_PRO = $$system(cat /etc/lsb-release | grep -ci -e "Kylin\ V10\ Professional" -e "Kylin\ V10\ SP1" -e "Kylin\ V10\ SP2")
isEqual(V10_PRO, 0) {
    DEFINES += KYLIN_V10
} else {

}

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

CONFIG += c++14
CONFIG += qt warn_on
CONFIG += release
CONFIG += static
CONFIG += link_pkgconfig
PKGCONFIG += libcrypto
#message($${QT_ARCH})

#contains(QT_ARCH, arm64) {
#    message("arm64")
#}
#else {
#    message("other arch")
#}

#QMAKE_CPPFLAGS *= $(shell dpkg-buildflags --get CPPFLAGS)
#QMAKE_CFLAGS   *= $(shell dpkg-buildflags --get CFLAGS)
#QMAKE_CXXFLAGS *= $(shell dpkg-buildflags --get CXXFLAGS)
#QMAKE_LFLAGS   *= $(shell dpkg-buildflags --get LDFLAGS)
QMAKE_CXXFLAGS += -g -fpermissive -fPIC
QMAKE_CFLAGS_DEBUG += -g

LIBS += -ldl -rdynamic -lX11

INCLUDEPATH += qtsingleapplication
DEPENDPATH += qtsingleapplication

SOURCES += \
    main.cpp \
    mainwindow.cpp \
    titlebar.cpp \
    importwidget.cpp \
    installwidget.cpp \
    infopage.cpp \
    utils.cpp \
    copythread.cpp \
    backendworker.cpp \
    xatom-helper.cpp \
    textlabel.cpp

HEADERS += \
    mainwindow.h \
    titlebar.h \
    importwidget.h \
    installwidget.h \
    infopage.h \
    common.h \
    utils.h \
    copythread.h \
    backendworker.h \
    xatom-helper.h \
    textlabel.h

# qtsingleapplication
contains( DEFINES, SINGLE_INSTANCE ) {
    INCLUDEPATH += qtsingleapplication
    DEPENDPATH += qtsingleapplication
    SOURCES += qtsingleapplication/qtsingleapplication.cpp qtsingleapplication/qtlocalpeer.cpp
    HEADERS += qtsingleapplication/qtsingleapplication.h qtsingleapplication/qtlocalpeer.h
}

unix {
    UI_DIR = .ui
    MOC_DIR = .moc
    OBJECTS_DIR = .obj
}

# for wayland
## libkf5wayland-dev libwayland-dev
#DEFINES += UKUI_WAYLAND
#contains(DEFINES, UKUI_WAYLAND) {
#    QT += KWaylandClient
#    PKGCONFIG += wayland-client
#    SOURCES += ukui-wayland/ukui-decoration-manager.cpp ukui-wayland/ukui-decoration-core.c
#    HEADERS += ukui-wayland/ukui-decoration-manager.h ukui-wayland/ukui-decoration-client.h
#}

RESOURCES += \
    res.qrc

TRANSLATIONS += \
    translations/kmre-apk-installer_zh_CN.ts \
    translations/kmre-apk-installer_bo_CN.ts
