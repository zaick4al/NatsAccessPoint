#ifndef SETTINGS_H
#define SETTINGS_H

#include "qurl.h"
#include <QObject>
#include <QSettings>
#include <QCoreApplication>
#include <QRandomGenerator>
#include <QRegularExpression>

namespace Singletones
{
class Settings : public QObject
{
    Q_OBJECT
public:
    explicit Settings(QObject *parent = nullptr);

    static Settings &instance();

    QList<QUrl> natsServerIps() const;
    void setNatsServerIps(const QList<QUrl> &newNatsServerIp);
    void setNatsServerIps(const QString &p_natsServersStr);


signals:


private:
    QSettings* m_settings;
    QHash<QString, QString> m_currentSettings;
    QList<QUrl> m_natsServerIps;
};
}
#endif // SETTINGS_H
