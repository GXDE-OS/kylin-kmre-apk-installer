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

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QStackedLayout>
#include <QThread>

class TitleBar;
class ImportWidget;
class InstallWidget;
class InfoPage;
class BackendWorker;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    virtual ~MainWindow();

    void initTitleBar();
    void initBackend();
    void showWindow();

public slots:
    // Single instance stuff
#ifdef SINGLE_INSTANCE
    void handleMessageFromOtherInstances(const QString& message);
#endif

    void onApkPackagesSelected(const QStringList &packages);
    void showInfoPage(const QString &info, const QString &pkgName, bool installSuccess);

signals:
    void requestAnalysisApkFile();
    void requestRunApp(const QString &packageName);
    void requestGetAndroidEnvInstalled();
    void requestfileName(const QString &fileName);

protected:
    void keyPressEvent(QKeyEvent *e) Q_DECL_OVERRIDE;
    void closeEvent(QCloseEvent *event) Q_DECL_OVERRIDE;
    void mouseReleaseEvent(QMouseEvent *e) Q_DECL_OVERRIDE;
    void mousePressEvent(QMouseEvent *e) Q_DECL_OVERRIDE;
    void mouseMoveEvent(QMouseEvent *e) Q_DECL_OVERRIDE;

private:
    QVBoxLayout *m_mainLayout = nullptr;
    QStackedLayout *m_centralLayout = nullptr;
    TitleBar *m_titleBar = nullptr;
    ImportWidget *m_importWidget = nullptr;
    InstallWidget *m_installWidget = nullptr;
    InfoPage *m_infoWidget = nullptr;
//    bool m_isWayland = false;
    QString m_loginUserName;
    QString m_loginUserId;
    BackendWorker *m_backendWorker = nullptr;
    QThread *m_backendThread = nullptr;
    bool m_is_draging = false;
    QPoint m_offset;
};

#endif // MAINWINDOW_H
