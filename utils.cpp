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

#include "utils.h"
#include "common.h"

#include <QStandardPaths>
#include <QFile>
#include <QDir>
#include <QDebug>
#include <QDirIterator>
#include <QProcessEnvironment>
#include <QtDBus/QDBusConnection>
#include <QtDBus/QDBusConnectionInterface>
#include <QtDBus/QDBusMessage>
#include <QtDBus/QDBusReply>
#include <QtDBus/QDBusInterface>
#include <QFontMetrics>
#include <QCryptographicHash>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QSettings>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <pwd.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/file.h>
#include <fcntl.h>
#include <sys/syslog.h>
#include <openssl/rsa.h>
#include <openssl/pem.h>
#include <openssl/err.h>
#include <openssl/bio.h>
#include <openssl/evp.h>

namespace kmre {
namespace utils {

#if defined(__aarch64__)
bool isFt1500aCpu()
{
    const QString content(readFile("/proc/cpuinfo"));
    if (content.isNull() || content.isEmpty()) {
        return false;
    }

    for (const QString& line : content.split('\n')) {
        if (line.startsWith("model name", Qt::CaseInsensitive)) {
            if (line.contains("FT1500", Qt::CaseInsensitive) ||
                line.contains("FT-1500", Qt::CaseInsensitive)) {
                return true;
            }
        }
    }

    return false;
}
#endif

const QString& getUserName()
{
    static QString userName = "";

    if (!userName.isEmpty()) {
        return userName;
    }

    struct passwd  pwd;
    struct passwd* result = 0;
    char buf[1024];

    memset(&buf, 0, sizeof(buf));
    uint32_t uid = getuid();
    (void)getpwuid_r(uid, &pwd, buf, 1024, &result);

    if (result && pwd.pw_name) {
        userName = pwd.pw_name;
    }
    else {
        try {
            userName = std::getenv("USER");
        } 
        catch (...) {
            try {
                userName = std::getenv("USERNAME");
            }
            catch (...) {
                syslog(LOG_ERR, "[%s] Get user name failed!", __func__);
                char name[16] = {0};
                snprintf(name, sizeof(name), "%u", getuid());
                userName = name;
            }
        }
    }

    userName.replace('\\', "_");// 在某些云环境，用户名中会有'\'字符，需将该字符转换为'_'字符
    return userName;
}

const QString& getUid()
{
    int uid = -1;
    static QString userId = "";

    if (!userId.isEmpty()) {
        return userId;
    }

    try {
        uid = getuid();
    }
    catch (...) {
        syslog(LOG_ERR, "[%s] Get user id failed!", __func__);
    }
    
    userId = QString::number(uid);
    return userId;
}

QString osVersion()
{
    QString distribDescription;
    if (QFile::exists("/etc/lsb-release")) {
        QSettings lsbSetting("/etc/lsb-release", QSettings::IniFormat);
        distribDescription = lsbSetting.value("DISTRIB_DESCRIPTION").toString().replace("\"", "");
    }

    return distribDescription;
}

QString makeContainerName()
{
    QString userName;

    userName = getUserName();

    return makeContainerName(userName, getuid());
}

QString makeContainerName(const QString& userName, int uid)
{
    QString containerName;

    if (userName.isNull() || userName.isEmpty() || userName.length() == 0) {
        return containerName;
    }

    if (uid < 0) {
        return containerName;
    }

    containerName = QString("kmre-%1-%2").arg(QString::number(uid)).arg(userName);

    return containerName;
}

QString readFile(const QString &path)
{
    QFile file(path);
    if (file.exists()) {
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            return "";
        }
        QTextStream stream(&file);
        QString str = stream.readAll();
        file.close();
        return str;
    }
    else {
        return "";
    }
}

QString readFileContent(const QString &path)
{
    QFile file(path);
    if (file.exists()) {
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            qWarning() << "Read file content failed to open" << path;
            return "";
        }

        const QString &str = QString::fromUtf8(file.readAll());
        file.close();

        return str;
    }

    return "";
}

bool isWayland()
{
    bool wayland = false;
    auto e = QProcessEnvironment::systemEnvironment();
    QString xdgSessionType = e.value(QStringLiteral("XDG_SESSION_TYPE"));
    QString waylandDisplay = e.value(QStringLiteral("WAYLAND_DISPLAY"));
    if (xdgSessionType == QLatin1String("wayland") || waylandDisplay.contains(QLatin1String("wayland"), Qt::CaseInsensitive)) {
        wayland = true;
    }

    return wayland;
}

