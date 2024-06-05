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

#include "backendworker.h"
#include "utils.h"

#include <QApplication>
#include <QFile>
#include <QDebug>
#include <QThread>
#include <QJsonObject>
#include <QJsonDocument>
#include <QJsonArray>
#include <QProcess>
#include <QDir>
#include <QTimer>
#include <QtDBus/QDBusConnection>
#include <QtDBus/QDBusMessage>
#include <QtDBus/QDBusInterface>
#include <QSvgGenerator>
#include <QImage>
#include <QPainter>

#include <sys/types.h>
#include <sys/stat.h>
#include <dlfcn.h>

using namespace kmre;

bool convertPngToSvg(const QString &pngPath, const QString &svgPath)
{
    if (pngPath.isEmpty()) {
        return false;
    }
    if (!QFile::exists(pngPath)) {
        return false;
    }

    QFile fp(pngPath);
    fp.open(QIODevice::ReadOnly);
    QByteArray datas = fp.readAll();
    fp.close();

    QImage image;
    image.loadFromData(datas);
    image = image.scaled(96, 96, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    QSvgGenerator generator;
    generator.setFileName(svgPath);
    generator.setSize(image.size());
    generator.setViewBox(QRect(0, 0, image.width(), image.height()));
    generator.setTitle("SVG");
    generator.setDescription("Kmre SVG File");
    QPainter painter;
    painter.begin(&generator);
    painter.drawImage(QPoint(0,0), image);
    painter.end();

    return true;
}

BackendWorker::BackendWorker(const QString &userName, const QString &userId, QObject *parent) :
    QObject(parent)
    , m_loginUserName(userName)
    , m_loginUserId(userId)
{
//    m_iconFilePath.clear();
    m_osVersion = utils::osVersion();
}

BackendWorker::~BackendWorker()
{
}

void BackendWorker::getCpuAndGpuInfo(QString &cpuType, QString &gpuVendor)
{
    bool androidReady = utils::isAndroidReady(m_loginUserName, m_loginUserId);
    if (androidReady) {
        QDBusInterface mSessionBusInterface("cn.kylinos.Kmre.Manager", "/cn/kylinos/Kmre/Manager", "cn.kylinos.Kmre.Manager", QDBusConnection::sessionBus());
        QString displayInfo;
        QDBusMessage response = mSessionBusInterface.call("getDisplayInformation");
        if (response.type() == QDBusMessage::ReplyMessage) {
            displayInfo = response.arguments().takeFirst().toString();
            if (!displayInfo.isEmpty()) {
                QJsonDocument jsonDocument = QJsonDocument::fromJson(displayInfo.toLocal8Bit().data());
                if (!jsonDocument.isNull()) {
                    QJsonObject jsonObject = jsonDocument.object();
                    if (!jsonObject.isEmpty() && jsonObject.size() > 0) {
                        if (jsonObject.contains("cpu_type")) {
                            cpuType = jsonObject.value("cpu_type").toString();
                        }
                        if (jsonObject.contains("gpu_vendor")) {
                            gpuVendor = jsonObject.value("gpu_vendor").toString();
                        }
                    }
                }
            }
        }
    }

    if (cpuType.isNull() || cpuType.isEmpty()) {
#if defined(__aarch64__)
        if (utils::isFt1500aCpu()) {
            cpuType = "FT1500A";
        }
        else {
            cpuType = "unknown";
        }
#else
        cpuType = "unknown";
#endif
    }

    if (gpuVendor.isNull() || gpuVendor.isEmpty()) {
        gpuVendor = "unknown";
    }
}

void BackendWorker::updateDekstopAndIcon(const QString &pkgName, const QString &application, const QString &applicationZh, const QString &version)
{
    QString cpuType = QString("");
    QString gpuVendor = QString("");
    getCpuAndGpuInfo(cpuType, gpuVendor);

    //qDebug() << "updateDekstopAndIcon pkgName:" << pkgName << ",application:" << application << ",applicationZh:" << applicationZh;
    if (!pkgName.isEmpty() && !application.isEmpty() && !applicationZh.isEmpty()) {
        utils::sendApkInfoToServer(m_osVersion, cpuType, gpuVendor, pkgName, application);
        if (pkgName != "com.antutu.benchmark.full" && pkgName != "com.antutu.benchmark.full.lite") {
            this->generateDesktop(pkgName, application, applicationZh, version);
            this->generateIcon(pkgName);
        }
    }
    else {
        QByteArray array = this->getInstalledAppListJsonStr();
        //qDebug() << "Installed: " << QString(array);
        QJsonParseError err;
        QJsonDocument document = QJsonDocument::fromJson(array, &err);//QJsonDocument document = QJsonDocument::fromJson(QString::fromStdString(std::string(running_info)).toLocal8Bit().data(), &err);
        if (err.error == QJsonParseError::NoError && !document.isNull() && !document.isEmpty() && document.isArray()) {
            QJsonArray jsonArray = document.array();
            foreach (QJsonValue val, jsonArray) {
                QJsonObject subObject = val.toObject();
                QString name = subObject.value("package_name").toString();
                if (name != "com.antutu.benchmark.full" && name != "com.antutu.benchmark.full.lite") {
                    //qDebug() << "Test App: " << subObject.value("app_name").toString() << subObject.value("package_name").toString() << subObject.value("version_name").toString();
                    bool b = this->generateDesktop(name, subObject.value("app_name").toString(), subObject.value("app_name").toString(), subObject.value("version_name").toString());
                    if (b) {
                        qDebug() << subObject.value("package_name").toString() << "installed!";
                        utils::sendApkInfoToServer(m_osVersion, cpuType, gpuVendor, subObject.value("package_name").toString(), subObject.value("app_name").toString());
                    }
                    this->generateIcon(subObject.value("package_name").toString());
                }
            }
        }
    }
}

bool BackendWorker::generateDesktop(const QString &pkgName, const QString &application, const QString &applicationZh, const QString &version)
{
    if (pkgName.isEmpty()) {
        return false;
    }

    const QString desktopsPath = QString("%1/.local/share/applications").arg(QDir::homePath());
    if (!QDir().exists(desktopsPath)) {
        QDir().mkdir(desktopsPath);
    }

    //generate desktop file
    // ~/.local/share/applications
    const QString destDesktopFile = QString("%1/.local/share/applications/%2.desktop").arg(QDir::homePath()).arg(pkgName);
    QFile desktopFp(destDesktopFile);
    if (desktopFp.exists()) {
        desktopFp.remove();
    }
    QFile file(destDesktopFile);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        return false;
    }
    QTextStream out(&file);
    if (version.isNull() || version.isEmpty()) {
        out << "[Desktop Entry]\n"
            "Terminal=false\n"
            "Type=Application\n"
            "StartupNotify=false\n"
            "Keywords=Android;App;Apk\n"
            "Categories=Android;App;Apk\n"
            "Name=" + application + "\n"
            "Name[zh_CN]=" + applicationZh + "\n"
            "Comment=" + application + "\n"
            "Comment[zh_CN]=" + applicationZh + "\n"
            "Exec=/usr/bin/startapp " + pkgName + "\n"
            "Icon=" + QDir::homePath() + "/.local/share/icons/" + pkgName + ".svg"
            << endl;
    }
    else {
        out << "[Desktop Entry]\n"
            "Terminal=false\n"
            "Type=Application\n"
            "StartupNotify=false\n"
            "Keywords=Android;App;Apk\n"
            "Categories=Android;App;Apk\n"
            "Name=" + application + "\n"
            "Name[zh_CN]=" + applicationZh + "\n"
            "Comment=" + application + "\n"
            "Comment[zh_CN]=" + applicationZh + "\n"
            "Exec=/usr/bin/startapp " + pkgName + " " + version + "\n"
            "Icon=" + QDir::homePath() + "/.local/share/icons/" + pkgName + ".svg"
            << endl;
    }
    out.flush();
    file.close();

    chmod(destDesktopFile.toStdString().c_str(), 0777);
    //QProcess::startDetached(QString("/usr/bin/update-desktop-database %1").arg(desktopsPath));

    return true;
}

