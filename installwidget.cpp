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

#include "installwidget.h"
#include "common.h"
#include "utils.h"
#include "copythread.h"
#include "textlabel.h"

#include <QApplication>
#include <QVBoxLayout>
#include <QDebug>
#include <QTimer>
#include <QProcess>
#include <QFileInfo>
#include <QDir>

using namespace kmre;

#include <QApplication>
#include <QBoxLayout>
#include <QDateTime>
#include <QFileInfo>
#include <QFormLayout>
#include <QLabel>
#include <QString>
#include <QPushButton>
#include <QScrollBar>
#include <QtDebug>
#include <QDialog>

namespace {

const int TITLE_MAXWIDTH = 160;

struct MetaData {
    QString key;
    const char *name;
};

static MetaData MetaDataList[] = {
    {"AppName",            QT_TRANSLATE_NOOP("MetadataName", "Name")},
    {"Version",          QT_TRANSLATE_NOOP("MetadataName", "Version")},
    {"Application",    QT_TRANSLATE_NOOP("MetadataName", "Application")},
    {"FileSize",            QT_TRANSLATE_NOOP("MetadataName", "File size")},
    {"", ""}
};

static int maxTitleWidth()
{
    int maxWidth = 0;
    QFont tf;
//    tf.setPixelSize(11);
    for (const MetaData* i = MetaDataList; ! i->key.isEmpty(); ++i) {
        maxWidth = qMax(maxWidth + 1, utils::stringWidth(tf, i->name));
    }

    return maxWidth;
}

}  // namespace


