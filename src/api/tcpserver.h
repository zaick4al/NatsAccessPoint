#ifndef WEBAPI_TCPSERVER_H
#define WEBAPI_TCPSERVER_H

#include <QObject>
#include <QTcpServer>
#include <QNetworkInterface>
#include <QHostAddress>
#include <QTcpSocket>
#include <QHash>
#include <QList>
#include "singletones/settings.h"
#include <QUuid>
#include <QTimer>

namespace WebApi
{
    class TcpServer : public QObject
    {
        Q_OBJECT
    public:
        explicit TcpServer(quint16 port, QString &service, QObject *parent = nullptr);

        QString service() const;
        void setService(const QString &newService);

    public slots:
        void onNatsMessage(QString &uuid, QByteArray &message);
    signals:
        void clientConnected(QString &uuid, QString &service);
        void messageReceived(QString &uuid, QByteArray &message);
        void clientDisconnected(QString &uuid);
    private:
        QTcpServer *m_server = nullptr;
        quint16 m_port;
        QString m_service;
        void onNewConnection();
        void initServer();
        QHash<QString, QTcpSocket*> m_clientConnections;
    private slots:
        void onTcpMessage();
        void onClientDisconnect();
    };

} // namespace WebApi

#endif // WEBAPI_TCPSERVER_H