//copy icon file from /tmp
void BackendWorker::generateIcon(const QString &pkgName)
{
    if (pkgName.isEmpty()) {
        return;
    }

    const QString iconsPath = QString("%1/.local/share/icons").arg(QDir::homePath());
    if (!QDir().exists(iconsPath)) {
        QDir().mkdir(iconsPath);
    }

    //copy icon file from /tmp
    // ~/.local/share/icons
    const QString linuxIconPath = QString("%1/%2.png").arg(iconsPath).arg(pkgName);
    const QString linuxSvgPath = QString("%1/%2.svg").arg(iconsPath).arg(pkgName);
    const QString containerIconPath = QString("/var/lib/kmre/kmre-%1-%2/data/local/icons/%3.png").arg(m_loginUserId).arg(m_loginUserName).arg(pkgName);
    /*const QFileInfo fi(m_iconFilePath);
    QFile dfp(linuxIconPath);
    if (fi.isFile() && fi.suffix().toLower() != "xml" && !dfp.exists()) {//有的apk文件解析出来的图片为.xml格式
        utils::copyFile(m_iconFilePath, linuxIconPath);
        QProcess::startDetached(QString("/usr/sbin/update-icon-caches %1").arg(iconsPath));
    }*/
    //如果沒有从apk解压到icon，则在安装完成后从android目录讲对应apk的icon拷贝出来使用（注意：安装完成后，需要等待一些时间再做拷贝操作，因为测试发现android里面图片没有在安装完成后立即生成）
    QFile cfp(containerIconPath);
    if (cfp.exists()) {
        QFile svgFp(linuxSvgPath);
        if (svgFp.exists()) {
            svgFp.remove();
        }
//      QString readline = utils::readFileContent("/usr/share/kylin-software-center/foot.txt");
//      resize png size
//      QString cmd = QString("convert -resize 96x96 %1 %2").arg(containerIconPath).arg(linuxIconPath);//depend on imagemagick
//      system(cmd.toStdString().c_str());
        QString readline = "<g id=\"移动数据角标_复制\"><g opacity=\"0.05\"><circle cx=\"80\" cy=\"80\" r=\"15\"/></g><g opacity=\"0.08\"><circle cx=\"80\" cy=\"80\" r=\"15.33\"/></g><g opacity=\"0.08\"><circle cx=\"80\" cy=\"80\" r=\"15.67\"/></g><g opacity=\"0.08\"><circle cx=\"80\" cy=\"80\" r=\"16\"/></g><circle cx=\"80\" cy=\"80\" r=\"15\" fill=\"#fff\"/><path d=\"M83.94,72A1.05,1.05,0,0,1,85,73.06V86.94A1.05,1.05,0,0,1,83.94,88H76.06A1.05,1.05,0,0,1,75,86.94V73.06A1.05,1.05,0,0,1,76.06,72h7.88m0-1H76.06A2.06,2.06,0,0,0,74,73.06V86.94A2.06,2.06,0,0,0,76.06,89h7.88A2.06,2.06,0,0,0,86,86.94V73.06A2.06,2.06,0,0,0,83.94,71Z\" fill=\"#575757\"/><circle cx=\"80\" cy=\"85\" r=\"1\" fill=\"#575757\"/></g>";

        if (utils::copyFile(containerIconPath, linuxIconPath)) {
            // covert png to svg
            if (convertPngToSvg(linuxIconPath, linuxSvgPath)) {
                // set android flag to svg
                QString cmd1 = QString("sed -i '$i %1' %2").arg(readline).arg(linuxSvgPath);
                system(cmd1.toStdString().c_str());
                QString cmd2 = QString("sed -i 's/\"96\"/\"83\"/g' %1").arg(linuxSvgPath);
                system(cmd2.toStdString().c_str());
                QString cmd3 = QString("sed -i 's/\"128\"/\"83\"/g' %1").arg(linuxSvgPath);
                system(cmd3.toStdString().c_str());
                QString cmd4 = QString("sed -i 's/\"256\"/\"83\"/g' %1").arg(linuxSvgPath);
                system(cmd4.toStdString().c_str());
                QString cmd5 = QString("sed -i 's/viewBox=\"0 0 128 128\"/viewBox=\"0 0 96 96\"/g' %1").arg(linuxSvgPath);
                system(cmd5.toStdString().c_str());
                QString cmd6 = QString("sed -i 's/viewBox=\"0 0 256 256\"/viewBox=\"0 0 96 96\"/g' %1").arg(linuxSvgPath);
                system(cmd6.toStdString().c_str());
                // delete png icon
                QFile fp(linuxIconPath);
                if (fp.exists()) {
                    fp.remove();
                }
            }
        }
        else {
            qDebug() << "copy icon fail!";
        }
//        QFileInfo fi("/usr/share/kylin-software-center/foot.txt");
//        if (fi.exists()) {
//        }
        //QProcess::startDetached(QString("/usr/sbin/update-icon-caches %1").arg(iconsPath));
    }
    else {
        QFile fp(linuxIconPath);
        if (fp.exists()) {
            QString readline = "<g id=\"移动数据角标_复制\"><g opacity=\"0.05\"><circle cx=\"80\" cy=\"80\" r=\"15\"/></g><g opacity=\"0.08\"><circle cx=\"80\" cy=\"80\" r=\"15.33\"/></g><g opacity=\"0.08\"><circle cx=\"80\" cy=\"80\" r=\"15.67\"/></g><g opacity=\"0.08\"><circle cx=\"80\" cy=\"80\" r=\"16\"/></g><circle cx=\"80\" cy=\"80\" r=\"15\" fill=\"#fff\"/><path d=\"M83.94,72A1.05,1.05,0,0,1,85,73.06V86.94A1.05,1.05,0,0,1,83.94,88H76.06A1.05,1.05,0,0,1,75,86.94V73.06A1.05,1.05,0,0,1,76.06,72h7.88m0-1H76.06A2.06,2.06,0,0,0,74,73.06V86.94A2.06,2.06,0,0,0,76.06,89h7.88A2.06,2.06,0,0,0,86,86.94V73.06A2.06,2.06,0,0,0,83.94,71Z\" fill=\"#575757\"/><circle cx=\"80\" cy=\"85\" r=\"1\" fill=\"#575757\"/></g>";
            if (convertPngToSvg(linuxIconPath, linuxSvgPath)) {
                // set android flag to svg
                QString cmd1 = QString("sed -i '$i %1' %2").arg(readline).arg(linuxSvgPath);
                system(cmd1.toStdString().c_str());
                QString cmd2 = QString("sed -i 's/\"96\"/\"83\"/g' %1").arg(linuxSvgPath);
                system(cmd2.toStdString().c_str());
                QString cmd3 = QString("sed -i 's/\"128\"/\"83\"/g' %1").arg(linuxSvgPath);
                system(cmd3.toStdString().c_str());
                QString cmd4 = QString("sed -i 's/\"256\"/\"83\"/g' %1").arg(linuxSvgPath);
                system(cmd4.toStdString().c_str());
                QString cmd5 = QString("sed -i 's/viewBox=\"0 0 128 128\"/viewBox=\"0 0 96 96\"/g' %1").arg(linuxSvgPath);
                system(cmd5.toStdString().c_str());
                QString cmd6 = QString("sed -i 's/viewBox=\"0 0 256 256\"/viewBox=\"0 0 96 96\"/g' %1").arg(linuxSvgPath);
                system(cmd6.toStdString().c_str());
                // delete png icon
                QFile fp(linuxIconPath);
                if (fp.exists()) {
                    fp.remove();
                }
            }
        }
    }
}

