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

#include "copythread.h"
#include "utils.h"

CopyThread::CopyThread(QObject *parent)
    : QThread(parent)
{

}


void CopyThread::run()
{
    bool b = kmre::utils::copyFile(m_apkPath, QString("%1/%2").arg(m_destPath).arg(m_fileName));
    emit copyFinished(m_fileName, b);
}
