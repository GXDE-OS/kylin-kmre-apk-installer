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

#include "titlebar.h"
#include "common.h"

#include <QApplication>
#include <QLabel>
#include <QPushButton>
#include <QHBoxLayout>
#include <QVariant>
#include <QMouseEvent>
#include <QX11Info>
#include <X11/Xlib.h>


#define BUTTON_SIZE 30
#define BUTTON_SPACE 4

TitleBar::TitleBar(/*bool isWayland, */QWidget *parent) : QWidget(parent)
{
    //this->setStyleSheet("QWidget{background-color:transparent;}");
    this->setBackgroundRole(QPalette::Base);
    this->setAutoFillBackground(true);
    this->setFixedHeight(DEFAULT_TITLEBAR_HEIGHT);

    this->initUI();
    this->initConnect();
}

TitleBar::~TitleBar()
{
    QLayoutItem *child;
    while ((child = m_lLayout->takeAt(0)) != 0) {
        if (child->widget())
            child->widget()->deleteLater();
        delete child;
    }

    while ((child = m_mLayout->takeAt(0)) != 0) {
        if (child->widget())
            child->widget()->deleteLater();
        delete child;
    }

    while ((child = m_rLayout->takeAt(0)) != 0) {
        if (child->widget())
            child->widget()->deleteLater();
        delete child;
    }
}

void TitleBar::setTitle(const QString &title)
{
    m_titleLabel->setText(title);
}

void TitleBar::setIcon(const QString &iconName)
{
    //const QIcon &icon = QIcon::fromTheme(iconName, QIcon::fromTheme("application-x-desktop"));
    QIcon icon = QIcon::fromTheme("kmre-apk-installer");//application-apk
    if (icon.isNull()) {
        icon = QIcon(iconName);
    }
    if (!icon.isNull()) {
        m_iconLabel->setPixmap(icon.pixmap(m_iconLabel->size()));
    }
}

void TitleBar::initLeftContent()
{
    QWidget *w = new QWidget;
    m_lLayout = new QHBoxLayout(w);
    m_lLayout->setContentsMargins(8, 0, 0, 0);
    m_lLayout->setSpacing(8);
    m_layout->addWidget(w, 1, Qt::AlignLeft);

    m_iconLabel = new QLabel;
    m_lLayout->addWidget(m_iconLabel);
    m_iconLabel->setFixedSize(24, 24);

    m_titleLabel = new QLabel;
    m_titleLabel->adjustSize();
    m_lLayout->addWidget(m_titleLabel);
}

void TitleBar::initMiddleContent()
{
    QWidget *w = new QWidget;
    w->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    m_mLayout = new QHBoxLayout(w);
    m_mLayout->setContentsMargins(0, 4, 0, 0);
    m_mLayout->setSpacing(0);
    m_layout->addWidget(w);
}

void TitleBar::initRightContent()
{
    QWidget *w = new QWidget;
    m_rLayout = new QHBoxLayout(w);
    m_rLayout->setContentsMargins(0, 0, 4, 0);
    m_rLayout->setSpacing(BUTTON_SPACE);
    m_layout->addWidget(w, 1, Qt::AlignRight);

    m_minBtn = new QPushButton;
    m_minBtn->setFixedSize(BUTTON_SIZE, BUTTON_SIZE);
    m_minBtn->setFlat(true);
    m_minBtn->setToolTip(tr("Minimize"));

    m_closeBtn = new QPushButton;
    m_closeBtn->setFixedSize(BUTTON_SIZE, BUTTON_SIZE);
    m_closeBtn->setFlat(true);
    m_closeBtn->setToolTip(tr("Close"));

    m_minBtn->setProperty("useIconHighlightEffect", 0x2);
    m_minBtn->setProperty("isWindowButton", 0x01);
    m_closeBtn->setProperty("isWindowButton", 0x02);
    m_closeBtn->setProperty("useIconHighlightEffect", 0x08);

    // /usr/share/icons/ukui-icon-theme-default/scalable/actions/
    m_minBtn->setIcon(QIcon::fromTheme("window-minimize-symbolic"));
    m_closeBtn->setIcon(QIcon::fromTheme("window-close-symbolic"));

    m_rLayout->addWidget(m_minBtn);
    m_rLayout->addWidget(m_closeBtn);
}

void TitleBar::initUI()
{
    m_layout = new QHBoxLayout(this);
    m_layout->setContentsMargins(0, 0, 0, 0);
    m_layout->setSpacing(0);
    this->setLayout(m_layout);

    this->initLeftContent();
    this->initMiddleContent();
    this->initRightContent();
}

void TitleBar::initConnect()
{
    connect(m_closeBtn, &QPushButton::clicked, [this](){emit sigClose();});
    connect(m_minBtn, &QPushButton::clicked, [this]() {emit sigMiniSize();});
}

