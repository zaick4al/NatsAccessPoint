#include "natshandler.h"

namespace WebApi
{
NatsHandler::NatsHandler() : m_currentSrv(-1), m_confReceived(false)
{
    QObject::connect(&m_client, &QtNats::Client::statusChanged, this, &NatsHandler::onConnect);
    QObject::connect(&m_client, &QtNats::Client::errorOccurred, this, &NatsHandler::onError);
    m_urls = Singletones::Settings::instance().natsServerIps();
    connectToServer();
}

void NatsHandler::connectToServer()
{
    m_confReceived = false;
    qDebug() << "\n" << "Nats is trying to connect to" << m_urls.toList() << "\n";
    QtNats::Options connectOpts;
    connectOpts.servers = m_urls;
    try{
    m_client.connectToServer(connectOpts);
    }
    catch(QtNats::Exception &except)
    {
        QString debugMsg = except.what();
        emit debugMessage(debugMsg);
        QTimer::singleShot(10000, this, &NatsHandler::connectToServer);
    }
}

void NatsHandler::disconnectFromServer()
{
    m_client.disconnect();
}

void NatsHandler::onTcpMessage(const QString &uuid, const QByteArray &messageText)
{
    QJsonObject mainobj;
    mainobj.insert("uuid", uuid);
    mainobj.insert("content", QString::fromUtf8(messageText));
    QtNats::Message msg;
    msg.data = QJsonDocument(mainobj).toJson(QJsonDocument::Compact);
    msg.subject = "foo";
    publish(msg);
}

void NatsHandler::onClientConnected(const QString &clientId, const QString &service)
{
    QByteArray clConnect = QString("client connection:" + clientId + "/" + service).toUtf8();
    QtNats::Message msg;
    msg.data = clConnect;
    msg.subject = "foobar";
    publish(msg);
    m_portsUuids.insert(clientId, "port");
}

void NatsHandler::onClientDisconnected(const QString &clientId)
{
    QByteArray clDisconnect = QString("client disconnected:" + clientId).toUtf8();
    QtNats::Message msg;
    msg.data = clDisconnect;
    msg.subject = "foo";
    publish(msg);
    m_portsUuids.remove(clientId);
}

void NatsHandler::publish(const QtNats::Message &message)
{
    try{
        m_client.publish(message);
    }
    catch(QtNats::Exception except){
        QString debugMsg = except.what();
        emit debugMessage(debugMsg);
    }
}

void NatsHandler::onTimer(){
    if(!m_confReceived)
    {
        QtNats::Message msg;
        msg.data = "accessPointRequest";
        msg.subject = "foobar";
        publish(msg);
        QTimer::singleShot(1000, this, &NatsHandler::onTimer);
    }
}

void NatsHandler::onMessage(const QtNats::Message &message)
{
    QByteArray msg = message.data;
    if(msg.contains("content"))
    {
        QJsonObject uuid = QJsonDocument::fromJson(msg).object();
        QString uuidDecoded = uuid.value("uuid").toString();
        QByteArray contentsDecoded = uuid.value("content").toString().toUtf8();
        emit messageReceived(uuidDecoded, contentsDecoded);
    }
    if(msg.contains("acpConfig"))
    {
        m_confReceived = true;
        QHash<QString, quint16> services;
        QJsonObject jobj =  QJsonDocument::fromJson(msg).object();
        QString newDate = jobj.value("date").toString();
        QJsonObject servicesJobj = jobj.value("services").toObject();
        for(const QString &key : servicesJobj.keys())
        {
            QJsonObject insideObj = servicesJobj.value(key).toObject();
            quint16 srcPort = insideObj.value("sourcePort").toInt();
            services.insert(key, srcPort);
        }
        emit configReceived(services);
        emit dateReceived(newDate);
    }
    else if(msg.contains("gwUpd"))
    {
        QJsonObject updateObj = QJsonDocument::fromJson(msg).object();
        emit gwUpdateReceived(updateObj);
    }
}

void NatsHandler::onConnect(QtNats::ConnectionStatus status)
{
    qDebug() << status;
    if(status == QtNats::ConnectionStatus::Disconnected || status == QtNats::ConnectionStatus::Connecting)
        if(m_qobjectConnected)
            disconnect(m_qobjectConnected);
        QString debugMsg = "Not Connected";
        emit debugMessage(debugMsg);
    if(status != QtNats::ConnectionStatus::Connected)
        return;
    QString debugMessag = "Connected";
    emit debugMessage(debugMessag);
    m_listenSubscription = m_client.subscribe("bar");
    m_qobjectConnected = connect(m_listenSubscription, &QtNats::Subscription::received, this, &NatsHandler::onMessage);
    onTimer();
}

void NatsHandler::onError(natsStatus error, const QString& text)
{
    qDebug() << text;
    if(text.contains("closed the connection"))
        connectToServer();
    QString signalText = text;
    emit errorThrown(signalText);
}
}
