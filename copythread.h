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

#ifndef COPYTHREAD_H
#define COPYTHREAD_H

#include <QThread>
#include <QPointer>

class CopyThread : public QThread
{
    Q_OBJECT

public:
    explicit CopyThread(QObject *parent = nullptr);

    void setFileAndPathName(const QString &fileName, const QString &apkPath, const QString &destPath) { m_fileName = fileName; m_apkPath = apkPath; m_destPath = destPath; }
    void run();

signals:
    void copyFinished(const QString &fileName, bool success) const;

private:
    QString m_fileName;
    QString m_apkPath;
    QString m_destPath;
};

#endif // COPYTHREAD_H
