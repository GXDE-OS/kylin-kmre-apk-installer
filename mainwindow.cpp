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
#include "common.h"
#include "utils.h"
#include "titlebar.h"
#include "importwidget.h"
#include "installwidget.h"
#include "infopage.h"
#include "xatom-helper.h"
#include "backendworker.h"

#include <QKeyEvent>
#include <QGuiApplication>
#include <QScreen>
#include <QFileInfo>
#include <QDir>
#include <QWindow>
#include <QX11Info>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QJsonParseError>
#include <QDebug>

#include <X11/Xlib.h>

using namespace kmre;

//#ifdef UKUI_WAYLAND
//#include "ukui-wayland/ukui-decoration-manager.h"
//#endif

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , m_mainLayout(new QVBoxLayout)
    , m_centralLayout(new QStackedLayout)
{
//    m_isWayland = utils::isWayland();
    m_loginUserName = utils::getUserName();
    m_loginUserId = utils::getUid();

#ifdef KYLIN_V10
    this->setWindowFlags(Qt::FramelessWindowHint);
#endif
    this->setFixedSize(WINDOW_WIDTH, WINDOW_HEIGHT);
    this->setWindowTitle(tr("KMRE APK Installer"));
    QIcon icon = QIcon::fromTheme("kmre-apk-installer");
    if (icon.isNull()) {
        icon = QIcon(":/res/kmre-apk-installer.svg");
    }
    this->setWindowIcon(icon);
    this->setBackgroundRole(QPalette::Base);
    this->setAutoFillBackground(true);
    this->setAttribute(Qt::WA_DeleteOnClose);
    this->setMouseTracking(true);
    this->setAcceptDrops(true);
    this->setFocusPolicy(Qt::ClickFocus);
    this->setAttribute(Qt::WA_AcceptTouchEvents);
    //disable style window manager
//    this->setProperty("useStyleWindowManager", false);//for UKUI 3.0

    this->initTitleBar();

    m_importWidget = new ImportWidget(this);
    m_installWidget = new InstallWidget(m_loginUserName, m_loginUserId, this);
    m_infoWidget = new InfoPage(this);

    m_centralLayout->setContentsMargins(0, 0, 0, 0);
    m_centralLayout->setSpacing(0);
    m_centralLayout->addWidget(m_importWidget);
    m_centralLayout->addWidget(m_installWidget);
    m_centralLayout->addWidget(m_infoWidget);
    m_centralLayout->setCurrentIndex(0);

    m_mainLayout->setContentsMargins(0, 0, 0, 0);
    m_mainLayout->setSpacing(0);
    m_mainLayout->addWidget(m_titleBar);
    m_mainLayout->addLayout(m_centralLayout);

    QWidget *centralWidget = new QWidget;
    centralWidget->setLayout(m_mainLayout);
    this->setCentralWidget(centralWidget);

    this->initBackend();

#ifndef KYLIN_V10
//#ifndef UKUI_WAYLAND
    // 添加窗管协议
    if (QX11Info::isPlatformX11()) {
        XAtomHelper::getInstance()->setUKUIDecoraiontHint(this->winId(), true);
        MotifWmHints hints;
        hints.flags = MWM_HINTS_FUNCTIONS|MWM_HINTS_DECORATIONS;
        hints.functions = MWM_FUNC_ALL;
        hints.decorations = MWM_DECOR_BORDER;
        XAtomHelper::getInstance()->setWindowMotifHint(this->winId(), hints);
    }
//#endif
#endif

    this->hide();
}

MainWindow::~MainWindow()
{
    if (m_backendWorker) {
        m_backendWorker->deleteLater();
    }
}

void MainWindow::initTitleBar()
{
    m_titleBar = new TitleBar(/*m_isWayland, */this);
    m_titleBar->setFixedSize(WINDOW_WIDTH, DEFAULT_TITLEBAR_HEIGHT);
//    m_titleBar->move(0, 0);
//    m_titleBar->raise();
    m_titleBar->setTitle(tr("KMRE APK Installer"));
    m_titleBar->setIcon(":/res/kmre-apk-installer.svg");

    connect(m_titleBar, &TitleBar::sigMiniSize, this, [=]() {
        this->showMinimized();
    });
    connect(m_titleBar, &TitleBar::sigClose, [this]() {
        this->close();
    });
}