InstallWidget::InstallWidget(const QString &userName, const QString &userId, QWidget *parent)
    : QScrollArea(parent)
    , m_loginUserName(userName)
    , m_loginUserId(userId)
    , m_progressBar(new QProgressBar)
    , m_installButton(new QPushButton(tr("Install")))
    , m_backButton(new QPushButton(tr("Back")))
    , m_iconLabel(new QLabel)
    , m_tipsLabel(new QLabel)
    , m_pkgName(QString())
    , m_timer(new QTimer(this))
    , m_progressValue(0)
    , m_maxTitleWidth(maxTitleWidth())
    , m_installing(false)
{
    this->setStyleSheet("QScrollArea{background: transparent;border-left: none;border-top : none;border-right : 1px solid rgba(0, 0, 0, .1);border-bottom: none;}");

    this->setFrameStyle(QFrame::NoFrame);
    this->setWidgetResizable(true);
//    this->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    this->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    this->verticalScrollBar()->setContextMenuPolicy(Qt::PreventContextMenu);

    QFrame *content = new QFrame();
    m_mainLayout = new QVBoxLayout(content);
    m_mainLayout->setContentsMargins(5, 10, 5, 10);
    m_mainLayout->setSpacing(0);

    m_installButton->setFixedSize(96, 32);
    m_backButton->setFixedSize(96, 32);
    m_installButton->setFocusPolicy(Qt::NoFocus);
    m_backButton->setFocusPolicy(Qt::NoFocus);

    QHBoxLayout *btnLayout = new QHBoxLayout;
    btnLayout->setSpacing(30);
    btnLayout->setContentsMargins(0, 0, 0, 0);
    btnLayout->addStretch();
    btnLayout->addWidget(m_installButton);
    btnLayout->addWidget(m_backButton);
    btnLayout->addStretch();

    m_iconLabel->setFixedSize(46, 46);
    m_tipsLabel->setAlignment(Qt::AlignCenter);
    m_tipsLabel->setFixedHeight(30);
    //m_tipsLabel->setStyleSheet("QLabel{font-family: Noto Sans CJK SC;}");//m_tipsLabel->setStyleSheet("QLabel{color: #888888;}");//font-size: 14px;
    m_tipsLabel->setVisible(false);

    m_progressBar = new QProgressBar(this);
    m_progressBar->setTextVisible(false);
    m_progressBar->setValue(0);
    m_progressBar->setFixedSize(WINDOW_WIDTH - 20, 10);
    m_progressBar->setStyleSheet("QProgressBar {height: 10px;border-radius: 5px;background-color: #C2CCD0;}QProgressBar::chunk {border-radius: 5px;background:qlineargradient(spread:pad, x1:0,y1:0,x2:1,y2:0,stop:0 #01FAFF,stop:1 #26B4FF);}");
    m_progressBar->setVisible(false);

    m_warningLabel = new QLabel;
    m_warningLabel->setStyleSheet("QLabel {color: #ff5a5a;}");
    m_warningLabel->setWordWrap(true);
    m_warningLabel->adjustSize();
    //m_warningLabel->setFixedHeight(50);
    m_warningLabel->setFixedWidth(WINDOW_WIDTH -20);
    m_warningLabel->setAlignment(Qt::AlignHCenter | Qt::AlignTop);
    m_warningLabel->setText(tr("Note: installing the third party APK may run abnormally"));

    m_mainLayout->addStretch();
    m_mainLayout->addWidget(m_iconLabel, 0, Qt::AlignHCenter);

    QFrame *infoWidget = new QFrame();
    m_infoLayout = new QFormLayout(infoWidget);
    m_infoLayout->setSpacing(3);
    m_infoLayout->setContentsMargins(10, 0, 10, 0);
//    m_infoLayout->setLabelAlignment(Qt::AlignLeft);
    m_mainLayout->setAlignment(m_infoLayout, Qt::AlignCenter);
    m_separator = new QLabel();
    m_separator->setFixedHeight(1);
    m_separator->setStyleSheet("QLabel{background: rgba(0, 0, 0, .1);}");
    m_separator->setVisible(false);
    m_mainLayout->addWidget(infoWidget, 0, Qt::AlignHCenter);
    m_mainLayout->addSpacing(5);
    m_mainLayout->addWidget(m_separator);

    m_mainLayout->addWidget(m_tipsLabel);
    m_mainLayout->addSpacing(5);
    m_mainLayout->addWidget(m_progressBar);
    m_mainLayout->setAlignment(m_progressBar, Qt::AlignHCenter);
    m_mainLayout->addSpacing(5);
    m_mainLayout->addLayout(btnLayout);
    m_mainLayout->addSpacing(5);
    m_mainLayout->addWidget(m_warningLabel);
    m_mainLayout->addStretch();
    this->setWidget(content);

    tipWidget = new QDialog(this);
    tipWidget->setWindowTitle(tr("Risk warning"));
    tipWidget->setFixedSize(300,150);
    QLabel *tip1 = new QLabel(tr("This application is not restricted and may have security risks"));
    tip1->setWordWrap(true);
    QLabel *tip2 = new QLabel(tr("Whether to continue the installation?"));
    tip2->setWordWrap(true);
    QPushButton *okBtn = new QPushButton(tr("ok"));
    QPushButton *cancelBtn = new QPushButton(tr("cancel"));
    QHBoxLayout *Btnlayout = new QHBoxLayout;
    Btnlayout->addWidget(cancelBtn, 0, Qt::AlignHCenter);
    Btnlayout->addWidget(okBtn, 0, Qt::AlignHCenter);
    QVBoxLayout *mainlayout = new QVBoxLayout(tipWidget);
    mainlayout->addStretch();
    mainlayout->addWidget(tip1, 0, Qt::AlignHCenter);
    mainlayout->addWidget(tip2, 0, Qt::AlignHCenter);
    mainlayout->addStretch();
    mainlayout->addLayout(Btnlayout);
    mainlayout->addSpacing(15);

    connect(m_installButton, &QPushButton::clicked, this, [=]() {
        tipWidget->show();
    });
    connect(cancelBtn, &QPushButton::clicked, this, [=]() {tipWidget->close();});
    connect(okBtn, &QPushButton::clicked, this, &InstallWidget::installApp);
    connect(okBtn, &QPushButton::clicked, this, [=]() {tipWidget->close();});
//    connect(m_installButton, &QPushButton::clicked, this, &InstallWidget::installApp);
    connect(m_backButton, &QPushButton::clicked, this, [=] () {
        this->reset();
        emit this->back();
    });

    m_timer->setTimerType(Qt::PreciseTimer);
    connect(m_timer, SIGNAL(timeout()), this, SLOT(onTimerOut()));
}

InstallWidget::~InstallWidget()
{
    m_installing = false;
    QLayoutItem *child;
    while ((child = m_mainLayout->takeAt(0)) != 0) {
        if (child->widget())
            child->widget()->deleteLater();
        delete child;
    }
}

void InstallWidget::clearLayout(QLayout *layout)
{
    QFormLayout *fl = static_cast<QFormLayout *>(layout);
    if (fl) {
        for (int i = 0; i < fl->rowCount(); i++) {
            QLayoutItem *li = fl->itemAt(i, QFormLayout::LabelRole);
            QLayoutItem *fi = fl->itemAt(i, QFormLayout::FieldRole);
            if (li) {
                if (li->widget()) delete li->widget();
                fl->removeItem(li);
            }
            if (fi) {
                if (fi->widget()) delete fi->widget();
                fl->removeItem(fi);
            }
        }
    }
}

const QString InstallWidget::trLabel(const char *str)
{
    return qApp->translate("MetadataName", str);
}

