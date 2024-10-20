#include "apihandler.h"

namespace Api {

ApiHandler::ApiHandler(QObject *parent)
    : QObject{parent}
{
    connect(m_natsHandler, &WebApi::NatsHandler::configReceived, this, &ApiHandler::onConfigReceived);
    connect(m_natsHandler, &WebApi::NatsHandler::gwUpdateReceived, this, &ApiHandler::onGwUpdateReceived);
    connect(m_natsHandler, &WebApi::NatsHandler::debugMessage, this, &ApiHandler::natsDebug);
}

qint64 ApiHandler::date() const
{
    return m_date;
}

void ApiHandler::setDate(const qint64 &newDate)
{
    m_date = newDate;
}

void ApiHandler::reconnect()
{
    m_natsHandler->connectToServer();
}

void ApiHandler::onConfigReceived(QHash<QString, quint16> config)
{
    for(const QString &key : config.keys())
    {
        if(m_tcpSrv.contains(key))
            continue;
        QString keyToServer = key;
        WebApi::TcpServer* srv = new WebApi::TcpServer(config.value(key), keyToServer);
        QString currService = srv->service();
        m_tcpSrv.insert(currService, srv);
        connect(m_tcpSrv.value(currService), &WebApi::TcpServer::clientConnected, m_natsHandler, &WebApi::NatsHandler::onClientConnected);
        connect(m_tcpSrv.value(currService), &WebApi::TcpServer::clientDisconnected, m_natsHandler, &WebApi::NatsHandler::onClientDisconnected);
        connect(m_natsHandler, &WebApi::NatsHandler::messageReceived, m_tcpSrv.value(currService), &WebApi::TcpServer::onNatsMessage);
        connect(m_tcpSrv.value(currService), &WebApi::TcpServer::messageReceived, m_natsHandler, &WebApi::NatsHandler::onTcpMessage);
    }
}

void ApiHandler::onGwUpdateReceived(QJsonObject mainObj)
{
    qint64 newDate = mainObj.value("date").toInt();
    QHash<QString, quint16> servicesHash;
    if(newDate >= date())
    {
        setDate(newDate);
        QJsonObject jobj = mainObj.value("services").toObject();
        for(const QString &key : jobj.keys())
        {
            QJsonObject internalJobj = jobj.value(key).toObject();
            QVariant srcPort = internalJobj.value("sourcePort");
            servicesHash.insert(key, srcPort.toInt());
        }
        purgeSrvs();
        QTimer::singleShot(1000, this, [=](){ApiHandler::onConfigReceived(servicesHash);});
    }
}

void ApiHandler::purgeSrvs()
{
    for(const auto &server : std::as_const(m_tcpSrv))
    {
        server->disconnect();
        server->deleteLater();
    }
    m_tcpSrv.clear();
}

} // namespace Api
