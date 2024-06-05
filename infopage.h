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

#ifndef INFOPAGE_H
#define INFOPAGE_H

#include <QWidget>
#include <QPushButton>
#include <QLabel>

class InfoPage : public QWidget
{
    Q_OBJECT

public:
    explicit InfoPage(QWidget *parent = 0);

    void setInfo(const QString &info, const QString &pkgName, bool installSuccess);

public slots:
    void settip(const QString &fileName);

signals:
    void signalClosed();
    void signalRunApp();

private:
    QPushButton *m_closeBtn = nullptr;
    QPushButton *m_runBtn = nullptr;
    QLabel *m_tipLabel = nullptr;
    QWidget *m_infoWidget = nullptr;
};

#endif // INSTALLPAGE_H