void InstallWidget::updateWaringInfo()
{

    if (m_backendWorker) {
        QString cpuType = QString("");
        QString gpuVendor = QString("");
        m_backendWorker->getCpuAndGpuInfo(cpuType, gpuVendor);
        if (cpuType == "FT1500A") {
            m_warningLabel->setText(tr("Note: installing the third party APK may run abnormally\nThe CPU is FT1500A, limited by CPU performance, video App and game App are not effective."));
        }
        else {
            if (gpuVendor == "GP101") {
                m_warningLabel->setText(tr("Note: installing the third party APK may run abnormally\nThe graphics card is %1, limited by graphics card performance, video App and game App are not effective.").arg("GP101"));
            }
            if (gpuVendor == "ZHAOXIN") {
                m_warningLabel->setText(tr("Note: installing the third party APK may run abnormally\nThe graphics card is %1, limited by graphics card performance, video App and game App are not effective.").arg(tr("Zhaoxin")));
            }
            if (gpuVendor == "VIRTUAL") {
                m_warningLabel->setText(tr("Note: installing the third party APK may run abnormally\nThe graphics card is %1, limited by graphics card performance, video App and game App are not effective.").arg(tr("virtual graphics card")));
            }
            if (gpuVendor == "JJM") {
                m_warningLabel->setText(tr("Note: installing the third party APK may run abnormally\nThe graphics card is %1, limited by graphics card performance, video App and game App are not effective.").arg(tr("JJM")));
            }
        }
    }
}

void InstallWidget::appendLine(const char *key, const QString &value)
{
    //qDebug() << "appendLine key:" << QString(key) << ",value:" << value;
    TextFieldLabel *field = new TextFieldLabel;
    field->setAlignment(Qt::AlignLeft | Qt::AlignTop);
    field->setText(utils::wrapStr(value, field->font(), m_maxFieldWidth));

    TextTitleLabel *title = new TextTitleLabel(trLabel(key) + ":");
    title->setMinimumHeight(field->minimumHeight());
    title->setFixedWidth(qMin(m_maxTitleWidth, TITLE_MAXWIDTH));
    title->setAlignment(Qt::AlignRight | Qt::AlignTop);

    m_infoLayout->addRow(title, field);
}

void InstallWidget::updateApkInfo(ApkInfo info)
{
    m_maxFieldWidth = width() - m_maxTitleWidth - (10 + 8) * 2;

    clearLayout(m_infoLayout);

    for (MetaData *i = MetaDataList; ! i->key.isEmpty(); i ++) {
        if (i->key == "AppName") {
            if (!info.name.isEmpty()) {
                this->appendLine(i->name, info.name);
            }
        }
        else if (i->key == "Version") {
            if (!info.versionName.isEmpty()) {
                this->appendLine(i->name, info.versionName);
            }
        }
        else if (i->key == "Application") {
            if (!info.application_label.isEmpty()) {
                this->appendLine(i->name, info.application_label);
            }
        }
//        else if (i->key == "ApplicationZh") {
//            if (!info.application_zh_label.isEmpty()) {
//                this->appendLine(i->name, info.application_zh_label);
//            }
//        }
        else if (i->key == "FileSize") {
            if (!info.size.isEmpty()) {
                this->appendLine(i->name, info.size);
            }
        }
    }

    m_separator->setVisible(m_infoLayout->count() > 0);
}

void InstallWidget::onTimerOut()
{
    if (m_progressValue >= 100) {
        m_progressValue = 0;
    }
    m_progressBar->setValue(m_progressValue++);
}

void InstallWidget::setPkgPath(const QString &pkgPath)
{
    m_pkgPath = pkgPath;
}

void InstallWidget::reset()
{
    m_installing = false;
    m_warningLabel->setText(tr("Note: installing the third party APK may run abnormally"));
    m_timer->stop();
    m_progressValue = 0;
    m_progressBar->setVisible(false);
    m_installButton->setVisible(true);
    m_backButton->setVisible(true);
    m_progressBar->setValue(0);
    m_tipsLabel->clear();
    m_tipsLabel->setVisible(false);
}

void InstallWidget::onCopyFinished(const QString &fileName, bool success)
{
    m_tipsLabel->setVisible(true);
    if (success) {
        //install apk file
        m_tipsLabel->setText(tr("Installing APK ......"));
        emit this->requestInstallApp(fileName, m_pkgName, m_application, m_applicationZh, m_version);
    }
    else {
        m_installing = false;
        m_tipsLabel->setText(tr("Copy APK failed"));
        m_timer->stop();
        m_progressValue = 0;
        m_progressBar->setVisible(false);
        m_backButton->setVisible(true);
    }
}

