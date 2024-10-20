#ifndef NATSHANDLER_H
#define NATSHANDLER_H

#include <QObject>
#include <qtnats.h>
#include "singletones/settings.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QTimer>

namespace WebApi
{
class NatsHandler : public QObject
{
    Q_OBJECT
public:
    NatsHandler();

public slots:
    void connectToServer();
    void disconnectFromServer();
    void onTcpMessage(const QString &uuid, const QByteArray &messageText);
    void onClientConnected(const QString &clientId, const QString &service);
    void onClientDisconnected(const QString &clientId);
signals:
    void messageReceived(QString &uuid, QByteArray &msg);
    void errorThrown(QString &errortxt);
    void configReceived(QHash<QString, quint16>);
    void dateReceived(QString &date);
    void gwUpdateReceived(QJsonObject &updObj);
    void disconnected();
    void debugMessage(QString &message);
protected:
    void publish(const QtNats::Message &message);
private slots:
    void onMessage(const QtNats::Message &message);
    void onConnect(QtNats::ConnectionStatus status);
    void onError(natsStatus error, const QString& text);

    void onTimer();
private:
    QtNats::Subscription *m_listenSubscription;
    QtNats::Client m_client;
    QHash<QString, QString> m_portsUuids;
    QList<QUrl> m_urls;
    int m_currentSrv;
    bool m_confReceived;
    QTimer m_timer;
    QMetaObject::Connection m_qobjectConnected;
};
}

#endif // NATSHANDLER_H