void MainWindow::initBackend()
{
    m_backendWorker = new BackendWorker(m_loginUserName, m_loginUserId);
    m_backendThread = new QThread;
    m_backendWorker->moveToThread(m_backendThread);
    connect(m_backendThread, &QThread::finished, this, [=] () {
        m_backendThread->deleteLater();
    });

    m_installWidget->setBackendWorker(m_backendWorker);

    connect(this, &MainWindow::requestGetAndroidEnvInstalled, m_backendWorker, &BackendWorker::onGetAndroidEnvInstalled);
    connect(m_backendWorker, &BackendWorker::sigAndroidEnvInstalled, this, [=] (int value) {
        if (value == 0) {
            showInfoPage(tr("KMRE environment not installed"), "", false);
        }
    });
    connect(m_backendWorker, &BackendWorker::unInstallFinished, this, [=] (const QString &appName, bool result) {
        qDebug() << "unInstallFinished:" << appName << result;
        if (result) {
            //delete icon
            const QString &iconFile = QString("%1/.local/share/icons/%2.png").arg(QDir::homePath()).arg(appName);
            QFile fp(iconFile);
            if (fp.exists()) {
                if (!fp.remove()) {
                    qCritical() << "Failed to remove icon file:" << iconFile;
                }
            }

            //delete desktop file
            const QString destDesktopFile = QString("%1/.local/share/applications/%2.desktop").arg(QDir::homePath()).arg(appName);
            QFile fp2(destDesktopFile);
            if (fp2.exists()) {
                if (!fp2.remove()) {
                    qCritical() << "Failed to remove desktop file:" << destDesktopFile;
                }
            }
        }
    }, Qt::QueuedConnection);

    connect(this, &MainWindow::requestAnalysisApkFile, m_installWidget, &InstallWidget::onAnalysisApkFile);
    connect(m_installWidget, &InstallWidget::back, this, [=]() {
        if (m_importWidget) {
            m_centralLayout->setCurrentWidget(m_importWidget);
        }
    });
    connect(m_installWidget, &InstallWidget::signalAndroidNotReady, this, [=]() {
        showInfoPage(tr("KMRE environment not installed,please go to the app store to start the mobile environment"), "", false);
    });
    connect(m_installWidget, &InstallWidget::requestInstallApp, m_backendWorker, &BackendWorker::installApp);
    connect(m_installWidget, &InstallWidget::sigError, this, [=](int code) {
        if (code == 0) {
            showInfoPage(tr("APK package does not exist or file type is abnormal!"), "", false);
        }
        else if (code == 1) {
            showInfoPage(tr("Invalid APK file!"), "", false);
        }
        else {
            showInfoPage(tr("Unknown error!"), "", false);
        }
    });
    connect(m_backendWorker, &BackendWorker::installFinished, this, [=] (const QString &fileName, const QString &pkgName, bool result) {
        QFont ft;
        QFontMetrics fm(ft);
        QString elided_text = fm.elidedText(fileName, Qt::ElideMiddle, 300);
        this->emit requestfileName(fileName);
        if (result) {
            this->showInfoPage(QString(tr("%1 install successfully")).arg(elided_text), pkgName, result);
        }
        else {
            this->showInfoPage(QString(tr("%1 install failed")).arg(elided_text), pkgName, result);
        }
        m_installWidget->reset();
    }, Qt::QueuedConnection);
//    connect(m_backendWorker, &BackendWorker::sendInstalledAppList, this, [=] (const QByteArray &array) {
//        bool hadItems = false;
//        QJsonParseError err;
//        QJsonDocument document = QJsonDocument::fromJson(array, &err);//QJsonDocument document = QJsonDocument::fromJson(QString::fromStdString(std::string(running_info)).toLocal8Bit().data(), &err);
//        if (err.error != QJsonParseError::NoError) {
//            qDebug() << "Json type error";
//            return;
//        }
//        if (document.isNull() || document.isEmpty())
//            return;

//        if (document.isArray()) {
//            QJsonArray jsonArray = document.array();
//            foreach (QJsonValue val, jsonArray) {
//                QJsonObject subObject = val.toObject();
//                qDebug() << "Test App: " << subObject.value("app_name").toString() << subObject.value("package_name").toString() << subObject.value("version_name").toString();
//                hadItems = true;
//            }
//        }
//    }, Qt::QueuedConnection);
//    connect(m_installWidget, &InstallWidget::requestCopyApkFile, this, [=] (const QString &fileName, const QString &apkPath, const QString &destPath) {
//        CopyThread *thread = new CopyThread;
//        thread->setFileAndPathName(fileName, apkPath, destPath);
//        connect(thread, &CopyThread::copyFinished, m_installWidget, &InstallWidget::onCopyFinished);
//        connect(thread, &CopyThread::finished, thread, &CopyThread::deleteLater, Qt::QueuedConnection);
//        thread->start();
//    });
    m_backendThread->start();

    //TODO: 目前只支持一次安装一个apk包
    connect(m_importWidget, &ImportWidget::apkPackagesSelected, this, &MainWindow::onApkPackagesSelected);
    connect(this, &MainWindow::requestRunApp, m_backendWorker, &BackendWorker::launchApp);
    connect(m_infoWidget, &InfoPage::signalClosed, this, &MainWindow::close);
    connect(m_infoWidget, &InfoPage::signalRunApp, this, [=]() {
        //emit this->requestRunApp(m_installWidget->getPkgName());
        m_backendWorker->launchApp(m_installWidget->getPkgName());
        this->close();
        exit(0);
    });
    connect(this, &MainWindow::requestfileName, m_infoWidget, &InfoPage::settip);
}