void InstallWidget::installApp()
{
    bool androidReady = utils::isAndroidReady(m_loginUserName, m_loginUserId);
    if (!androidReady) {
        emit signalAndroidNotReady();
        return;
    }

    m_timer->start(20);
    m_progressValue = 0;
    m_progressBar->setVisible(true);
    m_installButton->setVisible(false);
    m_backButton->setVisible(false);
    m_progressBar->setValue(m_progressValue);
    m_tipsLabel->clear();
    m_tipsLabel->setVisible(true);
    m_tipsLabel->setText(tr("Copying APK ......"));

    QFileInfo fi(m_pkgPath);
    if (fi.exists()) {
        m_installing = true;
        QString fileName = fi.fileName();
        QString destPath = QString("/var/lib/kmre/kmre-%1-%2/data/local/tmp").arg(m_loginUserId).arg(m_loginUserName);
        //emit this->requestCopyApkFile(fileName, m_pkgPath, destPath);
        CopyThread *thread = new CopyThread;
        thread->setFileAndPathName(fileName, m_pkgPath, destPath);
        connect(thread, &CopyThread::copyFinished, this, &InstallWidget::onCopyFinished);
        connect(thread, &CopyThread::finished, thread, &CopyThread::deleteLater, Qt::QueuedConnection);
        thread->start();
    }
    else {
        this->reset();
        m_tipsLabel->setText(tr("APK does not exists"));
    }
}

