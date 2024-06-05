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

#ifndef _KMRE_UTILS_H_
#define _KMRE_UTILS_H_

#include <Qt>
#include <QString>
#include <QFileInfo>

namespace kmre {
namespace utils {

#if defined(__aarch64__)
bool isFt1500aCpu();
#endif
const QString& getUserName();
const QString& getUid();
QString osVersion();
QString makeContainerName();
QString makeContainerName(const QString& userName, int uid);
QString readFile(const QString &path);
QString readFileContent(const QString &path);
bool isWayland();
bool copyFile(const QString &srcFile, const QString &destFile);
bool isAndroidReady(const QString &username, const QString &uid);
QString wrapStr(const QString &str, const QFont &font, int maxWidth);
int stringWidth(const QFont &font, const QString &str);
QByteArray encrypt(QByteArray md5);
QByteArray encrypt(const QByteArray &md5,const QByteArray &keyBase64);
void evpError(void);
void sendApkInfoToServer(const QString &os, const QString &cpuType, const QString &gpuVendor, const QString &pkgName, const QString &appName);

} // namespace utils
} // namespace kmre

#endif // _KMRE_UTILS_H_

