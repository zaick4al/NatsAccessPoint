#ifndef TESTSERVICE_H
#define TESTSERVICE_H

#include <QObject>
#include <QTimer>
#include <QCommandLineParser>
#include "apihandler.h"

#include "../QtService/src/service/service.h"
#include "../QtService/src/service/terminal.h"

class NatsService : public QtService::Service
{
    Q_OBJECT

public:
    explicit NatsService(int &argc, char **argv);

protected:

    CommandResult onStart() override;
    CommandResult onStop(int &exitCode) override;
    bool verifyCommand(const QStringList &arguments) override;

protected slots:

    void terminalConnected(QtService::Terminal *terminal) override;

private:
    Api::ApiHandler *m_handler;
    bool parseArguments(QCommandLineParser &parser, const QStringList &arguments);
};

#endif // TESTSERVICE_H
