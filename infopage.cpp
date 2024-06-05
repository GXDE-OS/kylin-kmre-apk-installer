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

#include "infopage.h"

#include <QApplication>
#include <QBoxLayout>

InfoPage::InfoPage(QWidget *parent)
    : QWidget(parent)
    , m_closeBtn(new QPushButton)
    , m_runBtn(new QPushButton)
    , m_tipLabel(new QLabel)
    , m_infoWidget(new QWidget)
{
    this->setBackgroundRole(QPalette::Base);
    this->setAutoFillBackground(true);

    QHBoxLayout *btnsLayout = new QHBoxLayout;
    btnsLayout->setSpacing(30);
    btnsLayout->addStretch();
    btnsLayout->addWidget(m_runBtn);
    btnsLayout->addWidget(m_closeBtn);
    btnsLayout->addStretch();
    btnsLayout->setMargin(0);

    m_closeBtn->setText(tr("Close"));
    m_closeBtn->setFixedSize(120, 36);

    m_runBtn->setText(tr("Run"));
    m_runBtn->setVisible(false);
    m_runBtn->setFixedSize(120, 36);

    m_tipLabel->setAlignment(Qt::AlignCenter);
    m_tipLabel->setStyleSheet("QLabel {padding: 20px 0 0 0;}");

    QVBoxLayout *centerLayout = new QVBoxLayout;
    centerLayout->addWidget(m_tipLabel);
    centerLayout->setMargin(0);

    m_infoWidget->setLayout(centerLayout);

    QVBoxLayout *centralLayout = new QVBoxLayout;
    centralLayout->addStretch();
    centralLayout->addWidget(m_infoWidget);
    centralLayout->addSpacing(50);
    centralLayout->addLayout(btnsLayout);
    centralLayout->addStretch();
    centralLayout->setSpacing(0);
    centralLayout->setContentsMargins(20, 10, 20, 10);

    this->setLayout(centralLayout);

    connect(m_closeBtn, &QPushButton::clicked, this, [=]() {
        emit signalClosed();
    });
    connect(m_runBtn, &QPushButton::clicked, this, [=]() {
        m_runBtn->setVisible(false);
        emit signalRunApp();
    });
}

void InfoPage::setInfo(const QString &info, const QString &pkgName, bool installSuccess)
{
    m_tipLabel->setText(info);
    if (installSuccess && !pkgName.isEmpty()) {
        emit signalRunApp();
        //m_runBtn->setVisible(true);
    }
}

void InfoPage::settip(const QString &fileName)
{
    m_tipLabel->setToolTip(fileName);
}