void MainWindow::showWindow()
{
    bool androidReady = utils::isAndroidReady(m_loginUserName, m_loginUserId);
    if (!androidReady) {
        showInfoPage(tr("KMRE environment not installed,please go to the app store to start the mobile environment"), "", false);
    }
    emit this->requestGetAndroidEnvInstalled();
    this->move(qApp->primaryScreen()->geometry().center() - geometry().center());
    this->show();

//#ifdef UKUI_WAYLAND
//    // wayland 窗口无边框，需要在窗口show()之后执行
//    if (m_isWayland) {
//        UKUIDecorationManager::getInstance()->removeHeaderBar(this->windowHandle());
//        UKUIDecorationManager::getInstance()->setCornerRadius(this->windowHandle(), 12, 12, 12, 12);
//    }
//#endif
}

void MainWindow::keyPressEvent(QKeyEvent *e)
{
    switch (e->key())
    {
#ifdef QT_DEBUG
    case Qt::Key_Escape:        qApp->quit();       break;
#endif
    default:;
    }
}

void MainWindow::showInfoPage(const QString &info, const QString &pkgName, bool installSuccess)
{
    if (m_infoWidget) {
        m_centralLayout->setCurrentWidget(m_infoWidget);
        m_infoWidget->setInfo(info, pkgName, installSuccess);
    }
}

void MainWindow::onApkPackagesSelected(const QStringList &packages)
{
    if (packages.length() == 0)
        return;

    /*qDebug() << "packages:" << packages;
    for (const auto &package : packages) {

    }*/

    const QFileInfo fi(packages.at(0));
    if (fi.exists() && fi.isFile() && fi.suffix().toLower() == "apk") {
        m_centralLayout->setCurrentIndex(1);
        m_installWidget->setPkgPath(packages.at(0));
        m_installWidget->updateWaringInfo();
        emit this->requestAnalysisApkFile();
    }
}

void MainWindow::closeEvent(QCloseEvent *event)
{
//#ifdef UKUI_WAYLAND
//    // 规避wayland下QOpenGLWidget窗口关闭时导致core文件生成的问题
//    if (m_isWayland) {
//        QThread::usleep(100000);
//    }
//#endif

    QMainWindow::closeEvent(event);
}