bool copyFile(const QString &srcFile, const QString &destFile)
{
    //qDebug() << "Utils::copyFile:" << srcFile << destFile;
    QFile fp(destFile);
    if (fp.exists()) {
        if (!fp.remove()) {
            qCritical() << "Failed to remove:" << destFile;
            //return false;
        }
    }

    return QFile::copy(srcFile, destFile);
}

bool isAndroidReady(const QString &username, const QString &uid)
{
    QString value;
    QDBusInterface *interface;
    interface = new QDBusInterface("cn.kylinos.Kmre", "/cn/kylinos/Kmre", "cn.kylinos.Kmre", QDBusConnection::systemBus());
    QDBusMessage response = interface->call("GetPropOfContainer", username, uid.toInt(), "sys.kmre.boot_completed");
    if (response.type() == QDBusMessage::ReplyMessage) {
        value = response.arguments().takeFirst().toString();
    }
    delete interface;
    if (value == "1") {
        return true;
    }
    else {
        return false;
    }
}

QString wrapStr(const QString &str, const QFont &font, int maxWidth)
{
    QFontMetrics fm(font);
    QString ns;
    QString ss;
    for (int i = 0; i < str.length(); i ++) {
        if (str.at(i).isSpace()|| fm.boundingRect(ss).width() > maxWidth) {
            ss = QString();
            ns += " ";
        }
        ns += str.at(i);
        ss += str.at(i);
    }
    return ns;
}

int stringWidth(const QFont &font, const QString &str)
{
    QFontMetrics fm(font);
    return fm.boundingRect(str).width();
}

// 使用RSA公钥进行加密
QByteArray encrypt(const QByteArray &md5, const QByteArray &keyBase64)
{
    BIO *bio = NULL;
    RSA *p_rsa = NULL;
    EVP_PKEY *key = NULL;
    EVP_PKEY_CTX *ctx = NULL;
    unsigned char *out;
    size_t outlen;
    QByteArray tempKey = QByteArray::fromBase64(keyBase64);
    if ((bio = BIO_new_mem_buf(tempKey.data(),tempKey.size())) == NULL) {
        evpError();
        return QByteArray();
    }

    if ((p_rsa=PEM_read_bio_RSA_PUBKEY(bio,NULL,NULL,NULL))==NULL) {
        evpError();
        BIO_free(bio);
        return QByteArray();
    }

    key = EVP_PKEY_new();
    if (key == NULL) {
        evpError();
        BIO_free(bio);
        RSA_free(p_rsa);
        return QByteArray();
    }

    if (EVP_PKEY_set1_RSA(key,p_rsa)<=0) {
        evpError();
        BIO_free(bio);
        RSA_free(p_rsa);
        EVP_PKEY_free(key);
        return QByteArray();
    }

    ctx = EVP_PKEY_CTX_new(key,NULL);
    if (ctx == NULL) {
        evpError();
        BIO_free(bio);
        RSA_free(p_rsa);
        EVP_PKEY_free(key);
        return QByteArray();
    }

    if (EVP_PKEY_encrypt_init(ctx) <=0) {
        evpError();
        BIO_free(bio);
        RSA_free(p_rsa);
        EVP_PKEY_free(key);
        EVP_PKEY_CTX_free(ctx);
        return QByteArray();
    }

    //  设置填充方式为OAEP
    if (EVP_PKEY_CTX_set_rsa_padding(ctx, RSA_PKCS1_OAEP_PADDING) <= 0) {
        evpError();
        BIO_free(bio);
        RSA_free(p_rsa);
        EVP_PKEY_free(key);
        EVP_PKEY_CTX_free(ctx);
        return QByteArray();
    }

    //  确定加密buf长度
    if (EVP_PKEY_encrypt(ctx, NULL, &outlen, (const unsigned char *)md5.data(), md5.size()) <= 0) {
        evpError();
        BIO_free(bio);
        RSA_free(p_rsa);
        EVP_PKEY_free(key);
        EVP_PKEY_CTX_free(ctx);
        return QByteArray();
    }

    out = (unsigned char *) OPENSSL_malloc(outlen);
    if (!out) {
        evpError();
        BIO_free(bio);
        RSA_free(p_rsa);
        EVP_PKEY_free(key);
        EVP_PKEY_CTX_free(ctx);
        return QByteArray();
    }

    //  进行加密
    if (EVP_PKEY_encrypt(ctx, out, &outlen, (const unsigned char *)md5.data(), md5.size()) <= 0) {
        evpError();
        BIO_free(bio);
        RSA_free(p_rsa);
        EVP_PKEY_free(key);
        EVP_PKEY_CTX_free(ctx);
        OPENSSL_free(out);
        return QByteArray();
    }

    QByteArray retByteArray((const char *)out,outlen);
    OPENSSL_free(out);
    BIO_free(bio);
    RSA_free(p_rsa);
    EVP_PKEY_free(key);
    EVP_PKEY_CTX_free(ctx);

    return retByteArray;
}

