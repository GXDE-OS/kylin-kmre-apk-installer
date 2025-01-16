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

#ifndef BACKENDWORKER_H
#define BACKENDWORKER_H

#include <QObject>

class BackendWorker : public QObject
{
    Q_OBJECT
public:
    explicit BackendWorker(const QString &userName, const QString &userId, QObject *parent = 0);
    ~BackendWorker();

    //void setIconFilePath(const QString &path) {m_iconFilePath = path;}
    void getCpuAndGpuInfo(QString &cpuType, QString &gpuVendor);
    int isAndroidEnvInstalled();

public slots:
    void updateDekstopAndIcon(const QString &pkgName, const QString &application, const QString &applicationZh, const QString &version);
    bool generateDesktop(const QString &pkgName, const QString &application, const QString &applicationZh, const QString &version);
    void generateIcon(const QString &pkgName);
//    void getInstalledAppList();
    QByteArray getInstalledAppListJsonStr();
    bool installApp(const QString &fileName, const QString &pkgName, const QString &application, const QString &applicationZh, const QString &version);
    bool unIntallApp(const QString &appName);
    void launchApp(const QString &packageName);
    void onGetAndroidEnvInstalled();

signals:
//    void sendInstalledAppList(const QByteArray &array);
    void installFinished(const QString &fileName, const QString &pkgName, bool result);
    void unInstallFinished(const QString &appName, bool result);
    void sigAndroidEnvInstalled(int value);

private:
    QString m_loginUserName;
    QString m_loginUserId;
    QString m_osVersion;
//    QString m_iconFilePath;
};

#endif // BACKENDWORKER_H