/***********************************************************
   Function:       getInstalledAppList
   Description:    获取android环境已安装的app列表
   Calls:
   Called By:
   Input:
   Output:
   Return:
   Others:
 ************************************************************/
//void BackendWorker::getInstalledAppList()
//{
//    QString libPath = "/usr/lib/libkmre.so";
//    if (!QFile::exists(libPath)) {
//        emit this->sendInstalledAppList(QString("[]").toUtf8());
//        return;
//    }

//    void *module_handle;
//    char *module_error;
//    module_handle = dlopen(libPath.toStdString().c_str(), RTLD_LAZY);
//    if (!module_handle) {
//        emit this->sendInstalledAppList(QString("[]").toUtf8());
//        return;
//    }

//    char *(*get_installed_applist)();
//    get_installed_applist = (char *(*)())dlsym(module_handle, "get_installed_applist");
//    if ((module_error = dlerror()) != NULL) {
//        dlclose(module_handle);
//        emit this->sendInstalledAppList(QString("[]").toUtf8());
//        return;
//    }

//    char *list_info = NULL;
//    list_info = get_installed_applist();
//    if (list_info) {
//        std::string runningInfo = std::string(list_info);
//        QByteArray byteArray(list_info, runningInfo.length());
//        emit this->sendInstalledAppList(byteArray);
//    }
//    else {
//        emit this->sendInstalledAppList(QString("[]").toUtf8());
//    }
//    dlclose(module_handle);
//}

