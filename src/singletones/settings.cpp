#include "settings.h"

namespace Singletones
{
    Settings::Settings(QObject *parent)
        : QObject{parent}
    {
        const QString settingsFile = QCoreApplication::applicationDirPath() + "/configs/" + "NatsAccessPoint.ini";
        m_settings = new QSettings(settingsFile, QSettings::IniFormat, this);

        if(m_settings->value("Nats/ServerIp").toString().isEmpty())
        {
            QString newNatsIp("192.168.102.1:4222,192.168.102.2:4");
            m_settings->setValue("Nats/ServerIp", newNatsIp);
        }
        setNatsServerIps(m_settings->value("Nats/ServerIp").toString());
    }

    Settings &Settings::instance()
    {
        static Settings SettingsInstance;
        return SettingsInstance;
    }

    QList<QUrl> Settings::natsServerIps() const
    {
        return m_natsServerIps;
    }

    void Settings::setNatsServerIps(const QList<QUrl> &newNatsServerIp)
    {
        if(m_natsServerIps == newNatsServerIp)
            return;
        m_natsServerIps = newNatsServerIp;
        QString newSettingsVal;
        for(QUrl url : m_natsServerIps)
        {
            QString comma = ",";
            if(newSettingsVal.isEmpty())
                comma = "";
            newSettingsVal = newSettingsVal + comma + url.toString();
        }
        m_settings->setValue("Nats/ServerIp", newSettingsVal);
    }

    void Settings::setNatsServerIps(const QString &p_natsServersStr)
    {
        QList<QUrl> urlList;
        QStringList ips = p_natsServersStr.split(QRegularExpression("\\s*,\\s*"), Qt::SkipEmptyParts);
        for(const QString &urlStr : ips)
            urlList << urlStr;
        setNatsServerIps(urlList);
    }
}
