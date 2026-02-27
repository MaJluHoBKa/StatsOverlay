#pragma once

#include <string>
#include <vector>
#include <windows.h>
#include <bcrypt.h>
#include <shlobj.h>
#include <winreg.h>
#include <intrin.h>

#include <QStandardPaths>
#include <QDir>
#include <QCryptographicHash>
#include <QJsonObject>
#include <QJsonDocument>
#include <QFile>
#include <QSysInfo>
#include <QString>

#pragma comment(lib, "Bcrypt.lib")
#pragma comment(lib, "Shell32.lib")

class HWIDActivator
{
public:
    static inline const char *SECRET_SEED = "bambam";

    /// Путь к файлу активации
    static QString getActivationFilePath()
    {
        QString basePath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
        QDir dir(basePath + "/StatsOverlay");
        if (!dir.exists())
            dir.mkpath(".");
        return dir.filePath("activation.json");
    }

    /// Получение HWID (на базе системной информации Qt)
    static QString getHWID()
    {
        QString hwidSource =
            QString("{"
                    "\"machine_id\":\"%1\","
                    "\"kernel\":\"%2\","
                    "\"cpu_arch\":\"%3\"}")
                .arg(QSysInfo::machineUniqueId(),
                     QSysInfo::kernelVersion(),
                     QSysInfo::currentCpuArchitecture());

        QByteArray hash = QCryptographicHash::hash(hwidSource.toUtf8(), QCryptographicHash::Sha256);
        return QString(hash.toHex());
    }

    /// Генерация ключа
    static QString generateKey()
    {
        QString hwid = getHWID();
        QString combined = QString("%1:%2").arg(SECRET_SEED, hwid);
        QByteArray hash = QCryptographicHash::hash(combined.toUtf8(), QCryptographicHash::Sha256);
        return QString(hash.toHex()).left(16).toUpper();
    }

    /// Попытка активации
    static bool activate(const QString &key)
    {
        if (key != generateKey())
            return false;

        QString hwid = getHWID();
        QString path = getActivationFilePath();

        QJsonObject obj;
        obj["hwid"] = hwid;
        obj["key"] = key;
        obj["activated"] = true;

        QJsonDocument doc(obj);
        QFile file(path);
        if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate))
            return false;

        file.write(doc.toJson(QJsonDocument::Indented));
        file.close();
        return true;
    }

    /// Проверка активации
    static bool checkActivation()
    {
        QString path = getActivationFilePath();
        QFile file(path);
        if (!file.exists() || !file.open(QIODevice::ReadOnly))
            return false;

        QByteArray data = file.readAll();
        file.close();

        QJsonDocument doc = QJsonDocument::fromJson(data);
        if (!doc.isObject())
            return false;

        QJsonObject obj = doc.object();
        QString hwid = obj.value("hwid").toString();
        QString key = obj.value("key").toString();
        bool activated = obj.value("activated").toBool(false);

        return (hwid == getHWID() && key == generateKey() && activated);
    }

private:
    /// SHA-256 хеш (WinAPI, для совместимости)
    static std::string sha256_hex(const std::string &input)
    {
        BCRYPT_ALG_HANDLE hAlg = nullptr;
        BCRYPT_HASH_HANDLE hHash = nullptr;
        if (BCryptOpenAlgorithmProvider(&hAlg, BCRYPT_SHA256_ALGORITHM, nullptr, 0) != 0)
            return "";

        if (BCryptCreateHash(hAlg, &hHash, nullptr, 0, nullptr, 0, 0) != 0)
        {
            BCryptCloseAlgorithmProvider(hAlg, 0);
            return "";
        }

        if (BCryptHashData(hHash, (PUCHAR)input.data(), (ULONG)input.size(), 0) != 0)
        {
            BCryptDestroyHash(hHash);
            BCryptCloseAlgorithmProvider(hAlg, 0);
            return "";
        }

        DWORD hashLen = 0, cb = sizeof(DWORD);
        if (BCryptGetProperty(hAlg, BCRYPT_HASH_LENGTH, (PUCHAR)&hashLen, sizeof(DWORD), &cb, 0) != 0)
        {
            BCryptDestroyHash(hHash);
            BCryptCloseAlgorithmProvider(hAlg, 0);
            return "";
        }

        std::vector<unsigned char> hash(hashLen);
        if (BCryptFinishHash(hHash, hash.data(), hashLen, 0) != 0)
        {
            BCryptDestroyHash(hHash);
            BCryptCloseAlgorithmProvider(hAlg, 0);
            return "";
        }

        BCryptDestroyHash(hHash);
        BCryptCloseAlgorithmProvider(hAlg, 0);

        static const char *hexDigits = "0123456789abcdef";
        std::string out;
        out.reserve(hashLen * 2);
        for (auto b : hash)
        {
            out.push_back(hexDigits[(b >> 4) & 0xF]);
            out.push_back(hexDigits[b & 0xF]);
        }
        return out;
    }

    /// Доп. низкоуровневые методы (если захочешь расширять)
    static std::string _get_machine_id()
    {
        HKEY hKey;
        if (RegOpenKeyExA(HKEY_LOCAL_MACHINE, "SOFTWARE\\Microsoft\\Cryptography", 0,
                          KEY_READ | KEY_WOW64_64KEY, &hKey) == ERROR_SUCCESS)
        {
            char buf[256];
            DWORD bufSize = sizeof(buf);
            if (RegQueryValueExA(hKey, "MachineGuid", nullptr, nullptr, (LPBYTE)buf, &bufSize) == ERROR_SUCCESS)
            {
                RegCloseKey(hKey);
                return std::string(buf, bufSize - 1);
            }
            RegCloseKey(hKey);
        }
        return "default_machine_id";
    }

    static std::string _get_processor_id()
    {
        try
        {
            int cpuInfo[4] = {-1};
            __cpuid(cpuInfo, 1);
            char buf[32];
            sprintf(buf, "%08X%08X", cpuInfo[0], cpuInfo[3]);
            return std::string(buf);
        }
        catch (...)
        {
            return "default_processor_id";
        }
    }

    static std::string _get_disk_serial()
    {
        DWORD serialNumber = 0;
        if (GetVolumeInformationA("C:\\", nullptr, 0, &serialNumber, nullptr, nullptr, nullptr, 0))
            return std::to_string(serialNumber);
        return "default_disk_serial";
    }
};
