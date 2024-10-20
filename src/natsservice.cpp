#include "natsservice.h"


NatsService::NatsService(int &argc, char **argv): //the reference here is important!
            Service{argc, argv}
{
    setTerminalActive(true);
    setStartWithTerminal(true);
}


QtService::Service::CommandResult NatsService::onStart()
{
    qDebug() << "Service was started with terminal mode:" << terminalMode();
    m_handler = new Api::ApiHandler(this);
    return CommandResult::Completed;
}

QtService::Service::CommandResult NatsService::onStop(int &exitCode)
{
    qDebug() << "Closing down service...";
    Q_UNUSED(exitCode)
    return CommandResult::Completed;
}

bool NatsService::verifyCommand(const QStringList &arguments)
{
    QCommandLineParser parser;
    if(parseArguments(parser, arguments)) {
        if(parser.isSet(QStringLiteral("help")))
            parser.showHelp();
        if(parser.isSet(QStringLiteral("version")))
            parser.showVersion();

        if(parser.isSet(QStringLiteral("passive")))
            setTerminalMode(Service::TerminalMode::ReadWritePassive);
        return true;
    } else
        return false;
}

void NatsService::terminalConnected(QtService::Terminal *terminal)
{
    qDebug() << "new terminal connected with args:" << terminal->command();
    connect(terminal, &QtService::Terminal::terminalDisconnected,
            this, [](){
                qDebug() << "A terminal just disconnected";
            });

    QCommandLineParser parser;
    if(!parseArguments(parser, terminal->command())) {
        terminal->disconnectTerminal();
        return;
    }

    if(parser.positionalArguments().startsWith(QStringLiteral("stop")))
        quit();
    else {
        terminal->write("Terminal connected.");
        connect(m_handler, &Api::ApiHandler::natsDebug, [=](QString &data){
            terminal->write(data.toStdString().c_str());
        });
        connect(terminal, &QtService::Terminal::readyRead,
                this, [=](){
                    auto data = terminal->readAll();
                    QString dataStr = data;
                    QStringList commands;
                    commands << "stop" << "help" << "version" << "reconnect";
                    data = data.replace("\n", "");
                    qDebug() << "teminal said:" << data;
                    if(!commands.contains(data))
                    {
                        terminal->write("No such command. Available commands are: stop, help, version, reconnect");
                        terminal->write("\n");
                        return;
                    }
                    switch(commands.indexOf(data))
                    {
                        case 0:
                            terminal->write("Service shutting down");
                            terminal->write("\n");
                            quit();
                            break;
                        case 1:
                            terminal->write("Avalable commands are: \n"
                                            "stop - stops the service \n"
                                            "help - shows this dialogue \n"
                                            "version - test value \n"
                                            "reconnect - manual reconnect command \n");
                            terminal->write("\n");
                            break;
                        case 2:
                            terminal->write("appdata");
                            terminal->write("\n");
                            break;
                        case 3:
                            terminal->write("Trying to reconnect");
                            terminal->write("\n");
                            m_handler->reconnect();
                    }

                });
    }
}

bool NatsService::parseArguments(QCommandLineParser &parser, const QStringList &arguments)
{
    parser.addHelpOption();
    parser.addVersionOption();
    parser.addOption({
        {QStringLiteral("p"), QStringLiteral("passive")},
        QStringLiteral("Run terminal service in passive (Non-Interactive) mode")
    });
    parser.addPositionalArgument(QStringLiteral("stop"),
                                 QStringLiteral("Stop the the service"),
                                 QStringLiteral("[stop]"));

    return parser.parse(arguments);
}
