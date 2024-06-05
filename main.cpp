/*
 * Copyright (c) KylinSoft Co., Ltd. 2016-2024.All rights reserved.
 *
 * Authors:
 *  Kobe Lee    lixiang@kylinos.cn
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 3.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "mainwindow.h"
#include "utils.h"

#include <QApplication>
#include <QtSingleApplication>
#include <QCommandLineParser>
#include <QDebug>
#include <QTranslator>
#include <QLocale>
#include <QLibraryInfo>

#include <unistd.h>

using namespace kmre;

inline bool root_check()
{
    if (geteuid() == 0) {
        return true;
    }

    return false;
}

int main(int argc, char *argv[])
{
//#ifndef QT_DEBUG
    if (root_check()) {
        qWarning() << "Don't use root to run this tool";
        return -1;
    }
//#endif

//    if (utils::isWayland()) {
//        qputenv("QT_QPA_PLATFORM", "wayland");
//    }

    // 适配4K屏
#if (QT_VERSION >= QT_VERSION_CHECK(5, 12, 0))
    QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);
#endif

#if (QT_VERSION >= QT_VERSION_CHECK(5, 14, 0))
    QApplication::setHighDpiScaleFactorRoundingPolicy(Qt::HighDpiScaleFactorRoundingPolicy::PassThrough);
#endif

#if QT_VERSION >= 0x040400
    // Enable icons in menus
    QCoreApplication::setAttribute(Qt::AA_DontShowIconsInMenus, false);
#endif

/*#if (QT_VERSION >= QT_VERSION_CHECK(5, 6, 0))
    QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);//导致界面很大
#endif*/
    //QApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);


#if SINGLE_INSTANCE
    QtSingleApplication a(argc, argv);

    QString locale = QLocale::system().name();
    QTranslator translator;
    if (locale == "zh_CN") {
        if(!translator.load("kmre-apk-installer_" + locale + ".qm", ":/translations/"))
            qDebug() << "Load translation file："<< "kmre-apk-installer_" + locale + ".qm" << " failed!";
        else
            a.installTranslator(&translator);
    }
    QTranslator qt_translator;
    if (locale == "zh_CN") {
        if(!qt_translator.load("qt_" + locale + ".qm", QLibraryInfo::location(QLibraryInfo::TranslationsPath)))
            qDebug() << "Load translation file："<< "qt_" + locale + ".qm" << " failed!";
        else
            a.installTranslator(&qt_translator);
    }
    if (locale == "bo_CN") {
        if(!translator.load("kmre-apk-installer_" + locale + ".qm", ":/translations/"))
            qDebug() << "Load translation file："<< "kmre-apk-installer_" + locale + ".qm" << " failed!";
        else
            a.installTranslator(&translator);
    }

    QCommandLineParser parser;
    parser.setApplicationDescription("KMRE APK Installer.");
    parser.addHelpOption();
    parser.addVersionOption();
    parser.addPositionalArgument("filename", "Apk package path.", "file [file..]");
    parser.process(a);

    const QStringList apkList = parser.positionalArguments();

    if (a.isRunning()) {
        //a.sendMessage(QApplication::arguments().length() > 1 ? QApplication::arguments().at(1) : a.applicationFilePath());
        a.sendMessage("Hello");
        QString command = "open_files";
        a.sendMessage(command +" "+ apkList.join(" <<sep>> "));
        qDebug() << "kmre-apk-installer had already running!";
        return EXIT_SUCCESS;
    }
    else {
        MainWindow *w = new MainWindow();
        a.setActivationWindow(w);
        w->showWindow();
        QObject::connect(&a, SIGNAL(messageReceived(const QString&)), w, SLOT(handleMessageFromOtherInstances(const QString&)));

        if (!apkList.isEmpty()) {
            QMetaObject::invokeMethod(w, "onApkPackagesSelected", Qt::QueuedConnection, Q_ARG(QStringList, apkList));
        }

        return a.exec();
    }
#endif
}