QByteArray BackendWorker::getInstalledAppListJsonStr()
{
    QString libPath = "/usr/lib/libkmre.so";
    if (!QFile::exists(libPath)) {
        return QString("[]").toUtf8();
    }

    void *module_handle;
    char *module_error;
    module_handle = dlopen(libPath.toStdString().c_str(), RTLD_LAZY);
    if (!module_handle) {
        return QString("[]").toUtf8();
    }

    char *(*get_installed_applist)();
    get_installed_applist = (char *(*)())dlsym(module_handle, "get_installed_applist");
    if ((module_error = dlerror()) != NULL) {
        dlclose(module_handle);
        return QString("[]").toUtf8();
    }

    char *list_info = NULL;
    list_info = get_installed_applist();
    if (list_info) {
        std::string runningInfo = std::string(list_info);
        QByteArray byteArray(list_info, runningInfo.length());
        dlclose(module_handle);
        return byteArray;
    }

    dlclose(module_handle);
    return QString("[]").toUtf8();
}

/***********************************************************
   Function:       installApp
   Description:    安装apk
   Calls:
   Called By:
   Input:
   Output:
   Return:
   Others:
 ************************************************************/
void BackendWorker::installApp(const QString &fileName, const QString &pkgName, const QString &application, const QString &applicationZh, const QString &version)
{
//    m_iconFilePath.clear();
    QThread::sleep(1);
    if (fileName.isEmpty()) {
        emit this->installFinished(fileName, pkgName, false);
        return;
    }

    QString libPath = "/usr/lib/libkmre.so";
    if (!QFile::exists(libPath)) {
        emit this->installFinished(fileName, pkgName, false);
        return;
    }

    void *module_handle;
    char *module_error;
    module_handle = dlopen(libPath.toStdString().c_str(), RTLD_LAZY);
    if (!module_handle) {
        emit this->installFinished(fileName, pkgName, false);
        return;
    }
    bool (*install_app)(char *filename, char *appname, char *pkgname);
    install_app = (bool(*)(char *, char*, char *))dlsym(module_handle, "install_app");
    if ((module_error = dlerror()) != NULL) {
        dlclose(module_handle);
        emit this->installFinished(fileName, pkgName, false);
        return;
    }
    //qDebug() << "BackendWorker::installApp " << fileName;
    //filename:apk名称，如 com.tencent.mm_8.0.0.apk
    //appname:应用名,如 微信
    //pkgname:包名，如 com.tencent.mm
    //qDebug() << "###install_app fileName:" << fileName <<  ",pkgName:" << pkgName << ", application:" << application;
    bool nRes = install_app(const_cast<char *>(fileName.toStdString().c_str()), const_cast<char *>("test"), pkgName.isEmpty() ? const_cast<char *>("test") : const_cast<char *>(pkgName.toStdString().c_str()));
    if (nRes) {
        //qDebug() << "BackendWorker::installApp success pkgName:" << pkgName;
        QThread::sleep(1);
        this->updateDekstopAndIcon(pkgName, application, applicationZh, version);
        emit this->installFinished(fileName, pkgName, true);
    }
    else {
        qDebug() << "BackendWorker::installApp failed: " << pkgName;
        emit this->installFinished(fileName, pkgName, false);
    }
    dlclose(module_handle);
}
//void BackendWorker::installApp(const QString &fileName, const QString &pkgName, const QString &application, const QString &applicationZh)
//{
//    QThread::sleep(2);
//    if (fileName.isEmpty() || pkgName.isEmpty()) {
//        emit this->installFinished(fileName, false);
//        return;
//    }
//    //qDebug() << "BackendWorker::installApp" << fileName << pkgName << application << applicationZh;

