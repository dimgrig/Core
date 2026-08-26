#include "CoreConfig.h"

#include <QCoreApplication>
#include <QStandardPaths>
#include <QDir>

#ifdef Q_OS_WIN
#include <windows.h>
#endif

const QStringList ConnectionType_sl = {{ConnectionType_to_text(ConnectionTypeSerial),
                                         ConnectionType_to_text(ConnectionTypeTCP)}};
const QStringList SerialBaudRate_sl = {{SerialBaudRate_to_text(SerialBaud1200),
                                         SerialBaudRate_to_text(SerialBaud2400),
                                         SerialBaudRate_to_text(SerialBaud4800),
                                         SerialBaudRate_to_text(SerialBaud9600),
                                         SerialBaudRate_to_text(SerialBaud19200),
                                         SerialBaudRate_to_text(SerialBaud38400),
                                         SerialBaudRate_to_text(SerialBaud57600),
                                         SerialBaudRate_to_text(SerialBaud115200)}};
const QStringList SerialDataBits_sl = {{SerialDataBits_to_text(SerialDataBits5),
                                         SerialDataBits_to_text(SerialDataBits6),
                                         SerialDataBits_to_text(SerialDataBits7),
                                         SerialDataBits_to_text(SerialDataBits8)}};
const QStringList SerialStopBits_sl = {{SerialStopBits_to_text(SerialStopBits_OneStop),
                                         SerialStopBits_to_text(SerialStopBits_TwoStop),
                                         SerialStopBits_to_text(SerialStopBits_OneAndHalfStop)}};
const QStringList SerialParity_sl = {{SerialParity_to_text(SerialParity_NoParity),
                                       SerialParity_to_text(SerialParity_EvenParity),
                                       SerialParity_to_text(SerialParity_OddParity),
                                       SerialParity_to_text(SerialParity_SpaceParity),
                                       SerialParity_to_text(SerialParity_MarkParity)}};
const QStringList BytesFormat_sl = {{BytesFormat_to_text(BigEndian),
                                      BytesFormat_to_text(BigEndianByteSwap),
                                      BytesFormat_to_text(LittleEndianByteSwap),
                                      BytesFormat_to_text(LittleEndian)}};

static QString s_userDataDir = "";
static QString s_databaseName = "";

// Вспомогательная функция для рекурсивного копирования каталога
static bool copyDirectoryRecursively(const QString &srcDir, const QString &dstDir)
{
    QDir src(srcDir);
    if (!src.exists()) {
        qWarning() << "Source directory does not exist:" << srcDir;
        return false;
    }

    QDir dst(dstDir);
    if (!dst.exists() && !dst.mkpath(".")) {
        qWarning() << "Cannot create destination directory:" << dstDir;
        return false;
    }

    bool success = true;
    const auto entries = src.entryList(QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot);
    for (const QString &entry : entries) {
        QString srcPath = srcDir + "/" + entry;
        QString dstPath = dstDir + "/" + entry;
        QFileInfo info(srcPath);
        if (info.isDir()) {
            if (!copyDirectoryRecursively(srcPath, dstPath)) {
                success = false;
            }
        } else {
            if (!QFile::copy(srcPath, dstPath)) {
                qWarning() << "Failed to copy file:" << srcPath << "to" << dstPath;
                success = false;
            }
        }
    }
    return success;
}

void CoreConfig::setUserDataDir(const QString &path)
{
    s_userDataDir = path;
    if (!s_userDataDir.isEmpty())
        QDir().mkpath(s_userDataDir);

    CoreConfig::initializeDefaultFiles(""); //QCoreApplication::applicationDirPath()
}

void CoreConfig::setDatabaseName(const QString &name) {
    s_databaseName = name + ".db";
}

QString CoreConfig::userDataDir()
{
    if (!s_userDataDir.isEmpty())
        return s_userDataDir;

// Автоматическое определение, если путь не был установлен явно
#ifdef Q_OS_WIN
    // Если QApplication уже есть, используем его метод
    if (QCoreApplication::instance()) {
        s_userDataDir = QCoreApplication::applicationDirPath();
    } else {
        // Иначе получаем путь через WinAPI (работает до создания QApplication)
        wchar_t path[MAX_PATH];
        DWORD len = GetModuleFileNameW(NULL, path, MAX_PATH);
        if (len > 0) {
            QString exePath = QString::fromWCharArray(path, len);
            s_userDataDir = QFileInfo(exePath).absolutePath();
        } else {
            s_userDataDir = QString(); // на крайний случай
        }
    }
    return s_userDataDir;
#else
    // Используем AppConfigLocation, зависящее от applicationName
    QString path = QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation);
    QDir().mkpath(path);
    return path;
#endif
}

QString CoreConfig::configFile() {
    return CoreConfig::userDataDir() + "/config.ini";
}

QString CoreConfig::calibrationFile() {
    return CoreConfig::userDataDir() + "/calibration.ini";
}

QString CoreConfig::webappFile() {
    return CoreConfig::userDataDir() + "/webapp.ini";
}

QString CoreConfig::userFile() {
    return CoreConfig::userDataDir() + "/user.dat";
}

QString CoreConfig::databaseFile() {
    return CoreConfig::userDataDir() + "/" + s_databaseName;
}

// Реализация:
void CoreConfig::initializeDefaultFiles(const QString &sourceDir)
{
    auto copyIfMissing = [&](const QString &fileName) {
        QString dest = userDataDir() ;
        if (dest == "") {
            dest = fileName;
        } else {
            dest += "/" + fileName;
        }
        if (!QFile::exists(dest)) {
            QString src = sourceDir;
            if (src == "") {
                src = fileName;
            } else {
                src += "/" + fileName;
            }
            if (QFile::exists(src)) {
                QFile::copy(src, dest);
                //qCDebug(logDebug) << "Copied" << fileName << "to" << dest;
            }
        }
    };

    copyIfMissing("config.ini");
    copyIfMissing("calibration.ini");
    copyIfMissing("webapp.ini");
    copyIfMissing(s_databaseName);

    // Копируем папку docroot (рекурсивно), если она существует в источнике
    QString docrootSrc = sourceDir.isEmpty() ? "docroot" : sourceDir + "/docroot";
    QString docrootDst = userDataDir() + "/docroot";

    if (QFile::exists(docrootSrc) && !QFile::exists(docrootDst)) {
        if (!copyDirectoryRecursively(docrootSrc, docrootDst)) {
            //qWarning() << "Failed to copy docroot from" << docrootSrc << "to" << docrootDst;
        } else {
            //qDebug() << "docroot copied successfully to" << docrootDst;
        }
    } else if (!QFile::exists(docrootSrc)) {
        //qDebug() << "docroot source not found, skipping copy";
    } else {
        //qDebug() << "docroot already exists at" << docrootDst << ", skipping copy";
    }
}
