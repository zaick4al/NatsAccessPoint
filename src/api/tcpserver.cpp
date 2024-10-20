#include "tcpserver.h"

namespace WebApi
{
    TcpServer::TcpServer(quint16 port, QString &service, QObject *parent)
    : QObject{parent}, m_service(service), m_port(port)
    {
        initServer();
        connect(m_server, &QTcpServer::newConnection, this, &TcpServer::onNewConnection);
    }

    void TcpServer::onNatsMessage(QString &uuid, QByteArray &message)
    {
        if(!m_clientConnections.contains(uuid))
            return;
        QByteArray messageDecoded = QByteArray::fromBase64(message, QByteArray::Base64Encoding);
        QTcpSocket *client = m_clientConnections.value(uuid);
        if(client)
            client->write(messageDecoded);

    }

    QString TcpServer::service() const
    {
        return m_service;
    }

    void TcpServer::setService(const QString &newService)
    {
        m_service = newService;
    }

    void TcpServer::onNewConnection()
    {
        QTcpSocket *clientConnection = m_server->nextPendingConnection();
        QString newClientId = QUuid::createUuid().toString();
        m_clientConnections.insert(newClientId, clientConnection);
        connect(clientConnection, &QAbstractSocket::disconnected,
                this, &TcpServer::onClientDisconnect);
        QString service = this->service();
        emit clientConnected(newClientId, service);
        connect(clientConnection, &QTcpSocket::readyRead, this, &TcpServer::onTcpMessage);
    }

    void TcpServer::onClientDisconnect()
    {
        QTcpSocket* clientConnection = qobject_cast<QTcpSocket*>(this->sender());
        if(!m_clientConnections.values().contains(clientConnection))
            return;
        QStringList keys = m_clientConnections.keys();
        QStringList::Iterator iterator = std::find_if(keys.begin(), keys.end(), [=]
                                                      (const QString &key)
                                                      {return m_clientConnections.value(key) == clientConnection;});
        QString key = keys.value(iterator - keys.begin());
        m_clientConnections.remove(key);
        emit clientDisconnected(key);
        clientConnection->deleteLater();
    }

    void TcpServer::initServer()
    {
        m_server = new QTcpServer(this);
        if (!m_server->listen(QHostAddress::Any, m_port)) {
            qDebug() << "Unable to start the server: " << m_server->errorString();
            return;
        }
        QString ipAddress;
        const QList<QHostAddress> ipAddressesList = QNetworkInterface::allAddresses();
        // use the first non-localhost IPv4 address
        for (const QHostAddress &entry : ipAddressesList) {
            if (entry != QHostAddress::LocalHost && entry.toIPv4Address()) {
                ipAddress = entry.toString();
                break;
            }
        }
        // if we did not find one, use IPv4 localhost
        if (ipAddress.isEmpty())
            ipAddress = QHostAddress(QHostAddress::LocalHost).toString();
        qDebug() << "\n" << "The server is running" << "\n" << "Ip:" << ipAddress << "\n"
                 << "Port: " << m_server->serverPort() << "\n" << "Service:" << service();
    }

    void TcpServer::onTcpMessage()
    {
        QTcpSocket* clientConnection = qobject_cast<QTcpSocket*>(this->sender());
        QString uuid;
        QList<QString> keysList = m_clientConnections.keys();
        QList<QString>::Iterator keysIterator = std::find_if(keysList.begin(), keysList.end(), [=](const QString &key){return m_clientConnections.value(key) == clientConnection;});
        uuid = keysList.value(keysIterator - keysList.begin());
        while (true) {
            QByteArray message;
            message = clientConnection->read(1024).toBase64(QByteArray::Base64Encoding);
            if (message.isEmpty())
                break;
            emit messageReceived(uuid, message);
        }
    }

} // namespace WebApi