//    QString libPath = "/usr/lib/libkmre.so";
//    if (!QFile::exists(libPath)) {
//        emit this->installFinished(fileName, false);
//        return;
//    }

//    void *module_handle;
//    char *module_error;
//    module_handle = dlopen(libPath.toStdString().c_str(), RTLD_LAZY);
//    if (!module_handle) {
//        emit this->installFinished(fileName, false);
//        return;
//    }
//    bool (*install_app)(char *filename, char *appname, char *pkgname);
//    install_app = (bool(*)(char *, char*, char *))dlsym(module_handle, "install_app");
//    if ((module_error = dlerror()) != NULL) {
//        dlclose(module_handle);
//        emit this->installFinished(fileName, false);
//        return;
//    }
//    qDebug() << "BackendWorker::installApp " << fileName;
//    bool nRes = install_app(fileName.toStdString().c_str(), pkgName.toStdString().c_str(), pkgName.toStdString().c_str());
//    if (nRes) {
//        qDebug() << "BackendWorker::installApp success";
//        emit this->installFinished(fileName, true);
//    }
//    else {
//        qDebug() << "BackendWorker::installApp failed";
//        emit this->installFinished(fileName, false);
//    }
//    dlclose(module_handle);
//}

/***********************************************************
   Function:       unIntallApp
   Description:    卸载apk
   Calls:
   Called By:
   Input:
   Output:
   Return:
   Others:
 ************************************************************/