void MainWindow::mousePressEvent(QMouseEvent *e)
{
    QMainWindow::mousePressEvent(e);
    if (e->button() == Qt::LeftButton && !e->isAccepted()) {
        m_is_draging = true;
        m_offset = mapFromGlobal(QCursor::pos());
    }
}

void MainWindow::mouseMoveEvent(QMouseEvent *e)
{
    //NOTE: when starting a X11 window move, the mouse move event
    //will unreachable when draging, and after draging we could not
    //get the release event correctly.
    //qDebug()<<"mouse move";
    QMainWindow::mouseMoveEvent(e);
    if (!m_is_draging)
        return;

    qreal  dpiRatio = qApp->devicePixelRatio();
    if (QX11Info::isPlatformX11()) {
        Display *display = QX11Info::display();
        Atom netMoveResize = XInternAtom(display, "_NET_WM_MOVERESIZE", False);
        XEvent xEvent;
        const auto pos = QCursor::pos();

        memset(&xEvent, 0, sizeof(XEvent));
        xEvent.xclient.type = ClientMessage;
        xEvent.xclient.message_type = netMoveResize;
        xEvent.xclient.display = display;
        xEvent.xclient.window = this->winId();
        xEvent.xclient.format = 32;
        xEvent.xclient.data.l[0] = pos.x() * dpiRatio;
        xEvent.xclient.data.l[1] = pos.y() * dpiRatio;
        xEvent.xclient.data.l[2] = 8;
        xEvent.xclient.data.l[3] = Button1;
        xEvent.xclient.data.l[4] = 0;

        XUngrabPointer(display, CurrentTime);
        XSendEvent(display, QX11Info::appRootWindow(QX11Info::appScreen()),
                   False, SubstructureNotifyMask | SubstructureRedirectMask,
                   &xEvent);
        //XFlush(display);

        XEvent xevent;
        memset(&xevent, 0, sizeof(XEvent));

        xevent.type = ButtonRelease;
        xevent.xbutton.button = Button1;
        xevent.xbutton.window = this->winId();
        xevent.xbutton.x = e->pos().x() * dpiRatio;
        xevent.xbutton.y = e->pos().y() * dpiRatio;
        xevent.xbutton.x_root = pos.x() * dpiRatio;
        xevent.xbutton.y_root = pos.y() * dpiRatio;
        xevent.xbutton.display = display;

        XSendEvent(display, this->effectiveWinId(), False, ButtonReleaseMask, &xevent);
        XFlush(display);

        if (e->source() == Qt::MouseEventSynthesizedByQt) {
            if (!this->mouseGrabber()) {
                this->grabMouse();
                this->releaseMouse();
            }
        }

        m_is_draging = false;
    } else {
        this->move((QCursor::pos() - m_offset) * dpiRatio);
    }
}

void MainWindow::mouseReleaseEvent(QMouseEvent *e)
{
    /*!
     * \bug
     * release event sometimes "disappear" when we request
     * X11 window manager for movement.
     */
    QMainWindow::mouseReleaseEvent(e);
    //qDebug()<<"mouse released";
    m_is_draging = false;
}

#ifdef SINGLE_INSTANCE
/***********************************************************
   Function:       handleMessageFromOtherInstances
   Description:    单实例情况下，若正在运行时，此时如果若再次打开一个进程，则会走这里
   Calls:
   Called By:
   Input:
   Output:
   Return:
   Others:
 ************************************************************/
void MainWindow::handleMessageFromOtherInstances(const QString& message)
{
    if (m_installWidget && m_installWidget->isInstalling()) {
        return;
    }

    int pos = message.indexOf(' ');
    if (pos > -1) {
        QString command = message.left(pos);
        QString arg = message.mid(pos+1);
        if (command == "open_files") {
            QStringList apkFileList = arg.split(" <<sep>> ");
            this->show();
            this->onApkPackagesSelected(apkFileList);
        }
    }
}
#endif

