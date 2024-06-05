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

#ifndef INSTALLWIDGET_H
#define INSTALLWIDGET_H

#include <QWidget>
#include <QLabel>
#include <QPushButton>
#include <QProgressBar>
#include <QVBoxLayout>

#include "backendworker.h"

#include <QWidget>
#include <QLabel>
#include <QScrollArea>
#include <QVector>

class QFormLayout;
class QVBoxLayout;

class ApkInfo
{
public:
    QString application_label;
    QString application_zh_label;
    QString name;
    QString versionName;
    QString size;
};

class InstallWidget : public QScrollArea
{
    Q_OBJECT
public:
    explicit InstallWidget(const QString &userName, const QString &userId, QWidget *parent = 0);
    ~InstallWidget();

    void setPkgPath(const QString &pkgPath);
    QString getPkgName() { return m_pkgName; }//TODO
    void setBackendWorker(BackendWorker *worker) {m_backendWorker = worker;}
    void appendLine(const char *key, const QString &value);
    void updateWaringInfo();
    bool isInstalling() { return m_installing; }

public slots:
    void onAnalysisApkFile();
    void installApp();
    void onTimerOut();
    void reset();
    void onCopyFinished(const QString &fileName, bool success);

signals:
    void back() const;
    void requestInstallApp(const QString &fileName, const QString &pkgName, const QString &application, const QString &applicationZh, const QString &version);
    void signalAndroidNotReady();
    void sigError(int code);

private:
    void clearLayout(QLayout* layout);
    const QString trLabel(const char *str);
    void updateApkInfo(ApkInfo info);

private:
    int m_maxTitleWidth;
    int m_maxFieldWidth;
    QFormLayout* m_infoLayout;
//    QFormLayout* m_exifLayout_details;
    QLabel* m_separator;

    QString m_pkgPath;
    QString m_pkgName;
    QString m_loginUserName;
    QString m_loginUserId;
    QVBoxLayout *m_mainLayout = nullptr;
    QLabel *m_iconLabel = nullptr;
    QLabel *m_tipsLabel = nullptr;
    QProgressBar *m_progressBar = nullptr;
    QLabel *m_warningLabel = nullptr;
    QPushButton *m_installButton = nullptr;
    QPushButton *m_backButton = nullptr;
    int m_progressValue;
    QTimer *m_timer = nullptr;
//    QString m_iconFilePath;
    QString m_application;
    QString m_applicationZh;
    QString m_version;
    BackendWorker *m_backendWorker = nullptr;
    bool m_installing;
    QWidget *tipWidget = nullptr;
};

#endif // INSTALLWIDGET_H
