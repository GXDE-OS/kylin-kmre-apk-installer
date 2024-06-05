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

#ifndef TITLEBAR_H
#define TITLEBAR_H

#include <QWidget>

class QLabel;
class QPushButton;
class QHBoxLayout;

class TitleBar : public QWidget
{
    Q_OBJECT
public:
    explicit TitleBar(/*bool isWayland = false, */QWidget *parent = nullptr);
    ~TitleBar();

    void initUI();
    void initConnect();
    void setTitle(const QString &title);
    void setIcon(const QString &iconName);

    void initLeftContent();
    void initMiddleContent();
    void initRightContent();

protected:
signals:
    void sigClose();
    void sigMiniSize();

private:
    QHBoxLayout *m_layout = nullptr;
    QHBoxLayout *m_lLayout = nullptr;
    QHBoxLayout *m_mLayout = nullptr;
    QHBoxLayout *m_rLayout = nullptr;
    QLabel *m_iconLabel = nullptr;
    QLabel *m_titleLabel = nullptr;
    QPushButton *m_minBtn = nullptr;
    QPushButton *m_closeBtn = nullptr;

};

#endif // TITLEBAR_H
