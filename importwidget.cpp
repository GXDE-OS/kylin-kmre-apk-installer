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

#include "importwidget.h"
#include "common.h"

#include <QDebug>
#include <QVBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QDragEnterEvent>
#include <QFileInfo>
#include <QMimeData>
#include <QUrl>
#include <QDir>
#include <QPixmap>
#include <QPainter>
#include <QFileDialog>

ImportWidget::ImportWidget(QWidget *parent)
    : QWidget(parent)
    , m_mainLayout(new QVBoxLayout)
{
    this->setBackgroundRole(QPalette::Base);
    this->setAutoFillBackground(true);
    this->setAcceptDrops(true);
    this->setAttribute(Qt::WA_NoMousePropagation);

//    const auto ratio = 1.0;
//    QPixmap iconPix = QPixmap(":/res/kmre.svg");
//    iconPix.setDevicePixelRatio(ratio);

    QLabel *iconLabel = new QLabel;
    iconLabel->setFixedSize(96, 96);
    iconLabel->setPixmap(QIcon(":/res/kmre.svg").pixmap(iconLabel->size()));

    QLabel *msgLabel = new QLabel;
    msgLabel->setText(tr("Drag and drop APK file here"));
    msgLabel->setAlignment(Qt::AlignCenter);

    QLabel *warningLabel = new QLabel;
    warningLabel->setStyleSheet("QLabel {color: #ff5a5a;}");
    warningLabel->setWordWrap(true);
    warningLabel->adjustSize();
    //warningLabel->setFixedHeight(50);
    warningLabel->setFixedWidth(WINDOW_WIDTH -20);
    warningLabel->setAlignment(Qt::AlignHCenter | Qt::AlignTop);
    warningLabel->setText(tr("Note: installing the third party APK may run abnormally"));

    QVBoxLayout *layout = new QVBoxLayout;
    layout->setSpacing(10);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(iconLabel);
    layout->setAlignment(iconLabel, Qt::AlignTop | Qt::AlignHCenter);
    layout->addWidget(msgLabel, 0, Qt::AlignHCenter);
    layout->addWidget(warningLabel, 0, Qt::AlignHCenter);

    QFrame *msgFrame = new QFrame;
    msgFrame->setFixedWidth(WINDOW_WIDTH);
    msgFrame->setLayout(layout);

    m_chooseBtn = new QPushButton;
    m_chooseBtn->setFixedSize(180, 36);
    m_chooseBtn->setFocusPolicy(Qt::NoFocus);
    m_chooseBtn->setText(tr("Select APK File"));

    m_mainLayout->setSpacing(10);
    m_mainLayout->setContentsMargins(0, 0, 0, 0);
    m_mainLayout->addStretch();
    m_mainLayout->addWidget(msgFrame);
    m_mainLayout->setAlignment(msgFrame, Qt::AlignTop | Qt::AlignCenter);
    m_mainLayout->addWidget(m_chooseBtn);
    m_mainLayout->setAlignment(m_chooseBtn, Qt::AlignCenter);
    m_mainLayout->addStretch();
    this->setLayout(m_mainLayout);

    connect(m_chooseBtn, &QPushButton::clicked, this, [=] () {
        QString historyDir = QDir::homePath();
        QFileDialog dialog;
        dialog.setFileMode(QFileDialog::ExistingFiles);
        dialog.setNameFilter("Android Package Files (*.apk)");
        dialog.setDirectory(historyDir);

        const int mode = dialog.exec();
        if (mode != QDialog::Accepted) {
            return;
        }

        const QStringList selected_files = dialog.selectedFiles();
        emit apkPackagesSelected(selected_files);
    });
}

ImportWidget::~ImportWidget()
{
    QLayoutItem *child;
    while ((child = m_mainLayout->takeAt(0)) != 0) {
        if (child->widget())
            child->widget()->deleteLater();
        delete child;
    }
}

void ImportWidget::dragEnterEvent(QDragEnterEvent *e)
{
    auto * const mime = e->mimeData();
    if (!mime->hasUrls()) {
        return e->ignore();
    }

    for (const auto &item : mime->urls()) {
        const QFileInfo info(item.path());
        if (info.isDir()) {
            return e->accept();
        }
        if (info.isFile() && info.suffix().toLower() == "apk") {
            return e->accept();
        }
    }

    e->ignore();
}

void ImportWidget::dropEvent(QDropEvent *e)
{
    auto * const mime = e->mimeData();
    if (!mime->hasUrls()) {
        return e->ignore();
    }

    e->accept();

    QStringList apkFileList;
    for (const auto &url : mime->urls())  {
        if (!url.isLocalFile()) {
            continue;
        }
        const QString local_path = url.toLocalFile();
        const QFileInfo info(local_path);
        if (info.isFile() && info.suffix().toLower() == "apk") {
            apkFileList << local_path;
        }
        else if (info.isDir()) {
            for (auto deb : QDir(local_path).entryInfoList(QStringList() << "*.apk", QDir::Files))
                apkFileList << deb.absoluteFilePath();
        }
    }

    apkFileList.sort();
    emit apkPackagesSelected(apkFileList);
}

void ImportWidget::dragMoveEvent(QDragMoveEvent *e)
{
    e->accept();
}