void BackendWorker::unIntallApp(const QString &appName)
{
    if (appName.isEmpty()) {
        emit this->unInstallFinished(appName, false);
        return;
    }

    QString libPath = "/usr/lib/libkmre.so";
    if (!QFile::exists(libPath)) {
        emit this->unInstallFinished(appName, false);
        return;
    }

    void *module_handle;
    char *module_error;
    module_handle = dlopen(libPath.toStdString().c_str(), RTLD_LAZY);
    if (!module_handle) {
        emit this->unInstallFinished(appName, false);
        return;
    }

    bool (*uninstall_app)(char *appname);
    uninstall_app = (bool(*)(char *))dlsym(module_handle, "uninstall_app");
    if ((module_error = dlerror()) != NULL) {
        dlclose(module_handle);
        emit this->unInstallFinished(appName, false);
        return;
    }
    bool nRes = uninstall_app(const_cast<char *>(appName.toStdString().c_str()));
    if (nRes) {
        emit this->unInstallFinished(appName, true);
    }
    else {
        emit this->unInstallFinished(appName, false);
    }
    dlclose(module_handle);
}

/***********************************************************
   Function:       launchApp
   Description:    启动app
   Calls:
   Called By:
   Input:
   Output:
   Return:
   Others:
 ************************************************************/
void BackendWorker::launchApp(const QString &packageName)
{
    qint64 pid;
    QProcess::startDetached("/usr/bin/startapp", QStringList() << packageName, QString(), &pid);
}

void BackendWorker::onGetAndroidEnvInstalled()
{
    int value = this->isAndroidEnvInstalled();
    emit this->sigAndroidEnvInstalled(value);
}

int BackendWorker::isAndroidEnvInstalled()
{
    int nRes = -1;
    QString libPath = "/usr/lib/libkmre.so";
    if (!QFile::exists(libPath)) {
        return nRes;
    }

    void *module_handle;
    char *module_error;
    module_handle = dlopen(libPath.toStdString().c_str(), RTLD_LAZY);
    if (!module_handle) {
        return nRes;
    }

    bool (*is_android_env_installed)();
    is_android_env_installed = (bool(*)())dlsym(module_handle, "is_android_env_installed");
    if ((module_error = dlerror()) != NULL) {
        dlclose(module_handle);
        return nRes;
    }
    bool ret = is_android_env_installed();
    dlclose(module_handle);

    if (ret) {
        nRes = 1;
    }
    else {
        nRes = 0;
    }

    return nRes;
}