//  打印错误信息
void evpError(void)
{
    int n = ERR_get_error();
    char szErr[512];
    char errStr[1024];
    ERR_error_string(n, szErr);
    sprintf(errStr, "error code = %d,code string = %s", n, szErr);
    qWarning() << "kmre-apk-installer:" << errStr;
}

void sendApkInfoToServer(const QString &os, const QString &cpuType, const QString &gpuVendor, const QString &pkgName, const QString &appName)
{
    QByteArray publicKey(
        "LS0tLS1CRUdJTiBQVUJMSUMgS0VZLS0tLS0KTUlJQklqQU5CZ2txaGtpRzl3MEJBUUVGQUFPQ0FR\
        OEFNSUlCQ2dLQ0FRRUFzdW1NTFJEdlFNb0tEQkRJODRqSgpqc1A0Mk55V0pWVEZob2Jra3ZiT05j\
        dExYTXVzRmo2TzJUblZYU3Z6VlVLSjRqZkpwT2l2WEphOVB5Z2wzYTRnClBzSU40enNCMEdOY0tr\
        R3VsS2RrV2x6S3lWQ2xlTzhiQnN6SjkwbTc3cWF6YWg3a1A0TUl0WTVFczBpSkpiR0oKN1MxcERj\
        MlJkNnVFQWJLaXJyRTFlNzlFTEd4am5VN2V5NWkyRDE2WWJoZEQwZ2lNa2RHR3piQXBKTWZWRVJR\
        TQo1NXorMFVqdS8zSFJhNFY3b3p2TGRPRE5HUURaeWNJU0l3VHBLbFR3RjBxazdCNjVhTUlJenQ1\
        dnhOK1lxYU1GClppZFRLNzcxNjdqNEExZ3F3MG45bjlybWVXUGRWZ3dudnRtVXp4Q1krNk05SXpK\
        TDI3eWpRUTV1WGQ3RVdMT3IKbndJREFRQUIKLS0tLS1FTkQgUFVCTElDIEtFWS0tLS0tCg==");

    // 将需要发送的信息以json格式保存
    QJsonObject jsonObj;
    jsonObj.insert("OS", QJsonValue(os));
    jsonObj.insert("CPU", QJsonValue(cpuType));
    jsonObj.insert("GPU", QJsonValue(gpuVendor));
    jsonObj.insert("pkgName", QJsonValue(pkgName));
    jsonObj.insert("appName", QJsonValue(appName));

    //  将数据转化为QString
    QString informationData(QJsonDocument(jsonObj).toJson(QJsonDocument::Compact));
    //  计算发送信息的MD5
    QCryptographicHash cryHash(QCryptographicHash::Algorithm::Md5);
    cryHash.addData(informationData.toLatin1());
    QByteArray md5(cryHash.result());

    //  加密发送信息的MD5
    QByteArray enMd5 = encrypt(md5, publicKey);
    //  将加密的MD5转化为BASE64格式
    QString enMd5Base64(enMd5.toBase64());
    //  填入需要调用的参数
    QList<QVariant> tempVar;
    //  填入应用名
    tempVar.push_back(QString("kmre-apk-installer"));
    //  填入信息标识(记录此种信息为哪一类信息)
    tempVar.push_back(QString("apk"));
    //  填入具体信息
    tempVar.push_back(QString(QJsonDocument(jsonObj).toJson(QJsonDocument::Compact)));
    //  填入加密并转为base64的MD5码
    tempVar.push_back(enMd5Base64);

    QDBusInterface daqInterface("com.kylin.daq", "/com/kylin/daq", "com.kylin.daq.interface", QDBusConnection::systemBus());
    QDBusMessage response = daqInterface.callWithArgumentList(QDBus::Block, QString("sendInfo"), tempVar);//QDBusMessage response = daqInterface.call("sendInfo", tempVar);
    if (response.type() == QDBusMessage::MessageType::ReplyMessage) {//QDBusMessage::ReplyMessage
        if (!response.arguments().isEmpty()) {
            int ret = response.arguments().takeFirst().toInt();
            qDebug() << "ret:" << ret;
        }
        else {
            qDebug() << "###Error Name: " << response.errorName() << " errorMessage:" << response.errorMessage();
        }
    }
}

}
}
