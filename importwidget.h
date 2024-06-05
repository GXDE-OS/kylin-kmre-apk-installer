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

#ifndef IMPORTWIDGET_H
#define IMPORTWIDGET_H

#include <QWidget>

class QPushButton;
class QVBoxLayout;

class ImportWidget : public QWidget
{
    Q_OBJECT

public:
    explicit ImportWidget(QWidget *parent = nullptr);
    ~ImportWidget();

signals:
    void apkPackagesSelected(const QStringList files) const;

protected:
    void dragEnterEvent(QDragEnterEvent *e) Q_DECL_OVERRIDE;
    void dropEvent(QDropEvent *e) Q_DECL_OVERRIDE;
    void dragMoveEvent(QDragMoveEvent *e) Q_DECL_OVERRIDE;

private:
    QVBoxLayout *m_mainLayout = nullptr;
    QPushButton *m_chooseBtn = nullptr;
};

#endif // IMPORTWIDGET_H