// aapt dump badging test.apk
void InstallWidget::onAnalysisApkFile()
{
//    m_iconFilePath.clear();
    m_application.clear();
    m_applicationZh.clear();
    m_pkgName.clear();
    m_version.clear();

    QFileInfo file_info(m_pkgPath);
    if (!file_info.exists()) {
        emit this->sigError(0);
        return;
    }

    qApp->processEvents();

    QString application_label = "";
    QString application_zh_label = "";
    QString fileSize = "";
    QString iconPath = "";
    QString version = "";
    QProcess process;

    //fileSize = QString("%1 Bytes").arg(file_info.size());
    fileSize = QString("%1 KB (%2 MB)").arg(file_info.size()/1024).arg(file_info.size()/1048576);

#ifndef KYLIN_V10
    if (QFileInfo("/usr/bin/aapt").exists()) {
        process.start("aapt", QStringList()<< "d" << "badging" << m_pkgPath);
        process.waitForStarted(5*1000);
        process.waitForFinished(10*1000);//process.waitForFinished(-1);
        const QString output = process.readAllStandardOutput();//process.readAll();
        //qDebug() << "aapt output:" << output;
        if (output.isEmpty() || output.isNull()) {
            emit this->sigError(1);
            return;
        }
        for (const auto &line : output.split('\n')) {
            if (line.startsWith("package:")) {
                //qDebug() << "aapt aapt info:" << line;
                const QStringList &info = line.split(' ');
                if (info.size() >= 5) {
                    for (int i=0;i<info.size();i++) {
                        if (info.at(i).startsWith("name=")) {
                            const QStringList &nameinfo = info.at(i).split('=');
                            m_pkgName = nameinfo.at(1).replace("\'", "").trimmed();//com.rightware.BasemarkOSII
                        }
                        if (info.at(i).startsWith("versionName=")) {
                            const QStringList &versioninfo = info.at(i).split('=');
                            version = versioninfo.at(1).replace("\'", "").trimmed();//2.0
                            m_version = version;
                        }
                    }
                }
                //package: name='com.ludashi.benchmark' versionCode='169' versionName='9.1.2.19.0816' platformBuildVersionName=''
            }
            if (line.startsWith("application-label:")) {//可能不存在该行
                //qDebug() << "aapt application-label info:" << line;
                const QStringList &info = line.split(':');
                if (info.size() == 2) {
                    application_label = info.at(1).replace("\'", "").trimmed();
                }
            }
            if (line.startsWith("application-label-zh-CN:")) {//可能不存在该行
                //qDebug() << "aapt application-label-zh-CN info:" << line;
                const QStringList &info = line.split(':');
                if (info.size() == 2) {
                    application_zh_label = info.at(1).replace("\'", "").trimmed();
                }
            }
            if (line.startsWith("application-label-zh:")) {//可能不存在该行
                const QStringList &info = line.split(':');
                if (info.size() == 2) {
                    if (application_zh_label.isEmpty()) {
                        application_zh_label = info.at(1).replace("\'", "").trimmed();
                    }
                }
            }
            if (line.startsWith("application: label=")) {
                //qDebug() << "aapt application label info:" << line;// application: label='State Council' icon='res/drawable-anydpi-v26/application_icon.xml'
                int firstIndex = line.indexOf(QChar('\''));
                int secondIndex = line.indexOf(QChar('\''), strlen("application: label='"));
                QString labelContent = line.left(secondIndex);
                QString labelStr = labelContent.right(labelContent.length() - 1 - firstIndex);
                if (application_label.isEmpty()) {
                    application_label = labelStr;
                }
                if (application_zh_label.isEmpty()) {
                    application_zh_label = labelStr;
                }

                const QStringList &info = line.split(' ');
                for (int i=0;i<info.size();i++) {
//                    if (info.at(i).startsWith("label=")) {
//                        const QStringList &labelinfo = info.at(i).split('=');//labelinfo.at(1)可能为空，如：application: label='' icon='res/drawable-mdpi/ic_launcher.png'
//                        if (application_label.isEmpty()) {
//                            application_label = labelinfo.at(1).replace("\'", "").trimmed();
//                        }
//                        if (application_zh_label.isEmpty()) {
//                            application_zh_label = labelinfo.at(1).replace("\'", "").trimmed();
//                        }
//                    }
                    if (info.at(i).startsWith("icon=")) {
                        //icon path
                        if (iconPath.isEmpty()) {
                            const QStringList &iconinfo = info.at(i).split('=');
                            iconPath = iconinfo.at(1).replace("\'", "").trimmed();
                        }
                    }
                }
            }
            if (line.startsWith("launchable-activity:")) {//launchable-activity: name='com.rightware.BasemarkOSII.BasemarkOSII'  label='Basemark OS II' icon=''
                int firstIndex = line.indexOf(QChar('\''));
                int secondIndex = line.indexOf(QChar('\''), strlen("application: label='"));
                QString labelContent = line.left(secondIndex);
                QString labelStr = labelContent.right(labelContent.length() - 1 - firstIndex);
                if (application_label.isEmpty()) {
                    application_label = labelStr;
                }
                if (application_zh_label.isEmpty()) {
                    application_zh_label = labelStr;
                }

                const QStringList &info = line.split(' ');
                for (int i=0;i<info.size();i++) {
//                    if (info.at(i).startsWith("label=")) {
//                        const QStringList &labelinfo = info.at(i).split('=');
//                        if (application_label.isEmpty()) {
//                            application_label = labelinfo.at(1).replace("\'", "").trimmed();
//                        }
//                        if (application_zh_label.isEmpty()) {
//                            application_zh_label = labelinfo.at(1).replace("\'", "").trimmed();
//                        }
//                    }
                    if (info.at(i).startsWith("icon=")) {
                        //icon path
                        if (iconPath.isEmpty()) {
                            const QStringList &iconinfo = info.at(i).split('=');
                            iconPath = iconinfo.at(1).replace("\'", "").trimmed();
                        }
                    }
                }
            }
        }

        if (!application_label.isEmpty() && !m_pkgName.isEmpty()) {
            if (application_zh_label.isEmpty()) {
                application_zh_label = application_label;
            }
            //qDebug() << "application_label:" << application_label << ",m_pkgName:" << m_pkgName << ",iconPath:" << iconPath;
            m_application = application_label;
            m_applicationZh = application_zh_label;

            ApkInfo apk { application_label, application_zh_label, m_pkgName, version, fileSize };
            QPixmap pixmap;
            pixmap = QPixmap(":/res/kmre.svg");
            pixmap = pixmap.scaled(m_iconLabel->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
            m_iconLabel->setPixmap(pixmap);
            this->updateApkInfo(apk);
            m_installButton->setVisible(true);
            m_backButton->setVisible(true);
        }
    }
    else {
        ApkInfo apk { application_label, application_zh_label, file_info.fileName(), version, fileSize };
        QPixmap pixmap;
        pixmap = QPixmap(":/res/kmre.svg");
        pixmap = pixmap.scaled(m_iconLabel->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
        m_iconLabel->setPixmap(pixmap);
        this->updateApkInfo(apk);
        m_installButton->setVisible(true);
        m_backButton->setVisible(true);
    }
#else
    ApkInfo apk { application_label, application_zh_label, file_info.fileName(), version, fileSize };
    QPixmap pixmap;
    pixmap = QPixmap(":/res/kmre.svg");
    pixmap = pixmap.scaled(m_iconLabel->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
    m_iconLabel->setPixmap(pixmap);
    this->updateApkInfo(apk);
    m_installButton->setVisible(true);
    m_backButton->setVisible(true);
#endif
}
