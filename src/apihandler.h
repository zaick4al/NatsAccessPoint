#ifndef API_APIHANDLER_H
#define API_APIHANDLER_H

#include <QObject>
#include "api/tcpserver.h"
#include "api/natshandler.h"

namespace Api {

class ApiHandler : public QObject
{
    Q_OBJECT
public:
    explicit ApiHandler(QObject *parent = nullptr);

    qint64 date() const;
    void setDate(const qint64 &newDate);

public slots:
    void reconnect();
signals:
    void natsDebug(QString &data);
private:
    QHash<QString, WebApi::TcpServer*> m_tcpSrv;
    WebApi::NatsHandler *m_natsHandler = new WebApi::NatsHandler();
    qint64 m_date = 0;

    void onGwUpdateReceived(QJsonObject mainObj);
    void purgeSrvs();
private slots:
    void onConfigReceived(QHash<QString, quint16>);
};

} // namespace Api

#endif // API_APIHANDLER_H
