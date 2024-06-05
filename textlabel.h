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

#ifndef TEXTLABEL_H
#define TEXTLABEL_H

#include <QLabel>

class TextTitleLabel : public QLabel {

    Q_OBJECT
public:
    explicit TextTitleLabel(const QString &text, QWidget *parent = 0);
};

class TextFieldLabel : public QLabel {

    Q_OBJECT
public:
    explicit TextFieldLabel(QWidget *parent = 0);

protected:
    void resizeEvent(QResizeEvent* event);
};

#endif // TEXTLABEL_H
